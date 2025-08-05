/**
 * @file SamFota.c
 * @brief Firmware Over-the-Air (FOTA) module implementation. 
 * @details This file contains the implementation of functions
 *        related to FOTA operations, including initialization, state handling, and firmware management.
 * @version 1.0
 * @date 2025-06-20
 * @author 	LiXianshuai <lixianshuai@sunseaaiot.com>
 * @copyright Copyright (c) 2025, SIMCom Wireless Solutions Limited. All rights reserved.
 * 
 * @note 
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include.h"

#include "SamFota.h"
#include "SamDebug.h"
#include "SamAtc.h"

// Define AT commands for FOTA operations
//#define AT_FOTA_CHECK     "AT+FOTACHECK=\"%s\"\r"
#define AT_FOTA_DOWNLOAD  "AT+CFOTA=%u,%u,\"%s\",%s,%s\r"
//#define AT_FOTA_VERIFY    "AT+FOTACHECKSUM\r"
//#define AT_FOTA_INSTALL   "AT+FOTAINSTALL\r"
//#define AT_FOTA_ABORT     "AT+FOTAABORT\r"
//#define AT_FOTA_STATUS    "AT+FOTASTATUS?\r"

#if 0
// Timeout values in milliseconds
#define FOTA_CHECK_TIMEOUT    30000
#define FOTA_DOWNLOAD_TIMEOUT 600000
#define FOTA_VERIFY_TIMEOUT   30000
#define FOTA_INSTALL_TIMEOUT  120000
#endif /* 0 */

/**
 * @brief Handle the unsolicited result code (URC) from the AT command.
 * @param context Pointer to the context, usually the FOTA module instance.
 * @param urcBuff Pointer to the URC buffer.
 * @return The result code indicating the handling status.
 */
static uint8_t handleAtUrc(void* context, char* urcBuff);

/**
 * @brief Update the FOTA state and notify the callback.
 * @param self Pointer to the FOTA module instance.
 * @param state New FOTA state.
 * @param error Error code, if any.
 */
static void updateState(Sam_Fota_t* self, Sam_Fota_State_t state, Sam_Fota_Error_t error);

/**
 * @brief Update the FOTA progress and notify the callback.
 * @param self Pointer to the FOTA module instance.
 * @param progress New progress value (0-100).
 */
static void updateProgress(Sam_Fota_t* self, uint8_t progress);

/**
 * @brief FOTA module process main function.
 * @param self Pointer to the FOTA module instance.
 * @return The current process state.
 */
uint8_t Sam_Fota_Process(struct Sam_Fota_t* self);


#define Sam_Mdm_Atc_checkAtRsp SamChkAtcRet
#define Sam_Mdm_Atc_sendAtCmd SamSendAtCmd
#define Sam_Mdm_Atc_sendAtSeg SamSendAtSeg

static void Sam_Mdm_Atc_clearAtRevBuff(Sam_Mdm_Atc_t* self) {
    if (self == NULL)
    {
        return;
    }

    self->retbufp= 0;
    self->retbuf[0] = 0;
}

/**
 * @brief Create a new FOTA module instance.
 * @param config FOTA configuration.
 * @return Pointer to the new FOTA module instance, or NULL on error.
 */
Sam_Fota_t* Sam_Fota_Create(const Sam_Fota_Config_t* config) {
    Sam_Fota_t* self = (Sam_Fota_t*)malloc(sizeof(Sam_Fota_t));
    if (self == NULL) {
        SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_ERROR, "Failed to allocate memory for FOTA module\n");
        return NULL;
    }

    sam_dbg_set_module_level(SAM_MOD_FOTA, SAM_DBG_LEVEL_TRACE);
    
    memset(self, 0, sizeof(Sam_Fota_t));
    
    // Initialize base structure
    self->base.state = FOTA_STATE_IDLE;
    self->base.step = 0;
    self->base.dcnt = 0;
    self->base.msclk = SamGetMsCnt(0);
    self->base.sclk = 0;
    
    // Set default values
    self->error = FOTA_ERROR_NONE;
    self->progress = 0;
    
    // Initialize configuration if provided
    if (config != NULL) {
        memcpy(&self->config, config, sizeof(Sam_Fota_Config_t));
    
        self->phatc = pAtcBusArray[self->config.atChannelId];	    
        self->runlink =	SamAtcFunLink(self->phatc, self, (SamMdmFunTag)Sam_Fota_Process, (SamUrcBcFunTag)handleAtUrc);
    }
    
    // Set default methods
    self->init = Sam_Fota_Init;
