/**
 * @file SamSms.h
 * @brief Definition of SMS client data structures and interfaces
 * @details This header file declares the data structures, enumerations, and function
 *          prototypes required for SMS client operations. It defines types for SMS
 *          context, message queues, state machines, and error codes, along with
 *          interfaces for initializing the client, sending/receiving messages, and
 *          managing SMS resources.
 * 
 * @version 1.0.0
 * @date 2025-08-01
 * @author <dong.chen@sunseaaiot.com>
 * @copyright Copyright (c) 2025, Your Company Inc. All rights reserved.
 * 
 * @note The SMS client uses AT commands (CSCA, CPMS, CMGF, etc.) to interact with
 *       the modem. UCS2 encoding is required for non-ASCII characters (e.g., Chinese).
 */

#ifndef __SAMSMS_H
#define __SAMSMS_H
#include "SamInc.h"
#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @brief SMS content encoding types
 * 
 * Specifies the encoding format for SMS content to ensure compatibility with network protocols.
 */
typedef enum {
    ENCODING_ASCII,      /**< ASCII encoding (supports English and digits only) */
    ENCODING_UCS2        /**< UCS2 encoding (supports Unicode characters for multi-language) */
} EncodingType;

/**
 * @brief SMS content language types
 * 
 * Indicates the language of SMS content, used to determine encoding and display logic.
 */
typedef enum {
    LANG_NONE,           /**< No specific language */
    LANG_CN,             /**< Chinese language (requires UCS2 encoding) */
    LANG_EN,             /**< English language (supports ASCII encoding) */
    LANG_OTHER,          /**< Other languages */
    LANG_DEFAULT = LANG_EN  /**< Default language (English) */
} LangType;

/**
 * @brief SMS message sending information
 * 
 * Stores detailed information of an SMS message to be sent, including content, encoding, and recipient.
 */
typedef struct {
    char *content;        /**< Pointer to SMS content string (null-terminated) */
    EncodingType encoding; /**< Encoding type of the content */
    uint32 length;      /**< Length of content in bytes (excluding null terminator) */
    LangType language;    /**< Language type of the content */
    char num[128];        /**< Recipient phone number (supports international format with country code) */
} send_msg_info_t;

/**
 * @brief Node structure for SMS send queue
 * 
 * Represents a single node in the linked list of pending SMS messages.
 */
typedef struct send_msg_node {
    send_msg_info_t send_sms_data;  /**< SMS message data */
    struct send_msg_node* next;     /**< Pointer to next node in the queue */
} send_msg_node_t;

/**
 * @brief SMS send queue structure
 * 
 * Manages a linked list of SMS messages waiting to be sent, tracking queue length.
 */
typedef struct {
    send_msg_node_t* send_sms_head;  /**< Head pointer of the send queue */
    unsigned int length;             /**< Number of messages in the queue */
} send_msg_list_t;

/**
 * @brief SMS storage memory types
 * 
 * Specifies the storage location for SMS messages (SIM card or device memory).
 */
typedef enum {
    SMS_MEM_TYPE_NONE,   /**< No storage specified */
    SMS_MEM_TYPE_SM,     /**< SIM card memory */
    SMS_MEM_TYPE_ME      /**< Device (modem) memory */
} sms_mem_type;

/**
 * @brief SMS configuration parameters
 * 
 * Stores basic configuration for SMS client, including service center number and storage settings.
 */
typedef struct {
    char sms_center[15];  /**< SMS service center number (e.g., "+8613800xxx500") */
    sms_mem_type mem_type; /**< Preferred storage type for SMS */
    bool is_new;          /**< Flag indicating new message handling */
} sms_cfg_t;

/**
 * @brief SMS context structure
 * 
 * Maintains runtime state for SMS client, including configuration, message queues, and read status.
 */
typedef struct {
    sms_cfg_t sms_cfg;        /**< SMS configuration parameters */
    send_msg_list_t send_msg_list;  /**< Queue of messages to be sent */
    int16 readIndex;        /**< Index of the SMS message to read ( -1 = no pending read) */
} sms_context_t;

