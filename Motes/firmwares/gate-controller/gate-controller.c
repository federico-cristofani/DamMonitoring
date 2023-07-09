#include "contiki.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "sys/log.h"
#include "dev/leds.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uiplib.h"

#include "sensor-sim.h"

#define LOG_MODULE "ctrl"
#ifndef LOG_LEVEL_APP
  #define LOG_LEVEL_APP LOG_LEVEL_DBG
#endif
#define LOG_LEVEL LOG_LEVEL_APP

#define SERVER_EP     "coap://[fd00::1]"
#define REGISTRY_URI  "/registry"
#define MSG_LEN   512
#define ADDR_SIZE 40
/*---------------------------------------------------------------------------*/
PROCESS(coap_br_proc, "controller-proc");
AUTOSTART_PROCESSES(&coap_br_proc);
/*---------------------------------------------------------------------------*/

extern coap_resource_t res_gate_1;
extern coap_resource_t res_alarm;

static bool registered = false;

static void handle_coap_response(coap_message_t *response){
  if(response->code != CREATED_2_01){
    LOG_WARN("Registration failed: %d\n", response->code);
  }
  else{
    LOG_INFO("Registration succeded\n");
    registered = true;
  }
}

PROCESS_THREAD(coap_br_proc, ev, data){

  static coap_endpoint_t server_ep;
  static coap_message_t request;      /* This way the packet can be treated as pointer as usual. */
  static char message[MSG_LEN];
  static int n_msg = 0;
  static char addr_buf[40];

  PROCESS_BEGIN();

  coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);

  while (uip_ds6_get_global(ADDR_PREFERRED) == NULL &&  uip_ds6_defrt_choose() == NULL) {
     PROCESS_PAUSE();
  }

  while(!registered){

    LOG_INFO("Sending registration message #%d\n", n_msg++);

    coap_init_message(&request, COAP_TYPE_CON, COAP_POST, 0);
    coap_set_header_uri_path(&request, REGISTRY_URI);
  
    uip_ds6_addr_t* addr = uip_ds6_get_global(ADDR_PREFERRED);

    // LOG_INFO_6ADDR(&addr->ipaddr);
    uiplib_ipaddr_snprint(addr_buf, ADDR_SIZE, &addr->ipaddr);
    
    snprintf(message, MSG_LEN,
      "[{\"name\":\"outflow-%d\", \"type\":\"gate\",\"description\":\"Gate controller\",\"tag\":\"%s\",\"uri\":\"coap://[%s]/actuator/gate/outflow-%d\",\"value\":0},"\
      "{\"name\":\"alarm-%d\", \"type\":\"alarm\",\"description\":\"Alarm controller\",\"uri\":\"coap://[%s]/actuator/alarm\",\"value\":0}]", 
      FLOW, (FLOW == 1 ? "Primary":"Emergency"), addr_buf, FLOW,FLOW, addr_buf);
    
    coap_set_payload(&request, (uint8_t *)message, strlen(message));

    COAP_BLOCKING_REQUEST(&server_ep, &request, handle_coap_response);  
  }

  leds_on(LEDS_GREEN);

  LOG_INFO("Starting Erbium server...\n");

  coap_activate_resource(&res_gate_1, "actuator/gate/outflow-"STR(FLOW));
  LOG_INFO("Resource activated: \"/actuator/gate/outflow-"STR(FLOW)"\n");

  coap_activate_resource(&res_alarm, "actuator/alarm");
  LOG_INFO("Resource activated: \"/actuator/gate/alarm\"\n");

  PROCESS_END();

}
/*---------------------------------------------------------------------------*/
