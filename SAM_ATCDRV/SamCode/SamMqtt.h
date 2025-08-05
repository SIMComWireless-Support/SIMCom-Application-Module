/**
 * @file SamMqtt.h
 * @brief Header file defining MQTT client data structures, enumerations, and function interfaces
 * 
 * This header file encapsulates all definitions necessary for implementing MQTT protocol 
 * communication functionality, including message structures for publishing/subscribing, 
 * state machine enumerations, and declarations for initialization, processing, and 
 * communication operations. These interfaces enable developers to easily integrate 
 * MQTT client capabilities with an MQTT broker.
 * 
 * @author <dong.chen@sunseaaiot.com>
 * @date 2025-07-01
 * @version 1.0.0
 * @copyright Copyright (c) 2025, Your Company Inc. All rights reserved.
 */
#ifndef __SAMMQTT_H
#define __SAMMQTT_H

#include "SamInc.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if 0
#define SUBTOPIC_MAX_COUNT 10

typedef struct{
    // CMQTTSUB or CMQTTSUBTOPIC para
    uint16 sub_topic_req_lenth; // Length of the subscription topic, range 1~1024
    uint8  sub_qos; // The QoS of the message, range 0~2
    uint8  sub_dup; // Dup flag, range 0~1; default value 0 if not set
    char *p_sub_topic; // Subscription topic string
}sub_msg_info_t;

typedef struct sub_msg_node {
    sub_msg_info_t sub_data;
	bool sub_finish;
    struct sub_msg_node* next;
} sub_msg_node_t;

typedef struct {
    sub_msg_node_t* sub_head;
    unsigned int whole_length;
	unsigned int left_cnt;
} sub_msg_list_t;
#endif
/**
 * @brief Data structure for MQTT publish messages.
 * @details Defines the parameters required to publish an MQTT message, 
 *          including topic, payload, quality of service (QoS), and retain flag.
 * 
 * This structure is used by the MQTT client to format messages 
 * before publishing to the broker.
 * 
 * @note All string pointers must point to valid memory; the client is responsible 
 *       for memory management of these buffers.
 */
typedef struct{
	//CMQTTTOPIC para, pub topic info
    uint16 pub_topic_req_lenth; // Length of the publish topic, range 1~1024
    char *p_pub_topic; // Publish topic string

    // CMQTTPAYLOAD para
    uint16 pub_msg_req_lenth; // Length of the publish message, range 1~10240
    char *p_pub_msg;  // Publish message string

    // CMQTTPUB para
    uint8  pub_qos; // The quality of service of the message, range 0~2
    uint8  pub_timeout; // The publishing timeout interval value, range 1~180S
    uint8 ratained; // The retain flag of the publish message. The value is 0 or 1. The default value is 0.
    uint8  pub_dup; // Dup flag, range 0~1; default value 0 if not set
}pub_msg_info_t;

/**
 * @brief Node structure for storing MQTT publish message context.
 */
typedef struct pub_msg_node {
    pub_msg_info_t pub_data;
    struct pub_msg_node* next;
} pub_msg_node_t;
/**
 * @brief Linked list structure for managing MQTT publish messages.
 */
typedef struct {
    pub_msg_node_t* pub_head;
    unsigned int length;
} pub_msg_list_t;

#define CLIENTID_LEN_MAX 128


/**
 * @brief MQTT client context structure for storing connection and message parameters.
 * @details This structure consolidates all configuration parameters required for 
 *          MQTT client operations, including connection settings, will messages, 
 *          subscription topics, and publish message lists. It follows the CMQTT 
 *          protocol specification for parameter organization.
 * 
 * @note All string pointers must point to valid memory; the client is responsible 
 *       for memory management of these buffers.
 */
