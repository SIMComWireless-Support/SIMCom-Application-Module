/**
 * @file SamMqttSrv.c
 * @brief Implementation of MQTT-related operations for test demo
 * @details This file provides the implementation details for the MQTT 
 *          interfaces declared in SamMqttSrv.h. It includes functions 
 *          for MQTT client initialization, connection management, message 
 *          publishing/subscription, and callback handler implementations.
 * 
 * @version 1.0.0
 * @date 2025-07-01
 * @author John Doe <dong.chen@sunseaaiot.com>
 * @copyright Copyright (c) 2025, Your Company Inc. All rights reserved.
 * 
 * @note This implementation uses the Paho MQTT C client library.
 *       Error handling is simplified for demo purposes and may need
 *       enhancement for production environments.
 */
#include "include.h"
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

/**
 * @brief Initialization Configuration for MQTT Client 1.
 */
char Mqtt1605CfgStr1[] = "\vCFGMQTT_C1\t0\t0\t\"client id0\"\t\"tcp://test.mosquitto.org:1883\"\t\"cmd_topic\"\t\"will_topic_test0\"\t\"will_msg_test0\"\v";
void *pMqttClient1 = NULL;


/**
 * @brief Initialization Configuration for MQTT Client 2.
 */
char Mqtt1605CfgStr2[] = "\vCFGMQTT_C2\t0\t1\t\"client id1\"\t\"tcp://test.mosquitto.org:1883\"\t\"cmd_topic\"\t\"will_topic_test1\"\t\"will_msg_test1\"\v";
void *pMqttClient2 = NULL;

/**
 * @brief Initialization of MQTT Mode.
 */
void mqtt_demo_init(void)
{
	TMqttTag *pmqtt = NULL;

	pmqtt = (TMqttTag *) malloc(sizeof(TMqttTag));
	if(pmqtt != NULL)
	{
		//pMqttClient1 = sam_mqtt_init(pmqtt, &init_mqtt_cfg1);//Mqtt1605CfgStr1
		pMqttClient1 = sam_mqtt_init(pmqtt, Mqtt1605CfgStr1);//Mqtt1605CfgStr1
		if(pMqttClient1 == NULL)
		{
			printf("sam_mqtt_init pMqttClient1 Fail\n");
			free(pmqtt);	
			pmqtt = NULL;	
		}
	}
	else
	{
		printf("malloc Fail:%s,%d\n", __FILE__, __LINE__);
	}
	
	pmqtt = (TMqttTag *) malloc(sizeof(TMqttTag));
	if(pmqtt != NULL)
	{
		//pMqttClient2 = sam_mqtt_init(pmqtt, &init_mqtt_cfg2);
		pMqttClient2 = sam_mqtt_init(pmqtt, Mqtt1605CfgStr2);
		if(pMqttClient2 == NULL)
		{
			printf("sam_mqtt_init pMqttClient2 Fail\n");
			free(pmqtt);	
			pmqtt = NULL;	
		}
	}
	else
	{
		printf("malloc Fail:%s,%d\n", __FILE__, __LINE__);
	}

}

/**
 * @brief Parse MQTT demo command input string
 * @details This function splits the input command string into two parts:
 *          command type and command parameters, separated by a colon.
 *          Format requirement: [CommandType]:[Parameters], total length <= 20 bytes
 *          Command type length: 1-10 bytes, parameters length: 0-10 bytes
 * 
 * @param[in] input_cmd Input command string in "type:context" format
 * @param[out] p_cmd_type Buffer to store parsed command type
 * @param[out] p_cmd_ctx Buffer to store parsed command parameters
 * 
 * @return uint8 Parsing result status code:
 *         - 1: Parsing successful
 *         - 0: Parsing failed (null pointer/over-length/format error)
 * 
 * @note Total input length must not exceed 20 bytes
 *       Both command type and parameters length must not exceed 10 bytes
 *       Output buffers will be null-terminated automatically
 */
uint8 mqtt_demo_parse_cmd(char *input_cmd, char *p_cmd_type, char *p_cmd_ctx) {
    
    if (input_cmd == NULL || p_cmd_type == NULL || p_cmd_ctx == NULL) {
        return 0;
    }
    
    
    uint16 input_len = strlen(input_cmd);
    if (input_len > 20) {
        return 0;
    }
    
    
    char *colon = strchr(input_cmd, ':');
    if (colon == NULL) {
        return 0;
    }
    
    
    uint16 type_len = colon - input_cmd;
    if (type_len == 0 || type_len > 10) {
        return 0;
    }
    
    
    uint16 ctx_len = input_len - type_len - 1;
    if (ctx_len > 10) {
        return 0;
    }
    
    
    memcpy(p_cmd_type, input_cmd, type_len);
    p_cmd_type[type_len] = '\0';
    
    
    if(ctx_len > 0)
    memcpy(p_cmd_ctx, colon + 1, ctx_len);
    p_cmd_ctx[ctx_len] = '\0';
    
    return 1;
}

