//---------------------------------------------------------------------------
/**
 * @file SamMqtt.c
 * @brief Implementation of MQTT client functionality
 * 
 * This file contains the implementation of the MQTT client interfaces declared in SamMqtt.h.
 * It provides the core functionality for MQTT communication, including initialization, connection management,
 * message publishing/subscribing, and state machine processing. The implementation follows the MQTT protocol
 * specification and provides a robust, thread-safe interface for interacting with MQTT brokers.
 *
 * @see SamMqtt.h for related definitions and function prototypes
 *
 * @author <dong.chen@sunseaaiot.com>
 * @date 2025-07-01
 * @version 1.0.0
 * @copyright Copyright (c) 2025, Your Company Inc. All rights reserved.
 */
#define __SAMMQTT_C

#include "include.h"
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

// init mqttinfo example
unsigned char sam_mqtt_urc_cb(void * pvscm, char * urcstr);

/**
 * @brief Initialize MQTT context structure
 *
 * This function copies MQTT client ID, server address, and subscription topic 
 * information from configuration parameters to the MQTT context structure,
 * allocating memory for these strings. If will message configuration is provided,
 * it initializes those fields as well.
 *
 * @param p_ctx Pointer to MQTT context structure that stores MQTT connection information
 * @param cfg_mqtt_info Pointer to MQTT initialization parameters structure containing configuration
 *
 * @return uint8 Returns initialization status:
 *               1 - Initialization successful
 *               0 - Initialization failed (null parameters, invalid string lengths, or memory allocation failure)
 *
 * @note This function allocates memory for strings. Caller should free memory 
 *       using corresponding cleanup function after use to avoid leaks
 * @warning If configuration pointers are null or string lengths are invalid, 
 *          the function frees allocated memory and returns failure
 */
uint8 sam_mqtt_context_init(mqtt_context_t *p_ctx, mqtt_init_para_for_cus_t * cfg_mqtt_info)
{
	uint8 client_id_len = 0;
	uint16 server_addr_len = 0;

    if(NULL == cfg_mqtt_info->p_client_id || NULL == cfg_mqtt_info->p_server_addr || NULL == cfg_mqtt_info->p_sub_topic)
		return 0;
	
	client_id_len = strlen(cfg_mqtt_info->p_client_id);
	if(0 == client_id_len)
		return 0;
    
	p_ctx->p_client_id = malloc(1 + client_id_len);
	if(NULL == p_ctx->p_client_id)
		return 0;
    memset(p_ctx->p_client_id, 0, 1 + client_id_len );
	memcpy(p_ctx->p_client_id, cfg_mqtt_info->p_client_id, client_id_len);

   server_addr_len = strlen(cfg_mqtt_info->p_server_addr);
   if(server_addr_len < 9 || server_addr_len > 256)
   {
        free(p_ctx->p_client_id);
        p_ctx->p_client_id = NULL;
   		return 0;
   }

   p_ctx->p_server_addr = malloc(1 + server_addr_len);
   if(NULL == p_ctx->p_server_addr)
   {
        free(p_ctx->p_client_id);
        p_ctx->p_client_id = NULL;
        return 0;
   }
   memset(p_ctx->p_server_addr, 0, 1 + server_addr_len );
   memcpy(p_ctx->p_server_addr, cfg_mqtt_info->p_server_addr, server_addr_len);

   p_ctx->sub_topic_req_lenth = strlen(cfg_mqtt_info->p_sub_topic);
   if(p_ctx->sub_topic_req_lenth < 1 || p_ctx->sub_topic_req_lenth > 1024)
   {
        free(p_ctx->p_client_id);
        p_ctx->p_client_id = NULL;

        free(p_ctx->p_server_addr);
        p_ctx->p_server_addr = NULL;
   		return 0;
   }

   p_ctx->p_sub_topic = malloc(1 + p_ctx->sub_topic_req_lenth);
   if(NULL == p_ctx->p_sub_topic)
   {
        free(p_ctx->p_client_id);
        p_ctx->p_client_id = NULL;

        free(p_ctx->p_server_addr);
        p_ctx->p_server_addr = NULL;
        
        return 0;
   }
   memset(p_ctx->p_sub_topic, 0, 1 + p_ctx->sub_topic_req_lenth );
   memcpy(p_ctx->p_sub_topic, cfg_mqtt_info->p_sub_topic, p_ctx->sub_topic_req_lenth);

   if(NULL != cfg_mqtt_info->p_will_topic && NULL != cfg_mqtt_info->p_will_msg) 
   {
   		p_ctx->willtopic_req_length = strlen(cfg_mqtt_info->p_will_topic);
		p_ctx->willmsg_req_length = strlen(cfg_mqtt_info->p_will_msg);
		if(p_ctx->willtopic_req_length < 1 || p_ctx->willtopic_req_length > 1024 || p_ctx->willmsg_req_length < 1 || p_ctx->willmsg_req_length > 1024)
		{
	   		p_ctx->willtopic_req_length = 0;
	  		p_ctx->p_willtopic = NULL;
	   		p_ctx->willmsg_req_length = 0;
			p_ctx->p_willmsg = NULL;
   		}
		else
		{
			p_ctx->p_willtopic = malloc(1 + p_ctx->willtopic_req_length);
			p_ctx->p_willmsg = malloc(1 + p_ctx->willmsg_req_length);
			if(NULL == p_ctx->p_willtopic || NULL == p_ctx->p_willmsg)
			{
			   	if(NULL != p_ctx->p_willtopic)
			   	{
					free(p_ctx->p_willtopic);
				    p_ctx->p_willtopic = NULL;
			   	}
			   	if(NULL != p_ctx->p_willmsg)
			   	{
					free(p_ctx->p_willmsg);
				    p_ctx->p_willmsg = NULL;
			   	}
				free(p_ctx->p_client_id);
		        p_ctx->p_client_id = NULL;

		        free(p_ctx->p_server_addr);
		        p_ctx->p_server_addr = NULL;

				free(p_ctx->p_sub_topic);
				p_ctx->p_sub_topic = NULL;
				
				return 0;				
			}
			memset(p_ctx->p_willtopic, 0, 1 + p_ctx->willtopic_req_length);
			memset(p_ctx->p_willmsg, 0, 1 + p_ctx->willmsg_req_length);
			memcpy(p_ctx->p_willtopic, cfg_mqtt_info->p_will_topic, p_ctx->willtopic_req_length);
			memcpy(p_ctx->p_willmsg, cfg_mqtt_info->p_will_msg, p_ctx->willmsg_req_length);
		}
   }
   else
   {
   		p_ctx->willtopic_req_length = 0;
  		p_ctx->p_willtopic = NULL;
   		p_ctx->willmsg_req_length = 0;
		p_ctx->p_willmsg = NULL;
   }

    //follow papa set default value
   p_ctx->server_type = 0;
   //p_ctx->willtopic_req_length = 0;
   //p_ctx->p_willtopic = NULL;
   //p_ctx->willmsg_req_length = 0;
   p_ctx->willmsg_qos = 1;
   //p_ctx->p_willmsg = NULL;
   p_ctx->keepalive_time = 60;
   p_ctx->clean_session = 0;
   p_ctx->p_username = NULL;
   p_ctx->p_password = NULL;
   p_ctx->sub_qos = 1;
   p_ctx->sub_dup = 0;

   memset(&p_ctx->pub_msg_list, 0, sizeof(pub_msg_list_t));
   return 1;

}

/**
 * @brief Release all dynamically allocated resources in the MQTT context
 *
 * This function safely frees all memory resources allocated during the 
 * initialization or operation of the MQTT context, including client ID, 
 * server address, topic strings, will messages, and the publish message list.
 *
 * @param p_ctx Pointer to the MQTT context structure to be cleaned up
 *
 * @note All dynamically allocated strings and the publish message list 
 *       are freed. After calling this function, the context is reset to
 *       its initial state and should not be used until reinitialized.
 *
 * @warning Passing a NULL pointer is safe and results in no operation.
 *          Double-free is prevented by checking each pointer before freeing.
 */
