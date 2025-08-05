/**
 * @file SamSocket.c
 * @brief Socket handling module implementation. 
 * @details This file contains the implementation of functions
 *        related to socket operations, including initialization, state handling, and data processing.
 * @version 1.0
 * @date 2025-05-15
 * @author lixianshuai
 * @copyright (c) Copyright 2025-2030, ae@sim.com
 * 
 * @note 
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include.h"

#include "SamSocket.h"
#include "SamDebug.h"
#include "SamAtc.h"

#define Sam_Mdm_Atc_checkAtRsp SamChkAtcRet
#define Sam_Mdm_Atc_sendAtCmd SamSendAtCmd
#define Sam_Mdm_Atc_sendAtSeg SamSendAtSeg

/**
 * @brief Transfer the state of the socket module.
 * @param self Pointer to the socket module instance.
 * @param stat The new state to transfer to.
 */
static void stateTransfer(struct Sam_Mdm_Socket_t *self, uint8_t stat);

/**
 * @brief Handle the unsolicited result code (URC) from the AT command.
 * @param context Pointer to the context, usually the socket module instance.
 * @param urcBuff Pointer to the URC buffer.
 * @return The result code indicating the handling status.
 */
static uint8_t handleAtUrc(void* context, char* urcBuff);


static uint8_t Sam_Mdm_Atc_getState(Sam_Mdm_Atc_t *phatc) {
    return phatc->state;
}

static char *Sam_Mdm_Atc_getRevBuff(Sam_Mdm_Atc_t *phatc) {
    return phatc->retbuf;
}

static uint16_t Sam_Mdm_Atc_getRevBuffLen(Sam_Mdm_Atc_t *phatc) {
    return phatc->retbufp;
}

static void Sam_Mdm_Atc_SetData(Sam_Mdm_Atc_t *phatc, char * databuf, uint16_t databufp) {
    phatc->databuf = databuf;
    phatc->databufp = databufp;
}

static void Sam_Mdm_Atc_SetType(Sam_Mdm_Atc_t *phatc, uint8_t  type) {
    phatc->type = type;
}


static void Sam_Mdm_Atc_clearAtRevBuff(Sam_Mdm_Atc_t* self) {
    if (self == NULL)
    {
        return;
    }

    self->retbufp= 0;
    self->retbuf[0] = 0;
}

static void Sam_Mdm_Atc_freeUse(Sam_Mdm_Atc_t* self) {
    if (self == NULL) return;
    
    self->state = IDLE_HATCSTA;
}

/**
 * @brief Initialize the socket module.
 * @param self Pointer to the socket module instance.
 * @param cfgstr Pointer to the configuration string.
 * @return true if initialization is successful, false otherwise.
 *
 * The configuration string format is as follows:
 * "\vCFGSCT_M1\t${atChannel}\t${atType}\t${socketId}\t${cipmode}\t${type}\t${rxform}\t${host}\t${port}\t${localport}\v"
 * where:
 * - ${atChannel}: AT channel ID (e.g., 0)
 * - ${atType}: AT type (e.g., 'A')
 * - ${socketId}: Socket ID, range 0~9
 * - ${cipmode}: CIP mode, refer to Sam_Mdm_Socket_Cipmode_t
 * - ${type}: Socket type, refer to Sam_Mdm_Socket_Type_t
 * - ${rxform}: RX get form type, refer to Sam_Mdm_Socket_Type_t
 * - ${host}: Server host name or IP address (e.g., "117.131.85.139")
 * - ${port}: Server port (e.g., 60057)
 * - ${localport}: Local port (e.g., 5000)
 */
bool Sam_Mdm_Socket_init(struct Sam_Mdm_Socket_t* self, const char * cfgstr) {
    if ((self == NULL)  || (cfgstr == NULL)) {
        return false;
    }    
    sam_dbg_set_module_level(SAM_MOD_SOCKET, SAM_DBG_LEVEL_TRACE);

// char cfgstr[] = "\vCFGSCT_M1\t0\tA\t0\t0\t0\t1\t117.131.85.142\t60044\t5000\v"
    // Parse the configuration string
    sscanf(cfgstr, "\vCFGSCT_M1\t%u\tA\t%u\t%u\t%u\t%u\t%s\t%u\t%u\v", 
        (uint32_t *)&self->config.atChannelId, 
        (uint32_t *)&self->config.socketId, 
        (uint32_t *)&self->config.cipmode,
        (uint32_t *)&self->config.type,
        (uint32_t *)&self->config.rxform,
        self->config.host,
        (uint32_t *)&self->config.port,
        (uint32_t *)&self->config.localport
        );

    if (self->config.type == SAM_MDM_SOCKET_TYPE_TCP_SERVER)
        self->config.srvIndex = self->config.socketId;
    else
        self->config.srvIndex = 0xFF;

    // Log the configuration information
    SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_DEBUG, "\vCFGSCT_M1\t%d\tA\t%d\t%d\t%d\t%d\t%s\t%d\t%d\v\r\n", 
        self->config.atChannelId, 
        self->config.socketId, 
        self->config.cipmode,
        self->config.type,
        self->config.rxform,
        self->config.host,
        self->config.port,
        self->config.localport);
    
    // Set initial state and step
    self->base.state = 0;  
    self->base.step = 0;   
    self->base.dcnt = 0;
    self->base.msclk = SamGetMsCnt(0);
    self->base.sclk = 0;
    self->phatc = pAtcBusArray[self->config.atChannelId];
	    
    self->runlink =	SamAtcFunLink(self->phatc, self, (SamMdmFunTag)Sam_Mdm_Socket_process, (SamUrcBcFunTag)handleAtUrc);
    SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_TRACE, "Socket module initialized. runlink = %d\r\n", self->runlink);