/**
 * @brief Callback function for handling received MQTT subscription messages
 *
 * Processes incoming MQTT messages received on subscribed topics. This demo callback
 * logs the message details (index, topic, payload) to the console. It performs basic
 * error checking to ensure the MQTT client pointer is valid before processing.
 *
 * @param pmqttobj Pointer to the MQTT client structure (TMqttTag) that received the message
 * @param index Numeric index identifying the message context (typically client or channel)
 * @param topic Null-terminated string containing the topic on which the message was received
 * @param pMsg Null-terminated string containing the message payload
 *
 * @note This is a simplified demo callback for basic message logging
 * @warning Does not handle message processing beyond logging; extend for production use
 */
void mqtt_demo_submsg_receive_cb(TMqttTag *pmqttobj, uint8 index, char *topic, char *pMsg)
{
	if(NULL == pmqttobj)
	{
		printf("mqtt_demo_submsg_receive_cb  pmqttobj is NULL!! \r\n");
		return;
	}

    printf("mqtt_demo_submsg_receive_cb enter: \r\n");
    printf("mqtt_demo_submsg_receive_cb index: %u\r\n", index);
    printf("mqtt_demo_submsg_receive_cb topic: %s\r\n", topic);
    printf("mqtt_demo_submsg_receive_cb pMsg: %s\r\n", pMsg);

	 if((NULL != topic) && (0 ==  strcmp(topic, "sub_test_topic")))
	 {
	 	uint8 res = sam_mqtt_close(pmqttobj);
		printf("mqtt_demo_submsg_receive_cb close res: %u!!\r\n", res);
	 }
}

#if 0

/**
 * @brief Callback function for handling received MQTT subscription messages
 *
 * Processes incoming MQTT messages received on subscribed topics. This demo callback
 * parses commands from the "cmd_topic" and executes corresponding MQTT operations
 * such as subscribing to new topics, publishing messages, or closing the connection.
 *
 * @param pmqttobj Pointer to the MQTT client structure (TMqttTag) that received the message
 * @param index Numeric index identifying the message context (typically client or channel)
 * @param topic Null-terminated string containing the topic on which the message was received
 * @param pMsg Null-terminated string containing the message payload
 *
 * @note Handles command messages in the format "<command_type>:<command_context>"
 * @warning Contains demo-specific logic; replace or extend for production use
 */
void mqtt_demo_submsg_receive_cb(TMqttTag *pmqttobj, uint8 index, char *topic, char *pMsg)
{
    printf("mqtt_demo_submsg_receive_cb enter: \r\n");
    printf("mqtt_demo_submsg_receive_cb index: %u\r\n", index);
    printf("mqtt_demo_submsg_receive_cb topic: %s\r\n", topic);
    printf("mqtt_demo_submsg_receive_cb pMsg: %s\r\n", pMsg);

    TMqttTag *p_mqtt = pmqttobj;
	if(NULL == p_mqtt)
	{
		printf("mqtt_demo_submsg_receive_cb  pmqttobj is NULL!! \r\n");
		return;
	}

	
    if(0 ==  strcmp(topic, "cmd_topic"))
    {
    	char cmd_type[11] = {0};
		char cmd_ctx[11] = {0};
		if(1 == mqtt_demo_parse_cmd(pMsg, cmd_type, cmd_ctx))
		{
		 if(0 == strcmp(cmd_type, "sub"))
		 {
		    if(1 == sam_mqtt_subscribe_topic(p_mqtt, cmd_ctx))
		    {
		    	 printf("mqtt_demo_submsg_receive_cb index: %u, sub topic:%s success!!\r\n", index, cmd_ctx);
		    }
			else
			{
				printf("mqtt_demo_submsg_receive_cb index: %u, sub topic:%s success!!\r\n", index, cmd_ctx);
			}
		 }
		 else if(0 == strcmp(cmd_type, "pub"))
		 {
		 	char pubmsg[20] = {0};
			sprintf(pubmsg, "index%u:pubmsg_test", index);
		    if(1 == sam_mqtt_publish_message(p_mqtt, cmd_ctx, pubmsg))
				
		    {
		    	 printf("mqtt_demo_submsg_receive_cb index: %u, pub topic:%s success!!\r\n", index, cmd_ctx);
		    }
			else
			{
				printf("mqtt_demo_submsg_receive_cb index: %u, pub topic:%s success!!\r\n", index, cmd_ctx);
			}
		 }
		 else if(0 == strcmp(cmd_type, "close"))
		 {
		 	uint8 res = sam_mqtt_close(p_mqtt);
			printf("mqtt_demo_submsg_receive_cb close res: %u!!\r\n", res);
		 }
		 
		}
    }

}
#endif