void sam_mqtt_context_release(mqtt_context_t *p_ctx) {
    if (NULL == p_ctx) {
        return;
    }
    
    
    if (p_ctx->p_client_id != NULL) {
        free(p_ctx->p_client_id);
        p_ctx->p_client_id = NULL;
    }
    
   
    if (p_ctx->p_willtopic != NULL) {
        free(p_ctx->p_willtopic);
        p_ctx->p_willtopic = NULL;
    }
    
    
    if (p_ctx->p_willmsg != NULL) {
        free(p_ctx->p_willmsg);
        p_ctx->p_willmsg = NULL;
    }
    
    
    if (p_ctx->p_server_addr != NULL) {
        free(p_ctx->p_server_addr);
        p_ctx->p_server_addr = NULL;
    }
    
    
    if (p_ctx->p_username != NULL) {
        free(p_ctx->p_username);
        p_ctx->p_username = NULL;
    }
    
    
    if (p_ctx->p_password != NULL) {
        free(p_ctx->p_password);
        p_ctx->p_password = NULL;
    }
    
    
    if (p_ctx->p_sub_topic != NULL) {
        free(p_ctx->p_sub_topic);
        p_ctx->p_sub_topic = NULL;
    }
    
    
    pub_msg_node_t* current = p_ctx->pub_msg_list.pub_head;
    while (current != NULL) {
        pub_msg_node_t* next = current->next;
        
        
        if (current->pub_data.p_pub_topic != NULL) {
            free(current->pub_data.p_pub_topic);
            current->pub_data.p_pub_topic = NULL;
        }
        if (current->pub_data.p_pub_msg != NULL) {
            free(current->pub_data.p_pub_msg);
            current->pub_data.p_pub_msg = NULL;
        }
        
        
        free(current);
        current = next;
    }
    
   
    p_ctx->pub_msg_list.pub_head = NULL;
    p_ctx->pub_msg_list.length = 0;
    
    
    p_ctx->server_type = 0;
    p_ctx->willtopic_req_length = 0;
    p_ctx->willmsg_req_length = 0;
    p_ctx->willmsg_qos = 1;
    p_ctx->keepalive_time = 60;
    p_ctx->clean_session = 0;
    p_ctx->sub_qos = 1;
    p_ctx->sub_dup = 0;
}

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
void * sam_mqtt_init(TMqttTag * pmqtt, char * cfgstr)
{
	char cli_id[MQTT_CLINTID_MAX_LEN + 1] = {0};
	char ser_addr[MQTT_SERVER_ADDR_MAX_LEN + 1] = {0};
	char init_sub_topic[MQTT_SUB_TOPIC_MAX_LEN + 1] = {0};
	char will_topic[MQTT_WILLTOPIC_MAX_LEN + 1] = {0};
	char will_msg[MQTT_WILLTMSG_MAX_LEN + 1] = {0};
	uint32 at_channel = 0;
	uint32 cli_index = 0;

	uint32 cfg_index = 0;

	if(cfgstr == NULL || pmqtt == NULL ) 
		return(NULL);

  	sam_dbg_set_module_level(SAM_MOD_MQTT, SAM_DBG_LEVEL_WARN);


    memset(pmqtt, 0, sizeof(TMqttTag));
    //WriteCfgTab(cfgstr, SCM9205CFG_HEADSTR,SCM9205CFG_SCTOPN, "2,\"TCP\",\"117.131.85.140\",60037");

   int scanres = sscanf(cfgstr, "\vCFGMQTT_C%u\t%u\t%u\t\"%[^\"]\"\t\"%[^\"]\"\t\"%[^\"]\"\t\"%[^\"]\"\t\"%[^\"]\"\v", 
    	&cfg_index,	&at_channel, &cli_index, cli_id, ser_addr, init_sub_topic, will_topic, will_msg );
	SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_init:scanres == %d, cfg_index == %d\r\n", scanres, cfg_index);

    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_init:at_channel == %u\r\n", at_channel);	
    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_init:cli_index == %u\r\n", cli_index);
    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_init:cli_id == %s\r\n", cli_id);
    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_init:ser_addr == %s\r\n", ser_addr);
    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_init:init_sub_topic == %s\r\n", init_sub_topic);
    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_init:will_topic == %s\r\n", will_topic);
    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_init:will_msg == %s\r\n", will_msg);
    mqtt_init_para_for_cus_t cfg_mqtt_info = {at_channel, cli_index, cli_id, ser_addr, init_sub_topic, will_topic, will_msg};
	


    if (0 == sam_mqtt_context_init(&pmqtt->mqtt_context, &cfg_mqtt_info))
    {
        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, ">>>mqtt ctx init fail!\r\n");
        return NULL;
    }

	pmqtt->phatc = pAtcBusArray[cfg_mqtt_info.at_channel];
    pmqtt->client_index = cfg_mqtt_info.client_index;

	
	pmqtt->sta = MQTT_STATUS_INIT;
	//pmqtt->step = 0;
	pmqtt->step = MQTT_INIT_STEP_CMQTTSTART;
	pmqtt->dcnt = 0;
	pmqtt->stim = 0;
	pmqtt->connect_status = MQTT_DISCONNECTED;
	pmqtt->close_req = 0;
	//pmqtt->stop_req = 0;

	pmqtt->fail_type = MQTT_FAIL_TYPE_NONE;
	pmqtt->init_fail_cont = 0;
	//pmqtt->connect_fail_cont = 0;
	pmqtt->sub_fail_cont = 0;
	pmqtt->pub_fail_cont = 0;
	pmqtt->close_fail_cont = 0;

	//pmqtt->dncnt = 0;
	pmqtt->urcbmk = 0;
	
	pmqtt->msclk = SamGetMsCnt(0);
	pmqtt->runlink =	SamAtcFunLink(pmqtt->phatc, pmqtt, sam_mqtt_proc, sam_mqtt_urc_cb);
	pmqtt->receive_data_cb = NULL;

  //	DebugTrace("@%s: Runlink=%u, DataPoint=%p\r\n", __FUNCTION__, pscm->runlink, pscm);

	return(pmqtt);
}
                      
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
unsigned char sam_mqtt_stop(void * pmqtt)
{
	//uint8 i;	
	TMqttTag * tpMqtt = NULL;
	if(pmqtt == NULL) return(0);
	tpMqtt = (TMqttTag *)pmqtt;
	//i =	SamAtcFunUnlink(tpMqtt->phatc, tpMqtt->runlink);
	SamAtcFunUnlink(tpMqtt->phatc, tpMqtt->runlink);
    sam_mqtt_context_release(&tpMqtt->mqtt_context);
	return('1');
}

/**
 * @brief Callback function for handling MQTT URC (Unsolicited Result Code) messages
 *
 * Processes incoming URC strings related to MQTT operations, including message reception,
 * connection status changes, and error notifications. Parses URC content to extract 
 * topic, payload, and status information, then triggers appropriate handling logic.
 *
 * @param pvscm Pointer to MQTT client structure (TMqttTag) associated with the URC
 * @param urcstr Null-terminated string containing the URC message to be processed
 *
 * @return unsigned char Returns RETCHAR_TRUE if URC is processed successfully;
 *                       RETCHAR_NONE if URC is unrecognized, invalid, or irrelevant to the client
 *
 * @note Handles various MQTT-related URC types including +CMQTTRXSTART, +CMQTTRXTOPIC,
 *       +CMQTTRXPAYLOAD, +CMQTTRXEND, +CMQTTCONNLOST, and +CMQTTNONET
 * @warning Both input parameters must be valid pointers; NULL inputs will return RETCHAR_NONE
 * @see sam_mqtt_proc for main MQTT processing logic
 */                      
unsigned char sam_mqtt_urc_cb(void * pvscm, char * urcstr)
{
    uint8 temp = 0;
    TMqttTag * pmqtt = (TMqttTag *)pvscm;
    if(NULL == pmqtt || NULL == urcstr)
        return RETCHAR_NONE;
    HdsAtcTag * phatc = pmqtt->phatc;
    if(NULL == phatc)
        return RETCHAR_NONE;
    
    temp = StrsCmp(urcstr, "+CMQTTRXSTART:\t+CMQTTRXTOPIC:\t+CMQTTRXPAYLOAD:\t+CMQTTRXEND:\t+CMQTTCONNLOST:\t+CMQTTNONET");
	if(temp >= 1 && temp <= 6)
	{
		SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_urc_cb:urc position temp == %u \r\n", temp);
		SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_urc_cb:urc ret buff == %s, len == %d \r\n", urcstr, strlen(urcstr));
		SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_urc_cb:pmqtt->client_index == %d \r\n", pmqtt->client_index);
	}
    switch(temp)
    {
        case 1:
			{
				uint32 index = 0;
                uint32 topic_len = 0;
				uint32 payload_len = 0;
				int res = sscanf(urcstr,"+CMQTTRXSTART: %u,%u,%u\r\n", &index, &topic_len, &payload_len);
            	SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_urc_cb:case1 sscanf res == %d\r\n", res);
            	SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_urc_cb:case1 index == %u, topic_len == %u, payload_len==%u\r\n", index, topic_len, payload_len);
				if(index != pmqtt->client_index)
				{
					return(RETCHAR_NONE);
				}
			}
            break;
        case 2:
            {//+CMQTTRXTOPIC: 0,9 
                //last_urcflag = 2;
   
                uint32 index = 0;
                uint32 topic_len = 0;
				//DebugTrace("sam_mqtt_urc_cb:phatc->retbuf == %s \r\n", phatc->retbuf);

				
				//DebugTrace("sam_mqtt_urc_cb:len of retbuf == %d \r\n", strlen(urcstr));
				int res = sscanf(urcstr,"+CMQTTRXTOPIC: %u,%u\r\n", &index, &topic_len);
                //DebugTrace("sam_mqtt_urc_cb:case2 phatc == %u, pmqtt->phatc = %u\r\n!!!", phatc,pmqtt->phatc);
                SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_urc_cb:case2 sscanf res == %d\r\n", res);
				//DebugTrace("sam_mqtt_urc_cb:case2 parsed == %d\r\n", parsed);
                SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_urc_cb:case2 index == %u, topic_len == %u\r\n", index,topic_len);

				if(index != pmqtt->client_index)
				{
					return(RETCHAR_NONE);
				}
				
				pmqtt->t_cnt = topic_len;
				if(pmqtt->t_cnt > MQTT_SUB_TOPIC_MAX_LEN)
					pmqtt->t_cnt = MQTT_SUB_TOPIC_MAX_LEN;

				uint16 remain_len = pmqtt->t_cnt + 2;
				uint16 read_len = 0;
				char dp_buf[MQTT_SUB_TOPIC_MAX_LEN + 3] = {0};
				int p = 0;

                memset(pmqtt->t_buf, 0, sizeof(pmqtt->t_buf) );
				//memcpy(pmqtt->t_buf, urcstr, pmqtt->t_cnt);
				
                while(remain_len > 0)
            	{
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_urc_cb:case2 read topic remain_len == %u\r\n", remain_len);
					 read_len = SamAtcDubRead(phatc, remain_len, dp_buf);
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_urc_cb:case2 read topic read_len == %u\r\n", read_len);
					 if(read_len > 0)
				 	 {
						 memcpy(&(pmqtt->t_buf[p]), dp_buf, read_len);
						 memset(dp_buf, 0, sizeof(dp_buf));

						 p = p + read_len;

						 remain_len = remain_len - read_len;
				 	}
            	}

				pmqtt->t_buf[pmqtt->t_cnt] = 0;
				pmqtt->t_buf[pmqtt->t_cnt + 1] = 0;
                SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_urc_cb:case2 pmqtt->t_buf == %s\r\n", pmqtt->t_buf);
            }
            break;
        case 3://+CMQTTRXPAYLOAD: 0,60
            {
				//last_urcflag = 3;

                uint32 index = 0;
                uint32 payload_len = 0;
				sscanf(phatc->retbuf,"+CMQTTRXPAYLOAD: %u,%u", &index, &payload_len);
                SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_urc_cb:case3 index == %u, payload_len == %u\r\n", index,payload_len);

				if(index != pmqtt->client_index)
				{
					return(RETCHAR_NONE);
				}
				
				pmqtt->m_cnt = payload_len;
				if(pmqtt->m_cnt > MQTT_SUB_PAYLOAD_MAX_LEN)
					pmqtt->m_cnt = MQTT_SUB_PAYLOAD_MAX_LEN;

				uint16 remain_len = pmqtt->m_cnt + 2;
				uint16 read_len = 0;
				char dp_buf[MQTT_SUB_PAYLOAD_MAX_LEN + 3] = {0};
				int p = 0;

                memset(pmqtt->m_buf, 0, sizeof(pmqtt->m_buf) );
				//memcpy(pmqtt->t_buf, urcstr, pmqtt->t_cnt);
				
                while(remain_len > 0)
            	{
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_urc_cb:case3 read payload remain_len == %u\r\n", remain_len);
					 read_len = SamAtcDubRead(phatc, remain_len, dp_buf);
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_urc_cb:case3 read payload read_len == %u\r\n", read_len);
					 if(read_len > 0)
				 	 {
						 memcpy(&(pmqtt->m_buf[p]), dp_buf, read_len);
						 memset(dp_buf, 0, sizeof(dp_buf));

						 p = p + read_len;

						 remain_len = remain_len - read_len;
				 	}
            	}

				pmqtt->m_buf[pmqtt->m_cnt] = 0;
				pmqtt->m_buf[pmqtt->m_cnt + 1] = 0;
                SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_urc_cb:case3 pmqtt->m_buf == %s\r\n", pmqtt->m_buf);
            }
            break;
        case 4://+CMQTTRXEND: 0
            {
				//last_urcflag = 4;

                uint32 index = 0;
                sscanf(phatc->retbuf,"+CMQTTRXEND: %u", &index);
				
                SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_DEBUG, "sam_mqtt_urc_cb:case4 index == %u\r\n", index);
				
				
				if(index != pmqtt->client_index)
				{
					return(RETCHAR_NONE);
				}

				
                if(pmqtt->t_cnt != strlen(pmqtt->t_buf) || pmqtt->m_cnt != strlen(pmqtt->m_buf))
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, "sam_mqtt_urc_cb:logic error!! \r\n");

                if(pmqtt->t_cnt > 0 && pmqtt->m_cnt > 0 && strlen(pmqtt->t_buf) > 0 && strlen(pmqtt->m_buf) > 0)
                {
                    if(NULL != pmqtt->receive_data_cb)
                    {
                        pmqtt->receive_data_cb(pmqtt, index, pmqtt->t_buf, pmqtt->m_buf );
                    }
                    memset(pmqtt->t_buf, 0, sizeof(pmqtt->t_buf));
                    memset(pmqtt->m_buf, 0, sizeof(pmqtt->m_buf));
                    pmqtt->t_cnt = 0;
                    pmqtt->m_cnt = 0;
                    
                }
                else
                {
                	memset(pmqtt->t_buf, 0, sizeof(pmqtt->t_buf));
                    memset(pmqtt->m_buf, 0, sizeof(pmqtt->m_buf));
                    pmqtt->t_cnt = 0;
                    pmqtt->m_cnt = 0;
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, "sam_mqtt_urc_cb:logic error!! \r\n");
                }
	
                //last_urcflag = 0;
            }
            break;
        case 5:
			{
				uint32 index = 0;
                uint32 cause = 0;
				
				int res = sscanf(urcstr,"+CMQTTCONNLOST: %u,%u", &index, &cause);
            	SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_WARN, "sam_mqtt_urc_cb:case5 sscanf res == %d\r\n", res);
            	SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_WARN, "sam_mqtt_urc_cb:case5 index == %u, cause == %u\r\n", index, cause);
				if(index != pmqtt->client_index)
				{
					return(RETCHAR_NONE);
				}
				pmqtt->connect_status = MQTT_DISCONNECTED;
			}
            break;
		case 6:
			{
				SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_WARN, "sam_mqtt_urc_cb:case6 +CMQTTNONET\r\n");
				pmqtt->connect_status = MQTT_DISCONNECTED;
				return(RETCHAR_NONE);
			}
			break;

        default:
            return(RETCHAR_NONE);
    }

	return(RETCHAR_TRUE);
}