//    if (self->config.socketId >= MAX_SOCKET_NUM) 
//    {
//        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_ERROR, "self->config.socketId >= MAX_SOCKET_NUM\n");
//        return false;
//    }
//    else 
//    {
//        if (self->parent->socket[self->config.socketId] != NULL){
//            SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_ERROR, "Socket[%d] already created.\n", self->config.socketId);
////            Sam_Mdm_Socket_Destroy(self->parent->socket[self->config.socketId] );
//        }
//        self->parent->socket[self->config.socketId] = self;    
//    }
    return true;
}

/**
 * @brief Deinitialize the socket module.
 * @param self Pointer to the socket module instance.
 * @return true if deinitialization is successful, false otherwise.
 */
bool Sam_Mdm_Socket_deinit(struct Sam_Mdm_Socket_t* self) {
    if (self == NULL) {
        return false;
    }
    // Perform deinitialization operations, such as closing devices
    SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_TRACE, "Socket module deinitialized.\r\n");
    return true;
}

/**
 * @brief Handle the unsolicited result code (URC) from the AT command.
 * @param context Pointer to the context, usually the socket module instance.
 * @param urcBuff Pointer to the URC buffer.
 * @return The result code indicating the handling status.
 */
static uint8_t handleAtUrc(void* context, char* urcBuff){
    if ((context == NULL) || (urcBuff == NULL)) {
        return RETCHAR_NONE;
    }
    
    Sam_Mdm_Socket_t *self = (Sam_Mdm_Socket_t *)context;
    char buf[256] = {0};
    uint8_t temp = 0;

    // Log the URC handling information
    if (self->config.type == SAM_MDM_SOCKET_TYPE_TCP_SERVER)
        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_DEBUG, "\r\n===========>Server[%d] handleAtUrc %s\r\n", self->config.srvIndex, urcBuff);
    else
        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_DEBUG, "\r\n===========>Socket[%d] handleAtUrc %s\r\n", self->config.socketId, urcBuff);
    
    // Construct the comparison string
    sprintf(buf, "+CIPRXGET: 1,%u\r\t+IPCLOSE: %u\t+CLIENT: ", self->config.socketId, self->config.socketId);
    temp = StrsCmp(urcBuff, buf);

    if (temp != 0)
    {
        if ((self->base.state == SAM_MDM_SOCKET_STATE_CLOSED)
            || (self->base.state == SAM_MDM_SOCKET_STATE_INIT))
        {
            SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_DEBUG, "Socket[%d] handleAtUrc ingnore...\r\n", self->config.socketId);
            return temp;
        }
    }

    if (temp == 1) 
    { // received +CIPRXGET: 1
        self->dnflag = true;
    }
    else if (temp == 2) 
    { // received +IPCLOSE:
        uint32_t link_num, reason;
        sscanf(urcBuff,"+IPCLOSE: %u,%u", &link_num, &reason);
        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_INFO, "Socket[%d] closed, reason:%u\r\n", link_num, reason);
        if (reason == SAM_MDM_SOCKET_CLOSED_REMOTE)
        {
            // closed by remote, two actions to select.
            // 1. auto reconnect byself
//            self->error = SAM_MDM_SOCKET_ERROR_REMOTE_CLOSE;
//            stateTransfer(self, SAM_MDM_SOCKET_STATE_ERROR);      
            
            // 2. call eventcall to notify to user, let user to open again.
            stateTransfer(self, SAM_MDM_SOCKET_STATE_CLOSED);      
            if (self->eventCallback != NULL)
            {
                self->eventCallback(link_num, SAM_MDM_SOCKET_EVENT_CLOSED_PASSIVE, NULL, self->context);
            }  
        }
    }
    else if (temp == 3) // tcp server accepted a new client socket.
    {
        uint32_t link_num, srvIndex, port;
        char remoteIp[64] = {0};
        sscanf(urcBuff,"+CLIENT: %u,%u,%s:%u", &link_num, &srvIndex, remoteIp, &port); // +CLIENT: 0,0,10.164.6.156:59371
//        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_INFO, "server[%d] handleAtUrc +CLIENT: %u,%u,%s:%u", self->config.srvIndex, link_num, srvIndex, remoteIp, port);

        if (srvIndex == self->config.srvIndex)
        {
            Sam_Mdm_Socket_t Client = {0};
            
            // Initialize the client socket configuration
            Client.config.atChannelId = self->config.atChannelId;
            Client.config.socketId = link_num;
            Client.config.srvIndex = srvIndex;
            Client.config.port = port;
            strcpy(Client.config.host, remoteIp);
            Client.config.type = SAM_MDM_SOCKET_TYPE_TCP;
            Client.config.cipmode = self->config.cipmode;
            Client.config.rxform = self->config.rxform;

            Client.base.state = SAM_MDM_SOCKET_STATE_CONNECTED;

            if (self->eventCallback != NULL)
            {
                self->eventCallback(srvIndex, SAM_MDM_SOCKET_EVENT_ACCEPT, (void *)&Client, self->context);
            }
        }
        else 
        {
            return RETCHAR_NONE;
        }
        
    }

    return temp;
}

/**
 * @brief Transfer the state of the socket module.
 * @param self Pointer to the socket module instance.
 * @param stat The new state to transfer to.
 */
static void stateTransfer(struct Sam_Mdm_Socket_t *self, uint8_t stat){
    if (self == NULL) {
        return ;
    }

    // Log the state transfer information
    if (self->config.type == SAM_MDM_SOCKET_TYPE_TCP_SERVER)
        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_INFO, "Server[%d] state transfer %d ==> %d\r\n", self->config.srvIndex, self->base.state, stat);
    else
        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_INFO, "Socket[%d] state transfer %d ==> %d\r\n", self->config.socketId, self->base.state, stat);
    self->base.state = stat;

    // Reset step, clock, and retry count
    self->base.step = 0;
    self->base.sclk = 0;
    self->base.dcnt = 0;

    if (stat == SAM_MDM_SOCKET_STATE_CONNECTED)
    {
        self->openReTryCnt = 0;
    }
    else if (stat == SAM_MDM_SOCKET_STATE_CLOSED)
    {        
        // Unlink the AT command functions and clear the socket module
        SamAtcFunUnlink(self->phatc, self->runlink);
//        memset(self, 0x00, sizeof(Sam_Mdm_Socket_t));
    }
    
    return;
}