/**
 * @brief SMS client state types
 * 
 * Defines the operational states of the SMS client state machine.
 */
typedef enum {
    SMS_STATUS_NONE,         /**< Initial state (uninitialized) */
    SMS_STATUS_INIT,         /**< Initialization in progress */
    SMS_STATUS_DATA_PROCESS, /**< Processing SMS messages (sending/receiving) */
    SMS_STATUS_CFG_CHECK,    /**< Configuration validation */
    SMS_STATUS_IDLE,         /**< Idle state (ready for operations) */
    SMS_STATUS_FAIL          /**< Error state (operation failed) */
} sms_state_type;

/**
 * @brief SMS client operation steps
 * 
 * Sub-states for each main state, controlling detailed workflow in the state machine.
 */
typedef enum {
    SMS_STEP_NONE,                  /**< No step active */
    
    // Steps for SMS_STATUS_INIT
    SMS_INIT_STEP_CSCA,             /**< Configure SMS service center address */
    SMS_INIT_STEP_CSCA_RES_CHECK,   /**< Check result of CSCA configuration */
    SMS_INIT_STEP_CPMS,             /**< Configure preferred message storage */
    SMS_INIT_STEP_CPMS_RES_CHECK,   /**< Check result of CPMS configuration */
    SMS_INIT_STEP_CMGF,             /**< Set message format (text mode) */
    SMS_INIT_STEP_CMGF_RES_CHECK,   /**< Check result of CMGF configuration */
    SMS_INIT_STEP_CNMI,             /**< Configure new message notification */
    SMS_INIT_STEP_CNMI_RES_CHECK,   /**< Check result of CNMI configuration */
    
    // Steps for SMS_STATUS_DATA_PROCESS
    SMS_DATAPROC_STEP_CSCS,         /**< Set character set for SMS */
    SMS_DATAPROC_STEP_CSCS_RES_CHECK,/**< Check result of CSCS configuration */
    SMS_DATAPROC_STEP_CSMP,         /**< Set message parameters (coding, validity) */
    SMS_DATAPROC_STEP_CSMP_RES_CHECK,/**< Check result of CSMP configuration */
    SMS_DATAPROC_STEP_CMGS,         /**< Send SMS message */
    SMS_DATAPROC_STEP_CMGS_RES_CHECK,/**< Check result of message sending */
    SMS_DATAPROC_STEP_CMGR,         /**< Read SMS message */
    SMS_DATAPROC_STEP_CMGR_RES_CHECK,/**< Check result of message reading */
    
    // Steps for SMS_STATUS_IDLE/SMS_STATUS_FAIL
    SMS_ILDE_STEP_FIRST,            /**< Placeholder for idle state */
    SMS_FAIL_STEP_FIRST,            /**< Placeholder for error state */
    SMS_STEP_MAX                    /**< Total number of steps (for validation) */
} sms_option_step_type;

/**
 * @brief SMS error types
 * 
 * Identifies specific failure reasons in SMS operations.
 */
typedef enum {
    SMS_FAIL_TYPE_NONE,         /**< No error */
    SMS_FAIL_TYPE_CSCA_FAIL,    /**< Failed to configure service center */
    SMS_FAIL_TYPE_CPMS_FAIL,    /**< Failed to configure message storage */
    SMS_FAIL_TYPE_CMGF_FAIL,    /**< Failed to set message format */
    SMS_FAIL_TYPE_CNMI_FAIL,    /**< Failed to configure notification */
    SMS_FAIL_TYPE_CSCS_FAIL,    /**< Failed to set character set */
    SMS_FAIL_TYPE_CSMP_FAIL,    /**< Failed to set message parameters */
    SMS_FAIL_TYPE_CMGS_FAIL,    /**< Failed to send message */
    SMS_FAIL_TYPE_CMGR_FAIL,    /**< Failed to read message */
    SMS_FAIL_TYPE_MAX           /**< Total number of error types */
} sms_fail_type;

