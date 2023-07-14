#include "sys/log.h"
#include "random.h"
#include "dev/leds.h"
#include "dev/button-hal.h"

/* custom header files*/
#include "common.h"
#include "buffer-size.h"
#include "sensor-sim.h"

#define LOG_MODULE "water-level"
#ifndef LOG_LEVEL_APP
  #define LOG_LEVEL_APP LOG_LEVEL_INFO
#endif
#define LOG_LEVEL LOG_LEVEL_APP

// Periodic timer to check the state of the MQTT client
#define DEFAULT_TIME_PERIOD        (CLOCK_SECOND * 1)

#define N_TOPIC       1
#define PUB_PERIOD_S  5
#define SUB_TOPIC     "simulation"
#define PUB_TOPIC     "water-level"
#define N_FLOW    3


/*---------------------------------------------------------------------------*/ 
/*--------------------- INTERNAL FUNCTIONS AND VARIABLES --------------------*/
/*---------------------------------------------------------------------------*/ 

// Gloabal variable 
static char* pub_topic = PUB_TOPIC;
static char* sub_topic = SUB_TOPIC;
static char record[APP_BUFFER_SIZE];

// Simulation variables
static int balance_flow = INIT_BALANCE_FLOW;
static unsigned long water_level = INIT_WATER_LEVEL;
static unsigned long reservoir = INIT_RESERVOIR;
static int flow[N_FLOW] = {MAX_INFLOW * INIT_INGATE, MAX_OUTFLOW_1 * INIT_OUTGATE, MAX_OUTFLOW_2 * INIT_OUTGATE};

/* Simulate value from flow sensor */
void simulate_read_water_level(void* timer){

  static int var; 
  static int wl_var;

  var = random_rand() % (balance_flow + 1000);
  wl_var = random_rand() % WATER_LEVEL_VAR;
  
  if(var % 2 == 0)
    var = -var;
  
  if(balance_flow > 0){
    wl_var = -wl_var;
  }

  reservoir += (balance_flow + var);
  water_level = (reservoir / SURFACE) + wl_var;
 
  LOG_DBG("Var: %d,%d, Balance flow: %d, Reservoir: %lu, Water-level: %lu\n", var, wl_var, balance_flow, reservoir, water_level);
  
  if(water_level < 0){
    water_level = 0;
    reservoir = 0;
  }
  if(water_level > MAX_WATER_LEVEL){
    water_level = MAX_WATER_LEVEL;
    reservoir = MAX_RESERVOIR;
  }

  ctimer_reset((struct ctimer*)timer);
};

/* Read flow sensor */
int read_water_level(){
  return water_level; /* Simulated value */
}


/*---------------------------------------------------------------------------*/ 
/*----------------------- MQTT CLIENT MESSAGE HANDLERS ----------------------*/
/*---------------------------------------------------------------------------*/ 
char* build_message(){
  snprintf(record, APP_BUFFER_SIZE, "{\"bn\":\"%s\",\"bu\": \"m\", \"e\":[{\"n\":\"water-level\", \"v\":%de-%d}]}", 
      URN(), read_water_level(), SCALE_EXP);
  return record;
}

void handle_message(const char* topic, uint16_t topic_len, const uint8_t* chunk, uint16_t chunk_len){

  static int new_gate_value = 0;
  static int res;
  static char gate[15];
  
  LOG_DBG("Message received: topic='%s' (len=%u), chunk_len=%u\n", topic, topic_len, chunk_len);

  for(int i = 0; i < N_FLOW; ++i){

    snprintf(gate, 15, "gate-%s", GATE(i));
    res = get_json_int_field((char*)chunk, gate, &new_gate_value); 
  
    if(res == 0 || new_gate_value < 0 || new_gate_value > 100){ /* Some errors produced bad value */
      LOG_WARN("Bad simulated value: %s\n", chunk);
    }
    else if(res == 1){ /* Changes in the simulation */
      flow[i] = new_gate_value * MAX_FLOW(i);
      balance_flow = flow[0];
      for(int i = 1; i < N_FLOW; ++i)
        balance_flow -= flow[i];
      LOG_INFO("New balance-flow: %d\n", balance_flow);
    }
    else{ /* Not changes */
      LOG_DBG("%s: no changes occured\n", gate);
    }
  }
}

/*------------------------------ MAIN PROCESS -------------------------------*/

extern int state_machine();
extern void init_mqtt(struct process*, clock_time_t, char*, char*);

// Main process
PROCESS(mqtt_client_proc, "mqtt-client-proc");
AUTOSTART_PROCESSES(&mqtt_client_proc);

PROCESS_THREAD(mqtt_client_proc, ev, data){
  
  // Event timer
  static clock_time_t time_period = DEFAULT_TIME_PERIOD;
  static struct etimer periodic_timer;
  static struct etimer led_timer;
  static struct ctimer simulate_timer;
  static button_hal_button_t *btn;
  static bool btn_release = true;

  PROCESS_BEGIN();

  ctimer_set(&simulate_timer, CLOCK_SECOND, simulate_read_water_level, &simulate_timer);

  LOG_INFO("MQTT client process: water-level-sensor\n");

  init_mqtt(&mqtt_client_proc, PUB_PERIOD_S * CLOCK_SECOND, pub_topic, sub_topic);

  // Set periodic timer to check the status 
  etimer_set(&periodic_timer, time_period);
  etimer_set(&led_timer, CLOCK_SECOND / 3);

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
    else if(ev == button_hal_release_event){ /* Reset water level to initial condition */
      if(btn_release){
        LOG_INFO("Reservoir set to init value\n");
        reservoir = INIT_RESERVOIR;
        leds_on(LEDS_LED2);
        etimer_restart(&led_timer);
      }
      else{
        btn_release = true;
      }
    }
    else if(ev == button_hal_periodic_event) { /* Simulate critical condition*/
      btn = (button_hal_button_t *)data;      
      if(btn->press_duration_seconds == 3) {
        LOG_INFO("Reservoir set to max value\n");
        reservoir = MAX_RESERVOIR;
        btn_release = false;
        leds_on(LEDS_LED2);
        etimer_restart(&led_timer);
      }
    }
    else if(ev == PROCESS_EVENT_TIMER && data == &led_timer){
      leds_off(LEDS_LED2);
    }
  } 
  PROCESS_END();  
}