/**
 * @brief Clear subscription-related information in the MQTT context
 *
 * Releases dynamically allocated memory for the subscription topic string
 * and resets the subscription topic length counter in the MQTT context structure.
 * Performs no operation if the input context pointer is NULL.
 *
 * @param pMqttCtxt Pointer to the MQTT context structure whose subscription info needs to be cleared
 *
 * @note Safely handles NULL input by checking the context pointer before operation
 * @warning Ensure the context pointer points to a valid MQTT context structure to avoid undefined behavior
 */
void clear_sub_info(mqtt_context_t *pMqttCtxt)
{
    if(NULL != pMqttCtxt)
    {
        if(NULL != pMqttCtxt->p_sub_topic)
        {
            free(pMqttCtxt->p_sub_topic);
            pMqttCtxt->p_sub_topic = NULL;
        }
        pMqttCtxt->sub_topic_req_lenth = 0;
    }
}

/**
 * @brief Add or update a subscription topic in the MQTT context
 *
 * Allocates memory for the specified subscription topic string, copies the topic content
 * into the MQTT context, and updates the topic length information. If a previous subscription
 * topic exists, its memory is freed before updating to the new topic. Performs no operation
 * if the input context pointer or topic string is NULL.
 *
 * @param pMqttCtxt Pointer to the MQTT context structure to store the subscription topic
 * @param pTopic Pointer to the null-terminated string representing the subscription topic
 *
 * @note Existing subscription topic memory is freed before assigning a new topic to prevent memory leaks
 * @warning The function assumes the input topic string is valid and null-terminated; invalid strings may cause unexpected behavior
 */
void add_sub_topic(mqtt_context_t *pMqttCtxt, char *pTopic)
{
    if(NULL == pMqttCtxt || NULL == pTopic)
        return;
    
    pMqttCtxt->sub_topic_req_lenth = strlen(pTopic);
    if(NULL != pMqttCtxt->p_sub_topic)
    {
        free(pMqttCtxt->p_sub_topic);
        pMqttCtxt->p_sub_topic = NULL;
    }
    pMqttCtxt->p_sub_topic = malloc(pMqttCtxt->sub_topic_req_lenth + 1);
    if(NULL != pMqttCtxt->p_sub_topic)
    {
        memset(pMqttCtxt->p_sub_topic, 0, pMqttCtxt->sub_topic_req_lenth + 1);
        memcpy(pMqttCtxt->p_sub_topic, pTopic, pMqttCtxt->sub_topic_req_lenth);
    }
    //pMqttCtxt->sub_qos  use the default value, will changed acordding customer request
    //pMqttCtxt->sub_dup  use the default value, will changed acordding customer request
    
}


/**
 * @brief Get the current head node of the publish message list
 *
 * Retrieves the head node of the specified publish message list without modifying the list structure.
 * Returns NULL if the input list pointer is NULL.
 *
 * @param plist Pointer to the publish message list structure (pub_msg_list_t)
 * @return pub_msg_node_t* Pointer to the head node (pub_head) of the list; NULL if plist is NULL
 */
pub_msg_node_t *sam_get_current_pub_node(pub_msg_list_t *plist) {
    if (plist == NULL) {
        return NULL;
    }    
    return plist->pub_head;
}

/**
 * @brief Delete the current head node from the publish message list
 *
 * Removes the specified head node from the publish message list, frees the dynamically allocated memory 
 * within the node (including publish topic and message), releases the node itself, and updates the list length.
 * Performs no operation if the input list pointer or node pointer is NULL. Validates that the node to be deleted 
 * is the current head node of the list; otherwise, it outputs an error message and exits.
 *
 * @param plist Pointer to the publish message list structure (pub_msg_list_t)
 * @param pNode Pointer to the head node (pub_msg_node_t) to be deleted from the list
 */
void sam_del_current_pub_note(pub_msg_list_t *plist, pub_msg_node_t *pNode) {
    if (plist == NULL || pNode == NULL) {
        return;
    }
    
    if (plist->pub_head != pNode) {
		SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, "Logical error: The node to be deleted is not the head node!\n");
        return;
    }
    
    plist->pub_head = pNode->next;
    
    
    if (pNode->pub_data.p_pub_topic != NULL) {
        free(pNode->pub_data.p_pub_topic);
    }
    if (pNode->pub_data.p_pub_msg != NULL) {
        free(pNode->pub_data.p_pub_msg);
    }
    
    free(pNode);
    
    if (plist->length > 0) {
        plist->length--;
    }
}

/**
 * @brief Insert a new publish message node into the message list
 *
 * Creates a new publish message node with the specified topic and message content,
 * initializes default values for QoS, timeout, and flags, then appends the node to the
 * end of the provided publish message list. Handles memory allocation for both the node
 * and its contained strings, ensuring proper cleanup on allocation failure. Returns a
 * pointer to the newly created node or NULL if any allocation fails or input parameters are invalid.
 *
 * @param plist Pointer to the publish message list structure to which the new node will be added
 * @param p_pub_topic Pointer to the null-terminated string representing the publish topic
 * @param p_pub_msg Pointer to the null-terminated string representing the publish message payload
 *
 * @return pub_msg_node_t* Pointer to the newly created and inserted node, or NULL on failure
 *
 * @note Default values assigned: QoS=1, Timeout=60 seconds, Retained=0, Dup=0
 * @warning Allocates memory dynamically; ensure proper cleanup using sam_del_current_pub_note()
 */
