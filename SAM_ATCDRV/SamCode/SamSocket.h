/**
 * @file SamSocket.h
 * @brief Socket handling module. 
 * @details This header file defines the structures, enums, and function prototypes
 *        related to socket operations in the project. It provides a comprehensive set of definitions
 *        for socket configuration, state management, and callback handling.
 * @version 1.0
 * @date 2025-05-15
 * @author lixianshuai
 * (c) Copyright 2025-2030, ae@sim.com
 * 
 * @note 
 *
 *
 */

#ifndef SAM_MDM_SOCKET_H
#define SAM_MDM_SOCKET_H

#include <stdint.h>
#include <stdbool.h>

#include "SamInc.h"
#include "SamMdm.h"

// Define the buffer length for downlink and uplink data
#define	TSCM_DNBUFLEN	1460
#define	TSCM_UPBUFLEN	1460

// Define the type alias for the AT command structure
#define Sam_Mdm_Atc_t HdsAtcTag

// Forward declaration of the Sam_Mdm_t structure
typedef struct Sam_Mdm_t Sam_Mdm_t;

/**
 * @brief Socket type enum. Defines the different types of sockets that can be created.
 */
typedef enum {
    SAM_MDM_SOCKET_TYPE_TCP,    /**< TCP socket */
    SAM_MDM_SOCKET_TYPE_UDP,    /**< UDP socket */
    SAM_MDM_SOCKET_TYPE_UDP_SERVER,    /**< UDP Server socket */
    SAM_MDM_SOCKET_TYPE_TCP_SERVER,    /**< TCP Server socket */
    SAM_MDM_SOCKET_TYPE_SSL     /**< SSL/TLS socket */
} Sam_Mdm_Socket_Type_t;

/**
 * @brief Socket CIP Mode enum. Defines the different CIP modes for the socket.
 */
typedef enum {
    SAM_MDM_SOCKET_CIPMODE_NONE,    /**< CIP NonTransparent mode */
    SAM_MDM_SOCKET_CIPMODE_TRANSPARENT,    /**< CIP Transparent mode */
} Sam_Mdm_Socket_Cipmode_t;

/**
 * @brief Socket RX Get form type enum. Defines the different data formats for receiving data.
 */
typedef enum {
    SAM_MDM_SOCKET_RXFORM_RAW,    /**< Rxget Form raw data mode */
    SAM_MDM_SOCKET_RXFORM_ASCII,    /**< ASCII mode */
    SAM_MDM_SOCKET_RXFORM_HEX    /**< HEX mode */
} Sam_Mdm_Socket_Rxform_t;

/**
 * @brief Socket state enum. Defines the different states that a socket can be in.
 */
typedef enum {
    SAM_MDM_SOCKET_STATE_INIT,    /**< Socket init */
    SAM_MDM_SOCKET_STATE_OPENING,   /**< Socket opening */
//    SAM_MDM_SOCKET_STATE_OPEN,      /**< Socket open */
//    SAM_MDM_SOCKET_STATE_CONNECTING, /**< Socket connecting */
    SAM_MDM_SOCKET_STATE_CONNECTED, /**< Socket connected */
    SAM_MDM_SOCKET_STATE_SENDING,   /**< Sending data */
    SAM_MDM_SOCKET_STATE_RECEIVING, /**< Receiving data */
    SAM_MDM_SOCKET_STATE_CLOSING, /**< Socket closing */
    SAM_MDM_SOCKET_STATE_CLOSED,    /**< Socket closed */
    SAM_MDM_SOCKET_STATE_ERROR      /**< Error */
} Sam_Mdm_Socket_State_t;

/**
 * @brief Socket error enum. Defines the different error conditions that can occur with a socket.
 */
typedef enum {
    SAM_MDM_SOCKET_ERROR_NONE,
    SAM_MDM_SOCKET_ERROR_NET_OPEN,
    SAM_MDM_SOCKET_ERROR_CIP_OPEN,
    SAM_MDM_SOCKET_ERROR_REMOTE_CLOSE,
    SAM_MDM_SOCKET_ERROR_AT_NORESPONSE
} Sam_Mdm_Socket_Error_t;

