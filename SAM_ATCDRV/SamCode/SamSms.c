//---------------------------------------------------------------------------
/**
 * @file SamSms.c
 * @brief Implementation of SMS client core functionality
 * @details This file provides the implementation details for the SMS client interfaces
 *          declared in SamSms.h. It includes core logic for SMS context initialization,
 *          resource management, state machine processing, message sending/receiving,
 *          and AT command interactions for SMS configuration and operations.
 * 
 * @version 1.0.0
 * @date 2025-08-01
 * @author <dong.chen@sunseaaiot.com>
 * @copyright Copyright (c) 2025, Your Company Inc. All rights reserved.
 * 
 * @note This implementation supports both ASCII and UCS2 encoding for SMS content.
 *       Error handling includes retry mechanisms for failed AT commands, with limits
 *       on retry attempts to prevent infinite loops.
 */

#include "include.h"
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

//#include <string.h>
#include <stdlib.h>
//#include <stdint.h>

/** Forward declaration of URC callback */
unsigned char sam_sms_urc_cb(void *pvsms, char *urcstr);

/**
 * @brief Initialize SMS context with configuration parameters
 * 
 * Copies SMS configuration (service center, storage type) into the context structure,
 * initializes message queues, and sets initial read state.
 * @param p_ctx Pointer to sms_context_t structure to initialize
 * @param cfg_sms_info Pointer to configuration parameters
 * @return 1 on success, 0 if input pointers are NULL
 */

uint8 sam_sms_context_init(sms_context_t *p_ctx, sms_cfg_t * cfg_sms_info)
{
	//uint8 client_id_len = 0;
	//uint16 server_addr_len = 0;

    if(NULL == p_ctx || NULL == cfg_sms_info)
		return 0;
    p_ctx->readIndex = -1;  // No pending read initially
   memcpy(&(p_ctx->sms_cfg), cfg_sms_info, sizeof(sms_cfg_t));

   memset(&p_ctx->send_msg_list, 0, sizeof(send_msg_list_t));
   return 1;

}

/**
 * @brief Release resources used by SMS context
 * 
 * Frees all memory allocated for the send message queue (nodes and content strings),
 * and resets queue pointers and length.
 * @param p_ctx Pointer to sms_context_t structure to clean up
 */
void sam_sms_context_release(sms_context_t *p_ctx) {

    if (NULL == p_ctx) {
        return;
    }
    // Free all nodes in send queue
    send_msg_node_t* current = p_ctx->send_msg_list.send_sms_head;
    while (current != NULL) {
        send_msg_node_t* next = current->next;
        // Free message content
        
        if (current->send_sms_data.content != NULL) {
            free(current->send_sms_data.content);
            current->send_sms_data.content = NULL;
        }
        
        free(current);
        current = next;
    }
    // Reset queue state
    p_ctx->send_msg_list.send_sms_head = NULL;
    p_ctx->send_msg_list.length = 0;
}

/**
 * @brief Initialize SMS client instance
 * 
 * Parses configuration string to extract service center number, AT channel, and storage type;
 * initializes SMS context and state machine; links to AT command processing.
 * @param psms Pointer to uninitialized TSmsTag structure
 * @param cfgstr Configuration string in format: "\vCFGSMS_C%u\t%u\t%u\t\"%[^\"]\"\v"
 *               (config index, AT channel, storage type, service center)
 * @return Pointer to initialized SMS client, or NULL on failure
 */

void * sam_sms_init(TSmsTag * psms, char * cfgstr)
{
	char sms_center[15] = {0};
	uint32 at_channel = 0;
	uint32 memtype = 0;
	uint32 cfg_index = 0;

	if(cfgstr == NULL || psms == NULL ) 
		return(NULL);

  	sam_dbg_set_module_level(SAM_MOD_SMS, SAM_DBG_LEVEL_WARN);


    memset(psms, 0, sizeof(TSmsTag));
    //WriteCfgTab(cfgstr, SCM9205CFG_HEADSTR,SCM9205CFG_SCTOPN, "2,\"TCP\",\"117.131.85.140\",60037");

    // Parse configuration string
   int scanres = sscanf(cfgstr, "\vCFGSMS_C%u\t%u\t%u\t\"%[^\"]\"\v", 
    	&cfg_index,	&at_channel, &memtype, sms_center );
	SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_DEBUG, "sam_sms_init:scanres == %d, cfg_index == %d\r\n", scanres, cfg_index);

    SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_DEBUG, "sam_sms_init:at_channel == %u\r\n", at_channel);	
    SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_DEBUG, "sam_sms_init:memtype == %u\r\n", memtype);
    SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_DEBUG, "sam_sms_init:sms_center == %s\r\n", sms_center);

    // Prepare configuration
    sms_cfg_t cfg_info = {0};
	uint8 len = (strlen(sms_center) < sizeof(cfg_info.sms_center)) ? (strlen(sms_center)) : (sizeof(cfg_info.sms_center) - 1);
	memcpy(cfg_info.sms_center, sms_center,len);
	cfg_info.is_new = true;
	cfg_info.mem_type = memtype;
	

    // Initialize context
    if (0 == sam_sms_context_init(&psms->sms_context, &cfg_info))
    {
        SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_ERROR, ">>>sms ctx init fail!\r\n");
        return NULL;
    }

    // Set up AT channel and state machine
	psms->phatc = pAtcBusArray[at_channel];
  
	
	psms->sta = SMS_STATUS_INIT;
	psms->step = SMS_INIT_STEP_CSCA;
	psms->dcnt = 0;
	psms->stim = 0;
	


	psms->fail_type = SMS_FAIL_TYPE_NONE;
	psms->init_fail_cont = 0;
	psms->send_fail_cont = 0;
	
	psms->msclk = SamGetMsCnt(0);
	psms->runlink =	SamAtcFunLink(psms->phatc, psms, sam_sms_proc, sam_sms_urc_cb);
	psms->receive_data_cb = NULL;

  //	DebugTrace("@%s: Runlink=%u, DataPoint=%p\r\n", __FUNCTION__, pscm->runlink, pscm);

	return(psms);
}
                      
