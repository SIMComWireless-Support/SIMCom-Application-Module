/**
 * @file 	SamMdm.c
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
 * 		Support AT Command set list:
 *		A Series :	  A7xxx, SIM76xx, SIM78xx, SIM79xx
 *		M Series :	  SIM7070, SIM7080, 
 */
//----------------------------------------------------------------------
#define __SAMMDM_C

#include "SamInc.h"

unsigned char SamMdmUrcCbfun(void * pvmdm, char * urcstr)
{
	TMdmTag * pmdm = NULL;
	uint8 temp;
	char myurcstr[256];
	
	pmdm = (TMdmTag *)pvmdm; 
	
	sprintf(myurcstr, "+SIMCARD: NOT AVAILABLE\t+CGEV: ME DETACH");
	temp = StrsCmp(urcstr, myurcstr);
	if(temp == 1)
	{
		pmdm->urcbmk |= 0x01;
		pmdm->conditon &= ~(CPINR_MDMCND);
	}
	else if(temp == 2)
	{
		pmdm->urcbmk |= 0x01;
		pmdm->conditon &= ~(PSREG_MDMCND);
	}
	return(RETCHAR_NONE);
}

TMdmTag * SamMdmInit(TMdmTag * pmdm, char * cfgstr)
{
	uint8 i, n;
	char str[256];
	
	if(cfgstr == NULL || pmdm == NULL || strlen(cfgstr) < 9) return(NULL);

	i = ReadCfgTab(cfgstr, CFGMDM_HEADSTR, CFGMDM_ATCCHL, str);
	n = (uint8)(str[0] - '0');
	if(i == 1 &&  n < ATCBUS_CHMAX)
	{
		memset(pmdm, 0x00, sizeof(TMdmTag));
		i = ReadCfgTab(cfgstr, CFGMDM_HEADSTR, CFGMDM_ATCSET, str);
		if(i == 1)
		{
			pmdm->atcset = str[0];
			pmdm->sta = INIT_MDMSTA;
		}
		i = ReadCfgTab(cfgstr, CFGMDM_HEADSTR, CFGMDM_PDNCFG, str);
		if(i > 1 && str[0] >= '0' && str[0] <= '9')
		{
			pmdm->pdncnt = str[0] - '0';
		}
		else
		{
			pmdm->pdncnt = 0;
		}
	}
	else
	{
		return(NULL);
	}
	

	
	pmdm->patc = pAtcBusArray[n];
	
	pmdm->patc->pMdmhost = (void *)pmdm;
	pmdm->patc->MdmUrcBcFun = SamMdmUrcCbfun;
	pmdm->urcbmk = 0;
	
	pmdm->cfg = cfgstr;
	pmdm->step = 0;
	pmdm->msclk = SamGetMsCnt(0);
	pmdm->stim = 0;
	pmdm->dcnt = 0;
	pmdm->conditon = 0;
	pmdm->uatcwot = 0;
	pmdm->uatcbuf[0] = 0;
	return(pmdm);
}

unsigned char SamMdmStop(void * pvmdm)
{
	TMdmTag * pmdm = NULL;
	pmdm = (TMdmTag * )pvmdm;
	if(pmdm == NULL) return(RETCHAR_NONE);
	pmdm->sta = 0;
    return(RETCHAR_NONE);
}