/**
 * @brief Handle the initialization state of the socket.
 * @param self Pointer to the socket module instance.
 * @return The result code indicating the handling status.
 *
 * This function checks and opens the network.
 * Step 0: Send AT commands ("AT+NETCLOSE\rAT+CIPMODE=%u\rAT+NETOPEN\r")
 * Step 1: Check the result of step 0; if return +NETOPEN: 0, goto opening state, else goto error state.
 */
static uint8_t handleInitState(struct Sam_Mdm_Socket_t *self) {
    uint8_t ratcret = 0;
    char buf[256] = {0};
    Sam_Mdm_Atc_t *phatc = self->phatc;
    if (phatc == NULL)
    {
        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_ERROR, "patc == NULL\r\n");
        return RETCHAR_FREE;
    }
    
    switch (self->base.step) {
        case 0: {
                if (self->base.sclk < self->openReTryCnt * 10) return RETCHAR_FREE; // open retry timer, 10s, 20s, 30s ... 
                
                while(Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) Sam_Mdm_Atc_clearAtRevBuff(phatc);

                Sam_Mdm_Atc_sendAtCmd(phatc, "AT+NETOPEN?\r", CRLF_HATCTYP, 10);
                self->base.step++;
                self->base.sclk = 0;
            }            
            break;
            
        case 1: {
                ratcret = Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n\t+NETOPEN:");
                if (ratcret == NOSTRRET_ATCRET)
                {
                    // continue wait
                    return RETCHAR_KEEP;
                }
                else if (ratcret == OVERTIME_ATCRET)
                {                    
                    self->base.dcnt++;
                    if(self->base.dcnt < 3)
                    {
                        self->base.step--;
                        self->base.sclk = 0;
                    }
                    else
                    {
                        self->error = SAM_MDM_SOCKET_ERROR_AT_NORESPONSE;
                        stateTransfer(self, SAM_MDM_SOCKET_STATE_ERROR);
                        return RETCHAR_KEEP;
                    }
                }
                else if (ratcret == 3) // received +NETOPEN:
                {                    
                    uint32_t result = 0;
                    sscanf((const char *)Sam_Mdm_Atc_getRevBuff( phatc),"+NETOPEN: %d", &result);
                    SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_TRACE, "step 1: +NETOPEN: %d\r\n", result);
                    if (result == 1) // NET OPENED
                    {
                        stateTransfer(self, SAM_MDM_SOCKET_STATE_OPENING);
                    }
                    else 
                    {
                        self->base.step++;
                        self->base.sclk = 0;
                        self->base.dcnt = 0;
                    }
                    Sam_Mdm_Atc_clearAtRevBuff(phatc);
                    while(Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) Sam_Mdm_Atc_clearAtRevBuff(phatc);
                }
            }
            break;

            
        case 2: {                
                while(Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) Sam_Mdm_Atc_clearAtRevBuff(phatc);

                sprintf(buf, "AT+CIPMODE=%u\rAT+NETOPEN\r", self->config.cipmode);
                Sam_Mdm_Atc_sendAtCmd(phatc, buf, CRLF_HATCTYP, 120);
                self->base.step++;
                self->base.sclk = 0;
            }            
            break;
            
        case 3: {
                ratcret = Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n\t+NETOPEN:\t+NETCLOSE\t+IPCLOSE\t+CIPCLOSE");
                if (ratcret == NOSTRRET_ATCRET)
                {
                    // continue wait
                    return RETCHAR_KEEP;
                }
                else if (ratcret == OVERTIME_ATCRET)
                {                    
                    self->base.dcnt++;
                    if(self->base.dcnt < 3)
                    {
                        self->base.step--;
                        self->base.sclk  = 0;
                    }
                    else
                    {
                        self->error = SAM_MDM_SOCKET_ERROR_AT_NORESPONSE;
                        stateTransfer(self, SAM_MDM_SOCKET_STATE_ERROR);
                        return RETCHAR_KEEP;
                    }
                }
                else if ((ratcret == 1) || (ratcret == 2) || (ratcret == DELAYFIN_ATCRET))
                {
                    if(Sam_Mdm_Atc_getState( phatc)!= SCED_HATCSTA)
                    {
                        Sam_Mdm_Atc_sendAtSeg(phatc);
                    }
                    Sam_Mdm_Atc_clearAtRevBuff(phatc);
                }
                else if (ratcret == 3) // received +NETOPEN:
                {                    
                    uint32_t result = 0;
                    sscanf((const char *)Sam_Mdm_Atc_getRevBuff( phatc),"+NETOPEN: %d", &result);
                    SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_TRACE, "step 3: +NETOPEN: %d\r\n", result);
                    if (result == 0) // NETOPEN NO ERROR
                    {
                        stateTransfer(self, SAM_MDM_SOCKET_STATE_OPENING);
                    }
                    else 
                    {
                        self->error = SAM_MDM_SOCKET_ERROR_NET_OPEN;
                        stateTransfer(self, SAM_MDM_SOCKET_STATE_ERROR);
                    }
                    Sam_Mdm_Atc_clearAtRevBuff(phatc);
                }
                else if ((ratcret == 4)||(ratcret == 5)||(ratcret == 6)) // received +NETCLOSE:, Ignore; or received +CIPCLOSE, close the old socket.
                {
                    Sam_Mdm_Atc_clearAtRevBuff(phatc);
                }
            }
            break;

            
        default:
            break;
    }

    return RETCHAR_KEEP;
}

/**
 * 处理 socket opening 状态
 * check and open net
 * step 0: send AT("AT+CIPCLOSE=%u\rAT+CIPRXGET=1\r")
 * step 1: check the result of step 0; send at segment in step 0 and goto step 2.
 * step 2: send AT(AT+CIPOPEN)
 * step 3: check the result of step 2
 */