/** Forward declaration of SMS client structure */
struct TSmsTag;

/**
 * @brief Callback function type for received SMS messages
 * 
 * Defines the interface for handling incoming SMS messages.
 * @param psms Pointer to SMS client instance
 * @param pnum Sender's phone number
 * @param pCtxt SMS content
 */
typedef void (* sam_sms_receive_data_cb)(struct TSmsTag *psms, char *pnum, char *pCtxt);

/**
 * @brief SMS client main structure
 * 
 * Encapsulates all runtime state for an SMS client instance, including state machine,
 * configuration, message queues, and callback pointers.
 */
typedef struct TSmsTag {
    sms_state_type sta;                  /**< Current state of the SMS client */
    sms_option_step_type step;           /**< Current step in the state machine */
    uint8 dcnt;                       /**< Retry counter for failed operations */
    uint8 runlink;                    /**< Link identifier for AT command processing */
    uint32 msclk;                      /**< Timestamp for timing operations */
    uint8 stim;                       /**< Second-level timer for state transitions */
    HdsAtcTag *phatc;                   /**< Pointer to AT command channel */
    sms_context_t sms_context;           /**< SMS context (configuration and queues) */
    sms_fail_type fail_type;             /**< Current error type (if in failure state) */
    uint8 init_fail_cont;              /**< Counter for initialization failures */
    uint8 send_fail_cont;              /**< Counter for message sending failures */
    char send_buf[512];                  /**< Buffer for encoding outgoing messages */
    char read_num[128];                  /**< Buffer for storing sender number of received SMS */
    char read_ctx[512];                  /**< Buffer for storing content of received SMS */
    sam_sms_receive_data_cb receive_data_cb; /**< Callback for received messages */
} TSmsTag;

/**
 * @brief Initialize SMS client instance
 * 
 * Initializes SMS context, configures basic parameters from a configuration string,
 * and sets up the state machine.
 * @param psms Pointer to uninitialized TSmsTag structure
 * @param cfgstr Configuration string containing SMS parameters
 * @return Pointer to initialized SMS client instance, or NULL on failure
 */
extern void *sam_sms_init(TSmsTag *psms, char *cfgstr);

/**
 * @brief Main SMS client processing function
 * 
 * Implements the state machine for SMS operations (initialization, sending, receiving).
 * Should be called periodically to process pending operations.
 * @param pvsms Pointer to SMS client instance (TSmsTag)
 * @return Status code indicating processing result
 */
extern unsigned char sam_sms_proc(void *pvsms);

/**
 * @brief Stop SMS client and release resources
 * 
 * Unlinks the SMS client from the AT command channel and frees all allocated memory.
 * @param pvsms Pointer to SMS client instance
 * @return 1 on success, 0 if input is NULL
 */
extern unsigned char sam_sms_stop(void *pvsms);

/**
 * @brief Queue an SMS message for sending
 * 
 * Adds an SMS message to the send queue, with specified content, recipient, encoding, and language.
 * @param psms Pointer to SMS client instance
 * @param p_content SMS content string
 * @param p_num Recipient phone number
 * @param encoding Encoding type (ASCII/UCS2)
 * @param language Language type of content
 * @return 1 if message is queued successfully, 0 on failure
 */
extern uint8 sam_sms_send_message(TSmsTag *psms, char *p_content, char *p_num, EncodingType encoding, LangType language);

/**
 * @brief Set callback for received SMS messages
 * 
 * Registers a callback function to be invoked when a new SMS message is received.
 * @param psms Pointer to SMS client instance
 * @param cb Callback function to register
 */
extern void sam_sms_set_receive_callback(TSmsTag *psms, sam_sms_receive_data_cb cb);

/**
 * @brief Close SMS client connection
 * 
 * Initiates cleanup of SMS client resources (placeholder for future expansion).
 * @param pvsms Pointer to SMS client instance
 * @return 1 on success, 0 if input is NULL
 */
extern uint8 sam_sms_close(TSmsTag *pvsms);

#ifdef __cplusplus
}
#endif
#endif