//    self->deinit = Sam_Fota_Deinit;
//    self->checkUpdate = Sam_Fota_CheckUpdate;
//    self->downloadFirmware = Sam_Fota_DownloadFirmware;
//    self->verifyFirmware = Sam_Fota_VerifyFirmware;
//    self->installFirmware = Sam_Fota_InstallFirmware;
//    self->abort = Sam_Fota_Abort;
    self->getState = Sam_Fota_GetState;
    self->getProgress = Sam_Fota_GetProgress;
    self->getError = Sam_Fota_GetError;
    self->setCallbacks = Sam_Fota_SetCallbacks;
    
    return self;
}

/**
 * @brief Destroy a FOTA module instance.
 * @param fota Pointer to the FOTA module instance.
 */
void Sam_Fota_Destroy(Sam_Fota_t* self) {
    if (self == NULL) {
        return;
    }
    
    // Cleanup resources
    if (self->phatc != NULL) {
        // Unregister URC handler
        SamAtcFunUnlink(self->phatc, self->runlink);
    }
    
    free(self);
}

/**
 * @brief Initialize the fota module.
 * @param self Pointer to the fota module instance.
 * @param cfgstr Pointer to the configuration string.
 * @return true if initialization is successful, false otherwise.
 *
 * The configuration string format is as follows:
 * "\vCFGSCT_M1\t${atChannel}\t${atType}\t${channel}\t${mode}\t${serverUrl}\t${username}\t${password}\v"
 * where:
 * - ${atChannel}: AT channel ID (e.g., 0)
 * - ${atType}: AT type (e.g., 'A')
 * - ${channel}: channel numbe, range 0~5
 * - ${mode}: download mode, refer to Sam_Fota_Mode_tt
 * - ${serverUrl}: The remote site server`s IP address or URL address. (e.g., "http://117.131.85.142:60058/FTPDATA/system_patch_22_23.bin")
 * - ${username}: user name (e.g., simcom)
 * - ${password}: password (e.g., simcom)
 */
bool Sam_Fota_Init(struct Sam_Fota_t* self, const char * cfgstr) {
    if (self == NULL || cfgstr == NULL) {
        SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_ERROR, "Invalid parameters for Sam_Fota_Init\n");
        return false;
    }
    sam_dbg_set_module_level(SAM_MOD_FOTA, SAM_DBG_LEVEL_TRACE);

// char cfgstr[] = "\vCFGSCT_M1\t0\tA\t0\t0\thttp://117.131.85.142:60058/FTPDATA/system_patch_22_23.bin\tsimcom\tsimcom\v"
    // Parse the configuration string
    sscanf(cfgstr, "\vCFGSCT_M1\t%u\tA\t%u\t%u\t%s\t%s\t%s\v", 
        (uint32_t *)&self->config.atChannelId, 
        (uint32_t *)&self->config.channel, 
        (uint32_t *)&self->config.mode,
        self->config.serverUrl,
        self->config.username,
        self->config.password
        );

    // Log the configuration information
    SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_INFO, "\vCFGSCT_M1\t%u\tA\t%u\t%u\t%s\t%s\t%s\v\n", 
        self->config.atChannelId, 
        self->config.channel, 
        self->config.mode,
        self->config.serverUrl,
        self->config.username,
        self->config.password);
    
    
    // Initialize base structure
    self->base.state = FOTA_STATE_IDLE;
    self->base.step = 0;
    self->base.dcnt = 0;
    self->base.msclk = SamGetMsCnt(0);
    self->base.sclk = 0;
    
    // Set default values
    self->error = FOTA_ERROR_NONE;
    self->progress = 0;
    
    self->phatc = pAtcBusArray[self->config.atChannelId];
	    
    self->runlink =	SamAtcFunLink(self->phatc, self, (SamMdmFunTag)Sam_Fota_Process, (SamUrcBcFunTag)handleAtUrc);
    SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_TRACE, "Fota module initialized. runlink = %d\n", self->runlink);
    return true;
}

