[<- Return to the main directory](../README.md)

<img src="simcom_logo.png" style="background-color:rgb(251, 252, 252); padding: 5px;" width="100">

# TTS Interface Instructions V1.00
SIMCom Wireless Solutions Limited  
SIMCom Headquarters Building, Building 3, No. 289  
Linhong Road, Changning District, Shanghai P.R. China  
Tel: 86-21-31575100  
support@simcom.com  
www.simcom.com  

|Document Title|Instructions for using TTS interface|
|---|---|
|Version|V1.01|
|Category|Application Documents|
|Status|Released|

## GENERAL NOTES
SIMCOM OFFERS THIS INFORMATION AS A SERVICE TO ITS CUSTOMERS, TO SUPPORT APPLICATION AND ENGINEERING EFFORTS THAT USE THE PRODUCTS DESIGNED BY SIMCOM. THE INFORMATION PROVIDED IS BASED UPON REQUIREMENTS SPECIFICALLY PROVIDED TO SIMCOM BY THE CUSTOMERS. SIMCOM HAS NOT UNDERTAKEN ANY INDEPENDENT SEARCH FOR ADDITIONAL RELEVANT INFORMATION, INCLUDING ANY INFORMATION THAT MAY BE IN THE CUSTOMER鈥橲 POSSESSION. FURTHERMORE, SYSTEM VALIDATION OF THIS PRODUCT DESIGNED BY SIMCOM WITHIN A LARGER ELECTRONIC SYSTEM REMAINS THE RESPONSIBILITY OF THE CUSTOMER OR THE CUSTOMER鈥橲 SYSTEM INTEGRATOR. ALL SPECIFICATIONS SUPPLIED HEREIN ARE SUBJECT TO CHANGE.

# COPYRIGHT
THIS DOCUMENT CONTAINS PROPRIETARY TECHNICAL INFORMATION WHICH IS THE PROPERTY OF SIMCOM WIRELESS SOLUTIONS LIMITED. COPYING TO OTHERS AND USING THIS DOCUMENT ARE FORBIDDEN WITHOUT EXPRESS AUTHORITY BY SIMCOM. OFFENDERS ARE LIABLE TO THE PAYMENT OF INDEMNIFICATIONS. ALL RIGHTS RESERVED BY SIMCOM IN THE PROPRIETARY TECHNICAL INFORMATION, INCLUDING BUT NOT LIMITED TO REGISTRATION GRANTING OF A PATENT, A UTILITY MODEL OR DESIGN. ALL SPECIFICATIONS SUPPLIED HEREIN ARE SUBJECT TO CHANGE WITHOUT NOTICE AT ANY TIME.

SIMCom Wireless Solutions Limited  
SIMCom Headquarters Building, Building 3, No. 289 Linhong Road, Changning District, Shanghai P.R. China  
Tel: +86 21 31575100  
Email: simcom@simcom.com  

For more information, please visit:  
https://www.simcom.com/technical_files.html  

For technical support, or to report documentation errors, please visit:  
https://www.simcom.com/online_questions.html or email to: support@simcom.com  

Copyright 漏 2025 SIMCom Wireless Solutions Limited All Rights Reserved.

# Version History

|Version|Date|Author|Description|
|---|---|---|---|
|V1.00|2025-7-23| |Initial version release|

# Document Introduction
This document introduces the interface definition based on VMCU framework TTS, as well as the interface usage method and sample program of TTS, guiding customers on how to call the TTS business of modules (such as A7670SA) on the MCU upper computer to develop TTS related business.

