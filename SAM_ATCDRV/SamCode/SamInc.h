/**
 * @file 	SamInc.h
 * @brief   This header file is a shared header file within the SamCode folder
 * @details This header file defines the data type，
 *	 		and the external functions required to support the SamCode component.
 *			
 * 
 * @version 1.0.0
 * @date 	2025-08-01
 * @author 	Alex <fanbing.kong@sunseaaiot.com>
 * @copyright Copyright (c) 2025, SIMCom Wireless Solutions Limited. All rights reserved.
 * 
 * @note 
 *
 *
 */
//---------------------------------------------------------------------------
#ifndef __SAMINC_H
#define __SAMINC_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C"
{
#endif

typedef unsigned char	uint8;                   
typedef signed   char 	int8;     
typedef unsigned short	uint16;  
typedef signed   short 	int16; 
typedef unsigned int   	uint32;  
typedef signed   int   	int32;   
typedef unsigned long   uint64;
typedef signed  long    int64;
typedef float			fp32;  
typedef double			fp64;   


typedef enum{
	RETCHAR_NONE = 0x00, 	//NONE  
	RETCHAR_FALSE = 0x00, 	//FALSE 
	RETCHAR_TRUE = 0x01, 	//TRUE

	RETCHAR_FREE = 0x10,	//Release resource
	RETCHAR_KEEP = 0x11,	//Keep resoure
	RETCHAR_CHERR = 0x12,	//Channel error

	RETCHAR_MDMIPOK	= 0xA0,	//MODEM IP LAYER IS OK 
	
	RETCHAR_ERROR = 0xE0,	//error
	RETCHAR_ERRPM = 0xE1,	//error Parameter error
	
}SamRetChar;

#include "SamSub.h"
#include "SamDebug.h"
#include "SamAtc.h"
#include "SamMdm.h"
#include "SamMqtt.h"
#include "SamSocket.h"
#include "SamAudio.h"
#include "SamTTS.h"
#include "SamFota.h"
#include "SamSms.h"

#define ATCBUS_CHMAX	1
extern HdsAtcTag *pAtcBusArray[ATCBUS_CHMAX];


//Functions that require external implementation
#define 	ATCCH_A		0x01
#define		DBGCH_A		0x02
extern unsigned int  GetSysTickCnt(void);
extern unsigned short SendtoCom(unsigned char com, char *dp, unsigned short dlen);
extern unsigned short ReadfoCom(unsigned char com, char *dp,  unsigned short dmax);


#ifdef __cplusplus
}
#endif


#endif 
/*****************************************************************************
**                            End Of File
******************************************************************************/