typedef struct{
    // cmqttaccq para
    char *p_client_id;  // Length range 1~128
    uint8 server_type;  // 0 for TCP; 1 for TLS or SSL

    // CMQTTWILLTOPIC para
    uint16 willtopic_req_length;  // Length of the will message topic
    char *p_willtopic; // Will topic string

    // CMQTTWILLMSG para
    uint16 willmsg_req_length;  // Length of the will message request
    uint8  willmsg_qos; // QoS of the will message
    char *p_willmsg; // Will message string

    // CMQTTCONNECT para
    char *p_server_addr; // Server address
    uint16 keepalive_time;  // Keep alive time
    uint8 clean_session;  // Clean session
    char *p_username;
    char *p_password;
    
    // CMQTTSUB or CMQTTSUBTOPIC para
    uint16 sub_topic_req_lenth; // Length of the subscription topic, range 1~1024
    uint8  sub_qos; // The QoS of the message, range 0~2
    uint8  sub_dup; // Dup flag, range 0~1; default value 0 if not set
    char *p_sub_topic; // Subscription topic string

    pub_msg_list_t pub_msg_list;
}mqtt_context_t;

//#define	TMQTT_DNBUFLEN	(1024+2+1)   // Maximum buffer size for MQTT received data
//#define	TMQTT_UPBUFLEN	(1024+2+1)    // Maximum buffer size for MQTT sent data

#define MQTT_SUB_TOPIC_MAX_LEN 1024
#define MQTT_SUB_PAYLOAD_MAX_LEN 1024
#define MQTT_PUB_TOPIC_MAX_LEN 1024
#define MQTT_PUB_PAYLOAD_MAX_LEN 1024

#define MQTT_CLINTID_MAX_LEN 128
#define MQTT_SERVER_ADDR_MAX_LEN 256
#define MQTT_WILLTOPIC_MAX_LEN 1024
#define MQTT_WILLTMSG_MAX_LEN 1024


/**
 * @brief MQTT state machine status types
 * 
 * This enumeration defines the possible states of an MQTT client
 * during its operational lifecycle, used for state-driven protocol handling.
 */
typedef enum
{
    MQTT_STATUS_NONE,             /**< Initial state, MQTT client not initialized */
    MQTT_STATUS_INIT,             /**< Initialization state, resources being allocated */
    MQTT_STATUS_DATA_PROCESS,     /**< Data processing state, handling MQTT protocol data */
    MQTT_STATUS_CONNECT_CLOSE_REQUEST_CHECK, /**< Connection close request check state */
    MQTT_STATUS_IDLE,             /**< Idle state, connected but no active processing */
    MQTT_STATUS_FAIL,             /**< Failure state, error encountered during operation */
    MQTT_STATUS_CONNECT_RESET     /**< Connection reset state, reconnecting process */
} mqtt_state_type;

/**
 * @brief MQTT protocol operation step definitions
 * 
 * This enumeration defines the detailed operational steps within each state
 * of the MQTT state machine. Each step corresponds to a specific phase
 * in the MQTT protocol interaction process.
 */

