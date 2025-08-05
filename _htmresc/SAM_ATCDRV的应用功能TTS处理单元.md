[<- 返回主目录](../README_cn.md)

<img src="simcom_logo.png" style="background-color:rgb(251, 252, 252); padding: 5px;" width="100">

# TTS接口使用说明V1.00
芯讯通无线科技(上海)有限公司  
上海市长宁区临虹路289号3号楼芯讯通总部大楼  
电话：86-21-31575100  
技术支持邮箱：support@simcom.com  
官网：www.simcom.com

|名称：|TTS接口使用说明|
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
官网：www.simcom.com

了解更多资料，请点击以下链接：  
http://cn.simcom.com/download/list-230-cn.html  

技术支持，请点击以下链接：  
http://cn.simcom.com/ask/index-cn.html 或发送邮件至support@simcom.com  

版权所有 © 芯讯通无线科技(上海)有限公司2022，保留一切权利。

# 版本历史
|版本|日期|作者|备注|
|---|---|---|---|
|V1.00|2025-7-23| |第一版|

# 文档介绍
本文档介绍了基于VMCU框架TTS的接口定义，以及TTS的接口使用方法和示例程序，指导客户如何在mcu上位机调用模组（比如A7670SA）的TTS业务开发TTS的相关业务。

# 目录
- [版权声明](#版权声明)
- [版本历史](#版本历史)
- [文档介绍](#文档介绍)
- [目录](#目录)
- [1 TTS 接口 API介绍](#1-tts-接口-api介绍)
  - [1.1 TTS功能初始化sam_tts_init](#11-tts功能初始化sam_tts_init)
  - [1.2 TTS去初始化 sam_tts_deinit](#12-tts去初始化-sam_tts_deinit)
  - [1.3 获取TTS工作状态sam_tts_get_status](#13-获取tts工作状态sam_tts_get_status)
  - [1.4 停止TTS播放sam_tts_stop_playing](#14-停止tts播放sam_tts_stop_playing)
  - [1.5 播放TTS语音sam_tts_play](#15-播放tts语音sam_tts_play)
  - [1.6 播放TTS语音并将数据保存到wav格式的文件中sam_tts_play_and_save_wav](#16-播放tts语音并将数据保存到wav格式的文件中sam_tts_play_and_save_wav)
  - [1.7 获取YOUNGTONE TTS库相关的参数sam_tts_get_YOUNGTONE_param](#17-获取youngtone-tts库相关的参数sam_tts_get_youngtone_param)
  - [1.8 设置YOUNGTONE TTS库相关的参数sam_tts_set_YOUNGTONE_param](#18-设置youngtone-tts库相关的参数sam_tts_set_youngtone_param)
  - [1.9 设置IFLY TTS库相关的参数sam_tts_set_IFLY_param](#19-设置ifly-tts库相关的参数sam_tts_set_ifly_param)
  - [1.10 获取IFLY TTS库相关的参数sam_tts_get_IFLY_param](#110-获取ifly-tts库相关的参数sam_tts_get_ifly_param)
  - [1.11 获取TTS是本端播放还是远端播放sam_tts_get_local_or_remote_status](#111-获取tts是本端播放还是远端播放sam_tts_get_local_or_remote_status)
  - [1.12 设置TTS是本端播放还是远端播放sam_tts_set_local_or_remote_status](#112-设置tts是本端播放还是远端播放sam_tts_set_local_or_remote_status)
  - [1.13 获取系统音量是否可设置sam_tts_get_sys_vol_setting_status](#113-获取系统音量是否可设置sam_tts_get_sys_vol_setting_status)
  - [1.14 设置系统音量是否可设置sam_tts_set_sys_vol_setting10](#114-设置系统音量是否可设置sam_tts_set_sys_vol_setting10)
- [2 TTS接口使用示例介绍](#2-tts接口使用示例介绍)
  - [2.1 Main函数入口](#21-main函数入口)
  - [2.2 播放UCS2数据编码格式的TTS语音](#22-播放ucs2数据编码格式的tts语音)
  - [2.3 播放GBK数据格式的TTS语音](#23-播放gbk数据格式的tts语音)
  - [2.4 播放ASCII编码格式的语音并保存语音](#24-播放ascii编码格式的语音并保存语音)
  - [2.5 设置YOUNGTONE TTS库参数](#25-设置youngtone-tts库参数)
  - [2.6 设置IFLY TTS库参数](#26-设置ifly-tts库参数)
- [3 TTS函数调用流程图](#3-tts函数调用流程图)

## 1 TTS 接口 API介绍
VMCU框架提供了一组TTS接口，用于处理TTS相关业务，通过相关接口调用可实现文本转换为音频输出的功能。

TTS接口定义在头文件SamTTS.h中，使用时需包含该文件。

### 1.1 TTS功能初始化sam_tts_init
|接口|int sam_tts_init(uint8 atcIndex,sam_tts_callback ttsCallback,sam_tts_urc_callback urcTTSCallback);|
|---|---|
|功能|TTS模块初始化函数，调用以下任意函数时，首先需要将TTS模块注册到系统中。此函数只需要调用一次。但是调用void sam_tts_deinit(void)函数后，如果还需要使用TTS模块，则需要再次调用此函数进行初始化。|
|参数|atcIndex： 选用哪一个串口通道，一般将此参数传入0。<br/>ttsCallback： 此参数传入一个回调函数指针，就是执行1.3至1.14里面的任意函数时，返回1.3至1.14函数的执行结果。<br/>urcTTSCallback： 此参数传入一个回调函数的指针，就是执行3至14里面的任意函数时，如果1.3至1.14里面的函数有URC上报，会通过此回调函数输出。|
|返回值|0：表示执行成功。<br/>-1：表示执行失败，请检查是否传入正确的参数。|
|备注|无|

### 1.2 TTS去初始化 sam_tts_deinit
|接口|void sam_tts_deinit(void);|
|---|---|
|功能|用于注销TTS模块。|
|参数|无|
|返回值|无|
|备注|无|

### 1.3 获取TTS工作状态sam_tts_get_status
|接口|int sam_tts_get_status(void);|
|---|---|
|功能|此函数用于获取TTS的工作状态，会通过回调函数返回TTS当前的状态是正在播放TTS，还是没有播放TTS。|
|参数|无|
|返回值|0：表示执行成功。<br/>-1：表示有其他的TTS任务正在执行。|
|备注|无|

### 1.4 停止TTS播放sam_tts_stop_playing
|接口|int sam_tts_stop_playing(void);|
|---|---|
|功能|停止TTS的播放|
|参数|无|
|返回值|0：表示执行成功<br/>-1：表示有其他的TTS任务正在执行|
|备注|无|

### 1.5 播放TTS语音sam_tts_play
|接口|int sam_tts_play(uint8 *pData,uint16 dataSize,TTS_PLAYING_DATA_FORMAT_E format);|
|---|---|
|功能|播放TTS语音。|
|参数|pData：如果数据是ASCII编码格式，中文是GBK编码格式，两种格式单独存在，或者混合存在，数据的最大长度是512字节（包括两个引号），如果是UCS2编码格式，包括引号在内，最大长度是510字节。<br/>dataSize：传入的pData数据的长度。<br/>format：pData所指的数据的编码格式。|
|返回值|0：表示执行成功。<br/>-1：请检查传入的参数是否正确。|
|备注|无|

### 1.6 播放TTS语音并将数据保存到wav格式的文件中sam_tts_play_and_save_wav
|接口|int sam_tts_play_and_save_wav(uint8 *pData,uint16 dataSize,char *fileName,TTS_PLAYING_DATA_FORMAT_E format);|
|---|---|
|功能|播放TTS语音并将数据保存到wav格式的文件中。|
|参数|pData：如果数据是ASCII编码格式，中文是GBK编码格式，两种格式单独存在，或者混合存在，数据的最大长度是50字节。TOUNGTONE TTS：中文多音字编码格式，多音字<pinyin=发音读音+声调>；IFLY TTS：中文多音字编码格式，多音字[=发音读音+声调]。如果数据是UCS2编码数据，数据的最大长度是50字节。<br/>dataSize：传入的pData数据的长度。<br/>fileName：输入路径和文件名，如果不指定路径，默认保存在C:盘，最大文件名长度为60字节，当前仅仅支持.wav文件名后缀。<br/>format：pData所指的数据的编码格式。|
|返回值|0：表示执行成功。<br/>-1：请检查传入的参数是否正确。|
|备注|无|

### 1.7 获取YOUNGTONE TTS库相关的参数sam_tts_get_YOUNGTONE_param
|接口|int sam_tts_get_YOUNGTONE_param(void);|
|---|---|
|功能|获取YOUNGTONE TTS库相关的参数，此函数会通过sam_tts_init设置的回调函数返回参数值。|
|参数|无|
|返回值|0：表示执行成功<br/>-1：表示有其他的TTS任务正在执行|
|备注|无|

### 1.8 设置YOUNGTONE TTS库相关的参数sam_tts_set_YOUNGTONE_param
|接口|int sam_tts_set_YOUNGTONE_param(TTS_param_T *pParam);|
|---|---|
|功能|设置YOUNGTONE TTS库相关的参数。|
|参数|pParam：是一个TTS_param_T结构体指针，结构体里面是一个数组：<br/> *  index 0:TTS lib volume,range (0,1,2),default:1.<br/> *  index 1:system volume,range (0,1,2,3),default:3.<br/> *  index 2:digitmode,range (0,1,2),default:0.<br/> *  index 3:pitch,range (0,1,2),default:1.<br/> *  index 4:speed,rough speed regulation,range (0,1,2),default:1;precision speed regulation,range [10...30].<br/>     *  index 5:digitreading,range (0,1),default:0.|
|返回值|0：表示执行成功<br/>-1：表示有其他的TTS任务正在执行|
|备注|无|

### 1.9 设置IFLY TTS库相关的参数sam_tts_set_IFLY_param
|接口|int sam_tts_set_IFLY_param(TTS_param_T *pParam);|
|---|---|
|功能|设置IFLY TTS库相关的参数。|
|参数|pParam是一个TTS_param_T结构体指针，结构体里面是一个数组：<br/> *  index 0:TTS lib volume,range (0,1,2),default:2.<br/> *  index 1:system volume,range (0...7),default:4.<br/> *  index 2:digitmode,range (0,1,2),default:0.<br/> *  index 3:pitch,range (0,1,2),default:1.<br/> *  index 4:speed,rough speed regulation,range (0,1,2),default:1;precision speed regulation,range [10...30].<br/>     *  index 5:ttslib,range (0,1),default:0.|
|返回值|0：表示执行成功<br/>-1：表示有其他的TTS任务正在执行|
|备注|无|

### 1.10 获取IFLY TTS库相关的参数sam_tts_get_IFLY_param
|接口|int sam_tts_get_IFLY_param(void);|
|---|---|
|功能|获取IFLY TTS库相关的参数，此函数会通过sam_tts_init设置的回调函数返回参数值。|
|参数|无|
|返回值|0：表示执行成功<br/>-1：表示有其他的TTS任务正在执行|
|备注|无|

### 1.11 获取TTS是本端播放还是远端播放sam_tts_get_local_or_remote_status
|接口|int sam_tts_get_local_or_remote_status(void);|
|---|---|
|功能|获取TTS是本端播放还是远端播放，获取的状态值通过sam_tts_init设置的回调函数返回。|
|参数|无|
|返回值|0：表示执行成功<br/>-1：表示有其他的TTS任务正在执行|
|备注|无|

### 1.12 设置TTS是本端播放还是远端播放sam_tts_set_local_or_remote_status
|接口|int sam_tts_set_local_or_remote_status(uint8 localOrRemote);|
|---|---|
|功能|设置TTS是本端播放还是远端播放。|
|参数|localOrRemote：0:Local Path,1:Remote Path。|
|返回值|0：表示执行成功<br/>-1：表示有其他的TTS任务正在执行|
|备注|无|

### 1.13 获取系统音量是否可设置sam_tts_get_sys_vol_setting_status
|接口|int sam_tts_get_sys_vol_setting_status(void);|
|---|---|
|功能|获取系统音量是否可设置，获取的状态值通过sam_tts_init设置的回调函数返回。|
|参数|无|
|返回值|0：表示执行成功<br/>-1：表示有其他的TTS任务正在执行|
|备注|无|

### 1.14 设置系统音量是否可设置sam_tts_set_sys_vol_setting10
|接口|int sam_tts_set_sys_vol_setting(uint8 sysVolSetting);|
|---|---|
|功能|设置系统音量是否可设置。|
|参数|sysVolSetting：0:volume setting is valid,1:volume setting is invalid.|
|返回值|0：表示执行成功<br/>-1：表示有其他的TTS任务正在执行|
|备注|无|

## 2 TTS接口使用示例介绍
本章节主要介绍应用程序如何调用TTS的接口实现TTS语音播放。

### 2.1 Main函数入口
TTS的初始化和其他模块的初始化一样，都在while循环之前TesterInit( )里面调用sam_tts_init()函数。

初始化完成后，如果应用想执行TTS的相关业务，比如播放TTS语音，可以在While循环体内部执行，开发者可根据自己的逻辑设计，后续章节会做进一步介绍。

### 2.2 播放UCS2数据编码格式的TTS语音
```c
uint8 *pData = "\"6B228FCE4F7F75288BED97F3540862107CFB7EDF\"";
sam_tts_play(pData,strlen(pData),TTS_PLAYING_UCS2_FORMAT);
```

### 2.3 播放GBK数据格式的TTS语音
```c
uint8 *pData = "\"去朝<pinyin=chao2>阳，看朝<pinyin=zhao1>阳\"";
sam_tts_play(pData,strlen(pData),TTS_PLAYING_ASCII_AND_GBK_OR_ONLY_GBK_FORMAT);
```

### 2.4 播放ASCII编码格式的语音并保存语音
```c
uint8 *pData = "\"hello world\"";
char *fileName = "\"C:/12.wav\"";
sam_tts_play_and_save_wav(pData,strlen(pData),fileName,TTS_PLAYING_ONLY_ASCII_FORMAT);
```

### 2.5 设置YOUNGTONE TTS库参数
```c
TTS_param_T param;
param.params[TTS_VOL] = 2;
param.params[TTS_SYS_VOL] = 3;
param.params[TTS_DIGIT_MODE] = 0;
param.params[TTS_PITCH] = 1;
param.params[TTS_SPEED] = 1;
param.params[TTS_DIGIT_READING_FOR_YOUNGTONE_OR_TTSLIB_FOR_IFLY] = 1;
sam_tts_set_YOUNGTONE_param(&param);
```

### 2.6 设置IFLY TTS库参数
```c
TTS_param_T param;
param.params[TTS_VOL] = 2;
param.params[TTS_SYS_VOL] = 3;
param.params[TTS_DIGIT_MODE] = 0;
param.params[TTS_PITCH] = 1;
param.params[TTS_SPEED] = 1;
param.params[TTS_DIGIT_READING_FOR_YOUNGTONE_OR_TTSLIB_FOR_IFLY] = 1;
sam_tts_set_IFLY_param(&mTTSTag,&param);
```

## 3 TTS函数调用流程图

<img src="tts.png" style="background-color:rgb(251, 252, 252); padding: 5px;" width="860" height="516">

[<- 返回主目录](../README_cn.md)
