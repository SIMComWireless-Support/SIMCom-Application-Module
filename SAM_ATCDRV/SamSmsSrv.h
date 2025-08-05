/**
 * @file SamSmsSrv.h
 * @brief Declaration of SMS demo service interfaces
 * @details This header file declares the functions for the SMS demo service, including
 *          initialization, message receive callback, and periodic processing. It serves
 *          as the interface between the demo application and the underlying SMS client
 *          module (SamSms.c/.h).
 * 
 * @version 1.0.0
 * @date 2025-08-01
 * @author <dong.chen@sunseaaiot.com>
 * @copyright Copyright (c) 2025, Your Company Inc. All rights reserved.
 * 
 * @note The demo service is designed for testing SMS client functionality and should
 *       be adapted for specific application requirements in production use.
 */

#ifndef __SAMSMSSRV_H
#define __SAMSMSSRV_H

//#include "SamInc.h"

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @brief Initialize SMS demo service
 * 
 * Allocates and initializes an SMS client instance with default configuration,
 * setting up the service center and basic parameters.
 */
void sms_demo_init(void);

/**
 * @brief Callback for received SMS messages in demo
 * 
 * Handles incoming SMS messages by printing sender number and content.
 * @param psmsobj Pointer to SMS client instance
 * @param pNum Sender's phone number
 * @param pCtx SMS content
 */
void sms_demo_receive_cb(TSmsTag *psmsobj, char *pNum, char *pCtx);

/**
 * @brief Periodic processing for SMS demo
 * 
 * Manages demo logic: registers receive callback, sends test messages at intervals,
 * and handles timing for periodic operations.
 */
void sms_demo_run();


#ifdef __cplusplus
}
#endif





#endif

