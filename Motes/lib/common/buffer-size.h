/*---------------------------------------------------------------------------*/
/*
 * Buffers for Client ID
 * Make sure they are large enough to hold the entire identifier
 */
#define CLIENT_ID_BUFFER_SIZE 64
/*---------------------------------------------------------------------------*/
/*
 * Buffers for topics
 * Make sure they are large enough to hold the entire topic
 */
#define TOPIC_BUFFER_SIZE 64
/*---------------------------------------------------------------------------*/
/*
 * The main MQTT buffers
 * We will need to increase if we start publishing more data
 */
#define APP_BUFFER_SIZE 512
/*---------------------------------------------------------------------------*/