pub_msg_node_t *sam_insert_new_pub_data_into_list(pub_msg_list_t *plist, char *p_pub_topic, char *p_pub_msg) {
    if (plist == NULL || p_pub_topic == NULL || p_pub_msg == NULL) {
        return NULL;
    }
    
    pub_msg_node_t *new_node = (pub_msg_node_t *)malloc(sizeof(pub_msg_node_t));
    if (new_node == NULL) {
        return NULL;
    }
    
    new_node->pub_data.pub_topic_req_lenth = (p_pub_topic != NULL) ? strlen(p_pub_topic) : 0;
    new_node->pub_data.p_pub_topic = NULL;
    new_node->pub_data.pub_msg_req_lenth = (p_pub_msg != NULL) ? strlen(p_pub_msg) : 0;
    new_node->pub_data.p_pub_msg = NULL;
    new_node->pub_data.pub_qos = 1;           // QoS默认值1
    new_node->pub_data.pub_timeout = 60;       // 超时默认值60
    new_node->pub_data.ratained = 0;          // 保留标志默认值0
    new_node->pub_data.pub_dup = 0;           // 重复标志默认值0
    
    if (p_pub_topic != NULL) {
        new_node->pub_data.p_pub_topic = (char *)malloc(new_node->pub_data.pub_topic_req_lenth + 1);
        if (new_node->pub_data.p_pub_topic != NULL) {
            strncpy(new_node->pub_data.p_pub_topic, p_pub_topic, new_node->pub_data.pub_topic_req_lenth);
            new_node->pub_data.p_pub_topic[new_node->pub_data.pub_topic_req_lenth] = '\0';
        }
    }

    if(NULL == new_node->pub_data.p_pub_topic)
    {
        free(new_node);
        return NULL;
    }
    
    if (p_pub_msg != NULL) {
        new_node->pub_data.p_pub_msg = (char *)malloc(new_node->pub_data.pub_msg_req_lenth + 1);
        if (new_node->pub_data.p_pub_msg != NULL) {
            strncpy(new_node->pub_data.p_pub_msg, p_pub_msg, new_node->pub_data.pub_msg_req_lenth);
            new_node->pub_data.p_pub_msg[new_node->pub_data.pub_msg_req_lenth] = '\0';
        }
    }

    if(NULL == new_node->pub_data.p_pub_msg)
    {
        free(new_node->pub_data.p_pub_topic);
        free(new_node);
        return NULL;
    }
    
    new_node->next = NULL;
    
    if (plist->pub_head == NULL) {
        plist->pub_head = new_node;
    } else {
        pub_msg_node_t *current = plist->pub_head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
    
    plist->length++;
    
    return new_node;
}

#define WMDMRET_BIT 0x80

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
unsigned char sam_mqtt_proc(void * pvmqtt)
{
	//uint8 i, j, ratcret;
	uint8 ratcret;
	//uint32 clk, n, m;
	uint32 clk;
	char buf[256] = {0};
	//char str[256] = {0};
	//char tempchar;

	TMqttTag * pmqtt = NULL;
	HdsAtcTag * phatc = NULL;
    mqtt_context_t *pMqttCtxt = NULL;

	pmqtt = pvmqtt;
	if(pmqtt == NULL) return('E'+1);
	
	phatc = pmqtt->phatc;
	if(phatc == NULL) return('E'+2);

    pMqttCtxt = &pmqtt->mqtt_context;

	clk = SamGetMsCnt(pmqtt->msclk);
	while(clk >= 1000)
	{
		pmqtt->msclk +=1000;
		pmqtt->stim += 1;
		clk -= 1000;
	}
	
	switch(pmqtt->sta)
	{
		case MQTT_STATUS_INIT :
            if(pmqtt->step == MQTT_INIT_STEP_CMQTTSTART && pmqtt->stim >= 1)
			{
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) phatc->retbufp = 0;

                memset(buf, 0, sizeof(buf));
                sprintf(buf, "AT+CMQTTSTART\r");
				SamSendAtCmd(phatc, buf, CRLF_HATCTYP, 90);
				//pmqtt->step++;//MQTT_INIT_STEP_CMQTTSTART_RES_CHECK
				pmqtt->step = MQTT_INIT_STEP_CMQTTSTART_RES_CHECK;
				pmqtt->stim = 0;
                pmqtt->dcnt = 0;
			}
			else if(pmqtt->step == MQTT_INIT_STEP_CMQTTSTART_RES_CHECK && pmqtt->stim >= 1)
			{
				ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t+CMQTTSTART:");
				if(ratcret == NOSTRRET_ATCRET)
				{
                	return(RETCHAR_KEEP);
				}
                else if(ratcret == OVERTIME_ATCRET)
				{
					pmqtt->dcnt++;
					if(pmqtt->dcnt < 3)
					{
						//pmqtt->step--;
						pmqtt->step = MQTT_INIT_STEP_CMQTTSTART;
                    	pmqtt->stim = 0;
					}
					else
					{
						pmqtt->sta = MQTT_STATUS_FAIL;
						pmqtt->fail_type = MQTT_FAIL_TYPE_MQTTSTART_FAIL;
						pmqtt->stim = 0;
					}
				}
                else if(ratcret == 1)
                {
                    //do nothing
                }
                else if(ratcret == 2)
                {
                    if( SCED_HATCSTA == phatc->state)
                    {
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>mqtt already start!\r\n");
                       // pmqtt->step++;
                       pmqtt->step = MQTT_INIT_STEP_CMQTTACCQ;
                    }
                    else
                    {
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, ">>>logic error!\r\n");
                    }
                }
				else if(ratcret == 3)
				{
                    
                    uint32 err = 0;
                    
					sscanf(phatc->retbuf,"+CMQTTSTART: %u", &err);
					//i = Strsearch(phatc->retbuf, ",0");
					if(0 == err)
					{
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>mqtt start success!\r\n");
						//pmqtt->step++;
						pmqtt->step = MQTT_INIT_STEP_CMQTTACCQ;
					}
                    else
                    {
                        pmqtt->sta = MQTT_STATUS_FAIL;
						pmqtt->fail_type = MQTT_FAIL_TYPE_MQTTSTART_FAIL;
						pmqtt->stim = 0;
                    } 
                   // pmqtt->stim = 0;
				}
				phatc->retbufp = 0;
                phatc->retbuf[0] = 0;
				
			}
           // else if(pmqtt->step == 2)
            else if(pmqtt->step == MQTT_INIT_STEP_CMQTTACCQ)
            {
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "AT+CMQTTACCQ=%u,\"%s\"\r", pmqtt->client_index, pMqttCtxt->p_client_id);
				SamSendAtCmd(phatc, buf, CRLF_HATCTYP, 90);
				//pmqtt->step++;
				pmqtt->step = MQTT_INIT_STEP_CMQTTACCQ_RES_CHECK;
				pmqtt->stim = 0;
                pmqtt->dcnt = 0;
            }
            else if(pmqtt->step == MQTT_INIT_STEP_CMQTTACCQ_RES_CHECK)
            {
				ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t+CMQTTACCQ:");
				if(ratcret == NOSTRRET_ATCRET)
				{
                	return(RETCHAR_KEEP);
				}
                else if(ratcret == OVERTIME_ATCRET)
				{
					pmqtt->dcnt++;
					if(pmqtt->dcnt < 3)
					{
						//pmqtt->step--;
						pmqtt->step = MQTT_INIT_STEP_CMQTTACCQ;
                    	pmqtt->stim = 0;
					}
					else
					{
						pmqtt->sta = MQTT_STATUS_FAIL;
						pmqtt->fail_type = MQTT_FAIL_TYPE_MQTTACCQ_FAIL;
						pmqtt->stim = 0;
					}
				}
                else if(ratcret == 1)
                {
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>mqtt CMQTTACCQ success!\r\n");
					//pmqtt->step++;
					pmqtt->step = MQTT_INIT_STEP_CMQTTWILLTOPIC;
                }
                else if(ratcret == 2)
                {
                   // pmqtt->sta = MQTT_STATUS_FAIL;// 后续增加变量记录fail原因，这里有两种情况(有err code和没err code)
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>mqtt CMQTTACCQ repeat!\r\n");
					pmqtt->step = MQTT_INIT_STEP_CMQTTWILLTOPIC;
                }
				else if(ratcret == 3)
				{
                    
                    uint32 err = 0;
					uint32 index = 0;
                    
					sscanf(phatc->retbuf,"+CMQTTACCQ: %u,%u", &index, &err);
					//i = Strsearch(phatc->retbuf, ",0");
					if(0 == err)
					{
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, ">>>format doesn't match the atc doc!\r\n");
						//pmqtt->step++;
					}
                    else
                    {
                        //pmqtt->sta = MQTT_STATUS_FAIL;
                        // do nothing
                    } 
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>+CMQTTACCQ: index == %u, err == %u\r\n",index, err);
					pmqtt->sta = MQTT_STATUS_FAIL;
					pmqtt->fail_type = MQTT_FAIL_TYPE_MQTTACCQ_FAIL;
					pmqtt->stim = 0;
				}
				phatc->retbufp = 0;
                phatc->retbuf[0] = 0;				
			}
			else if(pmqtt->step == MQTT_INIT_STEP_CMQTTWILLTOPIC)
			{
				if(NULL != pMqttCtxt->p_willtopic && pMqttCtxt->willtopic_req_length >= 1 && pMqttCtxt->willtopic_req_length <= 1024)
				{
		            memset(buf, 0, sizeof(buf));
		            sprintf(buf, "AT+CMQTTWILLTOPIC=%u,%u\r", pmqtt->client_index, pMqttCtxt->willtopic_req_length);
					SamSendAtCmd(phatc, buf, (CRLF_HATCTYP|RIGR_HATCTYP), 9);
					//pmqtt->step++;
					pmqtt->step = MQTT_INIT_STEP_CMQTTWILLTOPIC_RES_CHECK;
					pmqtt->stim = 0;
		            pmqtt->dcnt = 0;
					phatc->databuf =  pMqttCtxt->p_willtopic;
					phatc->databufp = pMqttCtxt->willtopic_req_length;	
				}
				else
				{
					//pmqtt->step += 4;
					pmqtt->step = MQTT_INIT_STEP_CMQTTCONNECT;
					phatc->retbufp = 0;
                    phatc->retbuf[0] = 0;	
				}
            }
			else if(pmqtt->step == MQTT_INIT_STEP_CMQTTWILLTOPIC_RES_CHECK)
			{
				ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t+CMQTTWILLTOPIC:\t>");
				if(ratcret == NOSTRRET_ATCRET)
				{
                	return(RETCHAR_KEEP);
				}
                else if(ratcret == OVERTIME_ATCRET)
				{
					pmqtt->dcnt++;
					if(pmqtt->dcnt < 3)
					{
						//pmqtt->step--;
						pmqtt->step = MQTT_INIT_STEP_CMQTTWILLTOPIC;
                    	pmqtt->stim = 0;
					}
					else
					{
						pmqtt->sta = MQTT_STATUS_FAIL;
						pmqtt->fail_type = MQTT_FAIL_TYPE_MQTTWILLTOPIC_FAIL;
						pmqtt->stim = 0;
					}
				}
                else if(ratcret == 1)
                {
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>mqtt CMQTTWILLTOPIC success!\r\n");
					//pmqtt->step++;
					pmqtt->step = MQTT_INIT_STEP_CMQTTWILLMSG;
                }
                else if(ratcret == 2)
                {
                    pmqtt->sta = MQTT_STATUS_FAIL;// 后续增加变量记录fail原因，这里有两种情况(有err code和没err code)
                    pmqtt->fail_type = MQTT_FAIL_TYPE_MQTTWILLTOPIC_FAIL;
					pmqtt->stim = 0;
                }
				else if(ratcret == 3)
				{
                    
                    uint32 err = 0;
					uint32 index = 0;
                    
					sscanf(phatc->retbuf,"+CMQTTWILLTOPIC: %u,%u", &index, &err);
					//i = Strsearch(phatc->retbuf, ",0");
					if(0 == err)
					{
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, ">>>format doesn't match the atc doc!\r\n");
						//pmqtt->step++;
					}
                    else
                    {
                        //pmqtt->sta = MQTT_STATUS_FAIL;
                        // do nothing
                    } 
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>+CMQTTWILLTOPIC: index == %u, err == %u\r\n",index, err);
                   // pmqtt->stim = 0;
				}
				phatc->retbufp = 0;
                phatc->retbuf[0] = 0;				
			}
			else if(pmqtt->step == MQTT_INIT_STEP_CMQTTWILLMSG)
			{
			    if(NULL != pMqttCtxt->p_willmsg && pMqttCtxt->willmsg_req_length >= 1 && pMqttCtxt->willmsg_req_length <= 1024)
		    	{
	                memset(buf, 0, sizeof(buf));
	                sprintf(buf, "AT+CMQTTWILLMSG=%u,%u,%u\r", pmqtt->client_index, pMqttCtxt->willmsg_req_length,pMqttCtxt->willmsg_qos);
					SamSendAtCmd(phatc, buf, (CRLF_HATCTYP|RIGR_HATCTYP), 9);
					//pmqtt->step++;
					pmqtt->step = MQTT_INIT_STEP_CMQTTWILLMSG_RES_CHECK;
					pmqtt->stim = 0;
	                pmqtt->dcnt = 0;
					phatc->databuf =  pMqttCtxt->p_willmsg;
					phatc->databufp = pMqttCtxt->willmsg_req_length;
		    	}
				else
				{
					//pmqtt->step += 2;
					pmqtt->step = MQTT_INIT_STEP_CMQTTCONNECT;
					phatc->retbufp = 0;
                    phatc->retbuf[0] = 0;	
				}
            }
			else if(pmqtt->step == MQTT_INIT_STEP_CMQTTWILLMSG_RES_CHECK)
			{
				ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t+CMQTTWILLMSG:\t>");
				if(ratcret == NOSTRRET_ATCRET)
				{
                	return(RETCHAR_KEEP);
				}
                else if(ratcret == OVERTIME_ATCRET)
				{
					pmqtt->dcnt++;
					if(pmqtt->dcnt < 3)
					{
						//pmqtt->step--;
						pmqtt->step = MQTT_INIT_STEP_CMQTTWILLMSG;
                    	pmqtt->stim = 0;
					}
					else
					{
						pmqtt->sta = MQTT_STATUS_FAIL;
						pmqtt->fail_type = MQTT_FAIL_TYPE_MQTTWILLMSG_FAIL;
						pmqtt->stim = 0;
					}
				}
                else if(ratcret == 1)
                {
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>mqtt CMQTTWILLMSG success!\r\n");
					//pmqtt->step++;
					pmqtt->step = MQTT_INIT_STEP_CMQTTCONNECT;
                }
                else if(ratcret == 2)
                {
                    pmqtt->sta = MQTT_STATUS_FAIL;// 后续增加变量记录fail原因，这里有两种情况(有err code和没err code)
					pmqtt->fail_type = MQTT_FAIL_TYPE_MQTTWILLMSG_FAIL;
					pmqtt->stim = 0;
                }
				else if(ratcret == 3)
				{
                    
                    uint32 err = 0;
					uint32 index = 0;
                    
					sscanf(phatc->retbuf,"+CMQTTWILLMSG: %u,%u", &index, &err);
					//i = Strsearch(phatc->retbuf, ",0");
					if(0 == err)
					{
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, ">>>format doesn't match the atc doc!\r\n");
						//pmqtt->step++;
					}
                    else
                    {
                        //pmqtt->sta = MQTT_STATUS_FAIL;
                        // do nothing
                    } 
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>+CMQTTWILLMSG: index == %u, err == %u\r\n",index, err);
                   // pmqtt->stim = 0;
				}
				phatc->retbufp = 0;
                phatc->retbuf[0] = 0;				
			}
			else if(pmqtt->step == MQTT_INIT_STEP_CMQTTCONNECT)
			{
                memset(buf, 0, sizeof(buf));
                sprintf(buf, "AT+CMQTTCONNECT=%u,\"%s\",%u,%u\r", pmqtt->client_index, pMqttCtxt->p_server_addr, pMqttCtxt->keepalive_time, pMqttCtxt->clean_session);
				SamSendAtCmd(phatc, buf, CRLF_HATCTYP, 90);
				//pmqtt->step++;
				pmqtt->step = MQTT_INIT_STEP_CMQTTCONNECT_RES_CHECK;
				pmqtt->stim = 0;
                pmqtt->dcnt = 0;
            }
			else if(pmqtt->step == MQTT_INIT_STEP_CMQTTCONNECT_RES_CHECK && pmqtt->stim >= 1)
			{
				ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t+CMQTTCONNECT:");
				if(ratcret == NOSTRRET_ATCRET)
				{
                	return(RETCHAR_KEEP);
				}
                else if(ratcret == OVERTIME_ATCRET)
				{
					pmqtt->dcnt++;
					if(pmqtt->dcnt < 3)
					{
						//pmqtt->step--;
						pmqtt->step = MQTT_INIT_STEP_CMQTTCONNECT;
                    	pmqtt->stim = 0;
					}
					else
					{
						pmqtt->sta = MQTT_STATUS_FAIL;
						pmqtt->fail_type = MQTT_FAIL_TYPE_CONNECT_FAIL;
						pmqtt->stim = 0;
					}
				}
                else if(ratcret == 1)
                {
                    //do nothing
                }
                else if(ratcret == 2)
                {
                	//pmqtt->connect_status = MQTT_DISCONNECTED;	
                    pmqtt->sta = MQTT_STATUS_FAIL;
					pmqtt->fail_type = MQTT_FAIL_TYPE_CONNECT_FAIL;
					pmqtt->stim = 0;
        			if(phatc->state != SCED_HATCSTA)
					{
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, ">>>logic error!\r\n");
					}
                }
				else if(ratcret == 3)
				{
                    
                    uint32 err = 0;
					uint32 index = 0;
                    
					sscanf(phatc->retbuf,"+CMQTTCONNECT: %u,%u",&index, &err);
					//i = Strsearch(phatc->retbuf, ",0");
					if(0 == err)
					{
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>mqtt connect server success!\r\n");
						//pmqtt->step++;
						pmqtt->step = MQTT_INIT_STEP_URC_MASK_SIGN;
						pmqtt->connect_status = MQTT_CONNECTED;
						pmqtt->init_fail_cont = 0;
						//pmqtt->connect_fail_cont = 0;
					}
                    else
                    {
                        //pmqtt->connect_status = MQTT_DISCONNECTED;	
                        pmqtt->sta = MQTT_STATUS_FAIL;
						pmqtt->fail_type = MQTT_FAIL_TYPE_CONNECT_FAIL;
                    } 
                    pmqtt->stim = 0;
					if(phatc->state != SCED_HATCSTA)
					{
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, ">>>logic error!\r\n");
					}
				}
				phatc->retbufp = 0;
                phatc->retbuf[0] = 0;
				
			}
			else if(pmqtt->step == MQTT_INIT_STEP_URC_MASK_SIGN)
			{
				phatc->state = IDLE_HATCSTA;
				//sprintf(buf, "+CMQTTRXSTART:\t+CMQTTRXTOPIC:\t+CMQTTRXPAYLOAD:\t+CMQTTRXEND:\t+CMQTTCONNLOST:");
				//sprintf(buf, "+CMQTTCONNLOST:");
				//pmqtt->urcbmk = SamAtcUrcSign(phatc, buf);
				//g_urc_cb_test.pfunProc = sam_mqtt_urc_cb;
				//g_urc_cb_test.pfunData = pmqtt;
				pmqtt->sta = MQTT_STATUS_DATA_PROCESS;
				//pmqtt->step=0;
				pmqtt->step = MQTT_DATAPROC_STEP_CMQTTSUB;
				pmqtt->dcnt=0;
				pmqtt->stim = 0;
				//DebugTrace(">>>Sockect Setup OK!\r\n");
			}
			break;
		//case TDAT_SCM9205STA :
		case MQTT_STATUS_DATA_PROCESS :
			if(pmqtt->step == MQTT_DATAPROC_STEP_CMQTTSUB && pmqtt->stim > 1)
			{
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) phatc->retbufp = 0;
                
                if(NULL != pmqtt->mqtt_context.p_sub_topic && pmqtt->mqtt_context.sub_topic_req_lenth > 0)
                {
                    sprintf(buf, "AT+CMQTTSUB=%u,%u,%u\r", pmqtt->client_index, pmqtt->mqtt_context.sub_topic_req_lenth,pmqtt->mqtt_context.sub_qos);
					SamSendAtCmd(phatc, buf, (CRLF_HATCTYP|RIGR_HATCTYP), 9);
					//pmqtt->step += 1;
					pmqtt->step = MQTT_DATAPROC_STEP_CMQTTSUB_RES_CHECK;
					//pmqtt->stim = 0;  //not used
					phatc->databuf = pmqtt->mqtt_context.p_sub_topic;
					phatc->databufp = pmqtt->mqtt_context.sub_topic_req_lenth;
                }
                else if(pMqttCtxt->pub_msg_list.pub_head != NULL && pMqttCtxt->pub_msg_list.length > 0)
                {
                    //pmqtt->step = 2;
                    pmqtt->step = MQTT_DATAPROC_STEP_CMQTTTOPIC;
                }
                else
                {
                    //pmqtt->step = 0;
					pmqtt->step = MQTT_ILDE_STEP_FIRST;
                    pmqtt->sta = MQTT_STATUS_IDLE;
                    pmqtt->stim = 0;
                }
			}
			else if(pmqtt->step == MQTT_DATAPROC_STEP_CMQTTSUB_RES_CHECK)
			{
				ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t>\t+CMQTTSUB:");
				if(ratcret == NOSTRRET_ATCRET)
				{
                	return(RETCHAR_KEEP);
				}
				else if(ratcret == OVERTIME_ATCRET||ratcret == 0x02)
				{
					pmqtt->sta = MQTT_STATUS_FAIL;
					//pmqtt->connect_status = MQTT_DISCONNECTED;
					pmqtt->fail_type = MQTT_FAIL_TYPE_SUB_FAIL;
					pmqtt->stim = 0;
				}
				else if(ratcret == 0x01) 
				{
                    //do nothing
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>CMQTTSUB cmd recieved:%s\r\n",phatc->retbuf);
				}
                else if(ratcret == 0x04)
                {
                    uint32 cli_index = 0;
                    uint32 err = 0;
                    
					sscanf(phatc->retbuf,"+CMQTTSUB: %u,%u", &cli_index, &err);
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>CMQTTSUB result:%s \r\n",phatc->retbuf);
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>CMQTTSUB result:cli_index == %u, err ==  %u\r\n",cli_index, err);
                    if(0 == err)
                    {
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>CMQTTSUB result:success!!\r\n");
						pmqtt->sub_fail_cont = 0;
                        if(phatc->state == SCED_HATCSTA)
    	                {
                            // pmqtt->step += 1;
                             pmqtt->step = MQTT_DATAPROC_STEP_CMQTTTOPIC;
                             pmqtt->stim = 0;
                             clear_sub_info(pMqttCtxt);
                            SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>sub topic seccess!\r\n");
                                 
    	                }
                        else
                        {
                            SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, ">>>logic error!\r\n");
                            SamSendAtSeg(phatc);//not used
                        }
                        //pmqtt->stim = 0;
                    }
                    else
                    {
                        pmqtt->sta = MQTT_STATUS_FAIL; //数据发布失败，需要在MQTT_STATUS_FAIL 处理异常，发布列表不需要改变，这种情况是有error code的
                        pmqtt->fail_type = MQTT_FAIL_TYPE_SUB_FAIL;
					    pmqtt->stim = 0;
						#if 0
						if(11 == err)
						{
							pmqtt->connect_status = MQTT_DISCONNECTED;
						}
						#endif
                    }
                }
				phatc->retbufp = 0;
			}
            else if(pmqtt->step == MQTT_DATAPROC_STEP_CMQTTTOPIC)
            {
                if(pMqttCtxt->pub_msg_list.pub_head != NULL && pMqttCtxt->pub_msg_list.length > 0)
                {
                    pub_msg_node_t *pCurPub = sam_get_current_pub_node(&pMqttCtxt->pub_msg_list);
                    
                    sprintf(buf, "AT+CMQTTTOPIC=%u,%u\r", pmqtt->client_index, pCurPub->pub_data.pub_topic_req_lenth);
					SamSendAtCmd(phatc, buf, (CRLF_HATCTYP|RIGR_HATCTYP), 9);
					//pmqtt->step += 1;
					pmqtt->step = MQTT_DATAPROC_STEP_CMQTTTOPIC_RES_CHECK;
					//pmqtt->stim = 0;  //not used
					phatc->databuf =  pCurPub->pub_data.p_pub_topic;
					phatc->databufp = pCurPub->pub_data.pub_topic_req_lenth;
                }
                else
                {
                    pmqtt->step = MQTT_ILDE_STEP_FIRST;
                    pmqtt->stim = 0;
                    pmqtt->sta = MQTT_STATUS_IDLE;
                }
            }
            else if(pmqtt->step == MQTT_DATAPROC_STEP_CMQTTTOPIC_RES_CHECK)
            {
				ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t+CMQTTTOPIC:\t>");
				if(ratcret == NOSTRRET_ATCRET)
				{
                	return(RETCHAR_KEEP);
				}
				else if(ratcret == OVERTIME_ATCRET||ratcret == 0x02)
				{
					pmqtt->sta = MQTT_STATUS_FAIL;
					//pmqtt->connect_status = MQTT_DISCONNECTED;
					pmqtt->fail_type = MQTT_FAIL_TYPE_PUB_FAIL;
					pmqtt->stim = 0;
				}
                else if(ratcret == 0x03) 
                {
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, ">>>CMQTTTOPIC error:%s!\r\n", phatc->retbuf);
                }
				else if(ratcret == 0x01) 
				{
					if(phatc->state == SCED_HATCSTA)
	                {
	                    //pscm->step = 0;
	                   // pscm->upcnt = 0;
	                   // pscm->stim = 0;
	                   //pmqtt->step += 1;
						 pmqtt->step = MQTT_DATAPROC_STEP_CMQTTPAYLOAD;
                       //pmqtt->stim = 0;
                       
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>pub topic set seccess!\r\n");
	                   
	                }
                    else
                    {
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, ">>>logic error!\r\n");
                        SamSendAtSeg(phatc);//not used
                    }
                    //pmqtt->stim = 0;
				}
				phatc->retbufp = 0;
			}
            else if(pmqtt->step == MQTT_DATAPROC_STEP_CMQTTPAYLOAD)
            {
                if(pMqttCtxt->pub_msg_list.pub_head != NULL && pMqttCtxt->pub_msg_list.length > 0)
                {
                    pub_msg_node_t *pCurPub = sam_get_current_pub_node(&pMqttCtxt->pub_msg_list);
                    
                    sprintf(buf, "AT+CMQTTPAYLOAD=%u,%u\r", pmqtt->client_index, pCurPub->pub_data.pub_msg_req_lenth);
					SamSendAtCmd(phatc, buf, (CRLF_HATCTYP|RIGR_HATCTYP), 9);
					//pmqtt->step += 1;
					pmqtt->step = MQTT_DATAPROC_STEP_CMQTTPAYLOAD_RES_CHECK;
					//pmqtt->stim = 0;  //not used
					phatc->databuf = pCurPub->pub_data.p_pub_msg;
					phatc->databufp = pCurPub->pub_data.pub_msg_req_lenth;
                }
                else
                {
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, ">>>logic error!\r\n");
                    //pmqtt->step = 0;
					pmqtt->step = MQTT_ILDE_STEP_FIRST;
                    pmqtt->stim = 0;
                    pmqtt->sta = MQTT_STATUS_IDLE;
                }
            }
            else if(pmqtt->step == MQTT_DATAPROC_STEP_CMQTTPAYLOAD_RES_CHECK)
            {
				ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t+CMQTTPAYLOAD:\t>");
				if(ratcret == NOSTRRET_ATCRET)
				{
                	return(RETCHAR_KEEP);
				}
				else if(ratcret == OVERTIME_ATCRET||ratcret == 0x02)
				{
					pmqtt->sta = MQTT_STATUS_FAIL;
					//pmqtt->connect_status = MQTT_DISCONNECTED;
					pmqtt->fail_type = MQTT_FAIL_TYPE_PUB_FAIL;
					pmqtt->stim = 0;
				}
                else if(ratcret == 0x03) 
                {
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, ">>>CMQTTPAYLOAD error:%s!\r\n", phatc->retbuf);
                    //此处可以将当前指令类型(哪条指令)和error cause保存下来，收到ratcret == 0x02 后一起进行异常处理
                }
				else if(ratcret == 0x01) 
				{
					if(phatc->state == SCED_HATCSTA)
	                {
	                    //pscm->step = 0;
	                   // pscm->upcnt = 0;
	                   // pscm->stim = 0;
	                   //pmqtt->step += 1;
						 pmqtt->step = MQTT_DATAPROC_STEP_CMQTTPUB;
                       //pmqtt->stim = 0;
                       
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>CMQTTPAYLOAD set seccess!\r\n");
	                   
	                }
                    else
                    {
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, ">>>logic error!\r\n");
                        SamSendAtSeg(phatc);//not used
                    }
                    //pmqtt->stim = 0;
				}
				phatc->retbufp = 0;
			}
            else if(pmqtt->step == MQTT_DATAPROC_STEP_CMQTTPUB)
            {
                if(pMqttCtxt->pub_msg_list.pub_head != NULL && pMqttCtxt->pub_msg_list.length > 0)
                {
                    pub_msg_node_t *pCurPub = sam_get_current_pub_node(&pMqttCtxt->pub_msg_list);
                    
                    sprintf(buf, "AT+CMQTTPUB=%u,%u,%u\r", pmqtt->client_index, pCurPub->pub_data.pub_qos, pCurPub->pub_data.pub_timeout);
					SamSendAtCmd(phatc, buf, CRLF_HATCTYP, 9);
					//pmqtt->step += 1;
					pmqtt->step = MQTT_DATAPROC_STEP_CMQTTPUB_RES_CHECK;
					//pmqtt->stim = 0;  
                }
                else
                {
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, ">>>logic error!\r\n");
                   // pmqtt->step = 0;
					pmqtt->step = MQTT_ILDE_STEP_FIRST;
                    pmqtt->stim = 0;
                    pmqtt->sta = MQTT_STATUS_IDLE;
                }
            }
            else if(pmqtt->step == MQTT_DATAPROC_STEP_CMQTTPUB_RES_CHECK)
            {
				ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t+CMQTTPUB:");
				if(ratcret == NOSTRRET_ATCRET)
				{
                	return(RETCHAR_KEEP);
				}
				else if(ratcret == OVERTIME_ATCRET||ratcret == 0x02)
				{
					pmqtt->sta = MQTT_STATUS_FAIL;   //对照at手册这条指令的fail的几种情况，这种情况肯定没有获取到error code,没有err 也是一种错误类型,有error code那次直接被丢掉了
					//pmqtt->connect_status = MQTT_DISCONNECTED;
					pmqtt->fail_type = MQTT_FAIL_TYPE_PUB_FAIL;
					pmqtt->stim = 0;
				}
                else if(ratcret == 0x03)
                {
                    uint32 cli_index = 0;
                    uint32 err = 0;
                    
					sscanf(phatc->retbuf,"+CMQTTPUB: %u,%u", &cli_index, &err);
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>CMQTTPUB result:%s \r\n",phatc->retbuf);
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>CMQTTPUB result:cli_index == %u, err ==  %u\r\n",cli_index, err);
                    if(0 == err)
                    {
                    	pmqtt->pub_fail_cont = 0;
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>CMQTTPUB result:success!!\r\n");
                        if(phatc->state == SCED_HATCSTA)
    	                {
                            pub_msg_node_t *pCurPub = sam_get_current_pub_node(&pMqttCtxt->pub_msg_list);
                            sam_del_current_pub_note(&pMqttCtxt->pub_msg_list, pCurPub);
                            if(pMqttCtxt->pub_msg_list.pub_head != NULL)
                            {
                                 //pmqtt->step = 2;
								 pmqtt->step = MQTT_DATAPROC_STEP_CMQTTTOPIC;
                                 pmqtt->stim = 0;
                            }
                            else
                            {
                                //pmqtt->step = 0;
								pmqtt->step = MQTT_ILDE_STEP_FIRST;
                                pmqtt->stim = 0;
                                pmqtt->sta = MQTT_STATUS_IDLE;
                            }
    	                }
                        else
                        {
                            SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, ">>>logic error!\r\n");
                            SamSendAtSeg(phatc);//not used
                        }
                        //pmqtt->stim = 0;
                    }
                    else
                    {
                        pmqtt->sta = MQTT_STATUS_FAIL; //数据发布失败，需要在MQTT_STATUS_FAIL 处理异常，发布列表不需要改变，这种情况是有error code的
                        pmqtt->fail_type = MQTT_FAIL_TYPE_PUB_FAIL;
						pmqtt->stim = 0;
                    }
                }
				else if(ratcret == 0x01) 
				{
                    //do nothing
                    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>recieved:%s\r\n",phatc->retbuf);
                    
				}
				phatc->retbufp = 0;
			}
            
			break;
		case MQTT_STATUS_CONNECT_CLOSE_REQUEST_CHECK:
			{
				//if(0 == pmqtt->step)//MQTT_STATUS_SWITCH_STEP_CMQTTDISC
				if(MQTT_STATUS_SWITCH_STEP_CMQTTDISC == pmqtt->step)
				{
					while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) phatc->retbufp = 0;
					if(1 == pmqtt->close_req)
					{
					   // if(MQTT_CONNECTED == pmqtt->connect_status)
				    	
						memset(buf, 0, sizeof(buf));
		                sprintf(buf, "AT+CMQTTDISC=%u\r", pmqtt->client_index);
						SamSendAtCmd(phatc, buf, CRLF_HATCTYP, 90);
						//pmqtt->step++;
						pmqtt->step = MQTT_STATUS_SWITCH_STEP_CMQTTDISC_RES_CHECK;
						pmqtt->stim = 0;
		                pmqtt->dcnt = 0;
					}
					else
					{
						 //pmqtt->step = 0;
						 pmqtt->step = MQTT_ILDE_STEP_FIRST;
	                     pmqtt->stim = 0;
	                     pmqtt->sta = MQTT_STATUS_IDLE;
					}
				}
				else if(MQTT_STATUS_SWITCH_STEP_CMQTTDISC_RES_CHECK == pmqtt->step)
				{
					ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t+CMQTTDISC:");
					if(ratcret == NOSTRRET_ATCRET)
					{
	                	return(RETCHAR_KEEP);
					}
	                else if(ratcret == OVERTIME_ATCRET)
					{
						pmqtt->dcnt++;
						if(pmqtt->dcnt < 3)
						{
							//pmqtt->step--;
							pmqtt->step = MQTT_STATUS_SWITCH_STEP_CMQTTDISC;
	                    	pmqtt->stim = 0;
						}
						else
						{
							pmqtt->sta = MQTT_STATUS_FAIL;
							pmqtt->fail_type = MQTT_FAIL_TYPE_CLOSE_FAIL;
							pmqtt->stim = 0;
						}
					}
	                else if(ratcret == 1)
	                {
	                    //do nothing
	                }
	                else if(ratcret == 2)
	                {	                	
	                    //pmqtt->sta = MQTT_STATUS_FAIL;
	                    //pmqtt->step++;
						pmqtt->step = MQTT_STATUS_SWITCH_STEP_CMQTTREL;
						pmqtt->stim = 0;
	                }
					else if(ratcret == 3)
					{
	                    
	                    uint32 err = 0;
						uint32 index = 0;
	                    
						sscanf(phatc->retbuf,"+CMQTTDISC: %u,%u",&index, &err);
						//i = Strsearch(phatc->retbuf, ",0");
						if(0 == err)
						{
                            SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>mqtt disconnect server success!\r\n");
							//pmqtt->step++;
							pmqtt->step = MQTT_STATUS_SWITCH_STEP_CMQTTREL;
							pmqtt->connect_status = MQTT_DISCONNECTED;
						}
	                    else
	                    {
	                       // pmqtt->sta = MQTT_STATUS_FAIL;
	                       //pmqtt->step++;
						   pmqtt->step = MQTT_STATUS_SWITCH_STEP_CMQTTREL;
	                    } 
	                    pmqtt->stim = 0;
						if(phatc->state != SCED_HATCSTA)
						{
                            SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_ERROR, ">>>logic error!\r\n");
						}
					}
					phatc->retbufp = 0;
	                phatc->retbuf[0] = 0;
					
				}
				else if(MQTT_STATUS_SWITCH_STEP_CMQTTREL == pmqtt->step)
				{
					memset(buf, 0, sizeof(buf));
	                sprintf(buf, "AT+CMQTTREL=%u\r", pmqtt->client_index);
					SamSendAtCmd(phatc, buf, CRLF_HATCTYP, 90);
					//pmqtt->step++;
					pmqtt->step = MQTT_STATUS_SWITCH_STEP_CMQTTREL_RES_CHECK;
					pmqtt->stim = 0;
	                pmqtt->dcnt = 0;
				}
				else if(MQTT_STATUS_SWITCH_STEP_CMQTTREL_RES_CHECK == pmqtt->step)
				{
					ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t+CMQTTREL:");
					if(ratcret == NOSTRRET_ATCRET)
					{
	                	return(RETCHAR_KEEP);
					}
	                else if(ratcret == OVERTIME_ATCRET)
					{
						pmqtt->dcnt++;
						if(pmqtt->dcnt < 3)
						{
							//pmqtt->step--;
							pmqtt->step = MQTT_STATUS_SWITCH_STEP_CMQTTREL;
	                    	pmqtt->stim = 0;
						}
						else
						{
							pmqtt->sta = MQTT_STATUS_FAIL;
							pmqtt->fail_type = MQTT_FAIL_TYPE_CLOSE_FAIL;
							pmqtt->stim = 0;

							//pmqtt->step++;
							//pmqtt->stim = 0;
						}
					}
	                else if(ratcret == 1)
	                {
	                    //pmqtt->step++;
						pmqtt->step = MQTT_STATUS_SWITCH_STEP_CMQTTSTOP;
						pmqtt->stim = 0;
	                }
	                else if(ratcret == 2)
	                {	                	
	                   // pmqtt->sta = MQTT_STATUS_FAIL;
	                    //pmqtt->step++;
					    pmqtt->step = MQTT_STATUS_SWITCH_STEP_CMQTTSTOP;
						pmqtt->stim = 0;
	                }
					else if(ratcret == 3)
					{
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>%s\r\n",phatc->retbuf);
					}
					phatc->retbufp = 0;
	                phatc->retbuf[0] = 0;
					
				}
				else if(MQTT_STATUS_SWITCH_STEP_CMQTTSTOP == pmqtt->step)
				{
					memset(buf, 0, sizeof(buf));
	                sprintf(buf, "AT+CMQTTSTOP\r");
					SamSendAtCmd(phatc, buf, CRLF_HATCTYP, 90);
					//pmqtt->step++;
					pmqtt->step = MQTT_STATUS_SWITCH_STEP_CMQTTSTOP_RES_CHECK;
					pmqtt->stim = 0;
	                pmqtt->dcnt = 0;
				}
				else if(MQTT_STATUS_SWITCH_STEP_CMQTTSTOP_RES_CHECK == pmqtt->step)
				{
					ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t+CMQTTSTOP:");
					if(ratcret == NOSTRRET_ATCRET)
					{
	                	return(RETCHAR_KEEP);
					}
	                else if(ratcret == OVERTIME_ATCRET)
					{
						pmqtt->dcnt++;
						if(pmqtt->dcnt < 3)
						{
							//pmqtt->step--;
							pmqtt->step = MQTT_STATUS_SWITCH_STEP_CMQTTSTOP;
	                    	pmqtt->stim = 0;
						}
						else
						{
							pmqtt->sta = MQTT_STATUS_FAIL;
							pmqtt->fail_type = MQTT_FAIL_TYPE_CLOSE_FAIL;
							pmqtt->stim = 0;

							//pmqtt->step++;
							//pmqtt->stim = 0;
						}
					}
	                else if(ratcret == 1)
	                {
	                   // pmqtt->step++;
					//	pmqtt->stim = 0;
	                }
	                else if(ratcret == 2)
	                {	                	
	                   // pmqtt->sta = MQTT_STATUS_FAIL;
	                    //pmqtt->step++;
					    pmqtt->step = MQTT_STATUS_SWITCH_STEP_SERVICE_CLOSE;
						pmqtt->stim = 0;
						pmqtt->close_req = 0;
	                }
					else if(ratcret == 3)
					{
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>%s\r\n",phatc->retbuf);	  
						//pmqtt->step++;
						pmqtt->step = MQTT_STATUS_SWITCH_STEP_SERVICE_CLOSE;
						pmqtt->stim = 0;
						pmqtt->close_req = 0;
					}
					phatc->retbufp = 0;
	                phatc->retbuf[0] = 0;
					
				}
				else if(MQTT_STATUS_SWITCH_STEP_SERVICE_CLOSE == pmqtt->step)
				{
					pmqtt->close_fail_cont = 0;
					sam_mqtt_stop(pmqtt);
					return(RETCHAR_FREE);
				}
				
			}
			break;
		case MQTT_STATUS_IDLE :
            {
			while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) phatc->retbufp = 0;
            
			//if(pscm->stim >= 60 || (pscm->urcbmk & phatc->urcbitmask) != 0 || pscm->upcnt != 0)
			if(MQTT_DISCONNECTED == pmqtt->connect_status && 1 != pmqtt->close_req)
			{
				pmqtt->sta = MQTT_STATUS_INIT;
				pmqtt->step = MQTT_INIT_STEP_CMQTTCONNECT;
				pmqtt->stim = 0;
			}
            else if((NULL != pmqtt->mqtt_context.p_sub_topic && pmqtt->mqtt_context.sub_topic_req_lenth > 0) || (pMqttCtxt->pub_msg_list.pub_head != NULL && pMqttCtxt->pub_msg_list.length > 0))  
			{
				pmqtt->sta = MQTT_STATUS_DATA_PROCESS;
				//pmqtt->step=0;
				pmqtt->step=MQTT_DATAPROC_STEP_CMQTTSUB;
                pmqtt->stim = 0;
				//pscm->dcnt=0;
			}
			else if(1 == pmqtt->close_req)
			{
				pmqtt->sta = MQTT_STATUS_CONNECT_CLOSE_REQUEST_CHECK;
				//pmqtt->step=0;
				pmqtt->step=MQTT_STATUS_SWITCH_STEP_CMQTTDISC;
                pmqtt->stim = 0;
				pmqtt->dcnt=0;
			}
			else
			{
                pmqtt->sta = MQTT_STATUS_DATA_PROCESS;
				//pmqtt->step=0;
				pmqtt->step=MQTT_DATAPROC_STEP_CMQTTSUB;
                pmqtt->stim = 0;
				return(RETCHAR_FREE);
			}
          }
			break;
		case MQTT_STATUS_FAIL:
            {
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) phatc->retbufp = 0;
				switch(pmqtt->fail_type)
				{
					case MQTT_FAIL_TYPE_CONNECT_FAIL:
					{
						if(pmqtt->stim >= 2)
						{
							pmqtt->init_fail_cont++;
							//pmqtt->connect_fail_cont++;
							pmqtt->fail_type = MQTT_FAIL_TYPE_NONE;
							if(pmqtt->init_fail_cont >= 6)
							{
								pmqtt->sta = MQTT_STATUS_CONNECT_RESET;
								pmqtt->step = MQTT_CONNECT_RESET_STEP_CMQTTDISC;
			                	pmqtt->stim = 0;

								pmqtt->init_fail_cont = 0;
							    //pmqtt->connect_fail_cont =0;
								return(RETCHAR_FREE);
							}
							else
							{
								pmqtt->sta = MQTT_STATUS_INIT;
							    pmqtt->step = MQTT_INIT_STEP_CMQTTSTART;
			                    pmqtt->stim = 0;							
							}
						}
					}
					break;

					case MQTT_FAIL_TYPE_MQTTSTART_FAIL:
					case MQTT_FAIL_TYPE_MQTTACCQ_FAIL:
					case MQTT_FAIL_TYPE_MQTTWILLTOPIC_FAIL:
					case MQTT_FAIL_TYPE_MQTTWILLMSG_FAIL:	
					{
						if(pmqtt->stim >= 2)
						{
							pmqtt->init_fail_cont++;
							
							pmqtt->fail_type = MQTT_FAIL_TYPE_NONE;
							if(pmqtt->init_fail_cont >= 6)
							{
								pmqtt->sta = MQTT_STATUS_CONNECT_RESET;
								pmqtt->step = MQTT_CONNECT_RESET_STEP_CMQTTDISC;
			                	pmqtt->stim = 0;

								pmqtt->init_fail_cont = 0;
							    //pmqtt->connect_fail_cont =0;
								return(RETCHAR_FREE);
							}
							else
							{
								pmqtt->sta = MQTT_STATUS_INIT;
							    pmqtt->step = MQTT_INIT_STEP_CMQTTSTART;
			                    pmqtt->stim = 0;							
							}
						}
					}
					break;

					case MQTT_FAIL_TYPE_SUB_FAIL:
					{
						if(pmqtt->stim >= 1)
						{   if(MQTT_CONNECTED == pmqtt->connect_status)
							{
								pmqtt->sub_fail_cont++;
								
								pmqtt->fail_type = MQTT_FAIL_TYPE_NONE;
								if(pmqtt->sub_fail_cont >= 3)
								{
									pmqtt->sta = MQTT_STATUS_CONNECT_RESET;
									pmqtt->step = MQTT_CONNECT_RESET_STEP_CMQTTDISC;
				                	pmqtt->stim = 0;
									pmqtt->sub_fail_cont = 0;
								    
									return(RETCHAR_FREE);
								}
								else
								{
									pmqtt->sta = MQTT_STATUS_DATA_PROCESS;
								    pmqtt->step = MQTT_DATAPROC_STEP_CMQTTSUB;
				                    //pmqtt->stim = 0;							
								}
							}
							else
							{
								pmqtt->sub_fail_cont = 0;
								pmqtt->fail_type = MQTT_FAIL_TYPE_NONE;
								pmqtt->sta = MQTT_STATUS_INIT;
								pmqtt->step = MQTT_INIT_STEP_CMQTTCONNECT;
								pmqtt->stim = 0;
							}
						}
					}
					break;

					case MQTT_FAIL_TYPE_PUB_FAIL:
					{
						if(pmqtt->stim >= 1)
						{   if(MQTT_CONNECTED == pmqtt->connect_status)
							{
								pmqtt->pub_fail_cont++;
								
								pmqtt->fail_type = MQTT_FAIL_TYPE_NONE;
								if(pmqtt->pub_fail_cont >= 3)
								{
									pmqtt->sta = MQTT_STATUS_CONNECT_RESET;
									pmqtt->step = MQTT_CONNECT_RESET_STEP_CMQTTDISC;
				                	pmqtt->stim = 0;
									pmqtt->pub_fail_cont = 0;
								    
									return(RETCHAR_FREE);
								}
								else
								{
									pmqtt->sta = MQTT_STATUS_DATA_PROCESS;
								    pmqtt->step = MQTT_DATAPROC_STEP_CMQTTTOPIC;
				                    pmqtt->stim = 0;							
								}
							}
							else
							{
								pmqtt->pub_fail_cont = 0;
								pmqtt->fail_type = MQTT_FAIL_TYPE_NONE;
								pmqtt->sta = MQTT_STATUS_INIT;
								pmqtt->step = MQTT_INIT_STEP_CMQTTCONNECT;
								pmqtt->stim = 0;
							}
						}
					}
					break;

					case MQTT_FAIL_TYPE_CLOSE_FAIL:
					{
						if(pmqtt->stim >= 1)
						{
								pmqtt->close_fail_cont++;
								pmqtt->fail_type = MQTT_FAIL_TYPE_NONE;
								if(pmqtt->close_fail_cont >= 3)
								{
									pmqtt->sta = MQTT_STATUS_CONNECT_CLOSE_REQUEST_CHECK;
									pmqtt->step = MQTT_STATUS_SWITCH_STEP_CMQTTDISC;
				                	pmqtt->stim = 0;
									pmqtt->close_fail_cont = 0;
								    
									return(RETCHAR_FREE);
								}
								else
								{
									pmqtt->sta = MQTT_STATUS_CONNECT_CLOSE_REQUEST_CHECK;
								    pmqtt->step = MQTT_STATUS_SWITCH_STEP_CMQTTDISC;
				                    pmqtt->stim = 0;							
								}
							}
					}
					break;

					default:
						pmqtt->sta = MQTT_STATUS_IDLE;  //考虑下这种情况下有没有必要把资源让出来 返回RETCHAR_FREE，交给idle，那就还是根据检查数据的情况进一步处理
						break;
				}
				
                
            }
            break;

		case MQTT_STATUS_CONNECT_RESET:
			{
				//if(0 == pmqtt->step)//MQTT_STATUS_SWITCH_STEP_CMQTTDISC
				if(MQTT_CONNECT_RESET_STEP_CMQTTDISC == pmqtt->step)
				{
					while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) phatc->retbufp = 0;
					memset(buf, 0, sizeof(buf));
	                sprintf(buf, "AT+CMQTTDISC=%u\r", pmqtt->client_index);
					SamSendAtCmd(phatc, buf, CRLF_HATCTYP, 90);
					//pmqtt->step++;
					pmqtt->step = MQTT_CONNECT_RESET_STEP_CMQTTDISC_RES_CHECK;
					pmqtt->stim = 0;
	                pmqtt->dcnt = 0;	
				}
				else if(MQTT_CONNECT_RESET_STEP_CMQTTDISC_RES_CHECK == pmqtt->step)
				{
					ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t+CMQTTDISC:");
					if(ratcret == NOSTRRET_ATCRET)
					{
	                	return(RETCHAR_KEEP);
					}
	                else if(ratcret == OVERTIME_ATCRET)
					{
						pmqtt->dcnt++;
						if(pmqtt->dcnt < 3)
						{
							//pmqtt->step--;
							pmqtt->step = MQTT_CONNECT_RESET_STEP_CMQTTDISC;
	                    	pmqtt->stim = 0;
						}
						else
						{
							
							pmqtt->step = MQTT_CONNECT_RESET_STEP_CMQTTREL;
							pmqtt->stim = 0;
						}
					}
	                else if(ratcret == 1)
	                {
	                    //do nothing
	                }
	                else if(ratcret == 2)
	                {	                	
	                    //pmqtt->sta = MQTT_STATUS_FAIL;
	                    //pmqtt->step++;
						pmqtt->step = MQTT_CONNECT_RESET_STEP_CMQTTREL;
						pmqtt->stim = 0;
	                }
					else if(ratcret == 3)
					{
	                    
	                    uint32 err = 0;
						uint32 index = 0;
	                    
						sscanf(phatc->retbuf,"+CMQTTDISC: %u,%u",&index, &err);
						//i = Strsearch(phatc->retbuf, ",0");
						if(0 == err)
						{
                            SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>CONNECT_RESET,mqtt disconnect server success!\r\n");
							//pmqtt->step++;
							pmqtt->step = MQTT_CONNECT_RESET_STEP_CMQTTREL;
							pmqtt->connect_status = MQTT_DISCONNECTED;
						}
	                    else
	                    {
	                       // pmqtt->sta = MQTT_STATUS_FAIL;
	                       //pmqtt->step++;
						   pmqtt->step = MQTT_CONNECT_RESET_STEP_CMQTTREL;
	                    } 
	                    pmqtt->stim = 0;
						if(phatc->state != SCED_HATCSTA)
						{
                            SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>logic error!\r\n");
						}
					}
					phatc->retbufp = 0;
	                phatc->retbuf[0] = 0;
					
				}
				else if(MQTT_CONNECT_RESET_STEP_CMQTTREL == pmqtt->step)
				{
					memset(buf, 0, sizeof(buf));
	                sprintf(buf, "AT+CMQTTREL=%u\r", pmqtt->client_index);
					SamSendAtCmd(phatc, buf, CRLF_HATCTYP, 90);
					//pmqtt->step++;
					pmqtt->step = MQTT_CONNECT_RESET_STEP_CMQTTREL_RES_CHECK;
					pmqtt->stim = 0;
	                pmqtt->dcnt = 0;
				}
				else if(MQTT_CONNECT_RESET_STEP_CMQTTREL_RES_CHECK == pmqtt->step)
				{
					ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t+CMQTTREL:");
					if(ratcret == NOSTRRET_ATCRET)
					{
	                	return(RETCHAR_KEEP);
					}
	                else if(ratcret == OVERTIME_ATCRET)
					{
						pmqtt->dcnt++;
						if(pmqtt->dcnt < 3)
						{
							//pmqtt->step--;
							pmqtt->step = MQTT_CONNECT_RESET_STEP_CMQTTREL;
	                    	pmqtt->stim = 0;
						}
						else
						{
							//pmqtt->sta = MQTT_STATUS_FAIL;
							//pmqtt->fail_type = MQTT_FAIL_TYPE_CLOSE_FAIL;
							pmqtt->step = MQTT_CONNECT_RESET_STEP_CMQTTSTOP;
							pmqtt->stim = 0;

							//pmqtt->step++;
							//pmqtt->stim = 0;
						}
					}
	                else if(ratcret == 1)
	                {
	                    //pmqtt->step++;
						pmqtt->step = MQTT_CONNECT_RESET_STEP_CMQTTSTOP;
						pmqtt->stim = 0;
	                }
	                else if(ratcret == 2)
	                {	                	
	                   // pmqtt->sta = MQTT_STATUS_FAIL;
	                    //pmqtt->step++;
					    pmqtt->step = MQTT_CONNECT_RESET_STEP_CMQTTSTOP;
						pmqtt->stim = 0;
	                }
					else if(ratcret == 3)
					{
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>%s\r\n",phatc->retbuf);	
						//pmqtt->step = MQTT_CONNECT_RESET_STEP_CMQTTSTOP;
						//pmqtt->stim = 0;

					}
					phatc->retbufp = 0;
	                phatc->retbuf[0] = 0;
					
				}
				else if(MQTT_CONNECT_RESET_STEP_CMQTTSTOP == pmqtt->step)
				{
					memset(buf, 0, sizeof(buf));
	                sprintf(buf, "AT+CMQTTSTOP\r");
					SamSendAtCmd(phatc, buf, CRLF_HATCTYP, 90);
					//pmqtt->step++;
					pmqtt->step = MQTT_CONNECT_RESET_STEP_CMQTTSTOP_RES_CHECK;
					pmqtt->stim = 0;
	                pmqtt->dcnt = 0;
				}
				else if(MQTT_CONNECT_RESET_STEP_CMQTTSTOP_RES_CHECK == pmqtt->step)
				{
					ratcret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n\t+CMQTTSTOP:");
					if(ratcret == NOSTRRET_ATCRET)
					{
	                	return(RETCHAR_KEEP);
					}
	                else if(ratcret == OVERTIME_ATCRET)
					{
						pmqtt->dcnt++;
						if(pmqtt->dcnt < 3)
						{
							//pmqtt->step--;
							pmqtt->step = MQTT_CONNECT_RESET_STEP_CMQTTSTOP;
	                    	pmqtt->stim = 0;
						}
						else
						{
							//pmqtt->sta = MQTT_STATUS_FAIL;
							//pmqtt->fail_type = MQTT_FAIL_TYPE_CLOSE_FAIL;
							pmqtt->sta = MQTT_STATUS_INIT;
							pmqtt->step = MQTT_INIT_STEP_CMQTTSTART;
							pmqtt->stim = 0;

							//pmqtt->step++;
							//pmqtt->stim = 0;
						}
					}
	                else if(ratcret == 1)
	                {
	                   // pmqtt->step++;
					//	pmqtt->stim = 0;
	                }
	                else if(ratcret == 2)
	                {	                	
	                   // pmqtt->sta = MQTT_STATUS_FAIL;
	                    //pmqtt->step++;
					    pmqtt->sta = MQTT_STATUS_INIT;
							pmqtt->step = MQTT_INIT_STEP_CMQTTSTART;
							pmqtt->stim = 0;
	                }
					else if(ratcret == 3)
					{
                        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>%s\r\n",phatc->retbuf);	  
						pmqtt->sta = MQTT_STATUS_INIT;
						pmqtt->step = MQTT_INIT_STEP_CMQTTSTART;
						pmqtt->stim = 0;
					}
					phatc->retbufp = 0;
	                phatc->retbuf[0] = 0;
					
				}
				
				
			}
			break;
		default :
			pmqtt->sta = MQTT_STATUS_IDLE;
			break;
	}
	return(RETCHAR_KEEP);
}

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
uint8 sam_mqtt_subscribe_topic(TMqttTag * pmqtt, char *pTopic)
{
    if(NULL == pmqtt || NULL == pTopic)
        return 0;
   // clear_sub_info(&pmqtt->mqtt_context);
    if(NULL == pmqtt->mqtt_context.p_sub_topic && 0 == pmqtt->mqtt_context.sub_topic_req_lenth)
    {
        add_sub_topic(&pmqtt->mqtt_context, pTopic);
        return 1;
    }
    else
    {
        return 0;
    }
}

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
uint8 sam_mqtt_publish_message(TMqttTag * pmqtt, char *pTopic, char *pMsg)
{
    uint8 res = 0;
    if(NULL == pmqtt || NULL == pTopic || NULL == pMsg )
        return 0;
    SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>sam_mqtt_publish_message, before add  list lenth == %u\r\n",pmqtt->mqtt_context.pub_msg_list.length);
    pub_msg_node_t *pNode = sam_insert_new_pub_data_into_list(&(pmqtt->mqtt_context.pub_msg_list), pTopic, pMsg);
    if(NULL != pNode)
    {
        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>sam_mqtt_publish_message success!!  list lenth == %u\r\n",pmqtt->mqtt_context.pub_msg_list.length);
        res = 1;
    }
    else
    {
        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>>sam_mqtt_publish_message fail!!  list lenth == %u\r\n",pmqtt->mqtt_context.pub_msg_list.length);
         res = 0;
    }
    return res;
}

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
uint8  sam_mqtt_get_connection_status(TMqttTag * pmqtt)
{
    if(pmqtt->connect_status == MQTT_CONNECTED)
        return 1;
    else
        return 0;
}

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
 void  sam_mqtt_set_receive_callback(TMqttTag * pmqtt, sam_mqtt_receive_data_cb cb)
 {
    if(NULL == pmqtt || NULL == cb)
        return;
    pmqtt->receive_data_cb = cb;
 }

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
uint8  sam_mqtt_close(TMqttTag * pvmqtt)
{
	if(NULL == pvmqtt)
		return 0;
    uint8 status = sam_mqtt_get_connection_status(pvmqtt);
	if(status)
	{
		pvmqtt->close_req = 1;
		return 1;
	}
	else
	{
        SAM_DBG_MODULE(SAM_MOD_MQTT, SAM_DBG_LEVEL_INFO, ">>> mqtt is not connectted!!\r\n");	  
		return 0;
	}
}


