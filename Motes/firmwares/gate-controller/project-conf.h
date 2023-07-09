/*---------------------------------------------------------------------------*/
#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_
/*---------------------------------------------------------------------------*/

/* Minimize collision probability */
#define IEEE802154_CONF_PANID 0xF1F1

/* Define log levels */
#define LOG_LEVEL_APP LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_COAP LOG_LEVEL_NONE

#ifdef COAP_MAX_CHUNK_SIZE 
#undef COAP_MAX_CHUNK_SIZE
#endif
#define COAP_MAX_CHUNK_SIZE 512

/*---------------------------------------------------------------------------*/
#endif /* PROJECT_CONF_H_ */
/*---------------------------------------------------------------------------*/