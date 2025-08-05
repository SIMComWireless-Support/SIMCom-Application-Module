/**
 * @file SamMqttSrv.h
 * @brief MQTT Test Demo Module. 
 * 
 * This header file defines the interfaces for MQTT-related operations 
 * required by the test demo, including MQTT initialization and callback 
 * function registration.
 *
 * @details 
 * This module provides a test framework for MQTT client operations.
 * It includes interfaces for initializing MQTT connections, 
 * registering callback handlers, and managing subscription topics.
 *
 * @version 1.0.0
 * @date 2025-07-01
 * @author <dong.chen@sunseaaiot.com>
 * @copyright Copyright (c) 2025, Your Company Inc. All rights reserved.
 */
#ifndef __SAMMQTTSRV_H
#define __SAMMQTTSRV_H

#include "SamInc.h"

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @brief Initialization of MQTT Mode.
 */
void mqtt_demo_init(void);

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
void mqtt_demo_submsg_receive_cb(TMqttTag *pmqttobj, uint8 index, char *topic, char *pMsg);

/**
 * @brief Run the MQTT demo client logic
 *
 * Sets up receive callbacks for two MQTT clients (pclient1 and pclient2) once they are connected.
 * Uses static flags to ensure each callback is set only once per client connection.
 *
 * @note The function checks connection status before setting callbacks to ensure valid client states
 * @warning Relies on global MQTT client pointers (pMqttClient1 and pMqttClient2) being properly initialized
 */
void mqtt_demo_client_run();


#ifdef __cplusplus
}
#endif





#endif

