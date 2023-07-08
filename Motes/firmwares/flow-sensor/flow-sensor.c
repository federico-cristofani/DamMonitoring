#include "random.h"
#include "sys/log.h"

/* custom header files*/
#include "common.h"
#include "buffer-size.h"
#include "sensor-sim.h"

#define LOG_MODULE "flow-sensor"
#ifndef LOG_LEVEL_APP
  #define LOG_LEVEL_APP LOG_LEVEL_DBG
#endif
#define LOG_LEVEL LOG_LEVEL_APP

// Periodic timer to check the state of the MQTT client
#define DEFAULT_TIME_PERIOD        (CLOCK_SECOND * 1)

#define N_TOPIC       1
#define PUB_PERIOD_S  5
#define SUB_TOPIC     "simulation"
#define PUB_TOPIC     "flow-rate"

/*---------------------------------------------------------------------------*/ 
/*--------------------- INTERNAL FUNCTIONS AND VARIABLES --------------------*/
/*---------------------------------------------------------------------------*/ 

// Gloabal variable 
static char* pub_topic = PUB_TOPIC;
static char* sub_topic = SUB_TOPIC;
static char record[APP_BUFFER_SIZE];

// Simulation variables
static int flow = MAX_FLOW * INIT_GATE;
static int target_flow = MAX_FLOW * INIT_GATE;

/* Simulate value from flow sensor */
void simulate_read_flow(void* timer){
 
  int max_value = ((target_flow) + VAR_THRESHOLD);
  int min_value = ((target_flow) - VAR_THRESHOLD);
    
  LOG_DBG("Sensor: "STR(FLOW_NAME)", flow: %d, target: %d\n", flow, target_flow);

  if(flow < min_value){
    flow += LINEAR_VAR(random_rand(), min_value, flow);
  }
  else if(flow > max_value){
    flow -= LINEAR_VAR(random_rand(), max_value, flow);
  }
  else{
    flow = STEADY_VAR(target_flow, random_rand(), max_value, min_value);
  }

  ctimer_reset((struct ctimer*)timer);
};

/* Read flow sensor */
int read_flow_sensor(){
  return flow;/* Simulated value */
}


/*---------------------------------------------------------------------------*/ 
/*----------------------- MQTT CLIENT MESSAGE HANDLERS ----------------------*/
/*---------------------------------------------------------------------------*/ 
char* build_message(){
  snprintf(record, APP_BUFFER_SIZE, "{\"bu\":\"m3/s\", \"e\":[{\"n\":\"%s\",\"v\":%de-%d}]}", STR(FLOW_NAME), read_flow_sensor(), SCALE_EXP);
  return record;
}

void handle_message(const char* topic, uint16_t topic_len, const uint8_t* chunk, uint16_t chunk_len){

  int new_target_flow = 0;
  int res;

  LOG_DBG("Message received: topic='%s' (len=%u), chunk_len=%u\n", topic, topic_len, chunk_len);

  res = get_json_int_field((char*)chunk, "gate-"STR(FLOW_NAME), &new_target_flow);
 
  if(res == 0 || new_target_flow < 0 || new_target_flow > 100){ /* Some errors produced bad value */
    LOG_WARN("Bad simulated value: %s\n", chunk);
  }
  else if(res == 1){ /* Changes in the simulation */
    target_flow = new_target_flow * MAX_FLOW;
    LOG_DBG("Gate \"%s\" set to %d%%, new target flow: %d\n", STR(FLOW_NAME), new_target_flow, target_flow);
  }
  else{ /* Not changes */
     LOG_DBG("No changes occured\n");
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

  PROCESS_BEGIN();

  ctimer_set(&simulate_timer, CLOCK_SECOND, simulate_read_flow, &simulate_timer);

  LOG_INFO("MQTT client process: flow-sensor\n");

  init_mqtt(&mqtt_client_process, PUB_PERIOD_S * CLOCK_SECOND, pub_topic, sub_topic);

  // Set periodic timer to check the status 
  etimer_set(&periodic_timer, time_period);

  while(1) {

    // Release control to system scheduler
    PROCESS_YIELD();
    
    // MQTT state machine
    if((ev == PROCESS_EVENT_TIMER && data == &periodic_timer) || ev == PROCESS_EVENT_POLL){
      time_period = state_machine();
      if(time_period < 0){
        PROCESS_EXIT();
      }
    }
    
    // Reset timer with new interval
    etimer_reset_with_new_interval(&periodic_timer, time_period);
  }

  PROCESS_END();
}