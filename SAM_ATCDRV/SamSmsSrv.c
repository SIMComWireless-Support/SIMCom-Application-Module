/**
 * @file SamSmsSrv.c
 * @brief Implementation of SMS demo service operations
 * @details This file provides the implementation for the SMS demo service, demonstrating
 *          the usage of the SMS client interfaces declared in SamSms.h and SamSmsSrv.h.
 *          It includes demo initialization, a receive callback for handling incoming
 *          messages, and periodic logic to send test messages at predefined intervals.
 * 
 * @version 1.0.0
 * @date 2025-08-01
 * @author <dong.chen@sunseaaiot.com>
 * @copyright Copyright (c) 2025, Your Company Inc. All rights reserved.
 * 
 * @note This is a demo implementation with hardcoded test messages and intervals.
 *       The service sends both ASCII (English) and UCS2 (Chinese) messages for testing.
 */

#include "include.h"
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

/** Default SMS configuration string: "\vCFGSMS_C<index>\t<at_channel>\t<mem_type>\t\"<service_center>\"\v" */
char Sms1605CfgStr1[] = "\vCFGSMS_C1\t0\t1\t\"+8613800210500\"\v";
/** Global pointer to SMS client instance */
void *pSmsObj = NULL;


/**
 * @brief Initialize SMS demo service
 * 
 * Allocates memory for the SMS client structure, initializes it with the default configuration,
 * and handles initialization failures.
 */

void sms_demo_init(void)
{
	TSmsTag *psms = NULL;
	printf("sms_demo sms_demo_init enter\n");
    // Allocate SMS client structure
	psms = (TSmsTag *) malloc(sizeof(TSmsTag));
	if(psms != NULL)
	{
		pSmsObj = sam_sms_init(psms, Sms1605CfgStr1);//Mqtt1605CfgStr1
		if(pSmsObj == NULL)
		{
			printf("sms_demo_init pSmsObj Fail\n");
			free(psms);	
			psms = NULL;	
		}
	}
	else
	{
		printf("malloc Fail:%s,%d\n", __FILE__, __LINE__);
	}
	

}

/**
 * @brief Callback for received SMS messages in demo
 * 
 * Validates input parameters and prints received message details (sender number and content).
 * @param psmsobj Pointer to SMS client instance
 * @param pnum Sender's phone number
 * @param pCtx SMS content string
 */

void sms_demo_receive_cb(TSmsTag *psmsobj, char *pnum, char *pCtx)
{
	if(NULL == psmsobj || NULL == pnum || NULL == pCtx)
	{
		printf("sms_demo_receive_cb  psmsobj is NULL!! \r\n");
		return;
	}

    printf("sms_demo_receive_cb enter: \r\n");
    printf("sms_demo_receive_cb pnum: %s\r\n", pnum);
    printf("sms_demo_receive_cb pCtx: %s\r\n", pCtx);
}

/**
 * @brief Periodic processing for SMS demo
 * 
 * - Registers the receive callback once (on first run)
 * - Maintains a 10-second timer to send test messages
 * - Sends up to 5 test messages (English and Chinese)
 */

void sms_demo_run()
{
	static uint8 cb_set_flag = 0;


	static uint32 send_cnt = 0;

	// Variables for tracking elapsed time (1-second intervals)
    //static uint32 msclk = SamGetMsCnt(0);
	static uint32 msclk = 0;
	static bool msclk_ini = false;
	static uint32 stim = 0;
	uint32 clk = 0;

	// Initialize timestamp on first run
	if(!msclk_ini)
	{
		msclk = SamGetMsCnt(0);
		msclk_ini = true;
	}

	// Calculate elapsed time in seconds
	clk = SamGetMsCnt(msclk);
	while(clk >= 1000)
	{
		msclk +=1000;
		stim += 1;
		clk -= 1000;
	}
	
	
	TSmsTag *psms = (TSmsTag *)pSmsObj;

    // Register receive callback once
	if(NULL != psms && 0 == cb_set_flag)
	{
		 sam_sms_set_receive_callback(psms, sms_demo_receive_cb);
		 cb_set_flag = 1;
	}

    // Send test messages every 10 seconds (up to 5 messages)
	if(stim >= 10 && send_cnt < 5 )
	{
	    if(NULL != psms)
    	{
			char send_msg[30] = {0};
            // Send English message (ASCII encoding)
		    sprintf(send_msg, "sms_send_msg%u", send_cnt);
			sam_sms_send_message(pSmsObj, send_msg, "13621615716", ENCODING_ASCII, LANG_EN);

            // Send Chinese message (UCS2 encoding: "你好" in UCS2)
			memset(send_msg, 0, sizeof(send_msg));
		    sprintf(send_msg, "4F60597D");
			printf("sms_demo send_msg: %s\r\n", send_msg);
			printf("sms_demo send_msg len: %u\r\n", (unsigned int)strlen(send_msg));
			sam_sms_send_message(pSmsObj, send_msg, "00310033003600320031003600310035003700310036", ENCODING_UCS2, LANG_CN);

            send_cnt++;  // Increment message counter

    	}

        stim = 0;  // Reset timer
	}
	
}