/**
 * @brief Deinitialize the FOTA module.
 * @param self Pointer to the FOTA module instance.
 * @return true if deinitialization is successful, false otherwise.
 */
bool Sam_Fota_Deinit(struct Sam_Fota_t* self) {
    if (self == NULL) {
        return false;
    }
    
    // Abort any ongoing operation
//    Sam_Fota_Abort(self);
    
    // Cleanup resources
    if (self->phatc != NULL) {
        SamAtcFunUnlink(self->phatc, self->runlink);
        self->phatc = NULL;
    }
    
    SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_INFO, "FOTA module deinitialized\n");
    return true;
}

#if 0
/**
 * @brief Initialize the FOTA module.
 * @param self Pointer to the FOTA module instance.
 * @param config FOTA configuration.
 * @return true if initialization is successful, false otherwise.
 */
bool Sam_Fota_Init(struct Sam_Fota_t* self, const Sam_Fota_Config_t* config) {
    if (self == NULL || config == NULL) {
        SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_ERROR, "Invalid parameters for Sam_Fota_Init\n");
        return false;
    }
    
    // Update configuration
    memcpy(&self->config, config, sizeof(Sam_Fota_Config_t));
    
    // Initialize AT command handler
    self->phatc = pAtcBusArray[0]; // Use default AT channel
    if (self->phatc == NULL) {
        SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_ERROR, "Failed to get AT command handler\n");
        return false;
    }
    
    // Register URC handler
    self->base.runlink = SamAtcFunLink(self->phatc, self, NULL, handleAtUrc);
    if (self->base.runlink == 0) {
        SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_ERROR, "Failed to register URC handler\n");
        return false;
    }
    
    SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_INFO, "FOTA module initialized\n");
    return true;
}

/**
 * @brief Deinitialize the FOTA module.
 * @param self Pointer to the FOTA module instance.
 * @return true if deinitialization is successful, false otherwise.
 */
bool Sam_Fota_Deinit(struct Sam_Fota_t* self) {
    if (self == NULL) {
        return false;
    }
    
    // Abort any ongoing operation
    Sam_Fota_Abort(self);
    
    // Cleanup resources
    if (self->phatc != NULL) {
        SamAtcFunUnlink(self->phatc, self->base.runlink);
        self->phatc = NULL;
    }
    
    SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_INFO, "FOTA module deinitialized\n");
    return true;
}

/**
 * @brief Check for firmware updates.
 * @param self Pointer to the FOTA module instance.
 * @return true if the check operation is started successfully, false otherwise.
 */
bool Sam_Fota_CheckUpdate(struct Sam_Fota_t* self) {
    if (self == NULL) {
        return false;
    }
    
    // Check if already in progress
    if (self->state != FOTA_STATE_IDLE && self->state != FOTA_STATE_FAILED) {
        SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_WARN, "FOTA operation already in progress\n");
        return false;
    }
    
    char atCmd[256];
    snprintf(atCmd, sizeof(atCmd), AT_FOTA_CHECK, self->config.serverUrl);
    
    // Send AT command to check for updates
    int result = Sam_Mdm_Atc_sendAtCmd(self->phatc, atCmd, FOTA_CHECK_TIMEOUT);
    if (result != 0) {
        SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_ERROR, "Failed to send AT command: %s\n", atCmd);
        updateState(self, FOTA_STATE_FAILED, FOTA_ERROR_NETWORK);
        return false;
    }
    
    // Update state
    updateState(self, FOTA_STATE_CHECKING, FOTA_ERROR_NONE);
    SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_INFO, "Checking for firmware updates...\n");
    return true;
}

/**
 * @brief Download the firmware.
 * @param self Pointer to the FOTA module instance.
 * @return true if the download operation is started successfully, false otherwise.
 */