/**
 * @brief Socket close reason enum. Defines the different reasons for a socket to be closed.
 */
typedef enum {
    SAM_MDM_SOCKET_CLOSED_LOCAL,
    SAM_MDM_SOCKET_CLOSED_REMOTE,
    SAM_MDM_SOCKET_COLSED_TIMEOUT
} Sam_Mdm_Socket_Close_Reason_t;

/**
 * @brief Socket event enum. Defines the different events that can occur with a socket.
 */
typedef enum {
    SAM_MDM_SOCKET_EVENT_NONE,
    SAM_MDM_SOCKET_EVENT_ACCEPT,    // TCP server accepted a new client socket
    SAM_MDM_SOCKET_EVENT_CLOSED_PASSIVE, // Closed by remote
} Sam_Mdm_Socket_Event_t;

/**
 * @brief Socket event callback function type.
 * @param socketId Socket ID.
 * @param event Current socket event.
 * @param msg Message associated with the event.
 * @param context User context.
 */
typedef void (*Sam_Mdm_Socket_Event_Callback_t)(uint8_t socketId, Sam_Mdm_Socket_Event_t event, void *msg, void* context);

/**
 * @brief Socket data callback function type.
 * @param socketId Socket ID.
 * @param data Data buffer.
 * @param length Data length.
 * @param context User context.
 */
typedef void (*Sam_Mdm_Socket_Data_Callback_t)(uint8_t socketId, const uint8_t* data, uint32_t length, void* context);

// Base class definition for the socket module
typedef struct Sam_Mdm_Base_t {
    uint8_t state;  // State variable
    uint8_t step;   // Step variable    
    uint8_t dcnt;   // at resend count
    uint32_t  msclk; // microsecond clock
    uint8_t sclk; // second clock
    uint8_t (*run)(struct Sam_Mdm_Base_t* self);  // Function pointer for internal processing
} Sam_Mdm_Base_t;

/**
 * @brief Socket configuration structure. Defines the configuration parameters for a socket.
 */
typedef struct {
    uint8_t atChannelId;           /**< AT channel ID */
    uint8_t socketId;               /**< Socket ID */
    Sam_Mdm_Socket_Cipmode_t cipmode; /**< cip mode */
    Sam_Mdm_Socket_Type_t type;     /**< Socket type */
    Sam_Mdm_Socket_Rxform_t rxform; /**< RX get form type */
    char host[64];               /**< Server host name or IP address */
    uint16_t port;                  /**< Server port */	
    uint16_t localport;          /**< local port for UDP, if type is TCP server, this is listerning port */
    uint8_t srvIndex;       /**< Server Index */
    
//    uint32_t timeoutMs;             /**< Connection timeout in milliseconds */
//    uint32_t bufferSize;            /**< Buffer size */
//    bool keepAlive;                 /**< Enable keep alive */
//    uint32_t keepAliveInterval;     /**< Keep alive interval in milliseconds */
} Sam_Mdm_Socket_Config_t;

/**
 * @brief Socket module structure. Defines the structure of the socket module, including its state,
 *        configuration, callbacks, and methods.
 */