static uint8_t handleOpeningState(struct Sam_Mdm_Socket_t *self) {
    uint8_t ratcret = 0;
    char buf[256] = {0};
    Sam_Mdm_Atc_t *phatc = self->phatc;
    if (phatc == NULL)
    {
        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_ERROR, "patc == NULL\r\n");
        return RETCHAR_FREE;
    }

    switch (self->base.step) {
        case 0:{
                while(Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) Sam_Mdm_Atc_clearAtRevBuff(phatc);

                if (self->config.type == SAM_MDM_SOCKET_TYPE_TCP_SERVER)
                    sprintf(buf, "AT+CIPRXGET=1\r");
                else
                    sprintf(buf, "AT+CIPCLOSE=%u\rAT+CIPRXGET=1\r", self->config.socketId);
                Sam_Mdm_Atc_sendAtCmd(phatc, buf, CRLF_HATCTYP, 10);
                self->base.step++;
                self->base.sclk = 0;
            }
            break;
            
        case 1:{
                ratcret = Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n");
                if (ratcret == NOSTRRET_ATCRET)
                {
                    // continue wait
                    return RETCHAR_KEEP;
                }
                else if (ratcret == OVERTIME_ATCRET)
                {                    
                    self->base.dcnt++;
                    if(self->base.dcnt < 3)
                    {
                        self->base.step--;
                        self->base.sclk  = 0;
                    }
                    else
                    {
                        self->error = SAM_MDM_SOCKET_ERROR_AT_NORESPONSE;
                        stateTransfer(self, SAM_MDM_SOCKET_STATE_ERROR);
                        return RETCHAR_KEEP;
                    }
                }
                else if ((ratcret == 1) || (ratcret == 2)) // received OK or ERROR:
                { 
                    if(Sam_Mdm_Atc_getState(phatc)== SCED_HATCSTA)
                    {
                        self->base.step++;
                        self->base.dcnt = 0;
                        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_TRACE, "step 1 ==> 2\r\n");
                    }
                    else
                    {
                        Sam_Mdm_Atc_sendAtSeg(phatc);
                    }
                    self->base.sclk = 0;
                }
                Sam_Mdm_Atc_clearAtRevBuff(phatc);
            }
            break;

        case 2: {
                while(Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) Sam_Mdm_Atc_clearAtRevBuff(phatc);

                if (self->config.type == SAM_MDM_SOCKET_TYPE_TCP)
                {
                    sprintf(buf, "AT+CIPOPEN=%u, \"TCP\", \"%s\", %u\r", self->config.socketId, self->config.host, self->config.port);
                }
                else if (self->config.type == SAM_MDM_SOCKET_TYPE_UDP)
                {
                    sprintf(buf, "AT+CIPOPEN=%u, \"UDP\",,, %u\r", self->config.socketId, self->config.localport);
                }
                else if (self->config.type == SAM_MDM_SOCKET_TYPE_TCP_SERVER)
                {
                    sprintf(buf, "AT+SERVERSTART=%u,%u\r", self->config.localport, self->config.srvIndex);
                }
                
                Sam_Mdm_Atc_sendAtCmd(phatc, buf, CRLF_HATCTYP, 120);
                self->base.step++;
                self->base.sclk = 0;
            }
            break;
            
        case 3: {
                ratcret = Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n\t+CIPOPEN:");
                if (ratcret == NOSTRRET_ATCRET)
                {
                    // continue wait
                    return RETCHAR_KEEP;
                }
                else if (ratcret == OVERTIME_ATCRET)
                {                    
                    self->base.dcnt++;
                    if(self->base.dcnt < 3)
                    {
                        self->base.step--;
                        self->base.sclk  = 0;
                    }
                    else
                    {
                        self->error = SAM_MDM_SOCKET_ERROR_AT_NORESPONSE;
                        stateTransfer(self, SAM_MDM_SOCKET_STATE_ERROR);
                        return RETCHAR_KEEP;
                    }
                }
                else if (ratcret == 1)
                {
                    if (self->config.type == SAM_MDM_SOCKET_TYPE_TCP_SERVER)
                        stateTransfer(self, SAM_MDM_SOCKET_STATE_CONNECTED);
                }
                else if (ratcret == 2)
                {
                    SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_ERROR, "AT+CIPOPEN return error\r\n");
                    self->error = SAM_MDM_SOCKET_ERROR_CIP_OPEN;
                    stateTransfer(self, SAM_MDM_SOCKET_STATE_ERROR);
                }
                else if (ratcret == 3) // received +CIPOPEN:
                {                    
                    uint32_t result = 0;
                    uint32_t sockid = 0;
                    sscanf((const char *)Sam_Mdm_Atc_getRevBuff(phatc),"+CIPOPEN: %u,%u", &sockid, &result);
                    if (sockid == self->config.socketId)
                    {
                        if (result == 0) // CIPOPEN NO ERROR
                        {
                            // register URC, 
                            // +CIPRXGET: 1,0 
                            // +IPCLOSE: 0,1
				// sprintf(buf, "+CIPRXGET: 1,%u\r\t+IPCLOSE: %u", self->config.socketId, self->config.socketId);
				// self->urcMask = Sam_Mdm_Atc_regUrc(phatc, buf, handleAtUrc, (void *)self);
                            stateTransfer(self, SAM_MDM_SOCKET_STATE_CONNECTED);
                        }
                        else 
                        {
                            SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_ERROR, "+CIPOPEN return error\r\n");
                            self->error = SAM_MDM_SOCKET_ERROR_CIP_OPEN;
                            stateTransfer(self, SAM_MDM_SOCKET_STATE_ERROR);
                        }

//                        while(Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) Sam_Mdm_Atc_clearAtRevBuff(phatc);
                        Sam_Mdm_Atc_freeUse(phatc);
                    }
                }
                Sam_Mdm_Atc_clearAtRevBuff(phatc);
            }
            break;

            
        default:
            break;
    }

    return RETCHAR_KEEP;
}