/**
 * @brief Stop SMS client and release resources
 * 
 * Unlinks the SMS client from the AT command processing chain and frees context resources.
 * @param psms Pointer to SMS client instance
 * @return '1' (as char) on success, 0 if input is NULL
 */

unsigned char sam_sms_stop(void * psms)
{
	//uint8 i;	
	TSmsTag * tpSms = NULL;
	if(psms == NULL) return(0);
	tpSms = (TSmsTag *)psms;
	SamAtcFunUnlink(tpSms->phatc, tpSms->runlink);
    sam_sms_context_release(&tpSms->sms_context);
	return('1');
}
/**
 * @brief Request to read an SMS message by index
 * 
 * Sets the read index in the SMS context if no read operation is currently pending.
 * @param psms Pointer to SMS client instance
 * @param index Index of the SMS message to read
 * @return true if read request is queued, false if busy
 */

bool sam_sms_read_req(TSmsTag * psms, int16 index)
{
	if(psms->sms_context.readIndex == -1)
	{
		psms->sms_context.readIndex = index;
	}
	else
	{   
		SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_DEBUG, "sam_sms_read_req read sms busy, please try later\r\n");
		return false;
	}
	return true;
}


/**
 * @brief URC callback for handling incoming SMS notifications
 * 
 * Processes "+CMTI" URCs (new message indicator) to trigger message reading.
 * @param pvscm Pointer to SMS client instance (TSmsTag)
 * @param urcstr URC string received from modem
 * @return RETCHAR_TRUE if URC is processed, RETCHAR_NONE otherwise
 */
                 
unsigned char sam_sms_urc_cb(void * pvscm, char * urcstr)
{
    uint8 temp = 0;
    TSmsTag * psms = (TSmsTag *)pvscm;
    if(NULL == psms || NULL == urcstr)
        return RETCHAR_NONE;
    HdsAtcTag * phatc = psms->phatc;
    if(NULL == phatc)
        return RETCHAR_NONE;
    
    // Check for "+CMTI" URC (new message notification)
    temp = StrsCmp(urcstr, "+CMTI:");
	//SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_DEBUG, "sam_sms_urc_cb:### temp == %u \r\n", temp);
	//SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_DEBUG, "sam_sms_urc_cb:### urcstr == %s \r\n", urcstr);
	if(temp == 1)
	{
		SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_DEBUG, "sam_sms_urc_cb:urc position temp == %u \r\n", temp);
	}
    switch(temp)
    {
        case 1:
			{
				uint32 index = 0;
                char memtype[10] = {0};
				int res = sscanf(urcstr,"+CMTI: \"%9[^\"]\",%u", memtype, &index);
            	SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_DEBUG, "sam_sms_urc_cb:case1 sscanf res == %d\r\n", res);
            	SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_DEBUG, "sam_sms_urc_cb:case1 index == %u, memtype == %s\r\n", index, memtype);
        sam_sms_read_req(psms, index);  // Queue read request
			}
            break;

        default:
            return(RETCHAR_NONE);
    }

	return(RETCHAR_TRUE);
}


/**
 * @brief Get the head node of the send message queue
 * 
 * Returns the first node in the send queue without modifying the queue.
 * @param plist Pointer to send message queue
 * @return Pointer to head node, or NULL if queue is empty/invalid
 */

send_msg_node_t *sam_get_current_send_sms_node(send_msg_list_t *plist) {
    if (plist == NULL) {
        return NULL;
    }    
    return plist->send_sms_head;
}


/**
 * @brief Remove the head node from the send message queue
 * 
 * Frees the content and node of the queue's head node, updates queue pointers.
 * @param plist Pointer to send message queue
 * @param pNode Pointer to head node to remove (must be current head)
 */

void sam_del_current_send_sms_note(send_msg_list_t *plist, send_msg_node_t *pNode) {
    if (plist == NULL || pNode == NULL || plist->send_sms_head == NULL) {
        return;
    }

    if (pNode != plist->send_sms_head) {
		SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_ERROR, "Logical error: The node to be deleted is not the head node!\n");
        return; 
    }

    send_msg_node_t *nextNode = pNode->next;
    // Free message content
    if (pNode->send_sms_data.content != NULL) {
        free(pNode->send_sms_data.content);
        pNode->send_sms_data.content = NULL;
    }

    free(pNode);
    pNode = NULL;

    plist->send_sms_head = nextNode;
    if (plist->length > 0) {
        plist->length--;
    }
}


