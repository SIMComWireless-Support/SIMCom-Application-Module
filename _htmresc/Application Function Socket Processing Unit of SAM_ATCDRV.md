[<- Return to Main Directory](../README.md)

<img src="simcom_logo.png" style="background-color:rgb(251, 252, 252); padding: 5px;" width="100">

# Application Function Socket Processing Unit of SAM_ATCDRV

LTE Module

SIMCom Wireless Solutions (Shanghai) Co., Ltd.  
SIMCom Headquarters Building, Building 3, No. 289 Linhong Road, Changning District, Shanghai  
Tel: 86-21-31575100  
Technical Support Email: [support@simcom.com](https://cn.simcom.com/online_questions.html)  
Official Website: [www.simcom.com](https://www.simcom.com)

| Name: | Socket Interface User Guide |
|---|---|
| Version: | V1.01 |
| Category: | Application Document |
| Status: | Released |

# Copyright Notice

This manual contains technical information of SIMCom Wireless Solutions (Shanghai) Co., Ltd. (referred to as SIMCom). Without the written permission of SIMCom, no unit or individual may copy excerpts or the entire content of this manual, or disseminate it in any form. Violators will be held legally responsible. SIMCom reserves all rights to intellectual property such as patents, utility models, or designs involved in the technical information. SIMCom reserves the right to update the specific content of this manual at any time without notice.

The copyright of this manual belongs to SIMCom. Anyone who copies, quotes, or modifies this manual without the written permission of our company will bear legal responsibility.

SIMCom Wireless Solutions (Shanghai) Co., Ltd.  
SIMCom Headquarters Building, Building 3, No. 289 Linhong Road, Changning District, Shanghai  
Tel: 86-21-31575100  
Email: simcom@simcom.com  
Official Website: [www.simcom.com](https://www.simcom.com)

For more information, please click the following link:  
http://cn.simcom.com/download/list-230-cn.html

For technical support, please click the following link:  
http://cn.simcom.com/ask/index-cn.html or send an email to support@simcom.com

Copyright © 2022 SIMCom Wireless Solutions (Shanghai) Co., Ltd. All rights reserved.

# Version History

| Version | Date | Author | Remarks |
|---|---|---|---|
| V1.00 | 2025-7-9 |  | First version |

[TOC]

# 1. Overview

This Socket sub-module provides a series of interfaces and functions for handling different types of socket operations, including TCP, UDP, TCP Server, UDP Server, and SSL/TLS sockets. The module supports multiple CIP modes and data reception formats, and provides event callback and data callback mechanisms to facilitate users in processing socket events and receiving data.

# 2. Header File Inclusion

Before using the interfaces of this module, the `SamSocket.h` header file needs to be included:

```c
#include "SamSocket.h"
```

# 3. Data Structures and Enumeration Types

## 3.1 Enumeration Types

### 3.1.1 `Sam_Mdm_Socket_Type_t`

Defines different types of sockets:

```c
typedef enum {
    SAM_MDM_SOCKET_TYPE_TCP,    /**< TCP socket */
    SAM_MDM_SOCKET_TYPE_UDP,    /**< UDP socket */
    SAM_MDM_SOCKET_TYPE_UDP_SERVER,    /**< UDP Server socket */
    SAM_MDM_SOCKET_TYPE_TCP_SERVER,    /**< TCP Server socket */
    SAM_MDM_SOCKET_TYPE_SSL     /**< SSL/TLS socket */
} Sam_Mdm_Socket_Type_t;
```

### 3.1.2 `Sam_Mdm_Socket_Cipmode_t`

Defines different CIP modes:

```c
typedef enum {
    SAM_MDM_SOCKET_CIPMODE_NONE,    /**< CIP NonTransparent mode */
    SAM_MDM_SOCKET_CIPMODE_TRANSPARENT,    /**< CIP Transparent mode */
} Sam_Mdm_Socket_Cipmode_t;
```

### 3.1.3 `Sam_Mdm_Socket_Rxform_t`

Defines different data reception formats:

```c
typedef enum {
    SAM_MDM_SOCKET_RXFORM_RAW,    /**< Rxget Form raw data mode */
    SAM_MDM_SOCKET_RXFORM_ASCII,    /**< ASCII mode */
    SAM_MDM_SOCKET_RXFORM_HEX    /**< HEX mode */
} Sam_Mdm_Socket_Rxform_t;
```

### 3.1.4 `Sam_Mdm_Socket_State_t`

Defines different states of the socket:

```c
typedef enum {
    SAM_MDM_SOCKET_STATE_INIT,    /**< Socket init */
    SAM_MDM_SOCKET_STATE_OPENING,   /**< Socket opening */
    SAM_MDM_SOCKET_STATE_CONNECTED, /**< Socket connected */
    SAM_MDM_SOCKET_STATE_SENDING,   /**< Sending data */
    SAM_MDM_SOCKET_STATE_RECEIVING, /**< Receiving data */
    SAM_MDM_SOCKET_STATE_CLOSING, /**< Socket closing */
    SAM_MDM_SOCKET_STATE_CLOSED,    /**< Socket closed */
    SAM_MDM_SOCKET_STATE_ERROR      /**< Error */
} Sam_Mdm_Socket_State_t;
```

### 3.1.5 `Sam_Mdm_Socket_Event_t`

Defines different events of the socket:

```c
typedef enum {
    SAM_MDM_SOCKET_EVENT_NONE,
    SAM_MDM_SOCKET_EVENT_ACCEPT,    // TCP server accepted a new client socket
    SAM_MDM_SOCKET_EVENT_CLOSED_PASSIVE, // Closed by remote
} Sam_Mdm_Socket_Event_t;
```

## 3.2 Data Structures

### 3.2.1 `Sam_Mdm_Socket_Config_t`

Defines the configuration parameters of the socket:

```c
typedef struct {
    uint8_t atChannelId;           /**< AT channel ID */
    uint8_t socketId;               /**< Socket ID */
    Sam_Mdm_Socket_Cipmode_t cipmode; /**< cip mode */
    Sam_Mdm_Socket_Type_t type;     /**< Socket type */
    Sam_Mdm_Socket_Rxform_t rxform; /**< RX get form type */
    char host[64];               /**< Server host name or IP address */
    uint16_t port;                  /**< Server port */
    uint16_t localport;          /**< local port for UDP, if type is TCP server, this is listening port */
    uint8_t srvIndex;       /**< Server Index */
} Sam_Mdm_Socket_Config_t;
```

### 3.2.2 `Sam_Mdm_Socket_t`

Defines the structure of the socket module, including state, configuration, callbacks, and methods:

```c
typedef struct Sam_Mdm_Socket_t {
    Sam_Mdm_Base_t base;          /**< Inherit from the base class */
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
    uint8        runlink;        //for run link in atclink 

    /* Methods */
    bool (*init)(struct Sam_Mdm_Socket_t* self, const char * cfgstr);
    bool (*deinit)(struct Sam_Mdm_Socket_t* self);
    bool (*close)(struct Sam_Mdm_Socket_t* self);
    uint32_t (*send)(struct Sam_Mdm_Socket_t* self, const uint8_t* data, uint32_t length);
    uint8_t (*process)(struct Sam_Mdm_Socket_t* self);
    uint8_t (*getState)(struct Sam_Mdm_Socket_t* self);
    bool (*setUserCallback)(struct Sam_Mdm_Socket_t* self, Sam_Mdm_Socket_Event_Callback_t eventCb, Sam_Mdm_Socket_Data_Callback_t dataCb, void* context);
} Sam_Mdm_Socket_t;
```

# 4. Interface Functions

## 4.1 `Sam_Mdm_Socket_Create`

| Interface | `Sam_Mdm_Socket_t* Sam_Mdm_Socket_Create(const Sam_Mdm_Socket_Config_t* config)` |
|:---:|:---|
| Function | Creates a new socket module instance. |
| Parameters | <ul><li>`config`：Configuration parameters of the socket module.</li></ul> |
| Return Value | <ul><li>Success: Returns a pointer to the new socket module instance.</li><li>Failure: Returns `NULL`.</li></ul> |

## 4.2 `Sam_Mdm_Socket_Destroy`

| Interface | `void Sam_Mdm_Socket_Destroy(Sam_Mdm_Socket_t* socket);` |
|:---:|:---|
| Function | Destroys a socket module instance. |
| Parameters | <ul><li>`socket`：Pointer to the socket module instance to be destroyed.</li></ul> |

## 4.3 `Sam_Mdm_Socket_init`

| Interface | `bool Sam_Mdm_Socket_init(struct Sam_Mdm_Socket_t* self, const char * cfgstr);` |
|:---:|:---|
| Function | Initializes a socket module instance. |
| Parameters | <ul><li>`self`：Pointer to the socket module instance to be initialized.</li><li>`cfgstr`：Configuration string, in the following format:<br>`"\vCFGSCT_M1\t${atChannel}\t${atType}\t${socketId}\t${cipmode}\t${type}\t${rxform}\t${host}\t${port}\t${localport}\v"`</li></ul> |
| Return Value | <ul><li>Success: Returns `true`.</li><li>Failure: Returns `false`.</li></ul> |

## 4.4 `Sam_Mdm_Socket_process`

| Interface | `uint8_t Sam_Mdm_Socket_process(struct Sam_Mdm_Socket_t* self);` |
|:---:|:---|
| Function | Processes the socket module. |
| Parameters | <ul><li>`self`：Pointer to the socket module instance to be processed.</li></ul> |
| Return Value | The current state of the socket module. |

## 4.5 `Sam_Mdm_Socket_setCallback`

| Interface | `bool Sam_Mdm_Socket_setCallback(struct Sam_Mdm_Socket_t* self, Sam_Mdm_Socket_Event_Callback_t eventCb, Sam_Mdm_Socket_Data_Callback_t dataCb, void* context);` |
|:---:|:---|
| Function | Sets the callback functions of the socket module. |
| Parameters | <ul><li>`self`：Pointer to the socket module instance for which to set the callback functions.</li><li>`eventCb`：Event callback function.</li><li>`dataCb`：Data callback function.</li><li>`context`：User context.</li></ul> |
| Return Value | <ul><li>Success: Returns `true`.</li><li>Failure: Returns `false`.</li></ul> |

## 4.6 `Sam_Mdm_Socket_Send`

| Interface | `uint32_t Sam_Mdm_Socket_Send(struct Sam_Mdm_Socket_t* self, const uint8_t* data, uint32_t length);` |
|:---:|:---|
| Function | Sends data through the socket. |
| Parameters | <ul><li>`self`：Pointer to the socket module instance through which to send data.</li><li>`data`：Data buffer to be sent.</li><li>`length`：Length of the data buffer.</li></ul> |
| Return Value | The actual number of bytes sent. |

## 4.7 `Sam_Mdm_Socket_Close`

| Interface | `bool Sam_Mdm_Socket_Close(struct Sam_Mdm_Socket_t* socket);` |
|:---:|:---|
| Function | Closes the socket. |
| Parameters | <ul><li>`socket`：Pointer to the socket module instance to be closed.</li></ul> |
| Return Value | <ul><li>Success: Returns `true`.</li><li>Failure: Returns `false`.</li></ul> |

## 4.8 `Sam_Mdm_Socket_getState`

| Interface | `uint8_t Sam_Mdm_Socket_getState(struct Sam_Mdm_Socket_t* self);` |
|:---:|:---|
| Function | Gets the current state of the socket. |
| Parameters | <ul><li>`self`：Pointer to the socket module instance for which to get the state.</li></ul> |
| Return Value | The current state of the socket. |

# 5. Usage Examples

## 5.1 Creating and Initializing a New Socket

```c
#include "SamSocket.h"

void create_and_init_socket() {
    Sam_Mdm_Socket_Config_t config = {0};
    config.atChannelId = 0;
    config.socketId = 0;
    config.cipmode = SAM_MDM_SOCKET_CIPMODE_NONE;
    config.type = SAM_MDM_SOCKET_TYPE_TCP;
    config.rxform = SAM_MDM_SOCKET_RXFORM_RAW;
    strcpy(config.host, "117.131.85.142");
    config.port = 60044;
    config.localport = 0;
    Sam_Mdm_Socket_t* socket = Sam_Mdm_Socket_Create(&config);

    if (socket != NULL) {
        char cfgstr[64];
        sprintf(cfgstr, "\vCFGSCT_M1\t%d\tA\t%d\t%d\t%d\t%d\t%s\t%d\t%d\v",
            config.atChannelId,
            config.socketId,
            config.cipmode,
            config.type,
            config.rxform,
            config.host,
            config.port,
            config.localport);

        if (Sam_Mdm_Socket_init(socket, cfgstr)) {
            // Initialization successful
        }
    }
}
```

## 5.2 Setting Callback Functions

```c
void SocketEventCallback(uint8_t socketId, Sam_Mdm_Socket_Event_t event, void *msg, void* context) {
    // Handle socket events
}

void SocketDataCallback(uint8_t socketId, const uint8_t* data, uint32_t length, void* context) {
    // Handle received data
}

void set_callbacks(Sam_Mdm_Socket_t* socket) {
    if (socket != NULL) {
        Sam_Mdm_Socket_setCallback(socket, SocketEventCallback, SocketDataCallback, (void *)socket);
    }
}
```

## 5.3 Sending Data

```c
void send_data(Sam_Mdm_Socket_t* socket) {
    if (socket != NULL) {
        const uint8_t data[] = "Hello World!";
        uint32_t length = sizeof(data) - 1;
        uint32_t sent = Sam_Mdm_Socket_Send(socket, data, length);
        if (sent == length) {
            // Data sent successfully
        }
    }
}
```

## 5.4 Closing the Socket

```c
void close_socket(Sam_Mdm_Socket_t* socket) {
    if (socket != NULL) {
        if (Sam_Mdm_Socket_Close(socket)) {
            // Closed successfully
            Sam_Mdm_Socket_Destroy(socket);
        }
    }
}
```

# 6. Notes

- Before using the socket module, ensure that the relevant dependent libraries and header files are correctly included.

- The format of the configuration string must be strictly in accordance with the specified format; otherwise, initialization may fail.

- After using the socket module, the `Sam_Mdm_Socket_Destroy` function needs to be called to destroy the instance to release resources.

[<- Return to Main Directory](../README_en.md)
