[<- 返回主目录](../README_cn.md)

<img src="simcom_logo.png" style="background-color:rgb(251, 252, 252); padding: 5px;" width="100">

# SAM_ATCDRV的应用功能FOTA处理单元

LTE模组

芯讯通无线科技(上海)有限公司  
上海市长宁区临虹路289号3号楼芯讯通总部大楼  
电话：86-21-31575100  
技术支持邮箱：[support@simcom.com](https://cn.simcom.com/online_questions.html)  
官网：[www.simcom.com](https://www.simcom.com)

|名称：|FOTA接口使用说明|
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

FOTA（Firmware Over-the-Air）子模块提供固件空中升级功能，支持从远程服务器检查固件更新、下载固件、验证固件完整性及安装固件的完整流程。该模块通过回调机制实时反馈升级进度和状态，便于用户监控升级过程，适用于需要远程更新设备固件的场景。

# 二、头文件包含

使用 FOTA 子模块前，需包含核心头文件：

``` c
#include "SamFota.h"
```

# 三、核心数据结构与枚举类型

## 3.1 枚举类型

### 3.1.1 `Sam_Fota_State_t`（FOTA 状态）

定义 FOTA 升级过程中的所有状态：

```c
typedef enum {
FOTA_STATE_IDLE,            /**< 空闲状态，等待升级指令 */
FOTA_STATE_DOWNLOADING,     /**< 固件下载中 */
FOTA_STATE_INSTALLING,      /**< 固件安装中 */
FOTA_STATE_SUCCESS,         /**< 升级成功 */
FOTA_STATE_FAILED,          /**< 升级失败 */
} Sam_Fota_State_t;
```

### 3.1.2 `Sam_Fota_Error_t`（FOTA 错误码）

定义升级过程中可能出现的错误类型：

```c
typedef enum {
FOTA_ERROR_NONE,            /**< 无错误 */
FOTA_ERROR_NETWORK,         /**< 网络错误（如连接失败、超时） */
FOTA_ERROR_DOWNLOAD,        /**< 下载错误（如下载中断、数据不完整） */
FOTA_ERROR_CHECKSUM,        /**< 校验和错误（固件完整性验证失败） */
FOTA_ERROR_INSTALL,         /**< 安装错误（如固件不兼容、写入失败） */
FOTA_ERROR_STORAGE,         /**< 存储错误（如存储空间不足） */
FOTA_ERROR_TIMEOUT,         /**< 操作超时 */
FOTA_ERROR_ABORTED,         /**< 升级被手动中止 */
} Sam_Fota_Error_t;
```

## 3.2 回调函数类型

### 3.2.1 `Sam_Fota_Progress_Callback_t`（进度回调）

用于实时反馈升级进度：

```c
typedef void (*Sam_Fota_Progress_Callback_t)(uint8_t progress, void* context);
```

**参数**：

- `progress`：升级进度（0-100，百分比）。
- `context`：用户上下文（初始化时传入）。

### 3.2.2 `Sam_Fota_Status_Callback_t`（状态回调）

用于通知升级状态变化及错误信息：

```c
typedef void (*Sam_Fota_Status_Callback_t)(Sam_Fota_State_t state, Sam_Fota_Error_t error, void* context);
```

**参数**：

- `state`：当前 FOTA 状态（见 `Sam_Fota_State_t`）。

- `error`：错误码（状态为 `FOTA_STATE_FAILED` 时有效）。

- `context`：用户上下文。

## 3.3 核心数据结构

### 3.3.1 `Sam_Fota_Config_t`（FOTA 配置）

定义 FOTA 升级的基础配置参数：

``` c
typedef struct {
    uint8_t atChannelId;                        /**< AT channel ID */
    uint8_t channel;                            /**< 0–5 means the channel numbe */
    Sam_Fota_Mode_t mode;               /**< download mode */
    char serverUrl[FOTA_MAX_URL_LENGTH+1];      /**< Server URL for firmware download */
    char username[FOTA_MAX_USERNAME_LENGTH+1];      /**< User name for firmware download */
    char password[FOTA_MAX_PASSWORD_LENGTH+1];      /**< password for firmware download */
} Sam_Fota_Config_t;
```

### 3.3.2 `Sam_Fota_t`（FOTA 模块实例）

FOTA 模块的核心结构体，包含状态、配置、回调及内部数据：

```c
typedef struct Sam_Fota_t {
    Sam_Mdm_Base_t base;          /**< Inherit from the base class */
    Sam_Mdm_Atc_t *phatc;         /**< Pointer to AT command handler */
    Sam_Fota_Config_t config;     /**< FOTA configuration */
    
    Sam_Fota_Progress_Callback_t progressCallback; /**< Progress callback */
    Sam_Fota_Status_Callback_t statusCallback;     /**< Status callback */
    void* context;                  /**< Callback context */

    Sam_Fota_Error_t error;         /**< Current error code */
    uint8_t progress;               /**< Download progress (0-100) */

    uint8 runlink; //for run link in atclink  
    Sam_Fota_Adiff_t adiff;
    
    /* Methods */
    bool (*init)(struct Sam_Fota_t* self, const char* cfgstr);
    bool (*deinit)(struct Sam_Fota_t* self);
    Sam_Fota_State_t (*getState)(struct Sam_Fota_t* self);
    uint8_t (*getProgress)(struct Sam_Fota_t* self);
    Sam_Fota_Error_t (*getError)(struct Sam_Fota_t* self);
    bool (*setCallbacks)(struct Sam_Fota_t* self, 
                        Sam_Fota_Progress_Callback_t progressCb, 
                        Sam_Fota_Status_Callback_t statusCb, 
                        void* context);

} Sam_Fota_t;
```

# 四、核心接口函数

## 4.1 模块初始化与销毁

### 4.1.1 `Sam_Fota_Create`（创建 FOTA 实例）

|接口|`Sam_Fota_t* Sam_Fota_Create(const Sam_Fota_Config_t* config);`|
|:---:|:---|
|功能|分配内存并初始化 FOTA 模块实例。|
|参数|<ul><li>`config`：FOTA 配置参数（可为 `NULL`，使用默认配置）。</li></ul>|
|返回值|<ul><li>成功：返回 FOTA 实例指针。</li><li>失败：返回 `NULL`（内存分配失败）。</li></ul>|

### 4.1.2 `Sam_Fota_Init`（初始化 FOTA 实例）

|接口|`bool Sam_Fota_Init(struct Sam_Fota_t* self, const Sam_Fota_Config_t* config);`|
|:---:|:---|
|功能|初始化 FOTA 实例的配置、AT 通道及内部状态。|
|参数|<ul><li>`self`：FOTA 实例指针。</li><li>`config`：FOTA 配置参数。</li></ul>|
|返回值|<ul><li>成功：`true`。</li><li>失败：`false`（参数无效或 AT 通道初始化失败）。</li></ul>|

### 4.1.3 `Sam_Fota_Deinit`（反初始化 FOTA 实例）

|接口|`bool Sam_Fota_Deinit(struct Sam_Fota_t* self);`|
|:---:|:---|
|功能|释放 FOTA 实例占用的资源，注销回调。|
|参数|<ul><li>`self`：FOTA 实例指针。</li></ul>|
|返回值|<ul><li>成功：`true`。</li><li>失败：`false`（参数无效）。</li></ul>|

### 4.1.4 `Sam_Fota_Destroy`（销毁 FOTA 实例）

|接口|`void Sam_Fota_Destroy(Sam_Fota_t* socket);`|
|:---:|:---|
|功能|彻底销毁 FOTA 实例，释放内存。|
|参数|<ul><li>`self`：FOTA 实例指针。</li></ul>|

## 4.2 升级流程控制

### 4.2.1 `Sam_Fota_SetCallbacks`（设置回调函数）

|接口|`bool Sam_Fota_SetCallbacks(struct Sam_Fota_t* self,Sam_Fota_Progress_Callback_t progressCb,Sam_Fota_Status_Callback_t statusCb,void* context);`|
|:---:|:---|
|功能|注册进度回调和状态回调函数。|
|参数|<ul><li>`self`：FOTA 实例指针。</li><li>`progressCb`：进度回调函数（可为 `NULL`）。</li><li>`statusCb`：状态回调函数（可为 `NULL`）。</li><li>`context`：用户上下文（回调时透传）。</li></ul>|
|返回值|<ul><li>成功：`true`。</li><li>失败：`false`（参数无效）。</li></ul>|

### 4.2.2 `Sam_Fota_GetState`（获取当前状态）

|接口|`Sam_Fota_State_t Sam_Fota_GetState(struct Sam_Fota_t* self);`|
|:---:|:---|
|功能|查询当前 FOTA 升级状态。|
|返回值|当前状态（`Sam_Fota_State_t` 枚举值）。|

### 4.3.4 `Sam_Fota_GetError`（获取错误码）

|接口|`Sam_Fota_Error_t Sam_Fota_GetError(struct Sam_Fota_t* self);`|
|:---:|:---|
|功能|查询最近一次错误的错误码。|
|返回值|错误码（`Sam_Fota_Error_t` 枚举值）。|

# 五、应用场景例程

## 5.1 基础 FOTA 升级流程

### 例程位置

`SamFotaSrv.c` 中包含 FOTA 升级的完整示例。

### 代码示例

```c
// 1. 定义回调函数

void FotaProgressCallback(uint8_t progress, void* context) {

printf("FOTA Progress: %d%%\n", progress);

}

void FotaStatusCallback(Sam_Fota_State_t state, Sam_Fota_Error_t error, void* context) {

printf("FOTA State: %d, Error: %d\n", state, error);
Sam_Fota_t* fota = (Sam_Fota_t*)context;

// 处理状态变化

switch (state) {
    case FOTA_STATE_SUCCESS:
        printf("FOTA upgrade success! Rebooting...\n");
        // 升级成功，可触发设备重启
        break;

    case FOTA_STATE_FAILED:

        printf("FOTA upgrade failed! Error: %d\n", error);
        // 处理失败逻辑（如记录日志、重试）
        break;

    default:
        break;
}
}

// 2. 初始化 FOTA 模块并执行升级

void fotaStart1(Sam_Fota_Mode_t mode, char *url, char *username, char *password)
{
    Sam_Fota_Config_t config = {0};
    config.atChannelId = 0;
    config.channel = 0;
    config.mode = mode;
    strcpy(config.serverUrl, url);
    strcpy(config.username, username);
    strcpy(config.password, password);

    
    // create fota instance using create or malloc
    fota = Sam_Fota_Create(&config);
    if (fota == NULL)
    {
        printf("Failed to create fota\r\n");
        return;
    }

    Sam_Fota_SetCallbacks(fota, fotaProcessCallback, fotaStateCallback, (void *)fota);
}
```

### 流程说明

1. **配置初始化**：设置固件服务器 URL、超时时间、重试次数等参数。
2. **实例创建**：通过 `Sam_Fota_Create` 和 `Sam_Fota_Init` 初始化 FOTA 模块。
3. **回调注册**：注册进度和状态回调，实时监控升级过程。
4. **升级触发**：调用 `Sam_Fota_CheckUpdate` 开始检查更新，后续流程（下载、验证、安装）通过回调自动触发（或手动调用接口）。

[<- 返回主目录](../README_cn.md)
