/*
 * Copyright (c) 2020, Carlo Vallati, University of Pisa
  * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "sys/log.h"
#include "dev/leds.h"
#include "mqtt.h"

#include "mqtt-client.h"
#include "buffer-size.h"

/*---------------------------- FUNCTIONS AND VARIABLES ----------------------------*/

// MQTT state machine
static uint8_t state = STATE_BOOT; 

// MQTT Broker IP
static const char *broker_ip = MQTT_CLIENT_BROKER_IP_ADDR;
static struct ctimer led_timer;

static struct process* _main_proc;
static char* _sub_topic;
static char* _pub_topic;
static clock_time_t _publish_time_period;

void handle_message(const char*, uint16_t, const uint8_t*, uint16_t);
char* build_message();

/*--------------------- HANDLERS AND UTILITY FUNCTIONS  ---------------------*/

static void mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data){
  static struct mqtt_message* msg_ptr = 0;

  switch(event) {
    case MQTT_EVENT_CONNECTED: { // Connected to broker
      LOG_INFO("MQTT connection established\n");
      state = STATE_CONNECTED;
      break;
    }
    case MQTT_EVENT_DISCONNECTED: { // Disconnection from broker
      LOG_WARN("MQTT disconnected\n");
      state = STATE_DISCONNECTED;  
      break;
    }
    case MQTT_EVENT_PUBLISH: { // Message on subscribed topics received 
      msg_ptr = data;
      handle_message(msg_ptr->topic, strlen(msg_ptr->topic), msg_ptr->payload_chunk, msg_ptr->payload_length);
      break;
    }
    case MQTT_EVENT_SUBACK: { // Subscription complete
  #if MQTT_311
      mqtt_suback_event_t *suback_event = (mqtt_suback_event_t *)data;

      if(suback_event->success) {
        LOG_DBG("Application is subscribed to topic successfully\n");
      } else {
        LOG_DBG("Application failed to subscribe to topic (ret code %x)\n", suback_event->return_code);
      }
  #else
      LOG_INFO("Application is subscribed successfully\n");
  #endif
      break;
    }
    case MQTT_EVENT_UNSUBACK: { // Unsubscription complete
      LOG_DBG("Application is unsubscribed successfully\n");
      break;
    }
    case MQTT_EVENT_PUBACK: { // Publish complete (requires QoS > 0)
      LOG_DBG("Publishing complete\n");
      break;
    }
    default: { // Unexpected event
      LOG_WARN("Application got a unhandled MQTT event: %i\n", event);
    }
  }
}

static bool have_connectivity(){
  if(uip_ds6_get_global(ADDR_PREFERRED) == NULL 
      #ifndef BR 
        || uip_ds6_defrt_choose() == NULL 
      #endif
  ){
    LOG_WARN("No connectivity\n");
    return false;
  }
  return true;
}

static void toggle_led(void* led){
  leds_toggle(*(leds_mask_t*)led);
  ctimer_restart(&led_timer);
}

static void led_off(void* led){
  leds_off(*(leds_mask_t*)led);
}

void init_mqtt(struct process* main_proc, clock_time_t publish_period, char* pub_topic, char* sub_topic){
  _main_proc = main_proc;
  _pub_topic = pub_topic;
  _sub_topic = sub_topic;
  _publish_time_period = publish_period;

  LOG_DBG("MQTT client initialized\n");
}

int state_machine(){
  
  // Mqtt broker parameter
  static char broker_address[CONFIG_IP_ADDR_STR_LEN];
  
  // Buffers
  static char client_id[CLIENT_ID_BUFFER_SIZE];
  static char app_buffer[APP_BUFFER_SIZE];

  // Mqtt state variables
  static struct mqtt_connection conn;
  static mqtt_status_t status;
  static int reconnection_trial = 0;

  // Leds
  static leds_mask_t disconnection_led = DISCONNECTION_LED;
  static leds_mask_t publish_led = PUBLISH_LED;

  static struct ctimer publish_led_timer;

  // State machine periodic
  static unsigned time_period = DEFAULT_TIME_PERIOD;

  if(state == STATE_BOOT){ // Build data structure, only once
    // Initialize the ClientID as MAC address
    snprintf(client_id, CLIENT_ID_BUFFER_SIZE, "%02x%02x%02x%02x%02x%02x",
                      linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
                      linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
                      linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

    // Broker registration (initialize data structure)					 
    mqtt_register(&conn, _main_proc, client_id, mqtt_event, MAX_TCP_SEGMENT_SIZE);

    // Initialize the state 
    state = STATE_INIT;
              
    //Set led timer (blink when disconnected)
    ctimer_set(&led_timer, LED_TOGGLE_PERIOD, toggle_led, &disconnection_led);
  }
              
  if(state == STATE_INIT){ // Check network availability 
      state = have_connectivity() ? STATE_NET_OK:STATE_INIT;
  } 
  
  if(state == STATE_NET_OK){ // Connect to MQTT server
    
    LOG_DBG("Connecting to broker ...\n");
    
    memcpy(broker_address, broker_ip, strlen(broker_ip));
    mqtt_connect(&conn, broker_address, DEFAULT_BROKER_PORT, (_publish_time_period * 3) / CLOCK_SECOND, MQTT_CLEAN_SESSION_ON);

    state = STATE_CONNECTING;
  }
  
  if(state == STATE_CONNECTED){ // Subscribe to topic (only one admitted by the implementation)
    
    LOG_DBG("Subscribing topic \"%s\" ...\n", _sub_topic);
    
    status = mqtt_subscribe(&conn, NULL, _sub_topic, MQTT_QOS_LEVEL_0);              
    if(status == MQTT_STATUS_OUT_QUEUE_FULL) {
      LOG_ERR("Tried to subscribe but command queue was full!\n");
      return -1;
    }
    reconnection_trial = 0; // Reset number of trials
    
    // Stop blinking led
    ctimer_stop(&led_timer);
    leds_off(disconnection_led);
    
    state = STATE_SUBSCRIBED;
    return _publish_time_period;
  }

  if(state == STATE_SUBSCRIBED){ // Publish messages
    leds_on(publish_led);

    LOG_DBG("Publish message on topic \"%s\"\n", _pub_topic);

    strncpy(app_buffer, build_message(), APP_BUFFER_SIZE);		
    mqtt_publish(&conn, NULL, _pub_topic, (uint8_t *)app_buffer, strlen(app_buffer), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);     
            
    time_period = _publish_time_period;

    ctimer_set(&publish_led_timer, CLOCK_SECOND / 3, led_off, &publish_led);
  } 

  else if (state == STATE_DISCONNECTED){ // Handle disconnection (infinite trials)
      
      // Blink led
      ctimer_reset(&led_timer);
    
      // Ensure disconnection
      mqtt_disconnect(&conn);
      
      // Reset the state
      state = STATE_INIT;    

      // Backoff period
      time_period = (reconnection_trial < MAX_BACKOFF_EXP) ? 
        (RECONNECT_INTERVAL << reconnection_trial) : (RECONNECT_INTERVAL << MAX_BACKOFF_EXP); 
      
      reconnection_trial += 1;

      LOG_WARN("Reconnection trial #%d in %u seconds\n", reconnection_trial, (unsigned)(time_period / CLOCK_SECOND));	

      return time_period;
  }

  return time_period;
}