// 处理 socket connected 状态
static uint8_t handleConnectedState(struct Sam_Mdm_Socket_t *self) {
    if (self == NULL) {
        return RETCHAR_FREE;
    }

    if (self->upcnt != 0)
    {
        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_TRACE, "socket have %u data to send.\r\n", self->upcnt );
        stateTransfer(self, SAM_MDM_SOCKET_STATE_SENDING);
        return RETCHAR_KEEP;
    }

    if (self->dnflag)
    {
        stateTransfer(self, SAM_MDM_SOCKET_STATE_RECEIVING);
        return RETCHAR_KEEP;
    }

    if (self->closeType != 0)
    {
        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_TRACE, "self->closeType:%d.\r\n", self->closeType );
        stateTransfer(self, SAM_MDM_SOCKET_STATE_CLOSING);
        return RETCHAR_KEEP;
    }

    return RETCHAR_FREE;
}

// 处理 socket sending 状态
static uint8_t handleSendingState(struct Sam_Mdm_Socket_t *self) {
    if (self == NULL) {
        return RETCHAR_FREE;
    }
    
    uint8_t ratcret = 0;
    char buf[256] = {0};
    Sam_Mdm_Atc_t *phatc = self->phatc;
    if (phatc == NULL)
    {
        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_ERROR, "patc == NULL\r\n");
        return RETCHAR_FREE;
    }

    switch (self->base.step) {
        case 0: {
                if (self->upcnt == 0)
                {
                    SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_TRACE, "handleSendingState nothing to send\r\n");
                    Sam_Mdm_Atc_freeUse(phatc);
                    stateTransfer(self, SAM_MDM_SOCKET_STATE_CONNECTED);
                    return RETCHAR_FREE;
                }
                
                while(Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) Sam_Mdm_Atc_clearAtRevBuff(phatc);

                if (self->config.type == SAM_MDM_SOCKET_TYPE_TCP)
                {
                    sprintf(buf, "AT+CIPSEND=%u,%u\r", self->config.socketId, self->upcnt);
                }
                else if (self->config.type == SAM_MDM_SOCKET_TYPE_UDP)
                {
                    sprintf(buf, "AT+CIPSEND=%u,%u,\"%s\",%u\r", self->config.socketId, self->upcnt, self->config.host, self->config.port);
                }
                Sam_Mdm_Atc_SetData(phatc, self->upbuf, self->upcnt);
                Sam_Mdm_Atc_sendAtCmd(phatc, buf, CRLF_HATCTYP|RIGR_HATCTYP, 120);
                self->base.step++;
                self->base.sclk = 0;
            }
            break;
            
        case 1: {
                ratcret = Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n\t+CIPSEND:\t>");
                if (ratcret == NOSTRRET_ATCRET)
                {
                    // continue wait
                    return RETCHAR_KEEP;
                }
                else if (ratcret == OVERTIME_ATCRET)
                {                    
                    self->base.dcnt++;
                    if(self->base.dcnt < 3)
                    {
                        self->base.step--;
                        self->base.sclk  = 0;
                    }
                    else
                    {
                        self->error = SAM_MDM_SOCKET_ERROR_AT_NORESPONSE;
                        stateTransfer(self, SAM_MDM_SOCKET_STATE_ERROR);
                        return RETCHAR_KEEP;
                    }
                }
                else if (ratcret == 3)
                {                    
                    uint32_t link_num, req_len, cnf_len;
                    sscanf((const char *)Sam_Mdm_Atc_getRevBuff(phatc),"+CIPSEND: %u,%u,%u", &link_num, &req_len, &cnf_len);
                    SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_INFO, "socket[%d] send date %d,  sent %d\r\n", link_num, req_len, cnf_len);
                    
                    if (cnf_len != 0)
                    {
                        memcpy(self->upbuf, &self->upbuf[cnf_len], self->upcnt-cnf_len);
                        self->upcnt -= cnf_len;
                        self->base.step = 0;
                        self->base.sclk = 0;
                        self->base.dcnt = 0;
                        Sam_Mdm_Atc_clearAtRevBuff(phatc);
                        while(Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) Sam_Mdm_Atc_clearAtRevBuff(phatc);
                    }
                }
                else if ((ratcret == 1) || (ratcret == 2)) // received OK or ERROR:
                { 
                    // do nothing
                }
                Sam_Mdm_Atc_clearAtRevBuff(phatc);
            }
            break;
        default:
            break;
    }

    return RETCHAR_KEEP;
}

