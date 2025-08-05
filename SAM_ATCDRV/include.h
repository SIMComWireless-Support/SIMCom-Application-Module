#ifndef __INCLUDE_H
#define __INCLUDE_H

#include "SamInc.h"
#include "SamMdmSrv.h"
#include "SamFotaSrv.h"
#include "SamSocketSrv.h"
#include "SamSmsSrv.h"
#include "SamMqttSrv.h"
#include "SamTester.h"


#define UARTRBLENMK		0x07FF
#define UARTSBLENMK		0x07FF
typedef struct{
	unsigned char 		recvbuf[UARTRBLENMK+1];
	uint16				rinp;
	uint16 				routp;
	unsigned char		sbuf[UARTSBLENMK+1];
	unsigned char 	* 	sendbuf;
	uint16 	sdp;
	uint16 	scnt;
	uint16 	ctrsta;
}UartTag;

//.CtrSta
#define OVERBUF_ERR		(1<<0)
#define HARDFLOW_CTR    (1<<1)
#define SEDBUFIN_STA    	(1<<2)
#define RLSD_ACT        	(1<<6)
#define RLSD_PIN        	(1<<7)
#define DSR_ACT         	(1<<8)
#define DSR_PIN         	(1<<9)
#define DTR_OUT         	(1<<10)
#define RTS_OUT         	(1<<11)
#define RI_PIN          	(1<<12)
#define CTS_ACT         	(1<<13)
#define RI_ACT          	(1<<14)
#define CTS_PIN         	(1<<15)

extern UartTag UartTA;




#endif
