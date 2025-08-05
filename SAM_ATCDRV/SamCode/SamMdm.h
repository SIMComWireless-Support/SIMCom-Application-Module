/**
 * @file 	SamMdm.h
 * @brief   Modem state processor
 * @details Responsible for managing the working status of the modem, 
 *  		configuring the interaction between the modem and the network based on the configuration string. 
 *			When the modem status is abnormal, perform relevant repair processing, 
 *			and also provide features and status parameters for the application layer. 
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
//----------------------------------------------------------------------

#ifndef __SAMMDM_H
#define __SAMMDM_H

#ifdef __cplusplus
extern "C"
{
#endif


#define SAMMDMCFG_SAMPLE1 "\vCFGMDM_A1\t0\tA\t2,1,\"IP\",\"cmiot\",1,\"user123\",\"psw123\",2,\"IP\",\"cmnet\",1,\"user123\",\"psw123\"\v"
#define CFGMDM_HEADSTR	"CFGMDM_"  //Modem configure string flag head
enum{
	CFGMDM_FLAGSH = 0,   //. 
	CFGMDM_ATCCHL = 1, //.
	CFGMDM_ATCSET,		//A: ASR,  M: QCOMM
	CFGMDM_PDNCFG,  //pdncnt,pdncid1,pdnip1,pdnapn1,pdnauth1,pdnusr1,pdnpwd1,pdncid2,pdnip2,pdnapn2,pdnauth2,pdnusr2,pdnpwd2,....
};
//.atcset //AT command  
#define ATCSET_A	'A'
#define ATCSET_M 	'M'


typedef struct{
    uint8	sta;
	uint8	step;
	uint8	dcnt;
	uint8	stim;
	uint32  msclk;

	char 	* cfg;
	HdsAtcTag * patc;
	
	uint32 	urcbmk;
	
    uint32  conditon;
	uint8	pdncnt;
	
	uint8	atcset;	//AT COMMAND SET
	
	char	imei[16];
	char	ccid[24];
	char	imsi[16];

	char	ipstrtab[256]; //for IP address string
	
	char 	netmode;	//G:GSM, W:WCDMA, L:LTE, 0x00
	char 	optmode;	//N:on line,  F: off line, flight 
	char 	mcc[4];		//e.g. 460
	char 	mnc[4];		//e.g. 000
	uint16	lachex;		//0x0001 ~ 0xFFFE
	
	uint8	csq;	
	volatile uint8	uatcwot;			//wait over time
	char 	uatcbuf[256];	//for user to send atc and waitr;
	
	
}TMdmTag;

//.sta
enum{
	NONE_MDMSTA	= 0x00,
	FUN0_MDMSTA = 0x01,
	INIT_MDMSTA = 0x02,
	FFUN_MDMSTA, 
	FAIL_MDMSTA,

};

//.condition
#define ATCOK_MDMCND	0x00000001	//AT commands work fine
#define CPINR_MDMCND	0x00000002	//SIM CARD READY 
#define PSREG_MDMCND	0x00000004	//PS IS REGESTED
#define IPACT_MDMCND	0x00000008	//IP LAYER IS READY
//wait for pin code 
#define WFPIN_MDMCND	0x00000020	//WAIT FOR PIN CODE

#define CFUN0_MDMCND  	0x80000000

//IP MASK BITS
#define	IPABIT_MDMCND	0x00000100
#define IPBMSK_MDMCND	0x0000FF00



/**
 * @brief Initialize the modem structure.
 *
 * This function initializes the modem structure based on the provided configuration string.
 * It reads the configuration parameters from the string and sets up the modem's ATC (AT Command) handler.
 *
 * @param pmdm Pointer to the modem structure to be initialized.
 * @param cfgstr Pointer to the configuration string.
 * @return Pointer to the initialized modem structure if successful, NULL otherwise.
 */
extern TMdmTag * SamMdmInit(TMdmTag * pmdm, char * cfgstr);

/**
 * @brief Process the modem operations.
 *
 * This function manages the modem's state machine, sending AT commands and handling responses.
 * It also handles special user ATC commands and schedules functional block tasks.
 *
 * @param pvmdm Pointer to the modem structure.
 * @return A return code indicating the result of the processing.
 */
extern unsigned char SamMdmProc(void * pvmdm);

/**
 * @brief Stop the modem operation.
 *
 * This function sets the modem state to zero, effectively stopping its operation.
 *
 * @param pvmdm Pointer to the modem structure.
 * @return Always returns RETCHAR_NONE.
 */
extern unsigned char SamMdmStop(void * pvmdm);

/**
 * @brief Callback function for handling URC (Unsolicited Result Code) messages from the modem.
 *
 * This function checks if the received URC string matches a predefined pattern.
 * If a match is found, it sets a bit in the URC callback mask of the modem structure.
 *
 * @param pvmdm Pointer to the modem structure.
 * @param urcstr Pointer to the received URC string.
 * @return Always returns RETCHAR_NONE.
 */
extern unsigned char SamMdmUrcCbfun(void * pvmdm, char * urcstr);





#ifdef __cplusplus
}
#endif


#endif