#define WMDMRET_BIT 0x80
unsigned char SamMdmProc(void * pvmdm)
{
	uint8 i, j, ratcret, funret;
	uint32 clk, n;
	char buf[256];
	char tbuf[256];
	char dbuf[256];
	char sbuf[256];
	
	HdsAtcTag * patc = NULL;
	TMdmTag * pmdm = NULL;

	pmdm = pvmdm;
	if(pmdm == NULL) return('E'+1);
	
	patc = pmdm->patc;
	if(patc == NULL) return('E'+2);
	
	clk = SamGetMsCnt(pmdm->msclk);
	while(clk >= 1000)
	{
		pmdm->msclk +=1000;
		pmdm->stim += 1;
		clk -= 1000;
	}

	if(pmdm->uatcwot >= 6 && patc->waitret == STOP_HATCTMW)
	{//Execute special ATC from App 
		if(pmdm->uatcwot != 0xFF && (pmdm->uatcbuf[0] == 'A' ||pmdm->uatcbuf[0] == 'a'))
		{
			while(SamChkAtcRet(patc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET);
			SamSendAtCmd(patc, pmdm->uatcbuf, CRLF_HATCTYP, pmdm->uatcwot);
			pmdm->uatcwot = 0xFF;
			pmdm->uatcbuf[0] = 0;
		}
		else if(pmdm->uatcwot == 0xFF)
		{
			ratcret = SamChkAtcRet(patc, "OK\r\n\tERROR\r\n");
			if(ratcret == 1 || ratcret == 2)
			{
				strcat(pmdm->uatcbuf, pmdm->patc->retbuf);
				pmdm->uatcwot = 1;
				DebugTrace("User ATC End!\r\n");
				pmdm->patc->retbuf[0] = 0;
				pmdm->patc->retbufp = 0;
				patc->waitret = STOP_HATCTMW;
			}
			else if(ratcret == OVERTIME_ATCRET)
			{
				pmdm->uatcbuf[0] = 0;
				pmdm->uatcwot = 0;
				DebugTrace("User ATC OV!\r\n");
			}
			else if(ratcret != NOSTRRET_ATCRET)
			{
				if(strlen(pmdm->uatcbuf) < 248)
				{
					strcat(pmdm->uatcbuf, pmdm->patc->retbuf);
				}
				pmdm->patc->retbuf[0] = 0;
				pmdm->patc->retbufp = 0;
			}
		}
		else
		{
			pmdm->uatcwot = 0;
			DebugTrace("User Send ATC finish!\r\n");
		}
		return(RETCHAR_NONE);
	}

	if(pmdm->sta == FFUN_MDMSTA && pmdm->step == 0)
	{//Task scheduling for each functional block
		for(; patc->fpt<MDMFUNARRAY_MAX; )
		{
			if(patc->fun[patc->fpt].pfunData != NULL && patc->fun[patc->fpt].pfunProc != NULL)
			{
				funret = patc->fun[patc->fpt].pfunProc(patc->fun[patc->fpt].pfunData);
				if(funret != RETCHAR_KEEP)
				{
					patc->fpt++;
					SamChkAtcRet(patc, "OK\r\n\tERROR\r\n"); // Try to Find URC in time
				}
				else 
				{
					pmdm->stim = 0;
					break;
				}
			}
			else
			{
				patc->fpt++;
			}
		}
		if(patc->fpt == MDMFUNARRAY_MAX) patc->fpt = 0;
	}
	
	switch(pmdm->sta)
	{
		case FUN0_MDMSTA :
			if(pmdm->step == 0)
			{
				while(SamChkAtcRet(patc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET);
				SamSendAtCmd(patc, "AT+CFUN=0\r", CRLF_HATCTYP, 9);
				pmdm->step += WMDMRET_BIT;
				pmdm->stim = 0;
				DebugTrace("Close %d ATC Channel of Modem!\r\n", pmdm->patc->comid);
			}
			else if(pmdm->step >= WMDMRET_BIT)
			{
				ratcret = SamChkAtcRet(patc, "OK\r\n\tERROR\r\n");
				if(ratcret == NOSTRRET_ATCRET)
				{
				    break;
				}
				else if(ratcret == 2 || ratcret == OVERTIME_ATCRET)
				{
					pmdm->step -= WMDMRET_BIT;
				}
				else if(ratcret == 1)
				{
					pmdm->step = 3;
					pmdm->stim = 0;
					DebugTrace("Close %d Ch of Modem!\r\n", pmdm->patc->comid);
				}
				patc->state = IDLE_HATCSTA;
				patc->retbufp = 0;
                patc->retbuf[0] = 0;
			}
			else
			{
				if(pmdm->stim >= 250)
				{
					pmdm->step = 0;
					pmdm->stim = 0;
				}
				else if((pmdm->stim & 0x01) == 0)
				{
					SamChkAtcRet(patc, "OK\r\n\tERROR\r\n");
				}
			}
			break;
		case INIT_MDMSTA :
			if(pmdm->step == 0)
			{
				pmdm->dcnt++;
				if(pmdm->dcnt > 9)
				{
					pmdm->sta = FAIL_MDMSTA;
					pmdm->step = 0;
					break;
				}
				SamSendAtCmd(patc, "AT\r", CRLF_HATCTYP, 3);
				pmdm->step += WMDMRET_BIT;
				pmdm->stim = 0;
				pmdm->imei[0] = 0;
				pmdm->imsi[0] = 0;
				pmdm->ccid[0] = 0;
				pmdm->conditon = 0;
				
				strcpy(pmdm->ipstrtab, "\v");
			}
			else if(pmdm->step == 1 && pmdm->stim >= 2)
			{
				pmdm->dcnt++;
				if(pmdm->dcnt > 6)
				{
					pmdm->sta = FAIL_MDMSTA;
					pmdm->step = 0;
					break;
				}
				SamSendAtCmd(patc, "AT\rATE0\rAT+CMEE=0\rAT+CGMR\rAT+CFUN=1\r\t1000\rAT+CPIN?\r\t1000\r", CRLF_HATCTYP, 9);
				pmdm->step += WMDMRET_BIT;
				pmdm->stim = 0;
			}
			else if(pmdm->step == 2 && pmdm->stim >= 2)
			{
				pmdm->dcnt++;
				if(pmdm->dcnt > 3)
				{
					pmdm->sta = FAIL_MDMSTA;
					pmdm->step = 0;
					break;
				}
				strcpy(buf, "AT+SIMEI?\rAT+CICCID\rAT+CCID\rAT+CIMI\r");
				tbuf[0] = 0;
				
				strcpy(tbuf, "AT+CFUN=4\r\t1000\r");
				n = ReadCfgTab(pmdm->cfg, CFGMDM_HEADSTR, CFGMDM_PDNCFG, sbuf);
				for(i=0; i<pmdm->pdncnt; i++)
				{
					if(GetPmrStr(sbuf, ',', i+1, dbuf, 120) < 1)
					{
						continue;
					}
					else
					{// //pdncnt,pdncid1,pdnip1,pdnapn1,pdnauth1,pdnusr1,pdnpwd1,pdncid2,pdnip2,pdnapn2,pdnauth2,pdnusr2,pdnpwd2,....
						strcat(tbuf, "AT+CGDCONT="); //1,"IP","cmiot"
		                GetPmrStr(sbuf, ',', (i*6)+1, dbuf, 120);
						strcat(tbuf, dbuf); 
						strcat(tbuf, ",");
						GetPmrStr(sbuf, ',', (i*6)+2, dbuf, 120);
						strcat(tbuf, dbuf);
						strcat(tbuf, ",");
						GetPmrStr(sbuf, ',', (i*6)+3, dbuf, 120);
						strcat(tbuf, dbuf);
						strcat(tbuf, "\r");
						
						if(GetPmrStr(sbuf, ',', (i*6)+4, dbuf, 120) < 1)
						{
							continue;
						}
						strcat(tbuf, "AT+CGAUTH="); // 1, 1,"PWR","USR"
						GetPmrStr(sbuf, ',', (i*6)+1, dbuf, 120);
						strcat(tbuf, dbuf);
						strcat(tbuf, ",");
						GetPmrStr(sbuf, ',', (i*6)+4, dbuf, 120);
						strcat(tbuf, dbuf);
						strcat(tbuf, ",");
						GetPmrStr(sbuf, ',', (i*6)+6, dbuf, 120);
						strcat(tbuf, dbuf);
						strcat(tbuf, ",");
						GetPmrStr(sbuf, ',', (i*6)+5, dbuf, 120);
						strcat(tbuf, dbuf);
						strcat(tbuf, "\r");
					}
				}
				strcat(tbuf, "AT+CFUN=1\r\t1000\r");
				
				if(Strsearch(tbuf, "AT+CGDCONT=") != 0)
				{
					strcat(buf, tbuf);
				}
				SamSendAtCmd(patc, buf, CRLF_HATCTYP, 15);
				pmdm->step += WMDMRET_BIT;
				pmdm->stim = 0;
				
			}
			else if(pmdm->step == 3 && pmdm->stim >= 2)
			{
				pmdm->dcnt++;
				if(pmdm->dcnt > 90)
				{
					pmdm->sta = FAIL_MDMSTA;
					pmdm->step = 0;
					break;
					
				}
				SamSendAtCmd(patc, "AT+CSQ;+CGATT?\r", CRLF_HATCTYP, 3);
				pmdm->step += WMDMRET_BIT;
				pmdm->stim = 0;
			}
			else if(pmdm->step == 4)
			{
				pmdm->dcnt++;
				if(pmdm->dcnt > 3)
				{
					pmdm->sta = FAIL_MDMSTA;
					pmdm->step = 0;
					break;
				}
				//SamSendAtCmd(patc, "AT+SIMEI?\rAT+CICCID\rAT+CCID\rAT+CIMI\r", CRLF_HATCTYP, 3);
				SamSendAtCmd(patc, "AT+CPSI?\r", CRLF_HATCTYP, 3);
				pmdm->step += WMDMRET_BIT;
				pmdm->stim = 0;
				
			}
			else if(pmdm->step == 5 && pmdm->stim >= 2)
			{
				pmdm->dcnt++;
				if(pmdm->dcnt > 3)
				{
					pmdm->sta = FAIL_MDMSTA;
					pmdm->step = 0;
					break;
				}
				else
				{
					buf[0] = 0;
					ReadCfgTab(pmdm->cfg, CFGMDM_HEADSTR, CFGMDM_PDNCFG, sbuf);
					for(i=0; i < pmdm->pdncnt; i++)
					{
						if(GetPmrStr(sbuf, ',', (i*6)+1, dbuf, 5) == 0)
						{
							continue;
						}
						else
						{
							
							if(pmdm->atcset == ATCSET_M)
	                        {
	                            snprintf(tbuf, 128, "AT+CNACT=%s,1\r", dbuf);
	                        }
	                        else
	                        {
	                            snprintf(tbuf, 128,"AT+CGACT=1,%s\r", dbuf);
	                        }	
						}
						strcat(buf, tbuf);
					}
					if(strlen(buf) < 5)
					{	
						pmdm->step++;
						pmdm->stim = 3;
						pmdm->dcnt = 0;
					}
					else
					{
						strcat(buf, "\t3000\r");
						SamSendAtCmd(patc, buf, CRLF_HATCTYP, 60);
						pmdm->step += WMDMRET_BIT;
						pmdm->stim = 0;
					}
				}
				
			}
			else if(pmdm->step == 6 && pmdm->stim >= 3)
			{
				pmdm->dcnt++;
				if(pmdm->dcnt > 3)
				{
					pmdm->sta = FAIL_MDMSTA;
					pmdm->step = 0;
					break;
				}
				SamSendAtCmd(patc, "AT+CGPADDR\rAT+CGCONTRDP\rAT+CSQ\rAT+CPSI?\r", CRLF_HATCTYP, 9);
				pmdm->step += WMDMRET_BIT;
				pmdm->stim = 0;
			}
			else if(pmdm->step == 7)
			{
				pmdm->sta = FFUN_MDMSTA;
				pmdm->step = 0;
				pmdm->stim = 0;
				pmdm->dcnt = 0;
			}
			else if(pmdm->step >= WMDMRET_BIT)
			{
				ratcret = SamChkAtcRet(patc, "OK\r\n\tERROR\r\n\t+CPIN:\t+CGATT:\t+CPSI:\t+CGPADDR:\t+SIMEI:\t+ICCID:\t");
				if(ratcret == NOSTRRET_ATCRET)
				{
				    break;
				}
				else if(ratcret == 2 || ratcret == OVERTIME_ATCRET)
				{
					if(patc->state != SCED_HATCSTA)
					{
						SamSendAtSeg(patc);
					}
					else
					{
						pmdm->step -= WMDMRET_BIT;
						patc->state = IDLE_HATCSTA;
					}
				}
				else if(ratcret == 1 || ratcret == DELAYFIN_ATCRET)
				{
					if(patc->state == SCED_HATCSTA)
					{
						pmdm->step -= WMDMRET_BIT;
                        patc->state = IDLE_HATCSTA;
						if(((pmdm->conditon & CPINR_MDMCND) != 0 && pmdm->step == 1)
							||((pmdm->conditon & PSREG_MDMCND) != 0 && pmdm->step == 3)
                            || pmdm->step >= 4||pmdm->step == 2||pmdm->step == 0)
						{
							pmdm->step++;
							pmdm->dcnt = 0;
							patc->waitret =	STOP_HATCTMW;
						}
					}
					else
					{
						SamSendAtSeg(patc);
					}
					pmdm->conditon |= ATCOK_MDMCND;
					if(pmdm->step == 1 && (pmdm->conditon & CFUN0_MDMCND) != 0)
					{
						pmdm->sta = FUN0_MDMSTA;
						pmdm->stim = 0;
                		pmdm->step = 0;
					}
				}
				else if(ratcret == 3)
				{
					if(Strsearch(pmdm->patc->retbuf, "+CPIN: READY") != 0)
					{
						pmdm->conditon |= CPINR_MDMCND;
                    	DebugTrace("SIM Card is Ready!\r\n");
					}
					else if(Strsearch(pmdm->patc->retbuf, "+CPIN: PIN") != 0)
					{
						pmdm->conditon &= ~CPINR_MDMCND;
						pmdm->conditon |= WFPIN_MDMCND;
                    	DebugTrace("SIM Card FOR PIN!\r\n");
					}
				}
				else if(ratcret == 4)
				{
					if(Strsearch(pmdm->patc->retbuf, "+CGATT: 1") != 0)
					{
						pmdm->conditon |= PSREG_MDMCND;
                    	DebugTrace("PS network is Ready!\r\n");
					}
				}
				else if(ratcret == 5)
				{
					if(Strsearch(pmdm->patc->retbuf, "+CPSI:") != 0)
					{

                    	//DebugTrace("Get IP1:%s\r\n", pmdm->ipsstr);
					}
				}
				else if(ratcret == 6)
				{//+CGPADDR: 1, "10.88.44.193"
					n = 8;
					buf[0] = 0;
					for(i=9, j=0; j<48; j++)
					{
                        if(j == 0)
                        {
                        	while((pmdm->patc->retbuf[i] < '1' || pmdm->patc->retbuf[i] > '9'))
                        	{
								i++;
                        	}
							n = pmdm->patc->retbuf[i++] - '1';
                            while(pmdm->patc->retbuf[i] < '1' || pmdm->patc->retbuf[i] > '9')
                            {
                                i++;
                            }
							j = 0;
                        }
						buf[j] = pmdm->patc->retbuf[i++];
						if((buf[j] < '0' || buf[j] > '9') && buf[j] != '.')
						{
							buf[j] = 0;
							break;
						}
					}
					if(n < 6 && buf[0] != 0)
					{
						WriteCfgTab(pmdm->ipstrtab, NULL, n, buf);
						pmdm->conditon |= ((IPABIT_MDMCND<<n) & IPBMSK_MDMCND);
                    	pmdm->conditon |= IPACT_MDMCND;
						DebugTrace("IPAddr:%s\r\n", buf);
					}
				}
				else if(ratcret == 7)
				{//+SIMEI: 868110062384530
					for(i=8, j=0; j<15; j++)
					{
						pmdm->imei[j] = pmdm->patc->retbuf[i++];
						if(pmdm->imei[j] < '0' || pmdm->imei[j] > '9')
						{
							pmdm->imei[j] = 0;
							break;
						}
					}
					DebugTrace("IMEI:%s\r\n", pmdm->imei);
				}
				else if(ratcret == 8)
				{//+ICCID: 89860121801636109288
					for(i=8, j=0; j<23; j++)
					{
						pmdm->ccid[j] = pmdm->patc->retbuf[i++];
						if(pmdm->ccid[j]== '\r' || pmdm->ccid[j] == '\n')
						{
							pmdm->ccid[j] = 0;
							break;
						}
					}
					DebugTrace("CCID:%s\r\n", pmdm->ccid);
				}
				else if(pmdm->ccid[0] != 0 && pmdm->imsi[0] == 0 && (pmdm->patc->retbuf[0] >= '0' && pmdm->patc->retbuf[0] <= '9'))
				{
					for(i=0, j=0; i<15 && j<pmdm->patc->retbufp; j++)
					{
						if(pmdm->patc->retbuf[j] < '0' || pmdm->patc->retbuf[j] > '9')
						{
							break;
						}
						else
						{
							pmdm->imsi[i++] = pmdm->patc->retbuf[j];
						}
					}
					pmdm->imsi[i++] = 0;
					DebugTrace("IMSI:%s\r\n", pmdm->imsi);
				}
				else if(pmdm->ccid[0] == 0 && pmdm->imsi[0] == 0 && pmdm->imei[0] != 0 && pmdm->atcset == ATCSET_M && (pmdm->patc->retbuf[0] >= '0' && pmdm->patc->retbuf[0] <= '9'))
				{
					for(i=0, j=0; i<23 && j<pmdm->patc->retbufp; j++)
					{
						if(pmdm->patc->retbuf[j] == '\r' || pmdm->patc->retbuf[j] == '\n')
						{
							break;
						}
						else
						{
							pmdm->ccid[i++] = pmdm->patc->retbuf[j];
						}
					}
					pmdm->ccid[i++] = 0;
					DebugTrace("CCID:%s\r\n", pmdm->ccid);
				}
				pmdm->patc->retbufp = 0;
                pmdm->patc->retbuf[0] = 0;
			}
			break;
		case FFUN_MDMSTA :
			if(pmdm->step == 1)
			{
				pmdm->dcnt++;
				if(pmdm->dcnt > 3)
				{
					pmdm->sta = FAIL_MDMSTA;
					pmdm->step = 0;
					break;
				}
				SamSendAtCmd(patc, "AT+CPIN?;+CSQ;+CPSI?\r", CRLF_HATCTYP, 6);
				pmdm->step += WMDMRET_BIT;
				pmdm->stim = 0;
			}
			else if(pmdm->step >= WMDMRET_BIT)
			{
				ratcret = SamChkAtcRet(patc, "OK\r\n\tERROR\r\n\t+CSQ:\t+CPIN:\t+CPSI:");
				if(ratcret == NOSTRRET_ATCRET)
				{
				    break;
				}
				else if(ratcret == 2 || ratcret == OVERTIME_ATCRET)
				{
					pmdm->step -= WMDMRET_BIT;
					patc->state = IDLE_HATCSTA;
					patc->waitret =	STOP_HATCTMW;
				}
				else if(ratcret == 1)
				{
					pmdm->stim = 0;
					pmdm->step = 0;
					patc->waitret =	STOP_HATCTMW;
				}
				else if(ratcret == 3)
				{
					if(Strsearch(pmdm->patc->retbuf, "+CPIN: READY") != 0)
					{
						pmdm->conditon |= CPINR_MDMCND;
					}
				}
				else if(ratcret == 4)
				{
					if(Strsearch(pmdm->patc->retbuf, "+CPIN: READY") != 0)
					{
						pmdm->conditon |= CPINR_MDMCND;
					}
				}
				else if(ratcret == 5)
				{
					if(Strsearch(pmdm->patc->retbuf, "NO SERVICE") != 0)
					{
						pmdm->sta = FAIL_MDMSTA;
						pmdm->step = 0;
					}
				}
				patc->retbufp = 0;
                patc->retbuf[0] = 0;
			}
			else if(pmdm->stim >= 30)
			{
				pmdm->stim = 0;
				pmdm->step = 1;
				pmdm->dcnt = 0;
			}
			
			break;
		case FAIL_MDMSTA :
			if(pmdm->step == 0)
			{
				SamAtcFunUrcBroadCast(patc, "+RESET NETWORK\r\n");
				SamSendAtCmd(patc, "AT+CFUN=0\r\t3000\rAT+CFUN=1\r", CRLF_HATCTYP, 30);
				pmdm->step += WMDMRET_BIT;
			}
			else if(pmdm->step >= WMDMRET_BIT)
			{
				ratcret = SamChkAtcRet(patc, "OK\r\n\tERROR\r\n");
				if(ratcret == NOSTRRET_ATCRET)
				{
				    break;
				}
				else if(ratcret == 1 || ratcret == 2 || ratcret == DELAYFIN_ATCRET||ratcret == OVERTIME_ATCRET)
				{
					if(patc->state != SCED_HATCSTA)
					{
						SamSendAtSeg(patc);
					}
					else
					{
						patc->state = IDLE_HATCSTA;
						patc->waitret =	STOP_HATCTMW;
						
						pmdm->step -= WMDMRET_BIT;
						pmdm->stim = 90;
						pmdm->step++;
					}
				}
				patc->retbufp = 0;
                patc->retbuf[0] = 0;
			}
			if(pmdm->stim >= 120)
			{
				pmdm->stim = 0;
				DebugTrace("MDM Wrong, Init Again!\r\n");
				pmdm->sta = INIT_MDMSTA;
                pmdm->step = 0;
				pmdm->dcnt = 0;
			}
			break;
		default :
			if(pmdm->stim >= 5)
			{
				pmdm->stim = 0;
				DebugTrace("MDM Wrong, Please Init Again!\r\n");
				pmdm->sta = INIT_MDMSTA;
				pmdm->step = 0;
				pmdm->dcnt = 0;
			}
			break;
	}
    return(RETCHAR_NONE);
}
