/**
 * @file SamFota.h
 * @brief Firmware Over-the-Air (FOTA) module. 
 * @details This header file defines the structures, enums, and function prototypes
 *        related to FOTA operations in the project. It provides a comprehensive set of definitions
 *        for FOTA configuration, state management, and callback handling.
 * @version 1.0
 * @date 2025-06-20
 * @author 	LiXianshuai <lixianshuai@sunseaaiot.com>
 * @copyright Copyright (c) 2025, SIMCom Wireless Solutions Limited. All rights reserved.
 * 
 * @note 
 *
 *
 */

#ifndef SAM_FOTA_H
#define SAM_FOTA_H

#include <stdint.h>
#include <stdbool.h>

#include "SamInc.h"
#include "SamMdm.h"
#include "SamSocket.h"

// Define the buffer length for FOTA operations
#define FOTA_MAX_URL_LENGTH     256
#define FOTA_MAX_USERNAME_LENGTH    128
#define FOTA_MAX_PASSWORD_LENGTH    128
//#define FOTA_MAX_VERSION_LENGTH 32
//#define FOTA_MAX_FILE_SIZE      (1024 * 1024)  // 1MB

/**
 * @brief FOTA mode enum. Defines fota mode that a FOTA download way.
 */
typedef enum {
    FOTA_MODE_FTP,            /**< FTP way */
    FOTA_MODE_HTTP,            /**< HTTP way */
} Sam_Fota_Mode_t;

/**
 * @brief FOTA adiff type enum. 
 */
typedef enum {
    FOTA_ADIFF_FULL,            /**< FULL system adiff */
    FOTA_ADIFF_MINI,            /**< MINI system adiff */
} Sam_Fota_Adiff_t;

/**
 * @brief FOTA state enum. Defines the different states that a FOTA operation can be in.
 */
typedef enum {
    FOTA_STATE_IDLE,            /**< FOTA is idle and waiting for commands */
//    FOTA_STATE_CHECKING,        /**< Checking for updates */
    FOTA_STATE_DOWNLOADING,     /**< Downloading firmware */
//    FOTA_STATE_VERIFYING,       /**< Verifying firmware integrity */
//    FOTA_STATE_READY,           /**< Firmware ready for installation */
    FOTA_STATE_INSTALLING,      /**< Installing firmware */
    FOTA_STATE_SUCCESS,         /**< FOTA completed successfully */
    FOTA_STATE_FAILED,          /**< FOTA failed */
} Sam_Fota_State_t;

/**
 * @brief FOTA error enum. Defines the different error conditions that can occur during a FOTA operation.
 */
typedef enum {
    FOTA_ERROR_NONE,            /**< No error */
//    FOTA_ERROR_NETWORK,         /**< Network error */
    FOTA_ERROR_ATERROR,         /**< AT Response error */
    FOTA_ERROR_DOWNLOAD,        /**< Download error */
    FOTA_ERROR_CHECKSUM,        /**< Checksum verification failed */
    FOTA_ERROR_INSTALL,         /**< Installation error */
    FOTA_ERROR_STORAGE,         /**< Storage error */
    FOTA_ERROR_ABORTED,         /**< Operation aborted */

    FOTA_ERROR_COMPLETED = 100,     /**<  FOTA COMPLETE, it will restart in 8s.*/
    FOTA_ERROR_URL_INVALID = 1001,     /**<  FOTA URL is invalid, maybe PDP was active.*/
    FOTA_ERROR_TIMEOUT,     /**<  FOTA timeout*/
    FOTA_ERROR_URL_UNKNOWN,     /**<  FOTA URL is unknown*/
    FOTA_ERROR_1004,     /**<  FOTA username or password is error*/
    FOTA_ERROR_1005,     /**<  FOTA file is not exist*/
    FOTA_ERROR_1006,     /**<  The size of FOTA file is invalid*/
    FOTA_ERROR_1007,     /**<  Get file failed*/
    FOTA_ERROR_1008,     /**<  Check file error*/
    FOTA_ERROR_1009,     /**<  Check file error*/
    FOTA_ERROR_1010,     /**<  Fota file too large*/
    FOTA_ERROR_1011,     /**<  Fota set flag error*/
    FOTA_ERROR_1012,     /**<  Fota parameter size error*/
} Sam_Fota_Error_t;