bool Sam_Fota_DownloadFirmware(struct Sam_Fota_t* self) {
    if (self == NULL) {
        return false;
    }
    
    // Check if in appropriate state
    if (self->state != FOTA_STATE_IDLE && self->state != FOTA_STATE_CHECKING && self->state != FOTA_STATE_FAILED) {
        SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_WARN, "Cannot start download from current state: %d\n", self->state);
        return false;
    }
    
    char atCmd[256];
    snprintf(atCmd, sizeof(atCmd), AT_FOTA_DOWNLOAD, self->config.serverUrl);
    
    // Send AT command to start download
    int result = Sam_Mdm_Atc_sendAtCmd(self->phatc, atCmd, FOTA_DOWNLOAD_TIMEOUT);
    if (result != 0) {
        SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_ERROR, "Failed to send AT command: %s\n", atCmd);
        updateState(self, FOTA_STATE_FAILED, FOTA_ERROR_NETWORK);
        return false;
    }
    
    // Update state
    updateState(self, FOTA_STATE_DOWNLOADING, FOTA_ERROR_NONE);
    SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_INFO, "Downloading firmware...\n");
    return true;
}

/**
 * @brief Verify the downloaded firmware.
 * @param self Pointer to the FOTA module instance.
 * @return true if the verification is successful, false otherwise.
 */
bool Sam_Fota_VerifyFirmware(struct Sam_Fota_t* self) {
    if (self == NULL) {
        return false;
    }
    
    // Check if in appropriate state
    if (self->state != FOTA_STATE_DOWNLOADING && self->state != FOTA_STATE_READY) {
        SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_WARN, "Cannot verify firmware from current state: %d\n", self->state);
        return false;
    }
    
    // Send AT command to verify firmware
    int result = Sam_Mdm_Atc_sendAtCmd(self->phatc, AT_FOTA_VERIFY, FOTA_VERIFY_TIMEOUT);
    if (result != 0) {
        SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_ERROR, "Failed to send AT command: %s\n", AT_FOTA_VERIFY);
        updateState(self, FOTA_STATE_FAILED, FOTA_ERROR_NETWORK);
        return false;
    }
    
    // Update state
    updateState(self, FOTA_STATE_VERIFYING, FOTA_ERROR_NONE);
    SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_INFO, "Verifying firmware integrity...\n");
    return true;
}

/**
 * @brief Install the verified firmware.
 * @param self Pointer to the FOTA module instance.
 * @return true if the installation is started successfully, false otherwise.
 */
bool Sam_Fota_InstallFirmware(struct Sam_Fota_t* self) {
    if (self == NULL) {
        return false;
    }
    
    // Check if in appropriate state
    if (self->state != FOTA_STATE_READY) {
        SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_WARN, "Cannot install firmware from current state: %d\n", self->state);
        return false;
    }
    
    // Send AT command to install firmware
    int result = Sam_Mdm_Atc_sendAtCmd(self->phatc, AT_FOTA_INSTALL, FOTA_INSTALL_TIMEOUT);
    if (result != 0) {
        SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_ERROR, "Failed to send AT command: %s\n", AT_FOTA_INSTALL);
        updateState(self, FOTA_STATE_FAILED, FOTA_ERROR_NETWORK);
        return false;
    }
    
    // Update state
    updateState(self, FOTA_STATE_INSTALLING, FOTA_ERROR_NONE);
    SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_INFO, "Installing firmware...\n");
    return true;
}

/**
 * @brief Abort the current FOTA operation.
 * @param self Pointer to the FOTA module instance.
 * @return true if the operation is aborted successfully, false otherwise.
 */
bool Sam_Fota_Abort(struct Sam_Fota_t* self) {
    if (self == NULL) {
        return false;
    }
    
    // Check if there's an operation to abort
    if (self->state == FOTA_STATE_IDLE || self->state == FOTA_STATE_SUCCESS || self->state == FOTA_STATE_FAILED) {
        SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_INFO, "No ongoing FOTA operation to abort\n");
        return true;
    }
    
    // Send AT command to abort
    int result = Sam_Mdm_Atc_sendAtCmd(self->phatc, AT_FOTA_ABORT, 5000);
    if (result != 0) {
        SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_ERROR, "Failed to send AT command: %s\n", AT_FOTA_ABORT);
        // Continue with state transition even if command fails
    }
    
    // Update state
    updateState(self, FOTA_STATE_IDLE, FOTA_ERROR_ABORTED);
    SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_INFO, "FOTA operation aborted\n");
    return true;
}
#endif /* 0 */


/**
 * @brief FOTA module process main function.
 * @param self Pointer to the FOTA module instance.
 * @return The current process state.
 */