typedef struct Sam_Mdm_Socket_t {
    Sam_Mdm_Base_t base;          /**< Inherit from the base class */
//    Sam_Mdm_t* parent;              /**< Parent modem instance */
    Sam_Mdm_Atc_t *phatc;
    Sam_Mdm_Socket_Config_t config; /**< Socket module configuration */
    
    Sam_Mdm_Socket_Event_Callback_t eventCallback; /**< Event callback */
    Sam_Mdm_Socket_Data_Callback_t dataCallback;   /**< Data callback */
    void* context;                  /**< Callback context */

    uint32_t urcMask;    
	
    char            upbuf[TSCM_UPBUFLEN];
    uint16_t        upcnt;
    char            dnbuf[TSCM_DNBUFLEN];
    uint16_t        dncnt;
    bool              dnflag;

    uint8_t         error;
    uint8_t         openReTryCnt;
    uint8_t         closeType; // 1-local close, 2-socket destroy

    uint8	runlink;	//for run link in atclink  
    
    /* Methods */
    bool (*init)(struct Sam_Mdm_Socket_t* self, const char * cfgstr);
    bool (*deinit)(struct Sam_Mdm_Socket_t* self);
//    bool (*open)(struct Sam_Mdm_Socket_t* self, const Sam_Mdm_Socket_Config_t* config);
    bool (*close)(struct Sam_Mdm_Socket_t* self);
    uint32_t (*send)(struct Sam_Mdm_Socket_t* self, const uint8_t* data, uint32_t length);
    uint8_t (*process)(struct Sam_Mdm_Socket_t* self);
    uint8_t (*getState)(struct Sam_Mdm_Socket_t* self);
    bool (*setUserCallback)(struct Sam_Mdm_Socket_t* self, Sam_Mdm_Socket_Event_Callback_t eventCb, Sam_Mdm_Socket_Data_Callback_t dataCb, void* context);
} Sam_Mdm_Socket_t;

/**
 * @brief Create a new socket module instance.
 * @param config Socket module configuration.
 * @return Pointer to the new socket module instance, or NULL on error.
 */
//Sam_Mdm_Socket_t* Sam_Mdm_Socket_Create(Sam_Mdm_t* parent, const Sam_Mdm_Socket_Config_t* config);
Sam_Mdm_Socket_t* Sam_Mdm_Socket_Create(const Sam_Mdm_Socket_Config_t* config);

/**
 * @brief Destroy a socket module instance.
 * @param socket Pointer to the socket module instance.
 */
void Sam_Mdm_Socket_Destroy(Sam_Mdm_Socket_t* socket);

/**
 * @brief Initialize a socket module instance.
 * @param self Pointer to the socket module instance.
 * @param cfgstr Configuration string.
 * @return true if initialization is successful, false otherwise.
 */
bool Sam_Mdm_Socket_init(struct Sam_Mdm_Socket_t* self, const char * cfgstr);

/**
 * @brief Process the socket module.
 * @param self Pointer to the socket module instance.
 * @return The current state of the socket module.
 */
uint8_t Sam_Mdm_Socket_process(struct Sam_Mdm_Socket_t* self);

/**
 * @brief Set the callback functions for the socket module.
 * @param self Pointer to the socket module instance.
 * @param eventCb Event callback function.
 * @param dataCb Data callback function.
 * @param context User context.
 * @return true if the callback functions are set successfully, false otherwise.
 */
bool Sam_Mdm_Socket_setCallback(struct Sam_Mdm_Socket_t* self, Sam_Mdm_Socket_Event_Callback_t eventCb, Sam_Mdm_Socket_Data_Callback_t dataCb, void* context);

/**
 * @brief Send data through the socket.
 * @param self Pointer to the socket module instance.
 * @param data Data buffer to be sent.
 * @param length Length of the data buffer.
 * @return The number of bytes sent.
 */
uint32_t Sam_Mdm_Socket_Send(struct Sam_Mdm_Socket_t* self, const uint8_t* data, uint32_t length);

/**
 * @brief Close the socket.
 * @param socket Pointer to the socket module instance.
 * @return true if the socket is closed successfully, false otherwise.
 */
bool Sam_Mdm_Socket_Close(struct Sam_Mdm_Socket_t* socket);

/**
 * @brief Get the current state of the socket.
 * @param self Pointer to the socket module instance.
 * @return The current state of the socket.
 */
uint8_t Sam_Mdm_Socket_getState(struct Sam_Mdm_Socket_t* self);

#endif /* SAM_MDM_SOCKET_H */