/**
 * @brief FOTA progress callback function type.
 * @param progress Current progress (0-100).
 * @param context User context.
 */
typedef void (*Sam_Fota_Progress_Callback_t)(uint8_t progress, void* context);

/**
 * @brief FOTA status callback function type.
 * @param state Current FOTA state.
 * @param error Error code, if any.
 * @param context User context.
 */
typedef void (*Sam_Fota_Status_Callback_t)(Sam_Fota_State_t state, Sam_Fota_Error_t error, void* context);

/**
 * @brief FOTA configuration structure. Defines the configuration parameters for a FOTA operation.
 */
typedef struct {
    uint8_t atChannelId;                        /**< AT channel ID */
    uint8_t channel;                            /**< 0¨C5 means the channel numbe */
    Sam_Fota_Mode_t mode;               /**< download mode */
    char serverUrl[FOTA_MAX_URL_LENGTH+1];      /**< Server URL for firmware download */
    char username[FOTA_MAX_USERNAME_LENGTH+1];      /**< User name for firmware download */
    char password[FOTA_MAX_PASSWORD_LENGTH+1];      /**< password for firmware download */
//    char currentVersion[FOTA_MAX_VERSION_LENGTH];  /**< Current firmware version */
//    uint32_t timeoutMs;                       /**< Timeout for network operations in milliseconds */
//    uint32_t retryCount;                      /**< Number of retries for failed operations */
//    bool verifyChecksum;                      /**< Enable checksum verification */
//    bool autoInstall;                         /**< Automatically install firmware after download */
} Sam_Fota_Config_t;

/**
 * @brief FOTA module structure. Defines the structure of the FOTA module, including its state,
 *        configuration, callbacks, and methods.
 */
typedef struct Sam_Fota_t {
    Sam_Mdm_Base_t base;          /**< Inherit from the base class */
    Sam_Mdm_Atc_t *phatc;         /**< Pointer to AT command handler */
    Sam_Fota_Config_t config;     /**< FOTA configuration */
    
    Sam_Fota_Progress_Callback_t progressCallback; /**< Progress callback */
    Sam_Fota_Status_Callback_t statusCallback;     /**< Status callback */
    void* context;                  /**< Callback context */

//    Sam_Fota_State_t state;         /**< Current FOTA state */
    Sam_Fota_Error_t error;         /**< Current error code */
    uint8_t progress;               /**< Download progress (0-100) */
//    uint32_t downloadedSize;        /**< Size of downloaded data in bytes */
//    uint32_t totalSize;             /**< Total size of the firmware in bytes */
//    char firmwarePath[128];         /**< Path to the downloaded firmware file */

    uint8	runlink;	//for run link in atclink  
    Sam_Fota_Adiff_t adiff;
    
    /* Methods */
    bool (*init)(struct Sam_Fota_t* self, const char* cfgstr);
//    bool (*init)(struct Sam_Fota_t* self, const Sam_Fota_Config_t* config);
    bool (*deinit)(struct Sam_Fota_t* self);
//    bool (*checkUpdate)(struct Sam_Fota_t* self);
//    bool (*downloadFirmware)(struct Sam_Fota_t* self);
//    bool (*verifyFirmware)(struct Sam_Fota_t* self);
//    bool (*installFirmware)(struct Sam_Fota_t* self);
//    bool (*abort)(struct Sam_Fota_t* self);
    Sam_Fota_State_t (*getState)(struct Sam_Fota_t* self);
    uint8_t (*getProgress)(struct Sam_Fota_t* self);
    Sam_Fota_Error_t (*getError)(struct Sam_Fota_t* self);
    bool (*setCallbacks)(struct Sam_Fota_t* self, 
                        Sam_Fota_Progress_Callback_t progressCb, 
                        Sam_Fota_Status_Callback_t statusCb, 
                        void* context);
} Sam_Fota_t;