/**
 * @brief Add a new message to the send queue
 * 
 * Creates a new queue node with the message content, recipient, encoding, and language;
 * appends the node to the end of the queue.
 * @param plist Pointer to send message queue
 * @param p_content SMS content string
 * @param p_num Recipient phone number
 * @param encoding Content encoding type
 * @param language Content language type
 * @return Pointer to new node, or NULL on failure
 */


send_msg_node_t *sam_insert_new_send_data_into_list(send_msg_list_t *plist, char *p_content, char *p_num, EncodingType encoding, LangType language) {
    if (plist == NULL || p_content == NULL || p_num == NULL) {
        return NULL;
    }

    // Allocate node
    send_msg_node_t *newNode = (send_msg_node_t *)malloc(sizeof(send_msg_node_t));
    if (newNode == NULL) {
        return NULL; // ÄÚ´æ·ÖÅäÊ§°Ü
    }
    memset(newNode, 0, sizeof(send_msg_node_t));

    // Set node properties
    newNode->send_sms_data.encoding = encoding;
    
    newNode->send_sms_data.language = (language == LANG_NONE) ? LANG_DEFAULT : language;

    // Copy content (add 0x1A as SMS termination)
    size_t content_len = 1 + strlen(p_content);
    newNode->send_sms_data.content = (char *)malloc(content_len + 1);
    if (newNode->send_sms_data.content == NULL) {
        free(newNode);
        return NULL;
    }


	
	memset(newNode->send_sms_data.content, 0, content_len + 1);
    strcpy(newNode->send_sms_data.content, p_content);
    newNode->send_sms_data.content[content_len - 1] = 0x1A;  // SMS end marker
    newNode->send_sms_data.length = content_len;

    // Copy recipient number
    size_t num_len = strlen(p_num);
    if (num_len < sizeof(newNode->send_sms_data.num)) {
        strcpy(newNode->send_sms_data.num, p_num);
    } else {
        memcpy(newNode->send_sms_data.num, p_num, sizeof(newNode->send_sms_data.num) - 1);
        newNode->send_sms_data.num[sizeof(newNode->send_sms_data.num) - 1] = '\0';
    }


    // Append to queue
    newNode->next = NULL;

    if (plist->send_sms_head == NULL) {
        plist->send_sms_head = newNode;
    } else {
        send_msg_node_t *tail = plist->send_sms_head;
        while (tail->next != NULL) {
            tail = tail->next;
        }
        tail->next = newNode;
    }

    plist->length++;

    return newNode;
}

/**
 * @brief Main SMS client processing function
 * 
 * Implements the state machine for SMS operations:
 * - Initialization (configure service center, storage, format, notifications)
 * - Data processing (send queued messages, read pending messages)
 * - Error handling and state transitions
 * @param pvsms Pointer to SMS client instance (TSmsTag)
 * @return RETCHAR_KEEP to continue processing, RETCHAR_FREE to release resources
 */

