#define __SAMMDMSEV_C

#include "include.h"

HdsAtcTag 	AtcA = {0};
HdsAtcTag * pAtcBusArray[ATCBUS_CHMAX] = {NULL};

char MdmACfgStr[256] ="\vCFGMDM_A1\t0\tA\t1,1,\"IP\",\"cmiot\",1,\"user123\",\"psw123\"\v";   //,2,\"IP\",\"cmnet\",1,\"user123\",\"psw123\"\v";
TMdmTag MdmABdy = {0};
void * pMdmA = NULL;

SamRetChar SamMdmSrvCmd(SamMdmOptCmdTag cmd, void * pin, void * pout)
{
	uint8  i;
	TMdmTag * pmdm = NULL;
	char * pstr;

	if(pMdmA == NULL) return(RETCHAR_FALSE);
	pmdm = (TMdmTag *)pMdmA;
	if(cmd > MDMCMD_CHKMDMIP && pout == NULL) return(RETCHAR_FALSE); 
	switch(cmd)
	{
		case MDMCMD_CHKMDMIP :
			if((pmdm->conditon & IPACT_MDMCND) != 0)
			{
				return(RETCHAR_MDMIPOK);
			}
			else
			{
				return(RETCHAR_FALSE);
			}
		case MDMCMD_GETIMEI :
			strcpy((char *)pout, pmdm->imei);
			return(RETCHAR_TRUE);
		case MDMCMD_GETIMSI :
			strcpy((char *)pout, pmdm->imsi);
			return(RETCHAR_TRUE);
		case MDMCMD_GETIP :
			if(pin == NULL)
			{
				i = 1;
			}
			else
			{
				i = *((uint8*)(pin));
				if(i > pmdm->pdncnt) i = 1;
			}
			ReadCfgTab(pmdm->ipstrtab, NULL, i-1, (char *)pout);
			return(RETCHAR_TRUE);
		case MDMCMD_CFGPDN :
			//2,1,\"IP\",\"cmiot\",1,\"user123\",\"psw123\",2,\"IP\",\"cmnet\",1,\"user123\",\"psw123\""
			pstr = (char *)pin;
			if(pstr == NULL || pstr[0]>'9'||pstr[0]<'0'||pstr[1]!=',') return(RETCHAR_FALSE);
			WriteCfgTab(MdmACfgStr, CFGMDM_HEADSTR, CFGMDM_PDNCFG, (char *)pin);
			return(RETCHAR_TRUE);
		default :
			return(RETCHAR_FALSE);
			break;
	}
	return(RETCHAR_FALSE);
}



void SamMdmSrvStart(void)
{
	TMdmTag * pmdm = NULL;
	pAtcBusArray[0] = SamAtcInit(&AtcA,  ATCCH_A);
	pmdm = (void *)&(MdmABdy);
	pMdmA = SamMdmInit(pmdm, MdmACfgStr);
	if(pMdmA == NULL)
	{
		DebugTrace("SamScmInit Fail\n");
		pmdm = NULL;
	}
}


void SamMdmSrvRun(void)
{
	SamMdmProc(pMdmA);
}