/**
 * @brief Create a new FOTA module instance.
 * @param config FOTA configuration.
 * @return Pointer to the new FOTA module instance, or NULL on error.
 */
Sam_Fota_t* Sam_Fota_Create(const Sam_Fota_Config_t* config);

/**
 * @brief Destroy a FOTA module instance.
 * @param fota Pointer to the FOTA module instance.
 */
void Sam_Fota_Destroy(Sam_Fota_t* fota);

/**
 * @brief Initialize the FOTA module.
 * @param self Pointer to the FOTA module instance.
 * @param config FOTA configuration.
 * @return true if initialization is successful, false otherwise.
 */
//bool Sam_Fota_Init(struct Sam_Fota_t* self, const Sam_Fota_Config_t* config);
bool Sam_Fota_Init(struct Sam_Fota_t* self, const char* cfgstr);

/**
 * @brief Deinitialize the FOTA module.
 * @param self Pointer to the FOTA module instance.
 * @return true if deinitialization is successful, false otherwise.
 */
bool Sam_Fota_Deinit(struct Sam_Fota_t* self);

#if 0
/**
 * @brief Check for firmware updates.
 * @param self Pointer to the FOTA module instance.
 * @return true if the check operation is started successfully, false otherwise.
 */
bool Sam_Fota_CheckUpdate(struct Sam_Fota_t* self);

/**
 * @brief Download the firmware.
 * @param self Pointer to the FOTA module instance.
 * @return true if the download operation is started successfully, false otherwise.
 */
bool Sam_Fota_DownloadFirmware(struct Sam_Fota_t* self);

/**
 * @brief Verify the downloaded firmware.
 * @param self Pointer to the FOTA module instance.
 * @return true if the verification is successful, false otherwise.
 */
bool Sam_Fota_VerifyFirmware(struct Sam_Fota_t* self);

/**
 * @brief Install the verified firmware.
 * @param self Pointer to the FOTA module instance.
 * @return true if the installation is started successfully, false otherwise.
 */
bool Sam_Fota_InstallFirmware(struct Sam_Fota_t* self);

/**
 * @brief Abort the current FOTA operation.
 * @param self Pointer to the FOTA module instance.
 * @return true if the operation is aborted successfully, false otherwise.
 */
bool Sam_Fota_Abort(struct Sam_Fota_t* self);
#endif /* 0 */

/**
 * @brief Get the current state of the FOTA operation.
 * @param self Pointer to the FOTA module instance.
 * @return The current FOTA state.
 */
Sam_Fota_State_t Sam_Fota_GetState(struct Sam_Fota_t* self);

/**
 * @brief Get the current progress of the FOTA operation.
 * @param self Pointer to the FOTA module instance.
 * @return The current progress (0-100).
 */
uint8_t Sam_Fota_GetProgress(struct Sam_Fota_t* self);

/**
 * @brief Get the current error code of the FOTA operation.
 * @param self Pointer to the FOTA module instance.
 * @return The current error code.
 */
Sam_Fota_Error_t Sam_Fota_GetError(struct Sam_Fota_t* self);

/**
 * @brief Set the callback functions for the FOTA module.
 * @param self Pointer to the FOTA module instance.
 * @param progressCb Progress callback function.
 * @param statusCb Status callback function.
 * @param context User context.
 * @return true if the callback functions are set successfully, false otherwise.
 */
bool Sam_Fota_SetCallbacks(struct Sam_Fota_t* self, 
                          Sam_Fota_Progress_Callback_t progressCb, 
                          Sam_Fota_Status_Callback_t statusCb, 
                          void* context);

#endif /* SAM_FOTA_H */