// 处理 socket receiving 状态
static uint8_t handleReceivingState(struct Sam_Mdm_Socket_t *self) {
    uint8_t ratcret = 0;
    char buf[256] = {0};
    Sam_Mdm_Atc_t *phatc = self->phatc;
    if (phatc == NULL)
    {
        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_ERROR, "patc == NULL\r\n");
        return RETCHAR_FREE;
    }

    switch (self->base.step) {
        case 0:{
                while(Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) Sam_Mdm_Atc_clearAtRevBuff(phatc);
                
                sprintf(buf, "AT+CIPRXGET=4,%u\r", self->config.socketId);
                Sam_Mdm_Atc_sendAtCmd(phatc, buf, CRLF_HATCTYP, 9);
                self->base.step++;
                self->base.sclk = 0;
            }
            break;
            
        case 1:{
                ratcret = Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n\t+CIPRXGET: 4");
                if (ratcret == NOSTRRET_ATCRET)
                {
                    // continue wait
                    return RETCHAR_KEEP;
                }
                else if (ratcret == OVERTIME_ATCRET)
                {                    
                    self->base.dcnt++;
                    if(self->base.dcnt < 3)
                    {
                        self->base.step--;
                        self->base.sclk  = 0;
                    }
                    else
                    {
                        self->dnflag = 0;
                        self->error = SAM_MDM_SOCKET_ERROR_AT_NORESPONSE;
                        stateTransfer(self, SAM_MDM_SOCKET_STATE_ERROR);
                        return RETCHAR_KEEP;
                    }
                }
                else if (ratcret == 3)
                {                    
                    uint32_t link_num = 0;
                    uint32_t rest_len = 0;
                    sscanf((const char *)Sam_Mdm_Atc_getRevBuff(phatc),"+CIPRXGET: 4,%u,%u", &link_num, &rest_len);
                    SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_INFO, "\r\nsocket[%d] received date rest %d\r\n", link_num, rest_len);
                    if (rest_len == 0)
                    {
                        Sam_Mdm_Atc_freeUse(phatc);
                        self->dnflag = false;
                        stateTransfer(self, SAM_MDM_SOCKET_STATE_CONNECTED);
                    }
                    else 
                    {
                        self->base.step++;
                        self->base.sclk = 0;
                        self->base.dcnt = 0;
                    }
                    Sam_Mdm_Atc_clearAtRevBuff(phatc);
                    while(Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) Sam_Mdm_Atc_clearAtRevBuff(phatc);
                }
                else if ((ratcret == 1) || (ratcret == 2)) // received OK or ERROR:
                { 
                    // nothing to do.
                }
                Sam_Mdm_Atc_clearAtRevBuff(phatc);
            }
            break;
            
        case 2:{
                while(Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) Sam_Mdm_Atc_clearAtRevBuff(phatc);

                uint8_t rxform = 0;
                if (self->config.rxform == SAM_MDM_SOCKET_RXFORM_ASCII)
                    rxform = 2;
                else
                    rxform = 3;
                sprintf(buf, "AT+CIPRXGET=%u,%u\r", rxform, self->config.socketId);
                Sam_Mdm_Atc_sendAtCmd(phatc, buf, CRLF_HATCTYP, 9);
                self->base.step++;
                self->base.sclk = 0;
            }
            break;
            
        case 3:{
                ratcret = Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n\t+CIPRXGET:");
                if (ratcret == NOSTRRET_ATCRET)
                {
                    // continue wait
                    return RETCHAR_KEEP;
                }
                else if (ratcret == OVERTIME_ATCRET)
                {                    
                    self->base.dcnt++;
                    if(self->base.dcnt < 3)
                    {
                        self->base.step--;
                        self->base.sclk  = 0;
                    }
                    else
                    {
                        self->dnflag = false;
                        self->error = SAM_MDM_SOCKET_ERROR_AT_NORESPONSE;
                        stateTransfer(self, SAM_MDM_SOCKET_STATE_ERROR);
                        return RETCHAR_KEEP;
                    }
                }
                else if (ratcret == 3)
                {                    
                    uint32_t link_num, rest_len, rxform, datelen;
                    sscanf((const char *)Sam_Mdm_Atc_getRevBuff(phatc),"+CIPRXGET: %u,%u,%u,%u", &rxform, &link_num, &datelen, &rest_len);
                    SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_INFO, "\r\nsocket[%d] received date %d,  rest %d\r\n", link_num, datelen, rest_len);
                    if (datelen == 0)
                    {
                        self->base.step = 0;
                        self->base.sclk = 0;
                        self->base.dcnt = 0;
                        Sam_Mdm_Atc_clearAtRevBuff(phatc);
                        while(Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) Sam_Mdm_Atc_clearAtRevBuff(phatc);
                    }
                    else 
                    {            
                        Sam_Mdm_Atc_SetData(phatc, self->dnbuf, datelen);
                        Sam_Mdm_Atc_SetType(phatc, BCNT_HATCTYP);
                    }
                }
                else if(ratcret == RECVBCNT_ATCRET)
                {
                    self->dncnt = Sam_Mdm_Atc_getRevBuffLen(phatc);
                    self->dnbuf[self->dncnt] = 0;
                    Sam_Mdm_Atc_SetData(phatc, NULL, ATCRDATAPT_VMAX);
                    Sam_Mdm_Atc_SetType(phatc, CRLF_HATCTYP);
                    SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_INFO, "==========:%s\r\n", self->dnbuf);
                    if (self->dataCallback != NULL)
                    {
                    	SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_TRACE, "Call user callback\r\n", self->dnbuf);
                        self->dataCallback(self->config.socketId, (const uint8_t*)self->dnbuf, self->dncnt, self->context);
                    }
                }
                else if ((ratcret == 1) || (ratcret == 2)) // received OK or ERROR:
                { 
                    self->base.step = 0;
                    self->base.sclk = 0;
                    self->base.dcnt = 0;
                }
                Sam_Mdm_Atc_clearAtRevBuff(phatc);
            }
            break;
            
        default:
            break;
    }

    return RETCHAR_KEEP;
}

