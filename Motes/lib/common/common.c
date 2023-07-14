#include "linkaddr.h"
#include "common.h"
#include "jsonparse.h"
#include "string.h"
#include "stdlib.h" /* atoi */
#include "stdio.h" /* snprintf */

#define URN_BUF_LEN  31
extern linkaddr_t linkaddr_node_addr; 

char urn_buf[URN_BUF_LEN];

char* URN(){
    snprintf(urn_buf, URN_BUF_LEN, "urn:dev:mac:%02x%02x%02x%02x%02x%02x%02x%02x/",
         linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1], 
         linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[3], 
         linkaddr_node_addr.u8[4], linkaddr_node_addr.u8[5], 
         linkaddr_node_addr.u8[7],linkaddr_node_addr.u8[6]);
  
  return urn_buf;
}

int get_json_int_field(const char* json, char* const key, int* value){
  
  struct jsonparse_state parser;
  int type;
  char key_buf[JSON_BUFFER];
  char value_buf[JSON_BUFFER];
  
  // Init data structure
  jsonparse_setup(&parser, json, strlen(json));
  
  // Iterate JSON string
  while ((type = jsonparse_next(&parser)) != JSON_TYPE_ERROR) {
    if (type == JSON_TYPE_PAIR_NAME) { /* Key */
      jsonparse_copy_value(&parser, key_buf, JSON_BUFFER);
    } 
    else if (type == JSON_TYPE_NUMBER && strncmp(key_buf, key, JSON_BUFFER) == 0) { /* Value */
      jsonparse_copy_value(&parser, value_buf, JSON_BUFFER);
      *value = atoi(value_buf);   
      return !(strcmp(value_buf, "0") != 0 && *value == 0); /* Check conversion from string */
    }
  }
  return -1;
}

int get_json_bool_field(const char* json, char* const key, bool* value){
  
  struct jsonparse_state parser;
  int type;
  char key_buf[JSON_BUFFER];
  char value_buf[JSON_BUFFER];
  
  // Init data structure
  jsonparse_setup(&parser, json, strlen(json));
  
  // Iterate JSON string
  while ((type = jsonparse_next(&parser)) != JSON_TYPE_ERROR) {
    if (type == JSON_TYPE_PAIR_NAME) { /* Key */
      jsonparse_copy_value(&parser, key_buf, JSON_BUFFER);
    } 
    else if (type == JSON_TYPE_TRUE && strncmp(key_buf, key, JSON_BUFFER) == 0) { /* Value */
      jsonparse_copy_value(&parser, value_buf, JSON_BUFFER);
      *value = true;
      return strncmp(value_buf, "true", JSON_BUFFER) == 0;   /* Check conversion from string */
      
    }
    else if (type == JSON_TYPE_FALSE && strncmp(key_buf, key, JSON_BUFFER) == 0) { /* Value */
      jsonparse_copy_value(&parser, value_buf, JSON_BUFFER);
      *value = false;
      return strncmp(value_buf, "false", JSON_BUFFER) == 0;   /* Check conversion from string */
    }
  }
  return -1;
}