unsigned char sam_sms_proc(void * pvsms)
{
	//uint8 i, j, ratcret;
	uint8 ratcret;
	//uint32 clk, n, m;
	uint32 clk;
	char buf[256] = {0};
	//char str[256] = {0};
	//char tempchar;

	TSmsTag * psms = NULL;
	HdsAtcTag * phatc = NULL;
    sms_context_t *pSmsCtxt = NULL;

	psms = pvsms;
	if(psms == NULL) return('E'+1);
	
	phatc = psms->phatc;
	if(phatc == NULL) return('E'+2);

    pSmsCtxt = &psms->sms_context;

    // Update second-level timer
	clk = SamGetMsCnt(psms->msclk);
	while(clk >= 1000)
	{
		psms->msclk +=1000;
		psms->stim += 1;
		clk -= 1000;
	}
	
    // State machine processing
	switch(psms->sta)
	{
		case SMS_STATUS_INIT :
            // Initialize SMS service center address (CSCA)
            if(psms->step == SMS_INIT_STEP_CSCA && psms->stim >= 1)
			{
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) phatc->retbufp = 0;

                memset(buf, 0, sizeof(buf));
                sprintf(buf, "AT+CSCA=\"%s\"\r", pSmsCtxt->sms_cfg.sms_center);

				SamSendAtCmd(phatc, buf, CRLF_HATCTYP, 90);
				psms->step = SMS_INIT_STEP_CSCA_RES_CHECK;
				psms->stim = 0;
                psms->dcnt = 0;
			}
            // Check CSCA configuration result
			else if(psms->step == SMS_INIT_STEP_CSCA_RES_CHECK && psms->stim >= 1)
			{
				ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n");
				if(ratcret == NOSTRRET_ATCRET)
				{
                	return(RETCHAR_KEEP);
				}
                else if(ratcret == OVERTIME_ATCRET)
				{
					psms->dcnt++;
					if(psms->dcnt < 3)
					{
						psms->step = SMS_INIT_STEP_CSCA;
                    	psms->stim = 0;
					}
					else
					{
						psms->sta = SMS_STATUS_FAIL;
						psms->fail_type = SMS_FAIL_TYPE_CSCA_FAIL;
						psms->stim = 0;
					}
				}
                else if(ratcret == 1)
                {
                    psms->step = SMS_INIT_STEP_CPMS;  // Proceed to storage config
                }
                else if(ratcret == 2)
                {
                    if( SCED_HATCSTA == phatc->state)
                    {
                        SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>SMS CENTER set fail!\r\n");
						psms->sta = SMS_STATUS_FAIL;
						psms->fail_type = SMS_FAIL_TYPE_CSCA_FAIL;
						psms->stim = 0;                    
					}
                    else
                    {
                        SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_ERROR, ">>>logic error!\r\n");
                    }
                }
				phatc->retbufp = 0;
                phatc->retbuf[0] = 0;
				
			}
            // Configure preferred message storage (CPMS)
            else if(psms->step == SMS_INIT_STEP_CPMS)
            {
                memset(buf, 0, sizeof(buf));
				if(SMS_MEM_TYPE_SM == pSmsCtxt->sms_cfg.mem_type)
				{
		            sprintf(buf, "AT+CPMS=\"SM\",\"SM\",\"SM\"\r");
				}
				else
				{
		            sprintf(buf, "AT+CPMS=\"ME\",\"ME\",\"ME\"\r");
				}
				SamSendAtCmd(phatc, buf, CRLF_HATCTYP, 90);
				psms->step = SMS_INIT_STEP_CPMS_RES_CHECK;
				psms->stim = 0;
                psms->dcnt = 0;
            }
            else if(psms->step == SMS_INIT_STEP_CPMS_RES_CHECK)
            {
				ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t+CPMS:\t+CMS ERROR:");
				if(ratcret == NOSTRRET_ATCRET)
				{
                	return(RETCHAR_KEEP);
				}
                else if(ratcret == OVERTIME_ATCRET)
				{
					psms->dcnt++;
					if(psms->dcnt < 3)
					{
						psms->step = SMS_INIT_STEP_CPMS;
                    	psms->stim = 0;
					}
					else
					{
						psms->sta = SMS_STATUS_FAIL;
						psms->fail_type = SMS_FAIL_TYPE_CPMS_FAIL;
						psms->stim = 0;
					}
				}
                else if(ratcret == 1)
                {
                    SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>  CPMS success!\r\n");
					psms->step = SMS_INIT_STEP_CMGF;
                }
                else if(ratcret == 2)
                {
                    SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>  CPMS fail!\r\n");
					psms->sta = SMS_STATUS_FAIL;
					psms->fail_type = SMS_FAIL_TYPE_CPMS_FAIL;
					psms->stim = 0;
                }
				else if(ratcret == 3)
				{
                    // do nothing
				}
				else if(ratcret == 4)
				{
					uint32 err = 0;
					sscanf(phatc->retbuf,"+CMS ERROR: %u", &err);
					SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>CPMS fail, +CMS ERROR: %u\r\n",err);
					psms->sta = SMS_STATUS_FAIL;
					psms->fail_type = SMS_FAIL_TYPE_CPMS_FAIL;
					psms->stim = 0;
				}
				phatc->retbufp = 0;
                phatc->retbuf[0] = 0;				
			}
			else if(psms->step == SMS_INIT_STEP_CMGF)
			{
	            memset(buf, 0, sizeof(buf));
	            sprintf(buf, "AT+CMGF=1\r");
				SamSendAtCmd(phatc, buf, CRLF_HATCTYP, 9);
				psms->step = SMS_INIT_STEP_CMGF_RES_CHECK;
				psms->stim = 0;
	            psms->dcnt = 0;
            }
			else if(psms->step == SMS_INIT_STEP_CMGF_RES_CHECK)
			{
				ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n");
				if(ratcret == NOSTRRET_ATCRET)
				{
                	return(RETCHAR_KEEP);
				}
                else if(ratcret == OVERTIME_ATCRET)
				{
					psms->dcnt++;
					if(psms->dcnt < 3)
					{
						psms->step = SMS_INIT_STEP_CMGF;
                    	psms->stim = 0;
					}
					else
					{
						psms->sta = SMS_STATUS_FAIL;
						psms->fail_type = SMS_FAIL_TYPE_CMGF_FAIL;
						psms->stim = 0;
					}
				}
                else if(ratcret == 1)
                {
                    psms->step = SMS_INIT_STEP_CNMI;
                }
                else if(ratcret == 2)
                {
                    if( SCED_HATCSTA == phatc->state)
                    {
                        SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>SMS CMGF set fail!\r\n");
						psms->sta = SMS_STATUS_FAIL;
						psms->fail_type = SMS_FAIL_TYPE_CMGF_FAIL;
						psms->stim = 0;                    
					}
                    else
                    {
                        SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_ERROR, ">>>logic error!\r\n");
                    }
                }
				phatc->retbufp = 0;
                phatc->retbuf[0] = 0;
				
			}
			else if(psms->step == SMS_INIT_STEP_CNMI)
			{
				memset(buf, 0, sizeof(buf));
                sprintf(buf, "AT+CNMI=2,1\r");
				SamSendAtCmd(phatc, buf, CRLF_HATCTYP, 9);
				psms->step = SMS_INIT_STEP_CNMI_RES_CHECK;
				psms->stim = 0;
                psms->dcnt = 0;           
			}
			else if(psms->step == SMS_INIT_STEP_CNMI_RES_CHECK)
			{
				ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t+CMS ERROR:");
				if(ratcret == NOSTRRET_ATCRET)
				{
                	return(RETCHAR_KEEP);
				}
                else if(ratcret == OVERTIME_ATCRET)
				{
					psms->dcnt++;
					if(psms->dcnt < 3)
					{
						psms->step = SMS_INIT_STEP_CNMI;
                    	psms->stim = 0;
					}
					else
					{
						psms->sta = SMS_STATUS_FAIL;
						psms->fail_type = SMS_FAIL_TYPE_CNMI_FAIL;
						psms->stim = 0;
					}
				}
                else if(ratcret == 1)
                {
                    SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>  CNMI success!\r\n");
					psms->sta = SMS_STATUS_DATA_PROCESS;
					psms->step = SMS_DATAPROC_STEP_CSCS;
					psms->dcnt=0;
					psms->stim = 0;
                }
                else if(ratcret == 2)
                {
                    SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>  CNMI fail!\r\n");
					psms->sta = SMS_STATUS_FAIL;
					psms->fail_type = SMS_FAIL_TYPE_CNMI_FAIL;
					psms->stim = 0;
                }
				else if(ratcret == 3)
				{
					uint32 err = 0;
					sscanf(phatc->retbuf,"+CMS ERROR: %u", &err);
					SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>CNMI fail, +CMS ERROR: %u\r\n",err);
					psms->sta = SMS_STATUS_FAIL;
					psms->fail_type = SMS_FAIL_TYPE_CNMI_FAIL;
					psms->stim = 0;
				}
				phatc->retbufp = 0;
                phatc->retbuf[0] = 0;				
			}
			break;
		case SMS_STATUS_DATA_PROCESS :
			if(psms->step == SMS_DATAPROC_STEP_CSCS && psms->stim > 1)
			{
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) phatc->retbufp = 0;
                
                if(pSmsCtxt->send_msg_list.send_sms_head != NULL && pSmsCtxt->send_msg_list.length > 0)
                {
                    send_msg_node_t *pCurSndMsg = sam_get_current_send_sms_node(&pSmsCtxt->send_msg_list);
					
					if(pCurSndMsg->send_sms_data.language == LANG_EN)
					{
						sprintf(buf, "AT+CSCS=\"IRA\"\r");
					}
					else
					{
						sprintf(buf, "AT+CSCS=\"UCS2\"\r");
					}
					
					SamSendAtCmd(phatc, buf, CRLF_HATCTYP, 9);
					psms->step = SMS_DATAPROC_STEP_CSCS_RES_CHECK;
                }
				else if(pSmsCtxt->readIndex != -1)
				{
					psms->step = SMS_DATAPROC_STEP_CMGR;
					psms->stim = 0;
				}
                else
                {
                    psms->step = SMS_ILDE_STEP_FIRST;
                    psms->stim = 0;
                    psms->sta = SMS_STATUS_IDLE;
                }
			}
			else if(psms->step == SMS_DATAPROC_STEP_CSCS_RES_CHECK)
			{
				ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n");
				if(ratcret == NOSTRRET_ATCRET)
				{
                	return(RETCHAR_KEEP);
				}
				else if(ratcret == OVERTIME_ATCRET||ratcret == 0x02)
				{
					psms->sta = SMS_STATUS_FAIL;
					psms->fail_type = SMS_FAIL_TYPE_CSCS_FAIL;
					psms->stim = 0;
				}
                else if(ratcret == 0x02) 
                {
                    SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_ERROR, ">>>CSCS error!\r\n");
					psms->sta = SMS_STATUS_FAIL;
					psms->fail_type = SMS_FAIL_TYPE_CSCS_FAIL;
					psms->stim = 0;
                }
				else if(ratcret == 0x01) 
				{
					psms->step = SMS_DATAPROC_STEP_CSMP;
                    SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>CSCS seccess!\r\n");
				}
				phatc->retbufp = 0;
			}
			else if(psms->step == SMS_DATAPROC_STEP_CSMP)
            {
                if(pSmsCtxt->send_msg_list.send_sms_head != NULL && pSmsCtxt->send_msg_list.length > 0)
                {
                    send_msg_node_t *pCurMsgNote = sam_get_current_send_sms_node(&pSmsCtxt->send_msg_list);
					if(pCurMsgNote->send_sms_data.language == LANG_EN)
					{
						sprintf(buf, "AT+CSMP=17,167,0,0\r");
					}
					else
					{
						sprintf(buf, "AT+CSMP=17,167,0,8\r");
					}
					SamSendAtCmd(phatc, buf, CRLF_HATCTYP, 9);
					psms->step = SMS_DATAPROC_STEP_CSMP_RES_CHECK;
                }
                else
                {
                    SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_ERROR, ">>>logic error!\r\n");
					psms->step = SMS_ILDE_STEP_FIRST;
                    psms->stim = 0;
                    psms->sta = SMS_STATUS_IDLE;
                }
            }
            else if(psms->step == SMS_DATAPROC_STEP_CSMP_RES_CHECK)
            {
				ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t+CMS ERROR:");
				if(ratcret == NOSTRRET_ATCRET)
				{
                	return(RETCHAR_KEEP);
				}
				else if(ratcret == OVERTIME_ATCRET||ratcret == 0x02)
				{
					psms->sta = SMS_STATUS_FAIL;
					psms->fail_type = SMS_FAIL_TYPE_CSMP_FAIL;
					psms->stim = 0;
				}
                else if(ratcret == 0x03) 
                {
                  
					uint32 err = 0;
					sscanf(phatc->retbuf,"+CMS ERROR: %u", &err);
					SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>CSMP fail, +CMS ERROR: %u\r\n",err);
					psms->sta = SMS_STATUS_FAIL;
					psms->fail_type = SMS_FAIL_TYPE_CSMP_FAIL;
					psms->stim = 0;
                }
				else if(ratcret == 0x01) 
				{
					psms->step = SMS_DATAPROC_STEP_CMGS;
                    SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>CSMP set seccess!\r\n");
				}
				phatc->retbufp = 0;
			}
			else if(psms->step == SMS_DATAPROC_STEP_CMGS)
			{
                if(pSmsCtxt->send_msg_list.send_sms_head != NULL && pSmsCtxt->send_msg_list.length > 0)
                {
                    send_msg_node_t *pCurMsgNode = sam_get_current_send_sms_node(&pSmsCtxt->send_msg_list);
					
					sprintf(buf, "AT+CMGS=\"%s\"\r", pCurMsgNode->send_sms_data.num);
					SamSendAtCmd(phatc, buf, (CRLF_HATCTYP|RISP_HATCTYP), 9);
					psms->step = SMS_DATAPROC_STEP_CMGS_RES_CHECK;
					phatc->databuf = pCurMsgNode->send_sms_data.content;
				    phatc->databufp = pCurMsgNode->send_sms_data.length;
					SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>CMGS send ctx==%s\r\n", phatc->databuf);
					SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>CMGS send length==%u\r\n", phatc->databufp);

#if 0
					if(pCurMsgNode->send_sms_data.language == LANG_EN)
					{
	                    sprintf(buf, "AT+CMGS=\"%s\"\r", pCurMsgNode->send_sms_data.num);
						SamSendAtCmd(phatc, buf, (CRLF_HATCTYP|RISP_HATCTYP), 9);
						psms->step = SMS_DATAPROC_STEP_CMGS_RES_CHECK;
						phatc->databuf = pCurMsgNode->send_sms_data.content;
					    phatc->databufp = pCurMsgNode->send_sms_data.length;
						SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>CMGS send ctx==%s\r\n", phatc->databuf);
						SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>CMGS send length==%u\r\n", phatc->databufp);
					}
                    else
                    {
                    	char tmpStr[128] = {0};
						conver_to_ucs2Str(pCurMsgNode->send_sms_data.num, pCurMsgNode->send_sms_data.encoding, tmpStr);
						SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>ucs2 tmp num==%s\r\n", tmpStr);

						sprintf(buf, "AT+CMGS=\"%s\"\r", tmpStr);
						SamSendAtCmd(phatc, buf, (CRLF_HATCTYP|RISP_HATCTYP), 9);
						psms->step = SMS_DATAPROC_STEP_CMGS_RES_CHECK;
						memset(psms->send_buf, 0, sizeof(psms->send_buf));
						conver_to_ucs2Str(pCurMsgNode->send_sms_data.content, pCurMsgNode->send_sms_data.encoding, psms->send_buf);
						SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>ucs2 tmp ctx==%s\r\n", psms->send_buf);

						phatc->databuf = psms->send_buf;
					    phatc->databufp = strlen(psms->send_buf);
                    }
#endif
                }
                else
                {
                    SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_ERROR, ">>>logic error!\r\n");
					psms->step = SMS_ILDE_STEP_FIRST;
                    psms->stim = 0;
                    psms->sta = SMS_STATUS_IDLE;
                }
            }
            else if(psms->step == SMS_DATAPROC_STEP_CMGS_RES_CHECK)
            {
				ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t+CMGS:\t+CMS ERROR:\t> ");
				if(ratcret == NOSTRRET_ATCRET)
				{
                	return(RETCHAR_KEEP);
				}
				else if(ratcret == OVERTIME_ATCRET||ratcret == 0x02)
				{
					psms->sta = SMS_STATUS_FAIL;   
					psms->fail_type = SMS_FAIL_TYPE_CMGS_FAIL;
					psms->stim = 0;
				}
				else if(ratcret == 0x04)
				{//+CMS ERROR: <err>
					uint32 err = 0;
					sscanf(phatc->retbuf,"+CMS ERROR: %u", &err);
					SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>CMGS ERR:%u \r\n",err);
					psms->sta = SMS_STATUS_FAIL;   
					psms->fail_type = SMS_FAIL_TYPE_CMGS_FAIL;
					psms->stim = 0;
				}
				else if(ratcret == 0x03)
				{//+CMGS: <mr>
					uint32 mr = 0;
					sscanf(phatc->retbuf,"+CMGS: %u", &mr);
					SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>CMGS mr:%u \r\n",mr);
				}
                else if(ratcret == 0x01)
                {
                	psms->send_fail_cont = 0;
                    SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>CMGS result:success!!\r\n");
                    send_msg_node_t *pCurMsgNode = sam_get_current_send_sms_node(&pSmsCtxt->send_msg_list);
                    sam_del_current_send_sms_note(&pSmsCtxt->send_msg_list, pCurMsgNode);
                    if(pSmsCtxt->send_msg_list.send_sms_head != NULL)
                    {
                        
						 psms->step = SMS_DATAPROC_STEP_CSCS;
                         psms->stim = 0;
                    }
					else if(pSmsCtxt->readIndex != -1)
					{
						psms->step = SMS_DATAPROC_STEP_CMGR;
						psms->stim = 0;
					}
                    else
                    {
						psms->step = SMS_ILDE_STEP_FIRST;
                        psms->stim = 0;
                        psms->sta = SMS_STATUS_IDLE;
                    }
                }
				phatc->retbufp = 0;
			}            
			else if(psms->step == SMS_DATAPROC_STEP_CMGR)
			{
                if(pSmsCtxt->readIndex != -1)
                {
                    sprintf(buf, "AT+CMGR=%u\r", pSmsCtxt->readIndex);
					SamSendAtCmd(phatc, buf, (CRLF_HATCTYP), 9);
					psms->step = SMS_DATAPROC_STEP_CMGR_RES_CHECK;
                }
                else
                {
					psms->step = SMS_ILDE_STEP_FIRST;
                    psms->stim = 0;
                    psms->sta = SMS_STATUS_IDLE;
                }
            }
			else if(psms->step == SMS_DATAPROC_STEP_CMGR_RES_CHECK)
			{
				pSmsCtxt->readIndex = -1;
				ratcret = SamChkAtcRet(phatc, "OK\r\n\t+CMGR:\t+CMS ERROR:");
				if(ratcret == NOSTRRET_ATCRET)
				{
                	return(RETCHAR_KEEP);
				}
				else if(ratcret == OVERTIME_ATCRET)
				{
					psms->sta = SMS_STATUS_FAIL;   
					psms->fail_type = SMS_FAIL_TYPE_CMGR_FAIL;
					psms->stim = 0;
					pSmsCtxt->readIndex = -1;
				}
				else if(ratcret == 0x03)
				{//+CMS ERROR: <err>
					uint32 err = 0;
					sscanf(phatc->retbuf,"+CMS ERROR: %u", &err);
					SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>CMGR ERR:%u \r\n",err);
					psms->sta = SMS_STATUS_FAIL;   
					psms->fail_type = SMS_FAIL_TYPE_CMGR_FAIL;
					psms->stim = 0;
					pSmsCtxt->readIndex = -1;
				}
				else if(ratcret == 0x02)
				{//
    				char num[128] = {0}; 
					memset(psms->read_num, 0, sizeof(psms->read_num));
				    if (sscanf(phatc->retbuf, "+CMGR: \"%*[^\"]\",\"%127[^\"]\"", num) == 1) {
						SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, "read num:%s \r\n",num);
						memcpy(psms->read_num, num, strlen(num));
				    } else {				       
						SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, "read num fail\r\n");
				    }
				}
				else if(ratcret == RETURNSR_ATCRET)
				{
				   memset(psms->read_ctx, 0, sizeof(psms->read_ctx));
				   int len = (strlen(phatc->retbuf) >=  (sizeof(psms->read_ctx) - 1))?(sizeof(psms->read_ctx) - 1): strlen(phatc->retbuf);
				   SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, "read ctx:%s \r\n",phatc->retbuf);
				   memcpy(psms->read_ctx, phatc->retbuf, len);
				}
                else if(ratcret == 0x01)
                {
                	//psms->send_fail_cont = 0;
                    SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>CMGR result:success!!\r\n");
					if(psms->receive_data_cb != NULL)
					{
						psms->receive_data_cb(psms, psms->read_num, psms->read_ctx);
					}
					memset(psms->read_num, 0, sizeof(psms->read_num));
					memset(psms->read_ctx, 0, sizeof(psms->read_ctx));
					psms->step = SMS_ILDE_STEP_FIRST;
                    psms->stim = 0;
                    psms->sta = SMS_STATUS_IDLE;
                }
				phatc->retbufp = 0;
			}
			

			break;
		case SMS_STATUS_IDLE:
            {
			while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) phatc->retbufp = 0;
            
            if( (pSmsCtxt->send_msg_list.send_sms_head != NULL && pSmsCtxt->send_msg_list.length > 0) || (pSmsCtxt->readIndex != -1)) 
			{
				psms->sta = SMS_STATUS_DATA_PROCESS;
				psms->step= SMS_DATAPROC_STEP_CSCS;
                psms->stim = 0;
				//pscm->dcnt=0;
			}
			else
			{
                psms->sta = SMS_STATUS_DATA_PROCESS;
				psms->step=SMS_DATAPROC_STEP_CSCS;
                psms->stim = 0;
				return(RETCHAR_FREE);
			}
          }
			break;
		case SMS_STATUS_FAIL:
            {
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) phatc->retbufp = 0;
				switch(psms->fail_type)
				{
					case SMS_FAIL_TYPE_CSCA_FAIL:
					case SMS_FAIL_TYPE_CPMS_FAIL:
					case SMS_FAIL_TYPE_CMGF_FAIL:	
					case SMS_FAIL_TYPE_CNMI_FAIL:	
					{
#if 0
						if(psms->stim >= 2)
						{
							psms->init_fail_cont++;
							psms->fail_type = SMS_FAIL_TYPE_NONE;
							if(psms->init_fail_cont >= 6)
							{
								psms->sta = SMS_STATUS_INIT;
								psms->step = SMS_INIT_STEP_CSCA;
			                	psms->stim = 0;

								psms->init_fail_cont = 0;
							    //pmqtt->connect_fail_cont =0;
								return(RETCHAR_FREE);
							}
							else
							{
								psms->sta = SMS_STATUS_INIT;
							    psms->step = SMS_INIT_STEP_CSCA;
			                    psms->stim = 0;							
							}
						}
#endif
					}
					break;

					case SMS_FAIL_TYPE_CSCS_FAIL:
					case SMS_FAIL_TYPE_CSMP_FAIL:
					case SMS_FAIL_TYPE_CMGS_FAIL:
					{
#if 0
						if(psms->stim >= 1)
						{
							psms->send_fail_cont++;
							
							psms->fail_type = SMS_FAIL_TYPE_NONE;
							if(psms->send_fail_cont >= 3)
							{
								psms->sta = SMS_STATUS_INIT;
								psms->step = SMS_INIT_STEP_CSCA;
			                	psms->stim = 0;
								psms->send_fail_cont = 0;
							    
								return(RETCHAR_FREE);
							}
							else
							{
								psms->sta = SMS_STATUS_DATA_PROCESS;
							    psms->step = SMS_DATAPROC_STEP_CSCS;
			                    psms->stim = 0;							
							}
						}
#endif
					}
					break;


					default:
						psms->sta = SMS_STATUS_IDLE; 
						break;
				}
				
                
            }
            break;

		default :
			psms->sta = SMS_STATUS_IDLE;
			break;
	}
	return(RETCHAR_KEEP);
}
/**
 * @brief Queue an SMS message for sending
 * 
 * Wrapper for sam_insert_new_send_data_into_list, with input validation.
 * @param psms Pointer to SMS client instance
 * @param p_content SMS content string
 * @param p_num Recipient phone number
 * @param encoding Content encoding type
 * @param language Content language type
 * @return 1 if message is queued, 0 on failure (invalid input or allocation error)
 */