typedef enum
{
	MQTT_STEP_NONE,
		
    // Steps for MQTT_STATUS_INIT
	MQTT_INIT_STEP_CMQTTSTART,
	MQTT_INIT_STEP_CMQTTSTART_RES_CHECK,
	MQTT_INIT_STEP_CMQTTACCQ,
	MQTT_INIT_STEP_CMQTTACCQ_RES_CHECK,
    MQTT_INIT_STEP_CMQTTWILLTOPIC,
    MQTT_INIT_STEP_CMQTTWILLTOPIC_RES_CHECK,
    MQTT_INIT_STEP_CMQTTWILLMSG,
    MQTT_INIT_STEP_CMQTTWILLMSG_RES_CHECK,
	MQTT_INIT_STEP_CMQTTCONNECT,
	MQTT_INIT_STEP_CMQTTCONNECT_RES_CHECK,
	MQTT_INIT_STEP_URC_MASK_SIGN,
	
    // Steps for MQTT_STATUS_DATA_PROCESS
    MQTT_DATAPROC_STEP_CMQTTSUB,
    MQTT_DATAPROC_STEP_CMQTTSUB_RES_CHECK,
    MQTT_DATAPROC_STEP_CMQTTTOPIC,
    MQTT_DATAPROC_STEP_CMQTTTOPIC_RES_CHECK,
    MQTT_DATAPROC_STEP_CMQTTPAYLOAD,
    MQTT_DATAPROC_STEP_CMQTTPAYLOAD_RES_CHECK,
    MQTT_DATAPROC_STEP_CMQTTPUB,
    MQTT_DATAPROC_STEP_CMQTTPUB_RES_CHECK,

    // Steps for MQTT_STATUS_CONNECT_CLOSE_REQUEST_CHECK
    MQTT_STATUS_SWITCH_STEP_CMQTTDISC,
    MQTT_STATUS_SWITCH_STEP_CMQTTDISC_RES_CHECK,
    MQTT_STATUS_SWITCH_STEP_CMQTTREL,
    MQTT_STATUS_SWITCH_STEP_CMQTTREL_RES_CHECK,
    MQTT_STATUS_SWITCH_STEP_CMQTTSTOP,
    MQTT_STATUS_SWITCH_STEP_CMQTTSTOP_RES_CHECK,
    MQTT_STATUS_SWITCH_STEP_SERVICE_CLOSE,

    // Steps for MQTT_STATUS_IDLE
    MQTT_ILDE_STEP_FIRST,  // Not used

    // Steps for MQTT_STATUS_FAIL
    MQTT_FAIL_STEP_FIRST,  // Not used

    // Steps for MQTT_STATUS_CONNECT_RESET
	MQTT_CONNECT_RESET_STEP_CMQTTDISC,
    MQTT_CONNECT_RESET_STEP_CMQTTDISC_RES_CHECK,
    MQTT_CONNECT_RESET_STEP_CMQTTREL,
    MQTT_CONNECT_RESET_STEP_CMQTTREL_RES_CHECK,
    MQTT_CONNECT_RESET_STEP_CMQTTSTOP,
    MQTT_CONNECT_RESET_STEP_CMQTTSTOP_RES_CHECK,


	MQTT_STEP_MAX
}mqtt_option_step_type;

/**
 * @brief MQTT failure type definitions
 * 
 * This enumeration defines the possible failure scenarios that can occur
 * during MQTT client operation. Each value represents a specific type of
 * error that may be encountered during protocol execution.
 */
typedef enum
{
	MQTT_FAIL_TYPE_NONE,
	MQTT_FAIL_TYPE_CONNECT_FAIL,
	MQTT_FAIL_TYPE_MQTTSTART_FAIL,	
	MQTT_FAIL_TYPE_MQTTACCQ_FAIL,	
	MQTT_FAIL_TYPE_MQTTWILLTOPIC_FAIL,	
	MQTT_FAIL_TYPE_MQTTWILLMSG_FAIL,
	MQTT_FAIL_TYPE_SUB_FAIL,
	MQTT_FAIL_TYPE_PUB_FAIL,
	MQTT_FAIL_TYPE_CLOSE_FAIL,
	MQTT_FAIL_TYPE_MAX
}mqtt_fail_type;

/**
 * @brief MQTT connection status definitions
 * 
 * This enumeration defines the possible connection states of an MQTT client.
 * It represents the high-level connection status after connection attempts
 * or during active communication.
 */
typedef enum
{
	MQTT_DISCONNECTED,
	MQTT_CONNECTED,
	MQTT_CONNECT_STATUS_MAX
}mqtt_connect_res_status;

struct TMqttTag;
typedef void (* sam_mqtt_receive_data_cb)(struct TMqttTag * pmqtt, uint8 index, char *topic, char *pMsg);

//#define	TMQTT_DNBUFLEN	1460
//#define	TMQTT_UPBUFLEN	1460

/**
 * @brief MQTT client data structure for maintaining runtime state and configuration.
 * @details This structure encapsulates the runtime context of an MQTT client, including 
 *          connection state, operation steps, failure counters, buffer storage, and callback 
 *          handlers. It serves as the primary data container for managing MQTT client operations.
 * 
 * @note This structure is thread-safe only if access to its members is properly synchronized.
 */