uint8_t Sam_Fota_Process(struct Sam_Fota_t* self) {
    if (self == NULL) {
        return RETCHAR_FREE;
    }
    
    Sam_Mdm_Atc_t *phatc = self->phatc;
    if (phatc == NULL)
    {
        SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_ERROR, "patc == NULL\n");
        return RETCHAR_FREE;
    }

    uint8_t result = RETCHAR_FREE;
    uint32_t clk = 0;
    clk = SamGetMsCnt(self->base.msclk);
    while(clk >= 1000)
    {
    	self->base.msclk +=1000;
    	self->base.sclk += 1;
    	clk -= 1000;
    }

    // 根据不同状态处理
    switch (self->base.state) {
        case FOTA_STATE_IDLE:
            {
                if (self->base.step == 0)
                { 
                    while(Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) Sam_Mdm_Atc_clearAtRevBuff(phatc);

                    char atCmd[1024];
                    snprintf(atCmd, sizeof(atCmd), AT_FOTA_DOWNLOAD, self->config.channel, self->config.mode, self->config.serverUrl, self->config.username, self->config.password);
                    Sam_Mdm_Atc_sendAtCmd(phatc, atCmd, CRLF_HATCTYP, 10);
                    self->base.step++;
                    self->base.sclk = 0;
                }
                else if (self->base.step == 1)
                {
                    uint8_t ratcret = 0;
                    ratcret = Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n");
                    if (ratcret == NOSTRRET_ATCRET)
                    {
                        // continue wait
                        return RETCHAR_KEEP;
                    }
                    else if (ratcret == OVERTIME_ATCRET)
                    {                    
                        self->base.dcnt++;
                        if(self->base.dcnt < 3)
                        {
                            self->base.step--;
                            self->base.sclk = 0;
                        }
                        else
                        {
                            updateState(self, FOTA_STATE_FAILED, FOTA_ERROR_TIMEOUT);
                            return RETCHAR_KEEP;
                        }
                    }
                    else if (ratcret == 1)
                    {
                        updateState(self, FOTA_STATE_DOWNLOADING, FOTA_ERROR_NONE);
                    }
                    else if (ratcret == 2)
                    {
                        updateState(self, FOTA_STATE_FAILED, FOTA_ERROR_ATERROR);
                    }
                    Sam_Mdm_Atc_clearAtRevBuff(phatc);
                }
                return RETCHAR_KEEP;
            }
            break;
            
        case FOTA_STATE_DOWNLOADING:    
        case FOTA_STATE_INSTALLING:   
        case FOTA_STATE_SUCCESS:
        case FOTA_STATE_FAILED:    
        default:
            break;
    }
    
    return result;
}


/**
 * @brief Get the current state of the FOTA operation.
 * @param self Pointer to the FOTA module instance.
 * @return The current FOTA state.
 */
Sam_Fota_State_t Sam_Fota_GetState(struct Sam_Fota_t* self) {
    if (self == NULL) {
        return FOTA_STATE_FAILED;
    }
    
    return self->base.state;
}

/**
 * @brief Get the current progress of the FOTA operation.
 * @param self Pointer to the FOTA module instance.
 * @return The current progress (0-100).
 */
uint8_t Sam_Fota_GetProgress(struct Sam_Fota_t* self) {
    if (self == NULL) {
        return 0;
    }
    
    return self->progress;
}

/**
 * @brief Get the current error code of the FOTA operation.
 * @param self Pointer to the FOTA module instance.
 * @return The current error code.
 */
Sam_Fota_Error_t Sam_Fota_GetError(struct Sam_Fota_t* self) {
    if (self == NULL) {
        return FOTA_ERROR_NONE;
    }
    
    return self->error;
}

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
                          void* context) {
    if (self == NULL) {
        return false;
    }
    
    self->progressCallback = progressCb;
    self->statusCallback = statusCb;
    self->context = context;
    
    return true;
}

/**
 * @brief Handle the unsolicited result code (URC) from the AT command.
 * @param context Pointer to the context, usually the FOTA module instance.
 * @param urcBuff Pointer to the URC buffer.
 * @return The result code indicating the handling status.
 */
