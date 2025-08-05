/**
 * @file 	SamAtc.h
 * @brief   AT command frame transmission and reception processing
 * @details Responsible for identifying and processing AT command frames, 
 * 			providing AT command exchange and status processing for various functional units, 
 *			as well as related URC processing callback registration interfaces. 
 *			It is also the driver layer of the modem maintenance processor 
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
#ifndef __SALATC_H
#define __SALATC_H


#ifdef __cplusplus
extern "C"
{
#endif


#define ATCMDBUFLEN	256
#define ATRETBUFLEN	512

#define ATURCBUFLEN	256
#define ATURCHDLCNT	16

typedef unsigned char (* SamMdmFunTag)(void * pd);
typedef unsigned char (* SamUrcBcFunTag)(void * pd, char * ustr);
typedef struct{
	void *	pfunData; 
	SamMdmFunTag pfunProc;
	SamUrcBcFunTag pBcFun;
}MdmFunTag;
#define MDMFUNARRAY_MAX	16


#define ATCRDATAPT_VMAX	5000
typedef struct{
	uint8  	comid;		//ATC com channel id
	uint8	logid;
	uint8	state;
	uint8 	type;		//recieve mode 
	
	uint32	msclk;
	uint16 	waitret;	//max wait time for return U_16ms		

	char	retbuf[ATRETBUFLEN];
	char	atcbuf[ATCMDBUFLEN];
	uint16  retbufp;
	uint16	atcbp;

	char *    databuf;     //user data interface
	uint16    databufp;		

	uint16	 delayms;	// delay U_ms

	void	* pMdmhost;
	SamUrcBcFunTag MdmUrcBcFun;
	

	MdmFunTag fun[MDMFUNARRAY_MAX];
	uint8 	fpt;

}HdsAtcTag;

//.state
#define  NONE_HATCSTA	0x00
#define	 IDLE_HATCSTA	0x01
#define	 SCMD_HATCSTA	0x02
#define	 SCED_HATCSTA	0x03
#define  HDAT_HATCSTA	0x08

//.type
#define BCNT_HATCTYP	0x00	// <-- n Bytes Need to receive a specified number of segments
#define	CRLF_HATCTYP	0x01	//-->AT+XXX\r, <-- OK\r\n  //\r\n
#define	RHCD_HATCTYP	0x02	//-->AT+XXX\r, <-- +CARECV: 7,3432423 //,COMMON
#define RISP_HATCTYP	0x04	//-->AT+XXX\r, <-- >  //>  >SPACE
#define RIGR_HATCTYP	0x08	//-->AT+XXX\r, <-- >  //>  Greater

//.waitret
#define OVER_HATCTMW	0x0000
#define STOP_HATCTMW	0xFFFF


//.ret
#define NOSTRRET_ATCRET 	0x00
#define DELAYFIN_ATCRET		0xF0	//Delay timing completed
#define RETURNSR_ATCRET		0xF1	//Received unknown string

#define RECVBCNT_ATCRET		0xFB	//Received the specified number of bytes
	
#define OVERTIME_ATCRET 	0xFF	//Receive waiting timeout


/**
 * @brief Initialize an HdsAtcTag structure.
 *
 * This function initializes the fields of an HdsAtcTag structure with default values.
 *
 * @param pmems Pointer to the HdsAtcTag structure to be initialized.
 * @param cid COM port ID.
 * @return Pointer to the initialized HdsAtcTag structure, or NULL if pmems is NULL.
 */
extern HdsAtcTag * SamAtcInit(HdsAtcTag * pmems, uint8 cid);


/**
 * @brief Send an AT command segment.
 *
 * This function processes and sends an AT command segment from the buffer.
 * It adds a carriage return if necessary, updates the buffer pointer and state,
 * and either sends the command or sets a delay.
 *
 * @param phat Pointer to the HdsAtcTag structure containing AT command information.
 */
extern void 	SamSendAtSeg(HdsAtcTag *        phat);

/**
 * @brief Send an AT command.
 *
 * This function prepares and sends an AT command. It checks the return status
 * of previous commands, sets the command buffer, and calls SamSendAtSeg to send
 * the command segment.
 *
 * @param phatc Pointer to the HdsAtcTag structure containing AT command information.
 * @param cmdstr Pointer to the AT command string.
 * @param type Type of the AT command.
 * @param timwm Timeout value for waiting for a response.
 * @return RETCHAR_TRUE if the command is sent successfully, RETCHAR_FALSE otherwise.
 */
extern uint8 	SamSendAtCmd(HdsAtcTag *        phatc, char * cmdstr, uint8 type, uint8 timwm);

/**
 * @brief Check the return status of an AT command.
 *
 * This function reads data from the COM port, checks for specific response strings,
 * and calls callback functions if necessary. It also handles delays and timeouts.
 *
 * @param phatc Pointer to the HdsAtcTag structure containing AT command information.
 * @param efsm Pointer to the string containing expected response strings.
 * @return The index of the matching response string, or other status codes.
 */
extern uint8 	SamChkAtcRet(HdsAtcTag * phatc,  char * efsm);


/**
 * @brief Link a callback function to an HdsAtcTag structure.
 *
 * This function finds an empty slot in the function array of an HdsAtcTag structure
 * and links the provided callback function and data pointer.
 *
 * @param phatc Pointer to the HdsAtcTag structure.
 * @param pfundat Pointer to the data to be passed to the callback function.
 * @param pfunpro Pointer to the function to be called.
 * @param pbcfun Pointer to the callback function.
 * @return The index of the linked function slot, or MDMFUNARRAY_MAX if no slot is available.
 */
extern uint8	SamAtcFunLink(HdsAtcTag * phatc, void * pfundat, SamMdmFunTag pfunpro, SamUrcBcFunTag pBcfun);

/**
 * @brief Unlink a callback function from an HdsAtcTag structure.
 *
 * This function clears the function pointers and data pointer in the specified slot
 * of the function array of an HdsAtcTag structure.
 *
 * @param phatc Pointer to the HdsAtcTag structure.
 * @param fid Index of the function slot to be unlinked.
 * @return The index of the unlinked function slot, or MDMFUNARRAY_MAX if the index is invalid.
 */
extern uint8 SamAtcFunUnlink(HdsAtcTag * phatc, uint8 fid);


/**
 * @brief Broadcast an URC (Unsolicited Result Code) notification to all registered callback functions.
 *
 * This function iterates through the array of registered callback functions and invokes each one
 * that is currently active (i.e., has non-NULL data and callback function pointers) with the provided
 * notification string. This allows multiple components to handle URCs from the modem without tight coupling.
 *
 * @param phatc Pointer to the HdsAtcTag structure containing the array of callback functions.
 * @param notifaction Pointer to the notification string to be broadcasted.
 * @return The number of callback functions successfully invoked (always returns 0 in the current implementation).
 */
extern uint8 SamAtcFunUrcBroadCast(HdsAtcTag * phatc, char * notifaction);


/**
 * @brief Read data from the COM port in URC mode(only the URC handle).
 *
 * This function reads a specified number of bytes from the COM port associated
 * with the HdsAtcTag structure.
 *
 * @param phatc Pointer to the HdsAtcTag structure.
 * @param len Number of bytes to read.
 * @param dp Pointer to the buffer to store the read data.
 * @return The number of bytes actually read.
 */
extern uint16 	SamAtcDubRead(HdsAtcTag * phatc, uint16 len, char * dp);




#ifdef __cplusplus
}
#endif





#endif