typedef struct TMqttTag {
	mqtt_state_type	sta;
	//uint8	step;
	mqtt_option_step_type	step;
	uint8	dcnt;
    uint8	runlink;	// For run link in atclink  
    uint32	msclk;	// For recording system clock
    uint8	stim;	// Second timer for user  

	uint8  client_index;
	mqtt_connect_res_status  connect_status;

	HdsAtcTag * phatc;
	mqtt_context_t mqtt_context;
	uint8 close_req;
	//uint8 stop_req;
	mqtt_fail_type fail_type;
	//uint8 connect_fail_cont;
	uint8 init_fail_cont;
	uint8 sub_fail_cont;
	uint8 pub_fail_cont;
	uint8 close_fail_cont;
	
	uint32 	urcbmk;
	
	char	t_buf[MQTT_SUB_TOPIC_MAX_LEN + 3];
	uint16	t_cnt;

    char	m_buf[MQTT_SUB_PAYLOAD_MAX_LEN + 3];
	uint16	m_cnt;

    sam_mqtt_receive_data_cb receive_data_cb;
}TMqttTag;

/**
 * @brief Parameters required for client initialization.
 */
typedef struct{
    uint8  at_channel;  // AT channel
    uint8  client_index; // Client index
    char *p_client_id;
    char *p_server_addr; // Server address
    char *p_sub_topic;

	char *p_will_topic;
	char *p_will_msg;
}mqtt_init_para_for_cus_t;

//extern void * sam_mqtt_init(TMqttTag * pmqtt, mqtt_init_para_for_cus_t * cfg_mqtt_info);

/**
 * @brief Initialize MQTT client instance with configuration string
 *
 * Parses configuration parameters from a formatted string, initializes MQTT context,
 * and sets up communication channel for MQTT operations. The configuration string 
 * should contain parameters for client ID, server address, topics, and communication settings.
 *
 * @param pmqtt Pointer to MQTT client structure to be initialized
 * @param cfgstr Configuration string containing MQTT parameters in predefined format
 *
 * @return void* Pointer to initialized MQTT client structure (same as input parameter)
 *               or NULL if initialization fails (invalid parameters or context init error)
 *
 * @note Configuration string format example:
 *       "\vCFGMQTT_C%d\t%d\t%d\t\"%[^\"]\"\t\"%[^\"]\"\t\"%[^\"]\"\t\"%[^\"]\"\t\"%[^\"]\"\v"
 *       where parameters are: config index, AT channel, client index, client ID, server address,
 *       subscription topic, will topic, will message
 *
 * @warning Both pmqtt and cfgstr must be valid pointers. Invalid configuration string format
 *          may lead to undefined behavior or initialization failure
 */
extern void * sam_mqtt_init(TMqttTag * pmqtt, char * cfgstr);

/**
 * @brief Main processing function for MQTT client operations
 *
 * Manages the entire lifecycle of MQTT client interactions, including initialization, connection,
 * data publishing/subscription, connection closure, and error handling. Implements state machine logic
 * to handle different MQTT states (initialization, data processing, idle, failure, etc.) and transitions
 * between them based on command responses and system events.
 *
 * @param pvmqtt Pointer to the MQTT client structure (TMqttTag) cast as void*
 *
 * @return unsigned char Return code indicating processing status:
 *                       - RETCHAR_KEEP: Continue processing in subsequent calls
 *                       - RETCHAR_FREE: Release resources and exit processing
 *                       - Other values: Error or specific state indicators
 *
 * @note Handles AT command interactions for MQTT operations (CMQTTSTART, CMQTTCONNECT, CMQTTSUB, etc.)
 *       and manages retry mechanisms for failed operations
 * @warning The function relies on proper initialization of the MQTT context and AT channel structures
 *          passed via the pvmqtt parameter
 */
extern unsigned char sam_mqtt_proc(void * pvmqtt);

/**
 * @brief Stop MQTT client and release associated resources
 *
 * This function stops the MQTT client operation by unlinking the MQTT processing function from the communication channel
 * and releasing all dynamically allocated resources in the MQTT context through the sam_mqtt_context_release function.
 *
 * @param pmqtt Pointer to the MQTT client structure (TMqttTag type) to be stopped
 *
 * @return unsigned char Returns '1' (as a character value) on successful stopping and resource release;
 *                       returns 0 if the input pointer is NULL (no operation performed)
 *
 * @note The function safely handles NULL input by returning immediately without any action
 * @warning Ensure the input pointer points to a valid, initialized MQTT client structure to avoid undefined behavior
 */
extern unsigned char sam_mqtt_stop(void * pvmqtt);

