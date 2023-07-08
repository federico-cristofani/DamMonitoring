#include "stdbool.h"

/*
 * Buffer used to parse JSON strings 
 */
#define JSON_BUFFER     20 

/**
 * \brief       Extracts the value associated to the specified key from a JSON string
 * \param json  The string to parse as JSON
 * \param key   The name of JSON key 
 * \param value Integer buffer where store the value associated to the key
 * \return      Positive integer in case of success, 0 in case of bad value, -1 in case of missing key
 *             
 */
int get_json_int_field(const char*, char* const, int* );

/**
 * \brief       Extracts the value associated to the specified key from a JSON string
 * \param json  The string to parse as JSON
 * \param key   The name of JSON key 
 * \param value Boolean buffer where store the value associated to the key
 * \return      Positive integer in case of success, 0 in case of bad value, -1 in case of missing key
 *             
 */
int get_json_bool_field(const char*, char* const, bool* );