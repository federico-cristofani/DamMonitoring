
/*---------------------------------------------------------------------------*/
#ifndef MQTT_CLIENT_H_
#define MQTT_CLIENT_H_
/*---------------------------------------------------------------------------*/

#define LOG_MODULE "mqtt-client"
#ifdef LOG_CONF_LEVEL_MQTT 
    #define LOG_LEVEL LOG_CONF_LEVEL_MQTT
#else
    #define LOG_LEVEL LOG_LEVEL_NONE
#endif

/*---------------------------------------------------------------------------*/

// MQTT broker address
#define MQTT_CLIENT_BROKER_IP_ADDR "fd00::1"

// Default config values
#define DEFAULT_BROKER_PORT         1883
#define DEFAULT_TIME_PERIOD        (CLOCK_SECOND * 1)

// We assume that the broker does not require authentication

// Various states
#define STATE_BOOT          0
#define STATE_INIT          1
#define STATE_NET_OK        2
#define STATE_CONNECTING    3
#define STATE_CONNECTED     4
#define STATE_SUBSCRIBED    5
#define STATE_DISCONNECTED  6

/*---------------------------------------------------------------------------*/
/* Maximum TCP segment size for outgoing segments of our socket */
#define MAX_TCP_SEGMENT_SIZE    32
#define CONFIG_IP_ADDR_STR_LEN  64

// Automatic reconnection parameters
#define RECONNECT_INTERVAL          (CLOCK_SECOND * 1)
#define MAX_BACKOFF_EXP             6
#define LED_TOGGLE_PERIOD           (CLOCK_SECOND * 1)
#define DISCONNECTION_LED           LEDS_RED

#define PUBLISH_LED                 LEDS_GREEN


#endif /* MQTT_CLIENT_H_ */
/*---------------------------------------------------------------------------*/