/**
 * @brief Runs the MQTT demo client logic for two clients
 *
 * Manages the lifecycle of two MQTT demo clients, including setting up subscription callbacks,
 * subscribing to test topics, and periodically publishing messages. Resets client states after
 * a predefined number of publications and handles connection closure requests.
 *
 * @note Uses static variables to track client states, counters, and timing
 * @warning This is a demo function with hardcoded topics and publication limits
 */
void mqtt_demo_client_run()
{
	// Static flags to ensure subscription callbacks are set only once per client
	static uint8 c1_sub_cb_set_flag = 0;
	static uint8 c2_sub_cb_set_flag = 0;

	// Static flags to ensure topic subscriptions are performed only once per client
	static uint8 c1_sub_flag = 0;
	static uint8 c2_sub_flag = 0;

    // Counters to track the number of messages published by each client
	static uint32 c1_pub_cnt = 0;
	static uint32 c2_pub_cnt = 0;

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
	
	
	TMqttTag *pclient1 = (TMqttTag *)pMqttClient1;
	TMqttTag *pclient2 = (TMqttTag *)pMqttClient2;

	// Set receive callback for client1 once connected (only once)
	if(NULL != pclient1 && 0 == c1_sub_cb_set_flag && 1 == sam_mqtt_get_connection_status(pclient1))
	{
		 sam_mqtt_set_receive_callback(pclient1, mqtt_demo_submsg_receive_cb);
		 c1_sub_cb_set_flag = 1;
	}

	// Set receive callback for client2 once connected (only once)
	if(NULL != pclient2 && 0 == c2_sub_cb_set_flag && 1 == sam_mqtt_get_connection_status(pclient2))
	{
		 sam_mqtt_set_receive_callback(pclient2, mqtt_demo_submsg_receive_cb);
		 c2_sub_cb_set_flag = 1;
	}	

	// Subscribe client1 to test topic once connected (only once)
	if(NULL != pclient1 && 0 == c1_sub_flag && 1 == sam_mqtt_get_connection_status(pclient1))
	{
		 sam_mqtt_subscribe_topic(pclient1, "sub_test_topic");
		 c1_sub_flag = 1;
	}

	// Subscribe client2 to test topic once connected (only once)
	if(NULL != pclient2 && 0 == c2_sub_flag && 1 == sam_mqtt_get_connection_status(pclient2))
	{
		 sam_mqtt_subscribe_topic(pclient2, "sub_test_topic");
		 c2_sub_flag = 1;
	}	

	// Every 10 seconds, publish messages and check publication limits
	if(stim >= 10)
	{
	    if(NULL != pclient1 && 1 == sam_mqtt_get_connection_status(pclient1))
    	{
			char pub_msg[30] = {0};
		    sprintf(pub_msg, "client1_pub_msg%u", c1_pub_cnt);
			sam_mqtt_publish_message(pclient1, "pub_test_topic", pub_msg);
			printf("mqtt_demo  client1 pub_test_topic:%s \r\n",pub_msg);

			c1_pub_cnt ++;
			if(c1_pub_cnt > 1000)
			{
				c1_pub_cnt = 0;
			}
    	}

	    if(NULL != pclient2 && 1 == sam_mqtt_get_connection_status(pclient2))
    	{
			char pub_msg[30] = {0};
		    sprintf(pub_msg, "client2_pub_msg%u", c2_pub_cnt);
			sam_mqtt_publish_message(pclient2, "pub_test_topic", pub_msg);
			printf("mqtt_demo  client2 pub_test_topic:%s \r\n",pub_msg);
			c2_pub_cnt ++;
			if(c2_pub_cnt > 1000)
			{
				c2_pub_cnt = 0;
			}
    	}
		stim = 0;
	}
	
}

