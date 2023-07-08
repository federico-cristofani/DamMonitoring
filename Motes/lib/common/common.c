#include "common.h"
#include "jsonparse.h"
#include "string.h"
#include "stdlib.h" /* atoi */

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