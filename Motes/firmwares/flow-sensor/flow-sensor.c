#include "random.h"
#include "sys/log.h"
#include "dev/button-hal.h"
#include "dev/leds.h"

/* custom header files*/
#include "common.h"
#include "buffer-size.h"
#include "sensor-sim.h"

#define LOG_MODULE "flow-sensor"
#ifndef LOG_LEVEL_APP
  #define LOG_LEVEL_APP LOG_LEVEL_INFO
#endif
#define LOG_LEVEL LOG_LEVEL_APP

// Periodic timer to check the state of the MQTT client
#define DEFAULT_TIME_PERIOD        (CLOCK_SECOND * 1)

#define N_TOPIC       1
#define PUB_PERIOD_S  5
#define SUB_TOPIC     "simulation"
#define PUB_TOPIC     "flow-rate"
#define N_SENSOR    1

#define FLOW_SENSORS OUTFLOW_1

typedef enum {
  FLOW_SENSORS
} flow_sensor_t;

/*---------------------------------------------------------------------------*/ 
/*--------------------- INTERNAL FUNCTIONS AND VARIABLES --------------------*/
/*---------------------------------------------------------------------------*/ 

// Gloabal variable 
static char* pub_topic = PUB_TOPIC;
static char* sub_topic = SUB_TOPIC;
static char record[APP_BUFFER_SIZE];

// Simulation variables
static int flow[3] = {MAX_INFLOW * INIT_INGATE, MAX_OUTFLOW_1 * INIT_OUTGATE, MAX_OUTFLOW_2 * INIT_OUTGATE};
static int target_flow[3] = {MAX_INFLOW * INIT_INGATE, MAX_OUTFLOW_1 * INIT_OUTGATE, MAX_OUTFLOW_2 * INIT_OUTGATE};
static int max_value;
static int min_value;

/* Simulate value from flow sensor */
void simulate_read_flow(void* timer){
 
  max_value = ((target_flow[FLOW]) + VAR_THRESHOLD);
  min_value = ((target_flow[FLOW]) - VAR_THRESHOLD);
    
  LOG_DBG("Sensor: %d, flow: %d, target: %d\n", FLOW, flow[FLOW], target_flow[FLOW]);

  if(flow[FLOW] < min_value){
    flow[FLOW] += LINEAR_VAR(random_rand(), min_value, flow[FLOW]);
  }
  else if(flow[FLOW] > max_value){
    flow[FLOW] -= LINEAR_VAR(random_rand(), max_value, flow[FLOW]);
  }
  else{
    flow[FLOW] = STEADY_VAR(target_flow[FLOW], random_rand(), max_value, min_value);
  }

  if(flow[FLOW] < 0){
    flow[FLOW] = 0;
  }

  if(flow[FLOW] > max_value){
    flow[FLOW] = max_value;
  }
  ctimer_reset((struct ctimer*)timer);
};

/* Read flow sensor */
int read_flow_sensor(flow_sensor_t sensor){
  return flow[sensor];/* Simulated value */
}


/*---------------------------------------------------------------------------*/ 
/*----------------------- MQTT CLIENT MESSAGE HANDLERS ----------------------*/
/*---------------------------------------------------------------------------*/ 
char* build_message(){
  snprintf(record, APP_BUFFER_SIZE, "{\"bu\":\"m3/s\", \"e\": [{\"n\":\"%s\", \"v\":%de-%d}]}", 
            STR(FLOW_NAME), read_flow_sensor(FLOW), SCALE_EXP);
  return record;
}

void handle_message(const char* topic, uint16_t topic_len, const uint8_t* chunk, uint16_t chunk_len){

  static int new_gate_value = 0;
  static int res;
  static char gate[15];
  
  LOG_DBG("Message received: topic='%s' (len=%u), chunk_len=%u\n", topic, topic_len, chunk_len);

  snprintf(gate, 15, "gate-%s", GATE(FLOW));
  res = get_json_int_field((char*)chunk, gate, &new_gate_value); 

  if(res == 0 || new_gate_value < 0 || new_gate_value > 100){ /* Some errors produced bad value */
    LOG_WARN("Bad simulated value: %s\n", chunk);
  }
  else if(res == 1){ /* Changes in the simulation */
    target_flow[FLOW] = new_gate_value * MAX_FLOW;
    LOG_INFO("Gate \"%s\" set to %d%%, new target flow: %d\n", gate, new_gate_value, target_flow[FLOW]);
  }
  else{ /* Not changes */
    LOG_DBG("%s: no changes occured\n", gate);
  }
  
}

/*------------------------------ MAIN PROCESS -------------------------------*/

extern int state_machine();
extern void init_mqtt(struct process*, clock_time_t, char*, char*);

// Main process
PROCESS(mqtt_client_process, "mqtt-client-proc");
AUTOSTART_PROCESSES(&mqtt_client_process);

PROCESS_THREAD(mqtt_client_process, ev, data){
  
  // Event timer
  static clock_time_t time_period = DEFAULT_TIME_PERIOD;
  static struct etimer periodic_timer;
  static struct ctimer simulate_timer;
#if FLOW == 0
  static struct etimer led_timer;
  static button_hal_button_t *btn;
  static bool btn_release = true;
#endif

  PROCESS_BEGIN();

  ctimer_set(&simulate_timer, CLOCK_SECOND, simulate_read_flow, &simulate_timer);

  LOG_INFO("MQTT client process: flow-sensor\n");

  init_mqtt(&mqtt_client_process, PUB_PERIOD_S * CLOCK_SECOND, pub_topic, sub_topic);

  // Set periodic timer to check the status 
  etimer_set(&periodic_timer, time_period);
#if FLOW == 0
  etimer_set(&led_timer, CLOCK_SECOND / 3);
#endif
  while(1) {

    // Release control to system scheduler
    PROCESS_YIELD();
    
    // MQTT state machine
    if((ev == PROCESS_EVENT_TIMER && data == &periodic_timer) || ev == PROCESS_EVENT_POLL){
      time_period = state_machine();
      if(time_period < 0){
        PROCESS_EXIT();
      }
       // Reset timer with new interval
      etimer_reset_with_new_interval(&periodic_timer, time_period);
    }
  #if FLOW == 0
    else if(ev == button_hal_release_event){ /* Simulate inflow increment */
      if(btn_release){
        if(target_flow[0] < MAX_INFLOW*100){
          LOG_INFO("Inflow incremented\n");
          target_flow[0] += ((target_flow[0] >= MAX_INFLOW*100) ? 0: MAX_INFLOW * 10);
          flow[0] = target_flow[0];
          leds_on(LEDS_YELLOW);
          etimer_restart(&led_timer);
        }
        else{
          LOG_INFO("Maximum inflow reached\n");
        }
      }
      else{
        btn_release = true;
      }
    }
    else if(ev == button_hal_periodic_event) { /* Simulate no inflow*/
      btn = (button_hal_button_t *)data;
      if(btn->press_duration_seconds == 3) {
        LOG_INFO("Inflow set to 0\n");
        target_flow[0] = 0;
        flow[0] = 0;
        btn_release = false;
        leds_on(LEDS_YELLOW);
        etimer_restart(&led_timer);
      }
    }
    else if(ev == PROCESS_EVENT_TIMER && data == &led_timer){
      leds_off(LEDS_YELLOW);
    }
    #endif
  }
  PROCESS_END();
}