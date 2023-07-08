#include "coap-engine.h"
#include "sys/log.h"
#include "sys/ctimer.h"
#include "dev/leds.h"
#include "jsonparse.h"

/* Custom header files */
#include "sensor-sim.h"
#include "buffer-size.h"
#include "common.h"

#define LOG_MODULE "res-alarm"
#ifndef LOG_LEVEL_APP
  #define LOG_LEVEL_APP LOG_LEVEL_INFO
#endif
#define LOG_LEVEL LOG_LEVEL_APP

#define BUFFER_LEN  256
#define STATE_PAR   "state"

static void res_get_handler(coap_message_t*, coap_message_t*, uint8_t*, uint16_t, int32_t*);
static void res_put_handler(coap_message_t*, coap_message_t*, uint8_t*, uint16_t, int32_t*);
void log_request(const char*, coap_message_t*);

RESOURCE(
    res_alarm,
    "title=\"Controls the alarm state: GET,PUT state=(true|false)\";rt=\"Alarm controller\";ct=50",
    res_get_handler,
    NULL,
    res_put_handler,
    NULL
);

static char response_buffer[BUFFER_LEN];

static struct alarm_t{
  bool state;
  struct ctimer timer;
  int led;
  int period;
} alarm = {.state = false, .led=LEDS_RED, .period = CLOCK_SECOND / 3};

static void blink(void* data){
  leds_toggle(alarm.led);
  ctimer_reset(&alarm.timer);
}

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
  log_request("GET", request);

  // Set response status code 
  coap_set_status_code(response, CONTENT_2_05);

  // Set response for client
  snprintf(response_buffer, BUFFER_LEN, "{\"n\":\"state\", \"vb\": %s}",  alarm.state ? "true":"false");
  coap_set_header_content_format(response, APPLICATION_JSON);
  coap_set_payload(response, response_buffer, strlen(response_buffer));
}

static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
  const char* new_state_param = NULL;
  bool new_state;
  int param_len = 0;
  int param_ok = 0;
  unsigned int request_ct_format;

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
    param_len = coap_get_post_variable(request, STATE_PAR, &new_state_param);
    if(param_len > 0){
      // Check parameter value
      param_ok = (strncmp(new_state_param, "true", strlen(new_state_param)) == 0 || 
        strncmp(new_state_param, "false", strlen(new_state_param)) == 0);     
      new_state = (strncmp(new_state_param, "true", strlen(new_state_param)) == 0);
    }
  }
  else if(request_ct_format == APPLICATION_JSON){ /* Content type APPLICATION/JSON */
    param_len = coap_get_payload(request, (const uint8_t**)&new_state_param);
    if(param_len > 0){
      param_ok = get_json_bool_field(new_state_param, STATE_PAR, &new_state);
      if(param_ok == -1){ /* Missing key */
        param_len = 0;
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
    strncpy(response_buffer, "{\"message\": \"Missing required state parameter\"}", BUFFER_LEN);
    coap_set_payload(response, response_buffer, strlen(response_buffer));  
    return;
  }
  if(param_ok != 1){
    coap_set_status_code(response, BAD_REQUEST_4_00);
    strncpy(response_buffer, "{\"message\": \"State must be boolean (true/false)\"}", BUFFER_LEN);
    coap_set_payload(response, response_buffer, strlen(response_buffer));
    return;
  }

  // Success
  if(alarm.state != new_state){
    alarm.state = new_state;
    LOG_INFO("New state: %d\n", new_state);
    if(alarm.state == true){
      ctimer_set(&alarm.timer, alarm.period, blink, NULL);
    }
    else{
      ctimer_stop(&alarm.timer);
      leds_off(alarm.led);
    }
  }

  coap_set_status_code(response, CHANGED_2_04);
  snprintf(response_buffer, BUFFER_LEN, "{\"message\": \"Alarm %s\"}", alarm.state ? "on":"off");
  coap_set_payload(response, response_buffer, strlen(response_buffer));

}

