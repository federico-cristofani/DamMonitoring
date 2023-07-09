/*---------------------------------------------------------------------------*/
#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_
/*---------------------------------------------------------------------------*/

/* Minimize collision probability */
#define IEEE802154_CONF_PANID 0xF1F1

/* Define log levels */
#define LOG_LEVEL_APP LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_COAP LOG_LEVEL_NONE

/* BR webserver configuration */
#ifndef WEBSERVER_CONF_CFS_CONNS
#define WEBSERVER_CONF_CFS_CONNS 2
#endif

#ifndef BORDER_ROUTER_CONF_WEBSERVER
#define BORDER_ROUTER_CONF_WEBSERVER 1
#endif

#ifdef BORDER_ROUTER_CONF_WEBSERVER
#define UIP_CONF_TCP 1
#endif

#ifdef COAP_MAX_CHUNK_SIZE 
#undef COAP_MAX_CHUNK_SIZE
#endif
#define COAP_MAX_CHUNK_SIZE 512
/*---------------------------------------------------------------------------*/
#endif /* PROJECT_CONF_H_ */
/*---------------------------------------------------------------------------*/