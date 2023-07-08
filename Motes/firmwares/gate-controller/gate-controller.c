#include "contiki.h"
#include "coap-engine.h"
#include "sys/log.h"
#include "sensor-sim.h"

#define LOG_MODULE "gate-ctrl"
#ifndef LOG_LEVEL_APP
  #define LOG_LEVEL_APP LOG_LEVEL_DBG
#endif
#define LOG_LEVEL LOG_LEVEL_APP

/*---------------------------------------------------------------------------*/
PROCESS(gate_controller_proc, "gate-controller-proc");
AUTOSTART_PROCESSES(&gate_controller_proc);
/*---------------------------------------------------------------------------*/

extern coap_resource_t res_gate_1;
extern coap_resource_t res_alarm;

PROCESS_THREAD(gate_controller_proc, ev, data)
{

  PROCESS_BEGIN();

  LOG_INFO("Starting Erbium server...\n");

  coap_activate_resource(&res_gate_1, "actuator/gate/"STR(FLOW_NAME));
  coap_activate_resource(&res_alarm, "actuator/alarm");

  LOG_INFO("Resource activated: \"/actuator/gate/"STR(FLOW_NAME)"\"\n");

  while(1){
    PROCESS_WAIT_EVENT();
  }

  PROCESS_END();

}
/*---------------------------------------------------------------------------*/
