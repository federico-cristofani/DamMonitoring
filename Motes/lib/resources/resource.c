#include "coap-engine.h"
#include "log.h"

#define LOG_MODULE "res"
#ifndef LOG_LEVEL_APP
  #define LOG_LEVEL_APP LOG_LEVEL_INFO
#endif
#define LOG_LEVEL LOG_LEVEL_APP

void log_request(const char* method, coap_message_t* request){
  LOG_INFO("%s request from <", method);
  LOG_INFO_6ADDR(&request->src_ep->ipaddr);
  if(request->content_format == APPLICATION_JSON || request->content_format == TEXT_PLAIN){
    LOG_INFO_(">, payload <%s>\n", (request->payload ? (char*)request->payload:"<none>"));
  }
  else{
    LOG_INFO_(">, payload length %d bytes\n", request->payload_len);
  }
}