// 处理 socket error 状态
static uint8_t handleClosingState(struct Sam_Mdm_Socket_t *self) {
    if (self == NULL) {
        return RETCHAR_FREE;
    }
    
    uint8_t ratcret = 0;
    char buf[256] = {0};
    Sam_Mdm_Atc_t *phatc = self->phatc;
    if (phatc == NULL)
    {
        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_ERROR, "patc == NULL\r\n");
        return RETCHAR_FREE;
    }
    
    switch (self->base.step) {
        case 0: {                
                while(Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) Sam_Mdm_Atc_clearAtRevBuff(phatc);

                if (self->config.type == SAM_MDM_SOCKET_TYPE_TCP_SERVER)
                    sprintf(buf, "AT+SERVERSTOP=%u\r", self->config.srvIndex);
                else
                    sprintf(buf, "AT+CIPCLOSE=%u\r", self->config.socketId);
                Sam_Mdm_Atc_sendAtCmd(phatc, buf, CRLF_HATCTYP, 120);
                self->base.step++;
                self->base.sclk = 0;
            }            
            break;
            
        case 1: {
                ratcret = Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n\t+CIPCLOSE:\t+SERVERSTOP:");
                if (ratcret == NOSTRRET_ATCRET)
                {
                    // continue wait
                    return RETCHAR_KEEP;
                }
                else if (ratcret == OVERTIME_ATCRET)
                {                    
                    self->base.dcnt++;
                    if(self->base.dcnt < 3)
                    {
                        self->base.step--;
                        self->base.sclk  = 0;
                    }
                    else
                    {
                        self->error = SAM_MDM_SOCKET_ERROR_AT_NORESPONSE;
                        stateTransfer(self, SAM_MDM_SOCKET_STATE_ERROR);
                        return RETCHAR_KEEP;
                    }
                }
                else if (ratcret == 3) // received +CIPCLOSE:
                {                    
                    uint32_t link_num = 0;
                    uint32_t result = 0;
                    sscanf((const char *)Sam_Mdm_Atc_getRevBuff( phatc),"+CIPCLOSE: %d,%d", &link_num, &result);
                    if (link_num == self->config.socketId)
                    {
                        memset(self->upbuf, 0x00, sizeof(self->upbuf));
                        memset(self->dnbuf, 0x00, sizeof(self->dnbuf));
                        stateTransfer(self, SAM_MDM_SOCKET_STATE_CLOSED);
                        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_TRACE, "socket self->closeType:%d.\r\n", self->closeType );
                        while(Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) Sam_Mdm_Atc_clearAtRevBuff(phatc);
                        if (self->closeType == 2)
                        {
                            Sam_Mdm_Socket_Destroy(self);
                            return RETCHAR_FREE;
                        }
                        self->closeType = 0;
                    }
                }
                else if (ratcret == 4) // received +SERVERSTOP:
                {                    
                    uint32_t link_num = 0;
                    uint32_t result = 0;
                    sscanf((const char *)Sam_Mdm_Atc_getRevBuff( phatc),"+SERVERSTOP: %d,%d", &link_num, &result);
                    if (link_num == self->config.srvIndex)
                    {
                        stateTransfer(self, SAM_MDM_SOCKET_STATE_CLOSED);
                        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_TRACE, "server self->closeType:%d.\r\n", self->closeType );
                        while(Sam_Mdm_Atc_checkAtRsp(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) Sam_Mdm_Atc_clearAtRevBuff(phatc);
                        if (self->closeType == 2)
                        {
                            Sam_Mdm_Socket_Destroy(self);
                            return RETCHAR_FREE;
                        }
                        self->closeType = 0;
                    }
                }
                else if ((ratcret == 1) || (ratcret == 2)) // received OK or ERROR:
                { 
                    // nothing to do.
                }                
                Sam_Mdm_Atc_clearAtRevBuff(phatc);
            }
            break;

            
        default:
            break;
    }

    return RETCHAR_KEEP;
}

// 处理 socket error 状态
static uint8_t handleErrorState(struct Sam_Mdm_Socket_t *self) {
    if (self == NULL) {
        return RETCHAR_FREE;
    }

    switch (self->error) {
        case SAM_MDM_SOCKET_ERROR_AT_NORESPONSE:            
        case SAM_MDM_SOCKET_ERROR_NET_OPEN:
        case SAM_MDM_SOCKET_ERROR_CIP_OPEN:
        case SAM_MDM_SOCKET_ERROR_REMOTE_CLOSE: {
                self->openReTryCnt ++;
                SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_WARN, "handleErrorState init retry count:%d.\r\n", self->openReTryCnt );
                stateTransfer(self, SAM_MDM_SOCKET_STATE_INIT);
            }
            break;
        default:
            break;
    }

    return RETCHAR_FREE;
}

bool Sam_Mdm_Socket_setCallback(struct Sam_Mdm_Socket_t* self, Sam_Mdm_Socket_Event_Callback_t eventCb, Sam_Mdm_Socket_Data_Callback_t dataCb, void* context) {
    if (self == NULL) {
        return false;
    }

    self->eventCallback = eventCb;    
    self->dataCallback = dataCb;
    self->context = context;

    return true;
}

uint32_t Sam_Mdm_Socket_Send(struct Sam_Mdm_Socket_t* self, const uint8_t* data, uint32_t length) {
    if ((self == NULL)  || (data == NULL)) {
        return 0;
    }

    if ((self->base.state == SAM_MDM_SOCKET_STATE_CLOSED) 
        || (self->base.state == SAM_MDM_SOCKET_STATE_OPENING) 
        ||(self->base.state == SAM_MDM_SOCKET_STATE_ERROR)
        ||(self->base.state == SAM_MDM_SOCKET_STATE_INIT))
    {
        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_ERROR, "Sam_Mdm_Socket_Send error state:%d\r\n", self->base.state);
        return 0;
    }

    uint32_t send_len =0;
    send_len = self->upcnt + length > TSCM_UPBUFLEN ? (uint32_t)(TSCM_UPBUFLEN -self->upcnt) : length;
    memcpy(&self->upbuf[self->upcnt], data, send_len);
    self->upcnt += send_len;

    SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_INFO, "Sam_Mdm_Socket_Send %u data\r\n", send_len);
    return send_len;
}

// 处理 socket module 响应
uint8_t Sam_Mdm_Socket_process(struct Sam_Mdm_Socket_t* self) {
    if (self == NULL) {
        return RETCHAR_FREE;
    }

    uint8_t result = RETCHAR_FREE;
    uint32_t clk = 0;
    clk = SamGetMsCnt(self->base.msclk);
    while(clk >= 1000)
    {
    	self->base.msclk +=1000;
    	self->base.sclk += 1;
    	clk -= 1000;
    }

    // 根据不同状态处理
    switch (self->base.state) {
        case SAM_MDM_SOCKET_STATE_INIT:
            result = handleInitState(self);
            break;
            
        case SAM_MDM_SOCKET_STATE_OPENING:
            result = handleOpeningState(self);
            break;
            
        case SAM_MDM_SOCKET_STATE_CONNECTED:
            result = handleConnectedState(self);
            break;
            
        case SAM_MDM_SOCKET_STATE_SENDING:
            result = handleSendingState(self);
            break;
            
        case SAM_MDM_SOCKET_STATE_RECEIVING:
            result = handleReceivingState(self);
            break;
            
        case SAM_MDM_SOCKET_STATE_CLOSING:
            result = handleClosingState(self);
            break;
            
        case SAM_MDM_SOCKET_STATE_CLOSED:
            // socket closed, do nothing.
            break;
            
        case SAM_MDM_SOCKET_STATE_ERROR:
            result = handleErrorState(self);
            break;
            
        default:
            break;
    }
    
    return result;
}