# Contents
- [COPYRIGHT](#copyright)
- [Version History](#version-history)
- [Document Introduction](#document-introduction)
- [Contents](#contents)
- [1 Introduction to TTS API Interface](#1-introduction-to-tts-api-interface)
  - [1.1 TTS function initialization sam_tts_init](#11-tts-function-initialization-sam_tts_init)
  - [1.2 Cancel TTS module sam_tts_deinit](#12-cancel-tts-module-sam_tts_deinit)
  - [1.3 Obtain the working status of TTS sam_tts_get_status](#13-obtain-the-working-status-of-tts-sam_tts_get_status)
  - [1.4 Stop playing TTS sam_tts_stop_playing](#14-stop-playing-tts-sam_tts_stop_playing)
  - [1.5 Play TTS voice sam_tts_play](#15-play-tts-voice-sam_tts_play)
  - [1.6 Play TTS voice and save the data to a WAV format file sam_tts_play_and_save_wav](#16-play-tts-voice-and-save-the-data-to-a-wav-format-file-sam_tts_play_and_save_wav)
  - [1.7 Get the parameters related to the YONGTONE TTS library sam_tts_get_YOUNGTONE_param](#17-get-the-parameters-related-to-the-yongtone-tts-library-sam_tts_get_youngtone_param)
  - [1.8 Set parameters related to the YOUNG TONE TTS library sam_tts_set_YOUNGTONE_param](#18-set-parameters-related-to-the-young-tone-tts-library-sam_tts_set_youngtone_param)
  - [1.9 Set parameters related to the IFLY TTS library sam_tts_set_IFLY_param](#19-set-parameters-related-to-the-ifly-tts-library-sam_tts_set_ifly_param)
  - [1.10 Get the parameters related to the IFLY TTS library sam_tts_get_IFLY_param](#110-get-the-parameters-related-to-the-ifly-tts-library-sam_tts_get_ifly_param)
  - [1.11 Obtain whether TTS is played locally or remotely sam_tts_get_local_or_remote_status](#111-obtain-whether-tts-is-played-locally-or-remotely-sam_tts_get_local_or_remote_status)
  - [1.12 Set TTS to play locally or remotely sam_tts_set_local_or_remote_status](#112-set-tts-to-play-locally-or-remotely-sam_tts_set_local_or_remote_status)
  - [1.13 Obtain whether the system volume can be set sam_tts_get_sys_vol_setting_status](#113-obtain-whether-the-system-volume-can-be-set-sam_tts_get_sys_vol_setting_status)
  - [1.14 Can the system volume be set sam_tts_set_sys_vol_setting](#114-can-the-system-volume-be-set-sam_tts_set_sys_vol_setting)
- [2 Introduction to TTS interface usage examples](#2-introduction-to-tts-interface-usage-examples)
  - [2.1 Main function entrance](#21-main-function-entrance)
  - [2.2 Play TTS speech in UCS2 data encoding format](#22-play-tts-speech-in-ucs2-data-encoding-format)
  - [2.3 Play TTS voice in GBK data format](#23-play-tts-voice-in-gbk-data-format)
  - [2.4 Play ASCII encoded speech and save the speech](#24-play-ascii-encoded-speech-and-save-the-speech)
  - [2.5 Set YONGTONE TTS library parameters](#25-set-yongtone-tts-library-parameters)
  - [2.6 Set IFLY TTS library parameters](#26-set-ifly-tts-library-parameters)
- [3 TTS function call flowchart](#3-tts-function-call-flowchart)

## 1 Introduction to TTS API Interface
The VMCU framework provides a set of TTS interfaces for processing TTS related services, which can be called to achieve the function of converting text into audio output.

The TTS interface is defined in the header file SamTTS.h and must be included when used.

### 1.1 TTS function initialization sam_tts_init

|Interface|int sam_tts_init(uint8 atcIndex,sam_tts_callback ttsCallback,sam_tts_urc_callback urcTTSCallback);|
|---|---|
|Function|The TTS module initialization function, when calling any of the following functions, first needs to register the TTS module in the system. This function only needs to be called once. But after calling the void sam_tts_deinit (void) function, if you still need to use the TTS module, you need to call this function again for initialization.|
|Parameters|atcIndex:Which serial port channel should be selected? Generally, this parameter is passed as 0.<br/>ttsCallback:This parameter is passed as a callback function pointer, which returns the execution result of any function from 3.3 to 3.14 when executed.<br/>urcTTSCallback:This parameter is passed as a pointer to a callback function, which means that when executing any function from 3.3 to 3.14, if the function from 3.3 to 3.14 has a URC report, it will be output through this callback function.|
|Return|0:indicates successful execution.<br/>-1:indicates execution failure. Please check if the correct parameters are passed in.|
|Notes|None|

### 1.2 Cancel TTS module sam_tts_deinit

|Interface|void sam_tts_deinit(void);|
|---|---|
|Function|This function is used to log out the TTS module.|
|Parameters|None|
|Return|None|
|Notes|None|

### 1.3 Obtain the working status of TTS sam_tts_get_status

|Interface|int sam_tts_get_status(void);|
|---|---|
|Function|This function is used to obtain the working status of TTS, and will return whether the current status of TTS is playing TTS or not through a callback function.|
|Parameters|None|
|Return|0:indicates successful execution<br/>-1:indicates that there are other TTS tasks in progress|
|Notes|None|

### 1.4 Stop playing TTS sam_tts_stop_playing

|Interface|int sam_tts_stop_playing(void);|
|---|---|
|Function|Stop playing TTS.|
|Parameters|None|
|Return|0:indicates successful execution<br/>-1:indicates that there are other TTS tasks in progress|
|Notes|None|

### 1.5 Play TTS voice sam_tts_play

|Interface|int sam_tts_play(uint8 *pData,uint16 dataSize,TTS_PLAYING_DATA_FORMAT_E format);|
|---|---|
|Function|Play TTS voice.|
|Parameters|pData: If the data is in ASCII encoding format and GBK encoding format in Chinese, the two formats exist separately or in combination. The maximum length of the data is 512 bytes (including two quotation marks). If it is in UCS2 encoding format, including quotation marks, the maximum length is 510 bytes.<br/>dataSize: The length of the pData data passed in.<br/>format: The encoding format of the data referred to by pData.|
|Return|0:indicates successful execution<br/>-1:Please check if the passed parameters are correct.|
|Notes|None|

### 1.6 Play TTS voice and save the data to a WAV format file sam_tts_play_and_save_wav

|Interface|int sam_tts_play_and_save_wav(uint8 *pData,uint16 dataSize,char *fileName,TTS_PLAYING_DATA_FORMAT_E format);|
|---|---|
|Function|Play TTS voice and save the data to a WAV format file.|
|Parameters|pData: If the data is in ASCII encoding format and GBK encoding format in Chinese, the two formats exist separately or in combination, and the maximum length of the data is 50 bytes. TOUNGTONE TTS: Chinese polyphonic character encoding format, polyphonic character<pinyin=pronunciation+tone>; IFLY TTS: Chinese polyphonic character encoding format, polyphonic characters [=pronunciation+tone].<br/>If the data is UCS2 encoded data, the maximum length of the data is 50 bytes.<br/>dataSize: The length of the pData data passed in.<br/>fileName: Enter the path and file name. If no path is specified, it will be saved on the C: drive by default. The maximum file name length is 60 bytes, and currently only supports the.wav file name extension.<br/>format: The encoding format of the data referred to by pData.|
|Return|0:indicates successful execution<br/>-1:Please check if the passed parameters are correct.|
|Notes|None|

### 1.7 Get the parameters related to the YONGTONE TTS library sam_tts_get_YOUNGTONE_param

|Interface|int sam_tts_get_YOUNGTONE_param(void);|
|---|---|
|Function|Get the parameters related to the YONGTONE TTS library, this function will return the parameter values through the callback function set by sam_tts_init.|
|Parameters|None|
|Return|0:indicates successful execution<br/>-1:indicates that there are other TTS tasks in progress|
|Notes|None|

### 1.8 Set parameters related to the YOUNG TONE TTS library sam_tts_set_YOUNGTONE_param

|Interface|int sam_tts_set_YOUNGTONE_param(TTS_param_T *pParam);|
|---|---|
|Function|Set parameters related to the YOUNG TONE TTS library.|
|Parameters|pParam: A pointer to the TTS_param_T structure, which contains an array:<br/> *  index 0:TTS lib volume,range (0,1,2),default:1.<br/> *  index 1:system volume,range (0,1,2,3),default:3.<br/> *  index 2:digitmode,range (0,1,2),default:0.<br/> *  index 3:pitch,range (0,1,2),default:1.<br/> *  index 4:speed,rough speed regulation,range (0,1,2),default:1;precision speed regulation,range [10...30].<br/>     *  index 5:digitreading,range (0,1),default:0.|
|Return|0:indicates successful execution<br/>-1:indicates that there are other TTS tasks in progress|
|Notes|None|

### 1.9 Set parameters related to the IFLY TTS library sam_tts_set_IFLY_param

|Interface|int sam_tts_set_IFLY_param(TTS_param_T *pParam);|
|---|---|
|Function|Set parameters related to the IFLY TTS library.|
|Parameters|pParam:a pointer to the TTS_param_T structure, which contains an array:<br/> *  index 0:TTS lib volume,range (0,1,2),default:2.<br/> *  index 1:system volume,range (0...7),default:4.<br/> *  index 2:digitmode,range (0,1,2),default:0.<br/> *  index 3:pitch,range (0,1,2),default:1.<br/> *  index 4:speed,rough speed regulation,range (0,1,2),default:1;precision speed regulation,range [10...30].<br/>     *  index 5:ttslib,range (0,1),default:0.|
|Return|0:indicates successful execution<br/>-1:indicates that there are other TTS tasks in progress|
|Notes|None|

### 1.10 Get the parameters related to the IFLY TTS library sam_tts_get_IFLY_param

|Interface|int sam_tts_get_IFLY_param(void);|
|---|---|
|Function|Get the parameters related to the IFLY TTS library. This function will return the parameter values through the callback function set by sam_tts_init.|
|Parameters|None|
|Return|0:indicates successful execution<br/>-1:indicates that there are other TTS tasks in progress|
|Notes|None|

### 1.11 Obtain whether TTS is played locally or remotely sam_tts_get_local_or_remote_status

|Interface|int sam_tts_get_local_or_remote_status(void);|
|---|---|
|Function|Obtain whether TTS is played locally or remotely, and return the obtained status value through the callback function set by sam_tts_init.|
|Parameters|None|
|Return|0:indicates successful execution<br/>-1:indicates that there are other TTS tasks in progress|
|Notes|None|

### 1.12 Set TTS to play locally or remotely sam_tts_set_local_or_remote_status

|Interface|int sam_tts_set_local_or_remote_status(uint8 localOrRemote);|
|---|---|
|Function|Set TTS to play locally or remotely.|
|Parameters|localOrRemote锛�0:Local Path,1:Remote Path銆倈
|Return|0:indicates successful execution<br/>-1:indicates that there are other TTS tasks in progress|
|Notes|None|

### 1.13 Obtain whether the system volume can be set sam_tts_get_sys_vol_setting_status

|Interface|int sam_tts_get_sys_vol_setting_status(void);|
|---|---|
|Function|Obtain whether the system volume can be set, and return the obtained status value through the callback function set by sam_tts_init.|
|Parameters|None|
|Return|0:indicates successful execution<br/>-1:indicates that there are other TTS tasks in progress|
|Notes|None|

### 1.14 Can the system volume be set sam_tts_set_sys_vol_setting

|Interface|int sam_tts_set_sys_vol_setting(uint8 sysVolSetting);|
|---|---|
|Function|Can the system volume be set.|
|Parameters|sysVolSetting锛�0:volume setting is valid,1:volume setting is invalid.|
|Return|0:indicates successful execution<br/>-1:indicates that there are other TTS tasks in progress|
|Notes|None|

## 2 Introduction to TTS interface usage examples
This chapter mainly introduces how the application calls the TTS interface to achieve TTS voice playback.

### 2.1 Main function entrance
The initialization of TTS, like the initialization of other modules, calls the sam_tts_init() function in TesterInit() before the while loop.

After initialization, if the application wants to perform TTS related services, such as playing TTS voice, it can be executed inside the While loop. Developers can design according to their own logic, and further introduction will be provided in subsequent chapters.

### 2.2 Play TTS speech in UCS2 data encoding format

```c
    uint8 *pData = "\"6B228FCE4F7F75288BED97F3540862107CFB7EDF\"";
    sam_tts_play(pData,strlen(pData),TTS_PLAYING_UCS2_FORMAT);
```

### 2.3 Play TTS voice in GBK data format

```c
    uint8 *pData = "\"鍘绘湞<pinyin=chao2>闃筹紝鐪嬫湞<pinyin=zhao1>闃砛"";
    sam_tts_play(pData,strlen(pData),TTS_PLAYING_ASCII_AND_GBK_OR_ONLY_GBK_FORMAT);
```

### 2.4 Play ASCII encoded speech and save the speech

```c
    uint8 *pData = "\"hello world\"";
    char *fileName = "\"C:/12.wav\"";
    sam_tts_play_and_save_wav(pData,strlen(pData),fileName,TTS_PLAYING_ONLY_ASCII_FORMAT);
```

### 2.5 Set YONGTONE TTS library parameters

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

### 2.6 Set IFLY TTS library parameters

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

## 3 TTS function call flowchart

<img src="tts.png" style="background-color:rgb(251, 252, 252); padding: 5px;" width="860" height="516">

[<- Return to the main directory](../README_en.md)
