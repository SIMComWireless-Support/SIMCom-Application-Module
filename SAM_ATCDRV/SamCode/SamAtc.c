/**
 * @file 	SamAtc.c
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

#define __SALATC_C

#include "SamInc.h"


void SamSendAtSeg(HdsAtcTag * phat)
{ 
	char disbuf[256];
	char * cmdstr;
	uint16 i, m;
	cmdstr = (char *)&(phat->atcbuf[phat->atcbp]);
	if(cmdstr[0] == 0x00 || cmdstr[0] == 0x0D) return;
	i = 0;
	while(cmdstr[i] != 0x0D && cmdstr[i] != 0x00) i++;
	if(cmdstr[i] == 0)
	{
		cmdstr[i++] = 0x0D;
	}
	else
	{
		i++;
	}
	phat->atcbp += i;
	phat->state = SCMD_HATCSTA;
	if(phat->atcbuf[phat->atcbp] == 0)
	{
		phat->state = SCED_HATCSTA;
	}
    phat->retbufp = 0;

	if(cmdstr[0]== '\t')
	{
		m = 0;
		for(i=1; cmdstr[i]<='9'&&cmdstr[i]>='0'; i++)
		{
			m = m*10;
			m += (cmdstr[i] - '0');
		}
		phat->delayms = m;
		DebugTrace("WM>%u ms Delay\r\n", m);
	}
    else
    {
        SendtoCom(phat->comid, cmdstr, i);
	    if(i>255) i = 255;
	    memcpy(disbuf, cmdstr, i);
	    disbuf[i] = 0;
        DebugTrace("SM%u:%u>%s\r\n", phat->comid, i, disbuf);
    }
    phat->msclk = SamGetMsCnt(0);
}



uint8 SamSendAtCmd(HdsAtcTag * phatc, char * cmdstr, uint8 type, uint8 timwm)
{
	uint8 ret;
	if(cmdstr == NULL || phatc == NULL) //phatc->state != IDLE_HATCSTA
	{//
		return(RETCHAR_FALSE);
	}
	
	do{
		ret = SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n");
		if(ret == RETURNSR_ATCRET) phatc->retbufp = 0;
	}while(ret!= NOSTRRET_ATCRET);
	
	phatc->atcbuf[0] = 0;
	strcpy(phatc->atcbuf, cmdstr);
	phatc->atcbp = 0;
	if(phatc->atcbuf[0] != 0x00)
	{
		if(timwm == 0x00)
		{
			phatc->waitret = STOP_HATCTMW;
		}
		else
		{
			phatc->waitret = timwm;  //timwm  : 1024 mS
			phatc->waitret *= 128;  // timm :  U_8ms
		}

		phatc->type = type;
		SamSendAtSeg(phatc);
		phatc->retbufp = 0;
		phatc->retbuf[0] = 0;
		return(RETCHAR_TRUE);
	}
	return(RETCHAR_FALSE);
}



uint8 SamChkAtcRet(HdsAtcTag * phatc, char * efsm)
{
	uint32 clk, n;
	uint8  temp, t;
	if((phatc->type & CRLF_HATCTYP) != 0)
	{
		while(ReadfoCom(phatc->comid, (char *)&temp, 1) ==1)
		{
			if((temp == 0x0D || temp == 0x0A) && phatc->retbufp == 0)
			{
				continue;
			}
			else if(temp == 0x0A)
			{
				phatc->retbuf[phatc->retbufp++] = 0x0A;
				phatc->retbuf[phatc->retbufp]  = 0x00;
				temp = StrsCmp(phatc->retbuf, efsm);
				DebugTrace("RM%u:%u:%u<%s",phatc->comid,temp,phatc->retbufp, phatc->retbuf);
				if(temp != 0)
				{
					return(temp); //return the index of return string;
				}
				else
				{
					for(t = 0; t < MDMFUNARRAY_MAX; t++)
					{
						if(phatc->fun[t].pfunData != NULL && phatc->fun[t].pBcFun != NULL)
						{
							if(RETCHAR_NONE == phatc->fun[t].pBcFun(phatc->fun[t].pfunData, phatc->retbuf))
							{
								continue;
							}
							else
							{
								DebugTrace("Fun_URCBcF=%u:%s",phatc->retbufp, phatc->retbuf);
								phatc->retbufp = 0;
								return(NOSTRRET_ATCRET);
							}
						}								
					}
					if(t == MDMFUNARRAY_MAX)
					{
						if(RETCHAR_NONE == phatc->MdmUrcBcFun(phatc->pMdmhost, phatc->retbuf))
						{
							return(RETURNSR_ATCRET);
						}
						else
						{
							DebugTrace("Mdm_URCBcF=%u:%s",phatc->retbufp, phatc->retbuf);
							phatc->retbufp = 0;
							return(NOSTRRET_ATCRET);
						}
					}
				}
			}
			else if((phatc->type & RHCD_HATCTYP) != 0 && temp == ',')
			{//HEADSTR: 10,xxxxxxxxxx
				phatc->retbuf[phatc->retbufp++] = ',';
				phatc->retbuf[phatc->retbufp]  = 0x00;
				temp = StrsCmp(phatc->retbuf, efsm);
				if(temp != 0)
				{
					DebugTrace("RM%u:%u:%u<%s",phatc->comid,temp, phatc->retbufp, phatc->retbuf);
					return(temp);
				}
			}
			else if((phatc->type & RISP_HATCTYP) != 0 && temp == ' ')
			{//>  space....to send 
				phatc->retbuf[phatc->retbufp++] = ' ';
				phatc->retbuf[phatc->retbufp]  = 0x00;
				temp = StrsCmp(phatc->retbuf, efsm);
				if(temp != 0 && phatc->databufp != ATCRDATAPT_VMAX && phatc->databuf != NULL)
				{
					SendtoCom(phatc->comid, phatc->databuf, phatc->databufp);
					DebugTrace("RM[%02X]%u<%s\r\n",temp, phatc->retbufp, phatc->retbuf);
        			DebugTrace("SM[%u]Bytes\r\n",phatc->databufp);
					phatc->retbufp = 0;
					phatc->retbuf[phatc->retbufp]  = 0x00;
					phatc->msclk = SamGetMsCnt(0);
					phatc->type &= ~RISP_HATCTYP;
				}
			}
			else if((phatc->type & RIGR_HATCTYP) != 0 && temp == '>')
			{//> Greater... 
				phatc->retbuf[phatc->retbufp++] = '>';
				phatc->retbuf[phatc->retbufp]  = 0x00;
				temp = StrsCmp(phatc->retbuf, efsm);
				if(temp != 0 && phatc->databufp != ATCRDATAPT_VMAX && phatc->databuf != NULL)
				{
					SendtoCom(phatc->comid, phatc->databuf, phatc->databufp);
					DebugTrace("RM[%02X]%u<%s\r\n",temp, phatc->retbufp, phatc->retbuf);
        			DebugTrace("SM[%u]Bytes\r\n",phatc->databufp);
					phatc->retbufp = 0;
					phatc->retbuf[phatc->retbufp]  = 0x00;
					phatc->msclk = SamGetMsCnt(0);
					phatc->type &= ~RIGR_HATCTYP;
				}
			}
			else if(phatc->retbufp < (ATRETBUFLEN -2))
			{
				phatc->retbuf[phatc->retbufp++] = temp;
			}
		}
	}
	else if(phatc->type == BCNT_HATCTYP && phatc->databufp != ATCRDATAPT_VMAX && phatc->databuf != NULL)
	{
		while(ReadfoCom(phatc->comid, (char *)&temp, 1) ==1)
		{
			phatc->databuf[phatc->retbufp++] = temp;
			if(phatc->retbufp >= phatc->databufp)
			{
				phatc->type &= ~BCNT_HATCTYP;
				phatc->type |= CRLF_HATCTYP;
				DebugTrace("RM[%u]Bytes\r\n", phatc->retbufp);
				return(RECVBCNT_ATCRET);
			}
		}
	}
	
	
	if(phatc->delayms != 0)
	{
		clk = SamGetMsCnt(phatc->msclk);
		if(clk >= (uint32)phatc->delayms)
		{
			phatc->delayms = 0;
            DebugTrace("WM< Fin Delay\r\n");
			return(DELAYFIN_ATCRET);
		}
		return(NOSTRRET_ATCRET);
	}
	else if(phatc->waitret != STOP_HATCTMW && phatc->waitret != OVER_HATCTMW)
	{
		n = phatc->waitret;
		n *= 8;
		clk = SamGetMsCnt(phatc->msclk);
		if(clk >= n)
		{
			phatc->waitret = OVER_HATCTMW;
			phatc->retbuf[phatc->retbufp] = 0x00;
	        DebugTrace("OT%u<%s\r\n", phatc->retbufp, phatc->retbuf);
			phatc->state = SCED_HATCSTA;
			phatc->retbufp = 0x00;
			return(OVERTIME_ATCRET);
		}
	}	
	return(NOSTRRET_ATCRET);
}

HdsAtcTag * SamAtcInit(HdsAtcTag * pmems, uint8 cid)
{
	uint8 i;
	HdsAtcTag * phatc = pmems;
	if(phatc == NULL) return(NULL);
	phatc->comid = cid;
	phatc->state = IDLE_HATCSTA;
	phatc->retbufp = 0;
	phatc->atcbp = 0;
	
	phatc->waitret = STOP_HATCTMW;
	phatc->delayms = 0;
	phatc->type = 0;
	phatc->databuf = NULL;
	phatc->databufp = ATCRDATAPT_VMAX;
	
	for(i = 0; i < MDMFUNARRAY_MAX; i++)
	{
		phatc->fun[i].pfunProc = NULL;
		phatc->fun[i].pBcFun = NULL;
		phatc->fun[i].pfunData = NULL;
	}

	phatc->MdmUrcBcFun = NULL;

	return(phatc);
}


uint8 SamAtcFunLink(HdsAtcTag * phatc, void * pfundat, SamMdmFunTag pfunpro, SamUrcBcFunTag pbcfun)
{
	uint8 i;
	for(i=0; i<MDMFUNARRAY_MAX; i++)
	{
		if(phatc->fun[i].pfunData != NULL) continue;
		phatc->fun[i].pfunData = pfundat;
		phatc->fun[i].pfunProc = pfunpro;
		phatc->fun[i].pBcFun = pbcfun;
		return(i);
	}
	return(MDMFUNARRAY_MAX);
}

uint8 SamAtcFunUnlink(HdsAtcTag * phatc, uint8 fid)
{
	if(fid >= MDMFUNARRAY_MAX) return(MDMFUNARRAY_MAX);
	if(phatc->fun[fid].pfunData != NULL || phatc->fun[fid].pfunProc != NULL)
	{
		phatc->fun[fid].pfunData = NULL;
		phatc->fun[fid].pfunProc = NULL;
		phatc->fun[fid].pBcFun = NULL;
		return(fid);
	}
	return(MDMFUNARRAY_MAX);
}

uint8 SamAtcFunUrcBroadCast(HdsAtcTag * phatc, char * notifaction)
{
	uint8 t, n;
	n = 0;
	for(t = 0; t < MDMFUNARRAY_MAX; t++)
	{
		if(phatc->fun[t].pfunData != NULL && phatc->fun[t].pBcFun != NULL)
		{
			phatc->fun[t].pBcFun(phatc->fun[t].pfunData, notifaction);
			n++;
		}								
	}
	return(n);
}


//Data in URC by Bytes be Read!
uint16 SamAtcDubRead(HdsAtcTag * phatc, uint16 len, char * dp)
{
	return(ReadfoCom(phatc->comid, dp, len));
}

