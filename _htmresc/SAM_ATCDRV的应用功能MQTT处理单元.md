[<- 返回主目录](../README_cn.md)

<img src="simcom_logo.png" style="background-color:rgb(251, 252, 252); padding: 5px;" width="100">

# SAM_ATCDRV的应用功能MQTT处理单元

LTE模组

芯讯通无线科技(上海)有限公司  
上海市长宁区临虹路289号3号楼芯讯通总部大楼  
电话：86-21-31575100  
技术支持邮箱：[support@simcom.com](https://cn.simcom.com/online_questions.html)  
官网：[www.simcom.com](https://www.simcom.com)

|名称：|MQTT接口使用说明|
|---|---|
|版本：|V1.01|
|类别：|应用文档|
|状态：|已发布|

# 版权声明

本手册包含芯讯通无线科技（上海）有限公司（简称：芯讯通）的技术信息。除非经芯讯通书面许可，任何单位和个人不得擅自摘抄、复制本手册内容的部分或全部，并不得以任何形式传播，违反者将被追究法律责任。对技术信息涉及的专利、实用新型或者外观设计等知识产权，芯讯通保留一切权利。芯讯通有权在不通知的情况下随时更新本手册的具体内容。

本手册版权属于芯讯通，任何人未经我公司书面同意进行复制、引用或者修改本手册都将承担法律责任。

芯讯通无线科技(上海)有限公司  
上海市长宁区临虹路289号3号楼芯讯通总部大楼  
电话：86-21-31575100  
邮箱：simcom@simcom.com  
官网：[www.simcom.com](https://www.simcom.com)

了解更多资料，请点击以下链接：  
http://cn.simcom.com/download/list-230-cn.html

技术支持，请点击以下链接：  
http://cn.simcom.com/ask/index-cn.html 或发送邮件至support@simcom.com

版权所有 © 芯讯通无线科技(上海)有限公司2022，保留一切权利。

# 版本历史

|版本|日期|作者|备注|
|---|---|---|---|
|V1.00|2025-7-9||第一版|

# 文档介绍

本文档介绍了基于VMCU框架MQTT的接口定义，以及MQTT的接口使用方法和示例程序，指导客户如何在mcu上位机调用模组（比如A7670SA）的mqtt业务开发MQTT的相关业务。

# 目录

- [SAM\_ATCDRV的应用功能MQTT处理单元](#sam_atcdrv的应用功能mqtt处理单元)
- [版权声明](#版权声明)
- [版本历史](#版本历史)
- [文档介绍](#文档介绍)
- [目录](#目录)
- [1  MQTT 接口 API介绍](#1-mqtt-接口-api介绍)
	- [1.1 MQTT功能初始化sam\_mqtt\_init](#11-mqtt功能初始化sam_mqtt_init)
	- [1.2 订阅MQTT主题 sam\_mqtt\_subscribe\_topic](#12-订阅mqtt主题-sam_mqtt_subscribe_topic)
	- [1.3 发布主题消息sam\_mqtt\_publish\_message](#13-发布主题消息sam_mqtt_publish_message)
	- [1.4 获取mqtt连接状态sam\_mqtt\_get\_connection\_status](#14-获取mqtt连接状态sam_mqtt_get_connection_status)
	- [1.5 注册回调函数sam\_mqtt\_set\_receive\_callback](#15-注册回调函数sam_mqtt_set_receive_callback)
	- [1.6 关闭当前MQTT连接sam\_mqtt\_close](#16-关闭当前mqtt连接sam_mqtt_close)
- [2  MQTT接口使用示例介绍](#2-mqtt接口使用示例介绍)
	- [2.1  Main函数入口](#21-main函数入口)
	- [2.2  MQTT的初始化示例](#22-mqtt的初始化示例)
	- [2.3  MQTT的数据收发示例](#23-mqtt的数据收发示例)
		- [2.3.1  mqtt消息的订阅和接收示例](#231-mqtt消息的订阅和接收示例)
		- [2.3.2  mqtt消息的发布示例](#232-mqtt消息的发布示例)
- [3  MQTT 的示例运行结果演示](#3-mqtt-的示例运行结果演示)

# 1  MQTT 接口 API介绍

VMCU框架提供了一组MQTT接口，用于处理MQTT数据收发业务，可进行mqtt消息的订阅和发布。

MQTT接口定义在头文件SamMqtt.h中，使用时需包含该文件。

以下所有接口中的参数都需要一个TMqttTag 的指针，指向一个mqtt的client实例，代表当前接口是针对当前client对象。

## 1.1 MQTT功能初始化sam_mqtt_init

|接口|void * sam_mqtt_init(TMqttTag * pmqtt, char * cfgstr);|
|---|---|
|功能|使用配置字符串初始化MQTT客户端实例，从格式化字符串解析配置参数，初始化MQTT上下文，并为MQTT操作设置通信通道。配置字符串应包含客户端ID、服务器地址、主题和通信设置等参数|
|参数|pmqtt： 指向待初始化的MQTT客户端结构的指针<br/>cfgstr： 包含预定义格式MQTT参数的配置字符串|
|返回值|返回指向已初始化的MQTT客户端结构的指针（与输入参数相同）<br/>如果初始化失败（无效参数或上下文初始化错误）则返回NULL|
|备注|配置字符串示例：       "\vCFGMQTT_C%d\t%d\t%d\t\"%[^\"]\"\t\"%[^\"]\"\t\"%[^\"]\"\t\"%[^\"]\"\t\"%[^\"]\"\v"<br/>其中参数依次为：<br/>配置索引、AT通道、客户端索引、客户端ID、服务器地址、订阅主题、遗嘱主题、遗嘱消息。<br/>比如：<br/>"\vCFGMQTT_C1\t0\t0\t\"client id0\"\t\"tcp://117.131.85.142:60022\"\t\"cmd_topic\"\t\"will_topic_test0\"\t\"will_msg_test0\"\v"|

## 1.2 订阅MQTT主题 sam_mqtt_subscribe_topic

|接口|uint8 sam_mqtt_subscribe_topic(TMqttTag * pmqtt, char *pTopic);|
|---|---|
|功能|订阅MQTT主题，将指定主题添加到MQTT上下文中进行订阅|
|参数|pmqtt 指向MQTT客户端结构(TMqttTag)的指针<br/>pTopic 指向表示要订阅主题的字符串的指针|
|返回值|返回值表示订阅状态<br/>1 - 订阅主题添加成功<br/>0 - 失败(输入无效，或已存在订阅主题)|
|备注|订阅主题时候如果返回0，则订阅失败，如果仍需订阅，则需要重新调用接口进行订阅|

## 1.3 发布主题消息sam_mqtt_publish_message

|接口|uint8  sam_mqtt_publish_message(TMqttTag * pmqtt, char *pTopic, char *pMsg);|
|---|---|
|功能|发布MQTT消息,将MQTT消息加入发布队列|
|参数|pmqtt 指向MQTT客户端结构(TMqttTag)的指针<br/>pTopic 指向表示发布主题的空终止字符串的指针<br/>pMsg 指向表示发布消息载荷的空终止字符串的指针|
|返回值|返回值表示操作状态：<br/>1 - 消息成功加入发布队列<br/>0 - 失败|
|备注|该接口将消息体添加到发布队列，实际消息传输通过MQTT处理循环异步执行|

## 1.4 获取mqtt连接状态sam_mqtt_get_connection_status

|接口|uint8  sam_mqtt_get_connection_status(TMqttTag * pmqtt);|
|---|---|
|功能|检查MQTT连接状态，确定MQTT客户端当前是否已连接到代理服务器|
|参数|pmqtt 指向MQTT客户端结构(TMqttTag)的指针|
|返回值| 0 ：结束成功<br/>  -1 ：结束失败|
|备注|无|

## 1.5 注册回调函数sam_mqtt_set_receive_callback

|接口| Void sam_mqtt_set_receive_callback(TMqttTag * pmqtt, sam_mqtt_receive_data_cb cb);|
|---|---|
|功能|设置MQTT消息接收回调函数,用于客户端接收已经订阅的消息,当MQTT客户端接收到新消息时将调用该函数。|
|参数|pmqtt 指向MQTT客户端结构(TMqttTag)的指针<br/>cb 指向sam_mqtt_receive_data_cb类型回调函数的指针|
|返回值|无|
|备注|1、回调函数类型定义：<br/>typedef void (* sam_mqtt_receive_data_cb)(struct TMqttTag * pmqtt, uint8 index, char *topic, char *pMsg);<br/>2、该注册函数调用会覆盖任何先前设置的回调函数。|

## 1.6 关闭当前MQTT连接sam_mqtt_close

|接口|uint8  sam_mqtt_close(TMqttTag * pvmqtt);|
|---|---|
|功能|请求关闭MQTT连接，通过设置MQTT客户端结构中的关闭请求标志来启动与MQTT代理的断开连接过程|
|参数|pvmqtt 指向MQTT客户端结构(TMqttTag)的指针|
|返回值|uint8类型 返回值表示操作状态：<br/>1 - 连接关闭请求成功发起<br/>0 - 失败(客户端未连接或输入参数无效)|
|备注|无|

# 2  MQTT接口使用示例介绍

本章节主要介绍应用程序如何调用mqtt的接口api进行数据的收发。应用程序首先同通过main函数调用接口进行mqtt的初始化，然后再调用接口进行数据的收发业务。

mqtt的初始化，主要包括AT通道、客户端索引、客户端ID、服务器地址、初始订阅主题、遗嘱主题、遗嘱消息等内容的初始化

mqtt数据的收发业务主要包含mqtt消息的订阅、发布、消息接收等

## 2.1  Main函数入口

MQTT的初始化和其他模块的初始化一样，都在while循环之前TesterInit( )里面调用(参考以下两段代码)，对应的api接口是mqtt_demo_init。MQTT初始化之后，MQTT的client会根据初始化配置字符串提供的服务器地址，clientid等信息，发起mqtt连接。同时会配置好遗嘱消息以及初始订阅的topic。Client和MQTT的连接状态可以根据sam_mqtt_get_connection_status判断。参考1.4章节的介绍。

main函数入口：
```c
int main(void)
{
    uint8 KeepRun;
    
 	printf("Begin Running VMCU Tester. %s %s\r\n",__DATE__, __TIME__);
 	
#ifdef WIN_COM_SELECT
	Win32_COM_Select();

 	if(SysInitUart(PortNumUsed, 115200) == RETCHAR_FALSE)
 	{
 		return(0);	
 	}
#else
	if(SysInitUart(ATC_COM, 115200) == RETCHAR_FALSE)
	{
		return(0);	
	}
#endif
	
	InitCom(UART_TA);
	TesterInit( ); //各个模块的初始化
	CmdBp = 0;
	KeepRun = 0xFF;
	while(KeepRun)
	{
		ComDrvPoll( );
		
		TesterProc( ); //各个模块的实例调用
		
		if(CmdBp > 3)
		{
			if(Strsearch(KeyBuffer, "STOP") == 1)
			{
				break;
			}
			else if(Strsearch(KeyBuffer, "DBG:") == 1)
			{
				
			}
		}
		Sleep(5);
	}
    
    CloseHandle(hSerial);
    return 0;
}

```
mqtt demo的初始化：
```c
void TesterInit(void)
{
    SamMdmSrvStart( );

#ifdef SAM_SOCKET_TEST
    newTcpClient(0, "117.131.85.142", 60044);
#endif /* SAM_SOCKET_TEST */
#ifdef SAM_MQTT_TEST
    mqtt_demo_init();  //mqtt demo的初始化
#endif
#ifdef SAM_TTS_TEST
    sam_demo_tts_init();
#endif
#ifdef SAM_AUDIO_TEST
    sam_demo_audio_init();
#endif
#ifdef SAM_FOTA_TEST
//    fotaStart1(1, "47.109.101.196:5050/SIMTEST/hjy/test2.bin", "SIMCOM", "simcom");
#endif /* SAM_FOTA_TEST */
	
}
```

初始化完成后，如果应用想执行MQTT的相关业务，比如发布消息，订阅消息或者接收消息，可以在While循环体内部执行，开发者可根据自己的逻辑设计。参考示例程序是统一放在TesterProc函数内部。MQTT的示例可以参考mqtt_demo_client_run的实现,参考以下代码实现。


mqtt 数据收发实例：
```c
void TesterProc(void)
{    
    SamMdmSevice( );    

#ifdef SAM_SOCKET_TEST
    testTcpClient();
#endif /* SAM_SOCKET_TEST */

#ifdef SAM_MQTT_TEST
    mqtt_demo_client_run();  //mqtt demo的收发数据示例
#endif
#ifdef SAM_TTS_TEST
    sam_demo_tts_proc();
#endif
#ifdef SAM_AUDIO_TEST
    sam_demo_audio_proc();
#endif

}

```

## 2.2  MQTT的初始化示例

Mqtt的初始化示例可以参考可以参考SamMqttSrv.c中mqtt_demo_init的函数实现，主要是根据配置的字符串去初始化mqttclient对象,可通过mqtt的api接口sam_mqtt_init去初始化，返回一个指向mqttclient对象的指针，初始化的目的就是初始化一个或者多个可用的mqttclient对象实例。后续所有mqtt的接口操作都是针对某一个mqttclient进行的操作。当前实例支持2个mqtt的client同时运行。

mqtt_demo_init的函数实现，client1 和 client2的初始化：
```c
/**
 * @brief Initialization of MQTT Mode.
 */
void mqtt_demo_init(void)
{
	TMqttTag *pmqtt = NULL;
	printf("mqtt_demo sam_mqtt_init enter\n");
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

```
重点关注一下mqtt client初始化需要用到的配置字符串的格式，对应sam_mqtt_init接口的第二个参数。

sam_mqtt_init接口的注释也有详细说明。以“\v”开头，以“\v”结束，中间每个字段的字符串配置以“\t”字符间隔，分别对应config index, AT channel, client index, client ID, server address,subscription topic, will topic, will message. 以以下配置为例

```c
@brief Initialization Configuration for MQTT Client 1.
char Mqtt1605CfgStr1[] ="\vCFGMQTT_C1\t0\t0\t\"client id0\"\t\"tcp://117.131.85.142:60022\"\t\"cmd_ topic\"\t\"wi11 topic_test0\"\t\"wi11_msg_test0\"\v";
void *pMqttClient1 = NULL;
```

解析出来的各个字段的值如下：

- config index为1，作为当前配置字符串的标识；
- at channel为0，用于指定当前mqttclient使用哪个at通道与模组进行通信,接口功能最终都是通过at指令实现的，所以要配置一个通道号。当前默认只有一个at通道；
- client index为0，对应示例中的mqttclient1. 这个值的取值范围是0~1
- Client id为“client id0”，注意双引号要用转义字符表示
- server address为：“tcp://117.131.85.142:60022”，同样，双引号要用转义字符。117.131.85.142 是mqtt服务器的地址，60022 是mqtt服务器的端口号。应用程序可以根据需要自行配置。
- subscription topic 为“cmd_topic”,初始化订阅一个topic为cmd_topic消息。允许应用程序在初始化的时候预先配置一个订阅的topic，mqtt的client在连接上服务器后会自动订阅这个topic的消息。
- will topic为“will_topic_test0”,mqtt的遗嘱消息主题。
- will message为“will_msg_test0”，mqtt的遗嘱消息内容。

备注：

所有配置都需要mqtt功能模块内核转化成at指令去实现，因此都是异步实现的，不过应用程序不需要关心内部实现机制，只需要配置参数即可。

以上字符串的配置随着业务的变化可进行功能扩展。

如果应用程序只需要一个client，那只需要初始化一个client即可。

## 2.3  MQTT的数据收发示例

MQTT的client初始化完成后，就可以进行mqtt消息的订阅和发布以及数据接收等操作了。如上文描述，在SamMqttSrv.c中mqtt_demo_client_run 是一个简单的数据收发示例，应用程序可以以此为参考，根据自己的业务逻辑去设计mqtt的应用。

mqtt_demo_client_run主要包含三部分示例内容：

1、针对每一个client，进行mqtt消息的订阅，示例中订阅了一个主题为“sub_test_topic”的消息

2、针对每一个client，注册回调函数，用于接收已订阅的mqtt消息.示例中注册了一个回调函mqtt_demo_submsg_receive_cb

3、针对每一个client，进行mqtt消息的发布，示例中演示的操作是每间隔10S进行一次消息的发布。

### 2.3.1  mqtt消息的订阅和接收示例

参考函数mqtt_demo_client_run，调用sam_mqtt_set_receive_callback设置回调函数用于接收一订阅的数据，函数原型和回调函数类型请参考1.5章节。调用sam_mqtt_subscribe_topic订阅主题，函数原型请参考1.2章节。需要注意的是，回调函数的设置和主题消息的订阅只需要执行一次即可，由于整个逻辑是在一个while循环中执行的，所以这里要做一定的限制。并且需要判断一下当前mqtt和服务器的连接状态是否正常.
<br/>

mqtt 订阅主题和消息接收：
```c
TMqttTag *pclient1 = (TMqttTag *)pMqttClient1;
TMqttTag *pclient2 = (TMqttTag *)pMqttClient2;

// Set receive callback for client1 once connected (only once)
if(NULL != pclient1 && 0 == c1_sub_cb_set_flag && 1 == sam_mqtt_get_connection_status(pclient1))
{
	 sam_mqtt_set_receive_callback(pclient1, mqtt_demo_submsg_receive_cb); //client1 注册回调函数
	 c1_sub_cb_set_flag = 1;
}

// Set receive callback for client2 once connected (only once)
if(NULL != pclient2 && 0 == c2_sub_cb_set_flag && 1 == sam_mqtt_get_connection_status(pclient2))
{
	 sam_mqtt_set_receive_callback(pclient2, mqtt_demo_submsg_receive_cb); //client2 注册回调函数
	 c2_sub_cb_set_flag = 1;
}	

// Subscribe client1 to test topic once connected (only once)
if(NULL != pclient1 && 0 == c1_sub_flag && 1 == sam_mqtt_get_connection_status(pclient1))
{
	 sam_mqtt_subscribe_topic(pclient1, "sub_test_topic");// client1 订阅主题
	 c1_sub_flag = 1;
}

// Subscribe client2 to test topic once connected (only once)
if(NULL != pclient2 && 0 == c2_sub_flag && 1 == sam_mqtt_get_connection_status(pclient2))
{
	 sam_mqtt_subscribe_topic(pclient2, "sub_test_topic");// client2 订阅主题
	 c2_sub_flag = 1;
}	
```



以下是回调函数的示例，mqtt收到订阅消息后可以根据自己的应用业务做相应的处理。示例中仅仅是将收到的数据打印了出来，并在收到topic为“sub_test_topic”的消息后，对当前client进行的close操作，调用了api接口sam_mqtt_close。close后，mqttclient主动断开mqtt服务器的连接，意味着当前mqttclient的业务不再需要了。index参数可用于区分是哪个client收到的订阅消息。参数pmqttobj是当前收到订阅消息的mqttclient对象的指针，如果后续进行mqtt消息的收发或者其他业务操作需要这个指针做参数。

<br/>

mqtt接收消息回调函数实现：
```c
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
```


备注：多个client的情况下，可以注册同一个回调函数，也可以注册不同的回调函数。

### 2.3.2  mqtt消息的发布示例

参考函数mqtt_demo_client_run，调用sam_mqtt_publish_message进行消息的发布，函数原型请参考1.3章节。示例中针对每一个client，每隔10S进行了一次消息的发布。为方便调试验证，发布的消息内容包含了当前client字符和消息的index。如果另外一个客户端订阅了该topic，示例中是“pub_test_topic”，每间隔10S左右就会收到对应的消息。
<br/>

mqtt消息的发布：
```c
// Every 10 seconds, publish messages and check publication limits
	if(stim >= 10)
	{
	    if(NULL != pclient1 && 1 == sam_mqtt_get_connection_status(pclient1))
    	{
			char pub_msg[30] = {0};
		    sprintf(pub_msg, "client1_pub_msg%u", c1_pub_cnt);
			sam_mqtt_publish_message(pclient1, "pub_test_topic", pub_msg);// client1 发布消息
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
			sam_mqtt_publish_message(pclient2, "pub_test_topic", pub_msg);// client2 发布消息
			printf("mqtt_demo  client2 pub_test_topic:%s \r\n",pub_msg);
			c2_pub_cnt ++;
			if(c2_pub_cnt > 1000)
			{
				c2_pub_cnt = 0;
			}
    	}
		stim = 0;
	}
```


备注：需要注意的是，这里只是提出发布消息的请求给mqtt内核实现，最终是通过调用at指令给模组实现的，实现机制是异步的。这里的时间间隔也是应用程序提出发布请求的时间间隔。订阅此消息的client收到消息的时间间隔不一定是严格的10S。

# 3  MQTT 的示例运行结果演示

验证示例运行结果的前提条件：

1、配置好mqtt的服务器端，并确保服务器是正常运行状态，将服务器地址端口号在mqtt的初始化字符串中配置好。

2、准备好另外一个可以进行收发数据的mqtt的客户端，连接同一个服务器。提前订阅好“pub_test_topic”主题消息。以下以MQTTX工具演示.

编译运行，根据log可以看到连接成功后客户端开始发布消息（参考图1），客户端工具MQTTX工具一直收到mqttclient1和mqttclient2发布的消息，间隔大约10S以上（参考图2）。客户端MQTTX 发布topic为sub_test_topic的消息（参考图3），mqttclient1和mqttclient2收到后打印出来并执行了colse的动作（参考图4）。

<br/>
图1 连接成功后mqtt客户端开始发布消息：
<br/>
<br/>
<img src="mqtt_img1.png" height="500">


<br/>
图2 客户端工具MQTTX 收到mqttclient1和mqttclient2发布的消息：
<br/>
<br/>
<img src="mqtt_img2.png" height="500">


<br/>
图3 客户端MQTTX 发布topic为sub_test_topic的消息：
<br/>
<br/>
<img src="mqtt_img3.png" height="500">


<br/>
图4 mqttclient1和mqttclient2收到订阅的topic为sub_test_topic的消息：
<br/>
<br/>
<img src="mqtt_img4.png" height="300">

[<- 返回主目录](../README_cn.md)
