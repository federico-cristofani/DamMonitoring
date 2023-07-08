#include "coap-engine.h"
#include "sys/log.h"
#include "sys/ctimer.h"
#include "dev/leds.h"
#include "jsonparse.h"

/* Custom header files */
#include "sensor-sim.h"
#include "buffer-size.h"
#include "common.h"

#define LOG_MODULE "res-gate"
#ifndef LOG_LEVEL_APP
  #define LOG_LEVEL_APP LOG_LEVEL_INFO
#endif
#define LOG_LEVEL LOG_LEVEL_APP

#define BUFFER_LEN  256
#define OPENING_LEVEL_PAR "opening_level"
#define BLINKS  5


void res_get_handler(coap_message_t*, coap_message_t*, uint8_t*, uint16_t, int32_t*);
void res_put_handler(coap_message_t*, coap_message_t*, uint8_t*, uint16_t, int32_t*);
void log_request(const char*, coap_message_t*);

RESOURCE(
  res_gate_1,
  "title=\"Controls the gate opening: GET,PUT opening_level=<opening-level>\";rt=\"Gate actuator\";ct=50",
  res_get_handler,
  NULL,
  res_put_handler,
  NULL
);

RESOURCE(
  res_gate_2,
  "title=\"Controls the gate opening: GET,PUT opening_level=<opening-level>\";rt=\"Gate actuator\";ct=50",
  res_get_handler,
  NULL,
  res_put_handler,
  NULL
);

static char response_buffer[BUFFER_LEN];

static struct gate_t{
  int opening_level;
  struct ctimer timer;
  int blinks;
  int led;
} gates[2] = {{.opening_level = INIT_OUTGATE, .blinks=BLINKS, .led = LEDS_GREEN},{.opening_level = INIT_OUTGATE, .blinks=BLINKS, .led = LEDS_BLUE}};

void blink(void* gate_ptr){
  struct gate_t* gate = (struct gate_t*)gate_ptr;
  leds_toggle(gate->led);
  
  if(gate->blinks > 0){
    gate->blinks -= 1;
    ctimer_reset(&gate->timer);
  }
  else{
    gate->blinks = BLINKS;
    leds_off(gate->led);
  }
}

void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

  int res_index = (int)((request->uri_path[request->uri_path_len-1])-'0') - 1;
  
  // Log request
  log_request("GET", request);

  // Set response status code
  coap_set_status_code(response, CONTENT_2_05);

  // Set response for client
  snprintf(response_buffer, BUFFER_LEN, "{\"n\":\"opening_level\", \"v\": %d, \"u\":\"percent\"}",  gates[res_index].opening_level);
  coap_set_header_content_format(response, APPLICATION_JSON);
  coap_set_payload(response, response_buffer, strlen(response_buffer));
}

void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

  const char* new_level_param = NULL;
  int new_level;
  int param_len = 0;
  int param_ok = 0;
  unsigned int request_ct_format;
  int res_index = (int)((request->uri_path[request->uri_path_len-1])-'0') - 1;

  // Log request
  log_request("PUT", request);

  // Prepare client response
  coap_set_header_content_format(response, APPLICATION_JSON);
  
  // Retrieve parameter
  if(coap_get_header_content_format(request, &request_ct_format) == 0){/* Missing content type format header option */
    coap_set_status_code(response, BAD_REQUEST_4_00);
    strncpy(response_buffer, "{\"message\": \"Missing required option 'content-type'\"}", BUFFER_LEN);
    coap_set_payload(response, response_buffer, strlen(response_buffer));
    return;
  }

  if(request_ct_format == TEXT_PLAIN){ /* Content type TEXT_PLAIN */
    param_len = coap_get_post_variable(request, OPENING_LEVEL_PAR, &new_level_param);
    if(param_len > 0){
      new_level = atoi(new_level_param);
      // Check parameter value
      param_ok = ((strcmp(new_level_param, "0") != 0 && new_level == 0) || new_level < 0 || new_level > 100) ? 0:1;     
    }
  }
  else if(request_ct_format == APPLICATION_JSON){ /* Content type APPLICATION/JSON */
    param_len = coap_get_payload(request, (const uint8_t**)&new_level_param);
    if(param_len > 0){
      param_ok = get_json_int_field(new_level_param, OPENING_LEVEL_PAR, &new_level);
      if(param_ok == -1){ /* Missing key */
        param_len = 0;
      }
      else if(param_ok != 1 || new_level < 0 || new_level > 100){ /* Bad value */
        param_ok = 0;
      }
    }
  }
  else{
    coap_set_status_code(response, BAD_REQUEST_4_00);
    strncpy(response_buffer, "{\"message\": \"Unsupported content type\"}", BUFFER_LEN);
    coap_set_payload(response, response_buffer, strlen(response_buffer));  
    return;
  }

  if(param_len == 0){
    coap_set_status_code(response, BAD_REQUEST_4_00);
    strncpy(response_buffer, "{\"message\": \"Missing required opening level parameter\"}", BUFFER_LEN);
    coap_set_payload(response, response_buffer, strlen(response_buffer));  
    return;
  }
  if(param_ok != 1){
    coap_set_status_code(response, BAD_REQUEST_4_00);
    strncpy(response_buffer, "{\"message\": \"Opening level must be integer between 0 and 100\"}", BUFFER_LEN);
    coap_set_payload(response, response_buffer, strlen(response_buffer));
    return;
  }

  // Success
  if(gates[res_index].opening_level != new_level){
    // Reset led 
    ctimer_stop(&gates[res_index].timer);
    leds_off(gates[res_index].led);
    gates[res_index].blinks = BLINKS;
    ctimer_set(&gates[res_index].timer, CLOCK_SECOND / 4, blink, &gates[res_index]); 
    
    coap_set_status_code(response, CHANGED_2_04);
  }
  else{
    coap_set_status_code(response, VALID_2_03);
  }
  
  gates[res_index].opening_level = new_level;
  LOG_INFO("New opening level: %d\n", new_level);
  
  snprintf(response_buffer, BUFFER_LEN, "{\"message\": \"Opening level set to %d%%\"}", gates[res_index].opening_level);
  coap_set_payload(response, response_buffer, strlen(response_buffer));
}
