/*
*********************************************************************************************************
*						
*			 Simcom EmbeddedAT Project Code Template
*                                         
*                          (c) Copyright 2014-2018, ae@sim.com
*                                           All Rights Reserved
*
* Version 	: V1.00
* By     	: AE001 2014/04/29
*********************************************************************************************************
*/
#ifndef __SAMMDMSEV_H
#define __SAMMDMSEV_H

#ifdef __cplusplus
extern "C"
{
#endif

//USER APPLICATION CONFIG LIST

/*
//SAMLIB cong
#define SAMMDMCFG_EXAMPLE 	"\vCFGMDM_A1\t2\tA\t1\t1,\"IP\",\"cmiot\"\t1,2,\"psw123456\",\"user\"\v"
#define CFGMDM_HEADSTR		"CFGMDM_"
#define CFGMDM_EPDPLEN		2
enum{
	CFGMDM_ATCCHL = 1,
	CFGMDM_ATCSET,	//A: ASR,  M: QCOMM
	CFGMDM_EPDCNT,
	CFGMDM_CNCFG,
	CFGMDM_AUCFG,
};
*/

typedef enum{

	MDMCMD_CHKSTA = 0,	//Check Modem Status;

	MDMCMD_CFGPDN,		//Configure APN, User, Psw,
	
	MDMCMD_CHKMDMIP,	//Check If Modem IP is OK;

	
	MDMCMD_GETIMEI,		//Read imei
	MDMCMD_GETCCID,		//Read ccid
	MDMCMD_GETIMSI,		//Read imsi
	MDMCMD_GETCSQ,		//Read Csq
	MDMCMD_GETIP,		//Get IP 

	
}SamMdmOptCmdTag;

extern SamRetChar SamMdmSrvCmd(SamMdmOptCmdTag cmd, void * pin, void * pout);


extern void SamMdmSrvStart(void);
extern void SamMdmSrvRun(void);
extern void SamMdmSrvStop(void);




#ifdef __cplusplus
}
#endif


#endif