static uint8_t handleAtUrc(void* context, char* urcBuff) {
    if (context == NULL || urcBuff == NULL) {
        return RETCHAR_NONE;
    }
    
    Sam_Fota_t* self = (Sam_Fota_t*)context;
    uint8_t temp = 0;
    
    temp = StrsCmp(urcBuff, "+CFOTA: FOTA,START\r\t+CFOTA: FOTA,START \t+CFOTA: FOTA,ERROR\r\t+CFOTA: FOTA,END\r\t+CFOTA: DOWNLOADING:\t+CFOTA: UPDATE:\t+CFOTA: UPDATE SUCCESS.\t+CFOTA: UPDATE FAIL\t+CFOTA: ");
//    SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_ERROR, "handleAtUrc: %s, temp=%d\n", urcBuff, temp);

    if (temp == 1) // received +CFOTA: FOTA,START\r
    {
        self->adiff = FOTA_ADIFF_FULL;
        updateState(self, FOTA_STATE_DOWNLOADING, FOTA_ERROR_NONE);
    }
    else if (temp == 2) // received +CFOTA: FOTA,START 
    {
        self->adiff = FOTA_ADIFF_MINI;
        updateState(self, FOTA_STATE_DOWNLOADING, FOTA_ERROR_NONE);
    }
    else if (temp == 3) // received +CFOTA: FOTA,ERROR\r
    {
        updateState(self, FOTA_STATE_FAILED, FOTA_ERROR_DOWNLOAD);
    }
    else if (temp == 4)  // received +CFOTA: FOTA,END\r
    {
        updateState(self, FOTA_STATE_SUCCESS, FOTA_ERROR_NONE);
    }
    else if (temp == 5)  // +CFOTA: DOWNLOADING:
    {
        uint32_t progress = 0;
        if (sscanf(urcBuff, "+CFOTA: DOWNLOADING:%u", &progress) == 1) {
            updateProgress(self, (uint8_t)progress);
        }
    }
    else if (temp == 6)  // +CFOTA: UPDATE:
    {
        uint32_t progress = 0;
        updateState(self, FOTA_STATE_INSTALLING, FOTA_ERROR_NONE);
        if (sscanf(urcBuff, "+CFOTA: UPDATE:%u", &progress) == 1) {
            updateProgress(self, (uint8_t)progress);
        }
    }
    else if (temp == 7)  // received +CFOTA: UPDATE SUCCESS.\r
    {
        if (self->adiff == FOTA_ADIFF_FULL)
            updateState(self, FOTA_STATE_SUCCESS, FOTA_ERROR_NONE);
    }
    else if (temp == 8)  // +CFOTA: UPDATE FAIL.
    {
        updateState(self, FOTA_STATE_FAILED, FOTA_ERROR_NONE);
    }
    else if (temp == 9)  // +CFOTA: 1001
    {
        uint32_t err = 0;
        if (sscanf(urcBuff, "+CFOTA: %u", &err) == 1) {
            if (err != 100)
                updateState(self, FOTA_STATE_FAILED, err);
        }
    }

    return temp;
}

/**
 * @brief Update the FOTA state and notify the callback.
 * @param self Pointer to the FOTA module instance.
 * @param state New FOTA state.
 * @param error Error code, if any.
 */
static void updateState(Sam_Fota_t* self, Sam_Fota_State_t state, Sam_Fota_Error_t error) {
    if (self == NULL) {
        return;
    }

    if (self->base.state == state) {
        return;
    }

    self->base.state = state;
    self->error = error;

    self->base.step = 0;
    self->base.sclk = 0;
    self->base.dcnt= 0;
    
    SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_INFO, "FOTA state updated: %d, error: %d\n", state, error);
    
    // Notify status callback if registered
    if (self->statusCallback != NULL) {
        self->statusCallback(state, error, self->context);
    }
}

/**
 * @brief Update the FOTA progress and notify the callback.
 * @param self Pointer to the FOTA module instance.
 * @param progress New progress value (0-100).
 */
static void updateProgress(Sam_Fota_t* self, uint8_t progress) {
    if (self == NULL) {
        return;
    }
    
    self->progress = progress;
    
    SAM_DBG_MODULE(SAM_MOD_FOTA, SAM_DBG_LEVEL_INFO, "FOTA progress updated: %d%%\n", progress);
    
    // Notify progress callback if registered
    if (self->progressCallback != NULL) {
        self->progressCallback(progress, self->context);
    }
}