uint8 sam_sms_send_message(TSmsTag * psms, char *p_content, char *p_num, EncodingType encoding, LangType language)
{
    uint8 res = 0;
    if(NULL == psms || NULL == p_content)
    {
        return 0;
    }

	if(language != LANG_EN  && encoding != ENCODING_UCS2)
	{
        SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>sam_sms_send_message encoding error\r\n");
		return 0;
	}
		
	
    SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>sam_sms_send_message, before add  list lenth == %u\r\n",psms->sms_context.send_msg_list.length);
    send_msg_node_t *pNode = sam_insert_new_send_data_into_list(&(psms->sms_context.send_msg_list), p_content, p_num,  encoding,  language);
    if(NULL != pNode)
    {
        SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>sam_sms_send_message success!!  list lenth == %u\r\n",psms->sms_context.send_msg_list.length);
        res = 1;
    }
    else
    {
        SAM_DBG_MODULE(SAM_MOD_SMS, SAM_DBG_LEVEL_INFO, ">>>sam_sms_send_message fail!!  list lenth == %u\r\n",psms->sms_context.send_msg_list.length);
        res = 0;
    }
    return res;
}


/**
 * @brief Set callback for received SMS messages
 * 
 * Registers a callback to be invoked when a new SMS is received and read.
 * @param psms Pointer to SMS client instance
 * @param cb Callback function (sam_sms_receive_data_cb)
 */

 void  sam_sms_set_receive_callback(TSmsTag * psms, sam_sms_receive_data_cb cb)
 {
    if(NULL == psms || NULL == cb)
        return;
    psms->receive_data_cb = cb;
 }
/**
 * @brief Close SMS client (placeholder)
 * 
 * Currently a stub function; future expansion for graceful shutdown.
 * @param pvsms Pointer to SMS client instance
 * @return 1 on success, 0 if input is NULL
 */

uint8  sam_sms_close(TSmsTag * pvsms)
{
	if(NULL == pvsms)
		return 0;
	return 1;
}