// Socket module's run function
uint8_t Sam_Mdm_Socket_run(struct Sam_Mdm_Base_t* self) {
    Sam_Mdm_Socket_t* sock = (Sam_Mdm_Socket_t*)self;
    if (self == NULL) {
        return RETCHAR_FREE;
    }
    
    // Add the logic to run the Socket module here
    
    return Sam_Mdm_Socket_process(sock);
}


#if 0
Sam_Mdm_Socket_t* Sam_Mdm_Socket_Create(Sam_Mdm_t* parent, const Sam_Mdm_Socket_Config_t* config) {
    if (parent == NULL) {
        SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_ERROR, "(parent == NULL)\n");
        return NULL;
    }
    
    Sam_Mdm_Socket_t* socket = (Sam_Mdm_Socket_t*)malloc(sizeof(Sam_Mdm_Socket_t));
    if (socket == NULL) {
        return NULL;
    }
    memset(socket, 0x00, sizeof(Sam_Mdm_Socket_t));    
        
    socket->base.state = 0;  // Set initial state
    socket->base.step = 0;   // Set initial step
    socket->base.msclk = SamGetMsCnt(0);;
    socket->base.sclk = 0;
    socket->base.run = Sam_Mdm_Socket_run;  // Assign the run function
	
    socket->parent = parent;
    
    socket->init = Sam_Mdm_Socket_init;
    socket->deinit = Sam_Mdm_Socket_deinit;
    socket->process = Sam_Mdm_Socket_process;
    socket->setUserCallback = Sam_Mdm_Socket_setCallback;
    socket->close = Sam_Mdm_Socket_Close;

    // use config parameter to init.
    if (config != NULL)
    {
        if (config->socketId >= MAX_SOCKET_NUM){
            SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_ERROR, "config->socketId(%d) >= MAX_SOCKET_NUM\n", config->socketId);
            return NULL;
        }
            
        socket->config = *config;  // 保存配置

        if (parent->socket[config->socketId] != NULL){
            SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_ERROR, "Socket[%d] already created.\n", config->socketId);
            Sam_Mdm_Socket_Destroy(parent->socket[config->socketId] );
        }
        parent->socket[config->socketId] = socket;    
    }
    
    return socket;
}
#endif /* 0 */


Sam_Mdm_Socket_t* Sam_Mdm_Socket_Create(const Sam_Mdm_Socket_Config_t* config) {
    
    Sam_Mdm_Socket_t* socket = (Sam_Mdm_Socket_t*)malloc(sizeof(Sam_Mdm_Socket_t));
    if (socket == NULL) {
        return NULL;
    }
    memset(socket, 0x00, sizeof(Sam_Mdm_Socket_t));    
        
    socket->base.state = 0;  // Set initial state
    socket->base.step = 0;   // Set initial step
    socket->base.dcnt = 0;
    socket->base.msclk = SamGetMsCnt(0);
    socket->base.sclk = 0;
    socket->base.run = Sam_Mdm_Socket_run;  // Assign the run function
    
    socket->init = Sam_Mdm_Socket_init;
    socket->deinit = Sam_Mdm_Socket_deinit;
    socket->process = Sam_Mdm_Socket_process;
    socket->setUserCallback = Sam_Mdm_Socket_setCallback;

    // use config parameter to init.
    if (config != NULL)
    {
    	#if 0
        if (config->socketId >= MAX_SOCKET_NUM){
            SAM_DBG_MODULE(SAM_MOD_SOCKET, SAM_DBG_LEVEL_ERROR, "config->socketId(%d) >= MAX_SOCKET_NUM\n", config->socketId);
            return NULL;
        }
        #endif
            
        socket->config = *config;  // 保存配置

        if (socket->config.type == SAM_MDM_SOCKET_TYPE_TCP_SERVER)
            socket->config.srvIndex = socket->config.socketId;
        else
            socket->config.srvIndex = 0xFF;
    
        socket->phatc = pAtcBusArray[socket->config.atChannelId];    	    
        socket->runlink =	SamAtcFunLink(socket->phatc, socket, (SamMdmFunTag)Sam_Mdm_Socket_process, (SamUrcBcFunTag)handleAtUrc);
    }
    
    return socket;
}


// Close socket
bool Sam_Mdm_Socket_Close(struct Sam_Mdm_Socket_t* socket) {
    if (socket == NULL) 
        return false;
        
    switch (socket->base.state)
    {
    case SAM_MDM_SOCKET_STATE_CLOSED:
    case SAM_MDM_SOCKET_STATE_CLOSING:
        {
            return true;
        }
        break;
        
    case SAM_MDM_SOCKET_STATE_OPENING:
    case SAM_MDM_SOCKET_STATE_ERROR:
    case SAM_MDM_SOCKET_STATE_INIT:
        {
            socket->closeType = 1;
            stateTransfer(socket, SAM_MDM_SOCKET_STATE_CLOSING);
        }
        break;
        
    default:
        {
            socket->closeType = 1; // do it when in idle state
        }
        break;
    }

    return true;
}

// get state
uint8_t Sam_Mdm_Socket_getState(struct Sam_Mdm_Socket_t* self){
    if (self == NULL) 
        return 0;

    return self->base.state;
}


// 销毁 Socket 模块实例
void Sam_Mdm_Socket_Destroy(Sam_Mdm_Socket_t* socket) {
    if (socket == NULL) 
        return;
    
    if (socket->base.state == SAM_MDM_SOCKET_STATE_CLOSED)
    {
//        Sam_Mdm_t* parent = socket->parent;
//        parent->socket[socket->config.socketId] = NULL;
        
        SamAtcFunUnlink(socket->phatc, socket->runlink);
        socket->deinit(socket);
        free(socket);
    }
    else 
    {
        socket->closeType = 2;
    }
}
