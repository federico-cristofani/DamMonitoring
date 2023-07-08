#include "sys/log.h"
#include "random.h"

/* custom header files*/
#include "common.h"
#include "buffer-size.h"
#include "sensor-sim.h"

#define LOG_MODULE "water-level"
#ifndef LOG_LEVEL_APP
  #define LOG_LEVEL_APP LOG_LEVEL_DBG
#endif
#define LOG_LEVEL LOG_LEVEL_APP

// Periodic timer to check the state of the MQTT client
#define DEFAULT_TIME_PERIOD        (CLOCK_SECOND * 1)

#define N_TOPIC       1
#define PUB_PERIOD_S  10
#define SUB_TOPIC     "simulation"
#define PUB_TOPIC     "water-level"


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

/* Simulate value from flow sensor */
void simulate_read_water_level(void* timer){

  int var = random_rand() % (balance_flow + 1000);
  if(var % 2 == 0)
    var = -var;

  int wl_var = random_rand() % WATER_LEVEL_VAR;
  if(balance_flow > 0){
    wl_var = -wl_var;
  }

  reservoir += (balance_flow + var);
  water_level = (reservoir / SURFACE) + wl_var;
 
  LOG_DBG("Var: %d,%d, Balance flow: %d, Reservoir: %lu, Water-level: %lu\n", var, wl_var, balance_flow, reservoir, water_level);
  
  if(water_level < 0){
    water_level = 0;
  }
  if(water_level > MAX_WATER_LEVEL){
    water_level = MAX_WATER_LEVEL;
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
  snprintf(record, APP_BUFFER_SIZE, "{\"bu\":\"m\", \"e\":[{\"n\":\"water-level\",\"v\":%de-%d}]}", read_water_level(), SCALE_EXP);
  return record;
}

void handle_message(const char* topic, uint16_t topic_len, const uint8_t* chunk, uint16_t chunk_len){
  
  int new_balance_flow = 0;
  int res;

  LOG_DBG("Message received: topic='%s' (len=%u), chunk_len=%u\n", topic, topic_len, chunk_len);

  res = get_json_int_field((char*)chunk, "balance-flow", &new_balance_flow);
 
  if(res == 0 || new_balance_flow < MIN_BALANCE_FLOW || new_balance_flow > MAX_BALANCE_FLOW){ /* Some errors produced bad value */
    LOG_WARN("Bad simulated value: %s\n", chunk);
  }
  else if(res == 1){ /* Changes in the simulation */
    balance_flow = new_balance_flow;
    LOG_INFO("New balance-flow: %d\n", new_balance_flow);
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

  ctimer_set(&simulate_timer, CLOCK_SECOND, simulate_read_water_level, &simulate_timer);

  LOG_INFO("MQTT client process: water-level-sensor\n");

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