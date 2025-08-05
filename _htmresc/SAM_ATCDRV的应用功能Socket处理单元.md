[<- 返回主目录](../README_cn.md)

<img src="simcom_logo.png" style="background-color:rgb(251, 252, 252); padding: 5px;" width="100">

# SAM_ATCDRV的应用功能Socket处理单元

LTE模组

芯讯通无线科技(上海)有限公司  
上海市长宁区临虹路289号3号楼芯讯通总部大楼  
电话：86-21-31575100  
技术支持邮箱：[support@simcom.com](https://cn.simcom.com/online_questions.html)  
官网：[www.simcom.com](https://www.simcom.com)

|名称：|Socket接口使用说明|
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

[TOC]

# 一、概述

本 Socket 子模块提供了一系列的接口和功能，用于处理不同类型的 socket 操作，包括 TCP、UDP、TCP Server、UDP Server 以及 SSL/TLS socket。该模块支持多种 CIP 模式和数据接收格式，同时提供了事件回调和数据回调机制，方便用户处理 socket 事件和接收数据。

# 二、头文件包含

在使用本模块的接口之前，需要包含`SamSocket.h`头文件：

```c
#include "SamSocket.h"
```

# 三、数据结构和枚举类型

## 3.1 枚举类型&#xA;

### 3.1.1 `Sam_Mdm_Socket_Type_t`

定义了不同类型的 socket：

```c
typedef enum {
    SAM_MDM_SOCKET_TYPE_TCP,    /**< TCP socket */
    SAM_MDM_SOCKET_TYPE_UDP,    /**< UDP socket */
    SAM_MDM_SOCKET_TYPE_UDP_SERVER,    /**< UDP Server socket */
    SAM_MDM_SOCKET_TYPE_TCP_SERVER,    /**< TCP Server socket */
    SAM_MDM_SOCKET_TYPE_SSL     /**< SSL/TLS socket */
} Sam_Mdm_Socket_Type_t;
```

### 3.1.2 `Sam_Mdm_Socket_Cipmode_t`

定义了不同的 CIP 模式：

```c
typedef enum {
    SAM_MDM_SOCKET_CIPMODE_NONE,    /**< CIP NonTransparent mode */
    SAM_MDM_SOCKET_CIPMODE_TRANSPARENT,    /**< CIP Transparent mode */
} Sam_Mdm_Socket_Cipmode_t;
```

### 3.1.3 `Sam_Mdm_Socket_Rxform_t`

定义了不同的数据接收格式：

``` c
typedef enum {
    SAM_MDM_SOCKET_RXFORM_RAW,    /**< Rxget Form raw data mode */
    SAM_MDM_SOCKET_RXFORM_ASCII,    /**< ASCII mode */
    SAM_MDM_SOCKET_RXFORM_HEX    /**< HEX mode */
} Sam_Mdm_Socket_Rxform_t;
```

### 3.1.4 `Sam_Mdm_Socket_State_t`

定义了 socket 的不同状态：

```c
typedef enum {
    SAM_MDM_SOCKET_STATE_INIT,    /**< Socket init */
    SAM_MDM_SOCKET_STATE_OPENING,   /**< Socket opening */
    SAM_MDM_SOCKET_STATE_CONNECTED, /**< Socket connected */
    SAM_MDM_SOCKET_STATE_SENDING,   /**< Sending data */
    SAM_MDM_SOCKET_STATE_RECEIVING, /**< Receiving data */
    SAM_MDM_SOCKET_STATE_CLOSING, /**< Socket closing */
    SAM_MDM_SOCKET_STATE_CLOSED,    /**< Socket closed */
    SAM_MDM_SOCKET_STATE_ERROR      /**< Error */
} Sam_Mdm_Socket_State_t;
```

### 3.1.5 `Sam_Mdm_Socket_Event_t`

定义了 socket 的不同事件：

```c
typedef enum {
    SAM_MDM_SOCKET_EVENT_NONE,
    SAM_MDM_SOCKET_EVENT_ACCEPT,    // TCP server accepted a new client socket
    SAM_MDM_SOCKET_EVENT_CLOSED_PASSIVE, // Closed by remote
} Sam_Mdm_Socket_Event_t;
```

## 3.2 数据结构&#xA;

### 3.2.1 `Sam_Mdm_Socket_Config_t`

定义了 socket 的配置参数：

```c
typedef struct {
    uint8_t atChannelId;           /**< AT channel ID */
    uint8_t socketId;               /**< Socket ID */
    Sam_Mdm_Socket_Cipmode_t cipmode; /**< cip mode */
    Sam_Mdm_Socket_Type_t type;     /**< Socket type */
    Sam_Mdm_Socket_Rxform_t rxform; /**< RX get form type */
    char host[64];               /**< Server host name or IP address */
    uint16_t port;                  /**< Server port */
    uint16_t localport;          /**< local port for UDP, if type is TCP server, this is listerning port */
    uint8_t srvIndex;       /**< Server Index */
} Sam_Mdm_Socket_Config_t;
```

### 3.2.2 `Sam_Mdm_Socket_t`

定义了 socket 模块的结构，包括状态、配置、回调和方法：

```c
typedef struct Sam_Mdm_Socket_t {
    Sam_Mdm_Base_t base;          /**< Inherit from the base class */
    Sam_Mdm_Atc_t *phatc;
    Sam_Mdm_Socket_Config_t config; /**< Socket module configuration */
    Sam_Mdm_Socket_Event_Callback_t eventCallback; /**< Event callback */
    Sam_Mdm_Socket_Data_Callback_t dataCallback;   /**< Data callback */
    void* context;                  /**< Callback context */
    uint32_t urcMask;
    char            upbuf[TSCM_UPBUFLEN];
    uint16_t        upcnt;
    char            dnbuf[TSCM_DNBUFLEN];
    uint16_t        dncnt;
    bool              dnflag;
    uint8_t         error;
    uint8_t         openReTryCnt;
    uint8_t         closeType; // 1-local close, 2-socket destroy
    uint8        runlink;        //for run link in atclink 

    /* Methods */
    bool (*init)(struct Sam_Mdm_Socket_t* self, const char * cfgstr);
    bool (*deinit)(struct Sam_Mdm_Socket_t* self);
    bool (*close)(struct Sam_Mdm_Socket_t* self);
    uint32_t (*send)(struct Sam_Mdm_Socket_t* self, const uint8_t* data, uint32_t length);
    uint8_t (*process)(struct Sam_Mdm_Socket_t* self);
    uint8_t (*getState)(struct Sam_Mdm_Socket_t* self);
    bool (*setUserCallback)(struct Sam_Mdm_Socket_t* self, Sam_Mdm_Socket_Event_Callback_t eventCb, Sam_Mdm_Socket_Data_Callback_t dataCb, void* context);
} Sam_Mdm_Socket_t;
```

# 四、接口函数

## 4.1 `Sam_Mdm_Socket_Create`

|接口|`Sam_Mdm_Socket_t* Sam_Mdm_Socket_Create(const Sam_Mdm_Socket_Config_t* config)`|
|:---:|:---|
|功能|创建一个新的 socket 模块实例。|
|参数|<ul><li>`config`：socket 模块的配置参数。</li></ul>|
|返回值|<ul><li>成功：返回指向新 socket 模块实例的指针。</li><li>失败：返回`NULL`。</li></ul>|

## 4.2 `Sam_Mdm_Socket_Destroy`

|接口|`void Sam_Mdm_Socket_Destroy(Sam_Mdm_Socket_t* socket);`|
|:---:|:---|
|功能|销毁一个 socket 模块实例。|
|参数|<ul><li>`socket`：指向要销毁的 socket 模块实例的指针。</li></ul>|

## 4.3 `Sam_Mdm_Socket_init`

|接口|`bool Sam_Mdm_Socket_init(struct Sam_Mdm_Socket_t* self, const char * cfgstr);`|
|:---:|:---|
|功能|初始化一个 socket 模块实例。|
|参数|<ul><li>`self`：指向要初始化的 socket 模块实例的指针。</li><li>`cfgstr`：配置字符串，格式如下：<br>`"\vCFGSCT_M1\t${atChannel}\t${atType}\t${socketId}\t${cipmode}\t${type}\t${rxform}\t${host}\t${port}\t${localport}\v"`</li></ul>|
|返回值|<ul><li>成功：返回`true`。</li><li>失败：返回`false`。</li></ul>|

## 4.4 `Sam_Mdm_Socket_process`

|接口|`uint8_t Sam_Mdm_Socket_process(struct Sam_Mdm_Socket_t* self);`|
|:---:|:---|
|功能|处理 socket 模块。|
|参数|<ul><li>`self`：指向要处理的 socket 模块实例的指针。</li></ul>|
|返回值|当前 socket 模块的状态。|

## 4.5 `Sam_Mdm_Socket_setCallback`

|接口|`bool Sam_Mdm_Socket_setCallback(struct Sam_Mdm_Socket_t* self, Sam_Mdm_Socket_Event_Callback_t eventCb, Sam_Mdm_Socket_Data_Callback_t dataCb, void* context);`|
|:---:|:---|
|功能|设置 socket 模块的回调函数。|
|参数|<ul><li>`self`：指向要设置回调函数的 socket 模块实例的指针。</li><li>`eventCb`：事件回调函数。</li><li>`dataCb`：数据回调函数。</li><li>`context`：用户上下文。</li></ul>|
|返回值|<ul><li>成功：返回`true`。</li><li>失败：返回`false`。</li></ul>|

## 4.6 `Sam_Mdm_Socket_Send`

|接口|`uint32_t Sam_Mdm_Socket_Send(struct Sam_Mdm_Socket_t* self, const uint8_t* data, uint32_t length);`|
|:---:|:---|
|功能|通过 socket 发送数据。|
|参数|<ul><li>`self`：指向要发送数据的 socket 模块实例的指针。</li><li>`data`：要发送的数据缓冲区。</li><li>`length`：数据缓冲区的长度。</li></ul>|
|返回值|实际发送的字节数。|

## 4.7 `Sam_Mdm_Socket_Close`

|接口|`bool Sam_Mdm_Socket_Close(struct Sam_Mdm_Socket_t* socket);`|
|:---:|:---|
|功能|关闭 socket。|
|参数|<ul><li>`socket`：指向要关闭的 socket 模块实例的指针</li></ul>|
|返回值|<ul><li>成功：返回`true`。</li><li>失败：返回`false`。</li></ul>|

## 4.8 `Sam_Mdm_Socket_getState`

|接口|`uint8_t Sam_Mdm_Socket_getState(struct Sam_Mdm_Socket_t* self);`|
|:---:|:---|
|功能|获取当前 socket 的状态。|
|参数|<ul><li>`self`：指向要获取状态的 socket 模块实例的指针。</li></ul>|
|返回值|当前 socket 的状态。|

# 五、使用方法示例

## 5.1 创建并初始化一个新的 socket&#xA;

```c
#include "SamSocket.h"

void create_and_init_socket() {
    Sam_Mdm_Socket_Config_t config = {0};
    config.atChannelId = 0;
    config.socketId = 0;
    config.cipmode = SAM_MDM_SOCKET_CIPMODE_NONE;
    config.type = SAM_MDM_SOCKET_TYPE_TCP;
    config.rxform = SAM_MDM_SOCKET_RXFORM_RAW;
    strcpy(config.host, "117.131.85.142");
    config.port = 60044;
    config.localport = 0;
    Sam_Mdm_Socket_t* socket = Sam_Mdm_Socket_Create(&config);

    if (socket != NULL) {
        char cfgstr[64];
        sprintf(cfgstr, "\vCFGSCT_M1\t%d\tA\t%d\t%d\t%d\t%d\t%s\t%d\t%d\v",
            config.atChannelId,
            config.socketId,
            config.cipmode,
            config.type,
            config.rxform,
            config.host,
            config.port,
            config.localport);

        if (Sam_Mdm_Socket_init(socket, cfgstr)) {
            // 初始化成功
        }
    }
}
```

## 5.2 设置回调函数&#xA;

```c
void SocketEventCallback(uint8_t socketId, Sam_Mdm_Socket_Event_t event, void *msg, void* context) {
    // 处理socket事件
}

void SocketDataCallback(uint8_t socketId, const uint8_t* data, uint32_t length, void* context) {
    // 处理接收到的数据
}

void set_callbacks(Sam_Mdm_Socket_t* socket) {
    if (socket != NULL) {
        Sam_Mdm_Socket_setCallback(socket, SocketEventCallback, SocketDataCallback, (void *)socket);
    }
}
```

## 5.3 发送数据&#xA;

```c
void send_data(Sam_Mdm_Socket_t* socket) {
    if (socket != NULL) {
        const uint8_t data[] = "Hello World!";
        uint32_t length = sizeof(data) - 1;
        uint32_t sent = Sam_Mdm_Socket_Send(socket, data, length);
        if (sent == length) {
            // 数据发送成功
        }
    }
}
```

## 5.4 关闭 socket&#xA;

```c
void close_socket(Sam_Mdm_Socket_t* socket) {
    if (socket != NULL) {
        if (Sam_Mdm_Socket_Close(socket)) {
            // 关闭成功
            Sam_Mdm_Socket_Destroy(socket);
        }
    }
}
```

# 六、注意事项

- 在使用 socket 模块之前，需要确保相关的依赖库和头文件已经正确包含。

- 配置字符串的格式必须严格按照规定的格式填写，否则可能导致初始化失败。

- 在使用完 socket 模块后，需要调用`Sam_Mdm_Socket_Destroy`函数销毁实例，以释放资源

[<- 返回主目录](../README_cn.md)
