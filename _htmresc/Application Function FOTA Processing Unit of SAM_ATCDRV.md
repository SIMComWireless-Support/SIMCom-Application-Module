[<- Return to Main Directory](../README.md)

<img src="simcom_logo.png" style="background-color:rgb(251, 252, 252); padding: 5px;" width="100">

# Application Function FOTA Processing Unit of SAM_ATCDRV

LTE Module

SimCom Wireless Solutions (Shanghai) Co., Ltd.

Building 3, 289 Linhong Road, Changning District, Shanghai

Tel: 86-21-31575100

Technical Support Email: [support@simcom.com](https://cn.simcom.com/online_questions.html)

Official Website: [www.simcom.com](https://www.simcom.com)


| Name:     | FOTA Interface User Guide |
| --------- | ------------------------- |
| Version:  | V1.01                     |
| Category: | Application Document      |
| Status:   | Released                  |

# Copyright Notice

This manual contains technical information of SimCom Wireless Solutions (Shanghai) Co., Ltd. (referred to as SimCom). Without the written permission of SimCom, no unit or individual may excerpt or copy part or all of the contents of this manual, or disseminate it in any form. Violators will be held legally responsible. SimCom reserves all rights to intellectual property such as patents, utility models, or designs involved in the technical information. SimCom reserves the right to update the specific contents of this manual at any time without notice.

The copyright of this manual belongs to SimCom. Any person who copies, quotes or modifies this manual without the written permission of our company will bear legal responsibility.

SimCom Wireless Solutions (Shanghai) Co., Ltd.

Building 3, 289 Linhong Road, Changning District, Shanghai

Tel: 86-21-31575100

Email: simcom@simcom.com

Official Website: [www.simcom.com](https://www.simcom.com)

For more information, please click the following link:

[http://cn.simcom.com/download/list-230-cn.html](http://cn.simcom.com/download/list-230-cn.html)

For technical support, please click the following link:

[http://cn.simcom.com/ask/index-cn.html](http://cn.simcom.com/ask/index-cn.html) or send an email to support@simcom.com

Copyright 漏 2022 SimCom Wireless Solutions (Shanghai) Co., Ltd. All rights reserved.

# Version History


| Version | Date     | Author | Remarks       |
| ------- | -------- | ------ | ------------- |
| V1.00   | 2025-7-9 |        | First version |

[TOC]

# 1. Overview

The FOTA (Firmware Over-the-Air) sub-module provides firmware over-the-air upgrade functionality, supporting the complete process of checking for firmware updates from a remote server, downloading firmware, verifying firmware integrity, and installing firmware. The module uses a callback mechanism to feedback upgrade progress and status in real-time, facilitating users to monitor the upgrade process, and is suitable for scenarios requiring remote update of device firmware.

# 2. Header File Inclusion

Before using the FOTA sub-module, the core header file must be included:

```c
#include "SamFota.h"
```

# 3. Core Data Structures and Enumeration Types

## 3.1 Enumeration Types

### 3.1.1 `Sam_Fota_State_t` (FOTA State)

Defines all states during the FOTA upgrade process:

```c
typedef enum {
   FOTA_STATE_IDLE,            /**< Idle state, waiting for upgrade command */
   FOTA_STATE_DOWNLOADING,     /**< Firmware is being downloaded */
   FOTA_STATE_INSTALLING,      /**< Firmware is being installed */
   FOTA_STATE_SUCCESS,         /**< Upgrade successful */
   FOTA_STATE_FAILED,          /**< Upgrade failed */
} Sam_Fota_State_t;
```

### 3.1.2 `Sam_Fota_Error_t` (FOTA Error Code)

Defines possible error types during the upgrade process:

```c
typedef enum {
   FOTA_ERROR_NONE,            /**< No error */
   FOTA_ERROR_NETWORK,         /**< Network error (such as connection failure, timeout) */
   FOTA_ERROR_DOWNLOAD,        /**< Download error (such as download interruption, incomplete data) */
   FOTA_ERROR_CHECKSUM,        /**< Checksum error (firmware integrity verification failed) */
   FOTA_ERROR_INSTALL,         /**< Installation error (such as firmware incompatibility, write failure) */
   FOTA_ERROR_STORAGE,         /**< Storage error (such as insufficient storage space) */
   FOTA_ERROR_TIMEOUT,         /**< Operation timeout */
   FOTA_ERROR_ABORTED,         /**< Upgrade manually aborted */
} Sam_Fota_Error_t;
```

## 3.2 Callback Function Types

### 3.2.1 `Sam_Fota_Progress_Callback_t` (Progress Callback)

Used to feedback upgrade progress in real-time:

```c
typedef void (*Sam_Fota_Progress_Callback_t)(uint8_t progress, void* context);
```

**Parameters**:

- `progress`: Upgrade progress (0-100, percentage).

- `context`: User context (passed in during initialization).

### 3.2.2 `Sam_Fota_Status_Callback_t` (Status Callback)

Used to notify upgrade status changes and error information:

```c
typedef void (*Sam_Fota_Status_Callback_t)(Sam_Fota_State_t state, Sam_Fota_Error_t error, void* context);
```

**Parameters**:

- `state`: Current FOTA state (see `Sam_Fota_State_t`).

- `error`: Error code (valid when the state is `FOTA_STATE_FAILED`).

- `context`: User context.

## 3.3 Core Data Structures

### 3.3.1 `Sam_Fota_Config_t` (FOTA Configuration)

Defines basic configuration parameters for FOTA upgrade:

```c
typedef struct {
   uint8_t atChannelId;                        /**< AT channel ID */
   uint8_t channel;                            /**< 0鈥�5 means the channel number */
   Sam_Fota_Mode_t mode;               /**< download mode */
   char serverUrl[FOTA_MAX_URL_LENGTH+1];      /**< Server URL for firmware download */
   char username[FOTA_MAX_USERNAME_LENGTH+1];      /**< User name for firmware download */
   char password[FOTA_MAX_PASSWORD_LENGTH+1];      /**< password for firmware download */
} Sam_Fota_Config_t;
```

### 3.3.2 `Sam_Fota_t` (FOTA Module Instance)

The core structure of the FOTA module, including state, configuration, callbacks, and internal data:

```c
typedef struct Sam_Fota_t {
   Sam_Mdm_Base_t base;          /**< Inherit from the base class */
   Sam_Mdm_Atc_t *phatc;         /**< Pointer to AT command handler */
   Sam_Fota_Config_t config;     /**< FOTA configuration */

   Sam_Fota_Progress_Callback_t progressCallback; /**< Progress callback */
   Sam_Fota_Status_Callback_t statusCallback;     /**< Status callback */
   void* context;                  /**< Callback context */
   Sam_Fota_Error_t error;         /**< Current error code */
   uint8_t progress;               /**< Download progress (0-100) */
   uint8 runlink; //for run link in atclink 
   Sam_Fota_Adiff_t adiff;

   /* Methods */
   bool (*init)(struct Sam_Fota_t* self, const char* cfgstr);
   bool (*deinit)(struct Sam_Fota_t* self);
   Sam_Fota_State_t (*getState)(struct Sam_Fota_t* self);
   uint8_t (*getProgress)(struct Sam_Fota_t* self);
   Sam_Fota_Error_t (*getError)(struct Sam_Fota_t* self);
   bool (*setCallbacks)(struct Sam_Fota_t* self,
                       Sam_Fota_Progress_Callback_t progressCb,
                       Sam_Fota_Status_Callback_t statusCb,
                       void* context);
} Sam_Fota_t;
```

# 4. Core Interface Functions

## 4.1 Module Initialization and Destruction

### 4.1.1 `Sam_Fota_Create` (Create FOTA Instance)

| Interface    | `Sam_Fota_t* Sam_Fota_Create(const Sam_Fota_Config_t* config);` |
|:---:|:---|
| Function     | Allocates memory and initializes the FOTA module instance. |
| Parameters   |<ul><li> `config`: FOTA configuration parameters (can be `NULL` to use default configuration). </li></ul>|
| Return Value | <ul><li> Success: Returns a pointer to the FOTA instance.</li><li>Failure: Returns `NULL` (memory allocation failed).</li></ul> |

### 4.1.2 `Sam_Fota_Init` (Initialize FOTA Instance)

| Interface    | `bool Sam_Fota_Init(struct Sam_Fota_t* self, const Sam_Fota_Config_t* config);` |
|:---:|:---|
| Function     | Initializes the configuration, AT channel, and internal state of the FOTA instance.|
| Parameters   |<ul><li>  `self`: Pointer to the FOTA instance.</li><li>`config`: FOTA configuration parameters. </li></ul> |
| Return Value | <ul><li> Success: `true`.</li><li>Failure: `false` (invalid parameters or AT channel initialization failed).</li></ul>  |

### 4.1.3 `Sam_Fota_Deinit` (Deinitialize FOTA Instance)

| Interface    | `bool Sam_Fota_Deinit(struct Sam_Fota_t* self);`|
|:---:|:---|
| Function     | Releases resources occupied by the FOTA instance and unregisters callbacks. |
| Parameters   | <ul><li> `self`: Pointer to the FOTA instance. </li></ul> |
| Return Value | <ul><li> Success: `true`.</li><li>Failure: `false` (invalid parameters). </li></ul> |

### 4.1.4 `Sam_Fota_Destroy` (Destroy FOTA Instance)

| Interface  | `void Sam_Fota_Destroy(Sam_Fota_t* socket);`|
|:---:|:---|
| Function   | Completely destroys the FOTA instance and releases memory. |
| Parameters | <ul><li> `self`: Pointer to the FOTA instance. </li></ul> |

## 4.2 Upgrade Process Control

### 4.2.1 `Sam_Fota_SetCallbacks` (Set Callback Functions)

| Interface    | `bool Sam_Fota_SetCallbacks(struct Sam_Fota_t* self, Sam_Fota_Progress_Callback_t progressCb, Sam_Fota_Status_Callback_t statusCb, void* context);`|
|:---:|:---|
| Function     | Registers progress callback and status callback functions.|
| Parameters   | <ul><li> `self`: Pointer to the FOTA instance.</li><li>`progressCb`: Progress callback function (can be `NULL`).</li><li>`statusCb`: Status callback function (can be `NULL`).</li><li>`context`: User context (passed through during callbacks). </li></ul> |
| Return Value | <ul><li> Success: `true`.</li><li>Failure: `false` (invalid parameters). </li></ul> |

### 4.2.2 `Sam_Fota_GetState` (Get Current State)

| Interface    | `Sam_Fota_State_t Sam_Fota_GetState(struct Sam_Fota_t* self);` |
|:---:|:---|
| Function     | Queries the current FOTA upgrade state. |
| Return Value | Current state (enumerated value of `Sam_Fota_State_t`).|

### 4.3.4 `Sam_Fota_GetError` (Get Error Code)

| Interface    | `Sam_Fota_Error_t Sam_Fota_GetError(struct Sam_Fota_t* self);` |
|:---:|:---|
| Function     | Queries the error code of the last error.|
| Return Value | Error code (enumerated value of `Sam_Fota_Error_t`).|

# 5. Application Scenario Examples

## 5.1 Basic FOTA Upgrade Process

### Example Location

A complete example of FOTA upgrade is included in `SamFotaSrv.c`.

### Code Example

```c
// 1. Define callback functions

void FotaProgressCallback(uint8_t progress, void* context) {
   printf("FOTA Progress: %d%%n", progress);
}

void FotaStatusCallback(Sam_Fota_State_t state, Sam_Fota_Error_t error, void* context) {
   printf("FOTA State: %d, Error: %dn", state, error);
   Sam_Fota_t* fota = (Sam_Fota_t*)context;
   // Handle state changes
   switch (state) {
       case FOTA_STATE_SUCCESS:
           printf("FOTA upgrade success! Rebooting...n");
           // Upgrade successful, can trigger device reboot
           break;

       case FOTA_STATE_FAILED:
           printf("FOTA upgrade failed! Error: %dn", error);
           // Handle failure logic (such as logging, retrying)
           break;

       default:
           break;
   }
}

// 2. Initialize FOTA module and perform upgrade
void fotaStart1(Sam_Fota_Mode_t mode, char *url, char *username, char *password)
{
   Sam_Fota_Config_t config = {0};
   config.atChannelId = 0;
   config.channel = 0;
   config.mode = mode;
   strcpy(config.serverUrl, url);
   strcpy(config.username, username);
   strcpy(config.password, password);

   // create fota instance using create or malloc
   fota = Sam_Fota_Create(&config);
   if (fota == NULL)
   {
       printf("Failed to create fotarn");
       return;
   }

   Sam_Fota_SetCallbacks(fota, fotaProcessCallback, fotaStateCallback, (void *)fota);
}
```

### Process Description

1.  **Configuration Initialization**: Set parameters such as firmware server URL, timeout time, and retry count.

2.  **Instance Creation**: Initialize the FOTA module through `Sam_Fota_Create` and `Sam_Fota_Init`.

3.  **Callback Registration**: Register progress and status callbacks to monitor the upgrade process in real-time.

4.  **Upgrade Trigger**: Call `Sam_Fota_CheckUpdate` to start checking for updates. The subsequent processes (download, verification, installation) are automatically triggered through callbacks (or manually called via interfaces).

[<- Return to Main Directory](../README_en.md)