// App interface
/**
 * @brief Subscribe to an MQTT topic
 *
 * Adds a specified topic to the MQTT context for subscription, but only if there is no existing
 * subscription topic in the context. If a topic is already present, the function returns failure.
 * Performs no operation if the input MQTT client structure or topic string is NULL.
 *
 * @param pmqtt Pointer to the MQTT client structure (TMqttTag)
 * @param pTopic Pointer to the null-terminated string representing the topic to subscribe to
 *
 * @return uint8 Return value indicating subscription status:
 *               1 - Subscription topic added successfully
 *               0 - Failure (invalid input, or an existing subscription topic is already present)
 *
 * @note Relies on add_sub_topic() for actual topic storage and memory allocation
 * @warning Does not automatically clear existing subscriptions; ensure prior subscriptions are cleared if needed
 */
extern uint8 sam_mqtt_subscribe_topic(TMqttTag * pmqtt, char *pTopic);

/**
 * @brief Enqueue an MQTT message for publishing
 *
 * Creates a new publish message node with the specified topic and message content,
 * then adds it to the MQTT client's publish message queue. Returns success if the message
 * is successfully enqueued, or failure if any input parameters are invalid or memory allocation fails.
 *
 * @param pmqtt Pointer to the MQTT client structure (TMqttTag)
 * @param pTopic Pointer to the null-terminated string representing the publish topic
 * @param pMsg Pointer to the null-terminated string representing the publish message payload
 *
 * @return uint8 Return value indicating operation status:
 *               1 - Message successfully enqueued for publishing
 *               0 - Failure (invalid input parameters or memory allocation failure)
 *
 * @note Uses sam_insert_new_pub_data_into_list() internally to handle node creation and queue insertion
 * @warning The actual message transmission is handled asynchronously via the MQTT processing loop
 */
extern uint8  sam_mqtt_publish_message(TMqttTag * pmqtt, char *pTopic, char *pMsg);

/**
 * @brief Check the MQTT connection status
 *
 * Determines whether the MQTT client is currently connected to the broker by examining
 * the connection status flag in the provided MQTT client structure.
 *
 * @param pmqtt Pointer to the MQTT client structure (TMqttTag)
 *
 * @return uint8 Connection status:
 *               1 - MQTT client is connected to the broker (status is MQTT_CONNECTED)
 *               0 - MQTT client is not connected (any other status)
 *
 * @note Status is based on the internal connection flag, not actual network state
 * @warning Does not verify network connectivity; returns cached status
 */
extern uint8  sam_mqtt_get_connection_status(TMqttTag * pmqtt);

/**
 * @brief Set the callback function for incoming MQTT messages
 *
 * Registers a callback function to be invoked when the MQTT client receives new messages.
 * The callback will be called with parameters including the MQTT client context, message index,
 * topic, and payload. Performs no operation if either the MQTT client structure pointer or
 * the callback function pointer is NULL.
 *
 * @param pmqtt Pointer to the MQTT client structure (TMqttTag)
 * @param cb Pointer to the callback function of type sam_mqtt_receive_data_cb
 *
 * @note The callback function prototype should match:
 *       void callback_function(TMqttTag *pmqtt, uint32_t index, char *topic, char *payload)
 * @warning Overwrites any previously set callback function
 */
extern  void  sam_mqtt_set_receive_callback(TMqttTag * pmqtt, sam_mqtt_receive_data_cb cb);

/**
 * @brief Request to close the MQTT connection
 *
 * Initiates the process to disconnect from the MQTT broker by setting the close request flag
 * in the MQTT client structure, but only if the client is currently connected. Returns success
 * if the connection close request is queued, or failure if the client is not connected or the input
 * parameter is invalid.
 *
 * @param pvmqtt Pointer to the MQTT client structure (TMqttTag)
 *
 * @return uint8 Return value indicating operation status:
 *               1 - Connection close request successfully initiated
 *               0 - Failure (client not connected or invalid input parameter)
 *
 * @note Actual disconnection is handled asynchronously via the MQTT processing loop
 * @warning Does not immediately terminate the connection; use sam_mqtt_stop() for immediate resource cleanup
 */
extern uint8  sam_mqtt_close(TMqttTag * pvmqtt);



#ifdef __cplusplus
}
#endif





#endif
