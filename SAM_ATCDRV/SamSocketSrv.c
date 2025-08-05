
#include "include.h"
#include "SamSocketSrv.h"

// Global variables
Sam_Mdm_Socket_t sock = {0};
Sam_Mdm_Socket_t *socket[10] = {0};
Sam_Mdm_Socket_t *tcpServer[4] = {};
uint8_t buff[1024+1] = {0};
char *defcfgstr = "\vCFGSCT_M1\t0\tA\t0\t0\t0\t1\t117.131.85.139\t60057\t0\v";

uint32_t test_msclk = 0;
uint8_t test_sclk = 0;

/**
 * @brief Callback function for socket events.
 * @param socketId Socket ID.
 * @param event Socket event type.
 * @param msg Pointer to the event message.
 * @param context Pointer to the user context.
 */
void SocketEventCallback(uint8_t socketId, Sam_Mdm_Socket_Event_t event, void *msg, void* context);

/**
 * @brief Callback function for socket data reception.
 * @param socketId Socket ID.
 * @param data Pointer to the received data buffer.
 * @param length Length of the received data.
 * @param context Pointer to the user context.
 */
void SocketDataCallback(uint8_t socketId, const uint8_t* data, uint32_t length, void* context);

/**
 * @brief Callback function for socket events.
 * @param socketId Socket ID.
 * @param event Socket event type.
 * @param msg Pointer to the event message.
 * @param context Pointer to the user context.
 */
void SocketEventCallback(uint8_t socketId, Sam_Mdm_Socket_Event_t event, void *msg, void* context)
{
    Sam_Mdm_Socket_t * pSock = (Sam_Mdm_Socket_t *)context;
    if (pSock == NULL)
    {
        return;
    }
    
    switch (event)
    {
    case SAM_MDM_SOCKET_EVENT_ACCEPT:
        {
            Sam_Mdm_Socket_t *pClient = (Sam_Mdm_Socket_t *) msg;
            uint8_t link_num = 0;

            if (msg == NULL)
            {
                return;
            }

            link_num = pClient->config.socketId;
            // Log the event information
            printf("TCP server[%d] accepted a new client socket[%d]\r\n", socketId, link_num);
            acceptTcpClient(link_num, pClient);
            printf("client socket[%d] state == %d\r\n", link_num, Sam_Mdm_Socket_getState(pClient));
        }
        break;

    case SAM_MDM_SOCKET_EVENT_CLOSED_PASSIVE:
        {
            // Log the event information
            printf("Socket[%d] closed by remote.\r\n", socketId);
        }
        break;
        
    default:
        break;
    }
}

/**
 * @brief Callback function for socket data reception.
 * @param socketId Socket ID.
 * @param data Pointer to the received data buffer.
 * @param length Length of the received data.
 * @param context Pointer to the user context.
 */
void SocketDataCallback(uint8_t socketId, const uint8_t* data, uint32_t length, void* context)
{
    memcpy(buff, data, length);
    buff[length] = '\0';
    
    // Log the received data information
    printf("\n======socket[%d] received %d bytes data:==========\n%s\n====================================\n", socketId, length, buff);

#define IS_LINE(s)      (strncasecmp((char *)buff, (s), sizeof(s) - 1) == 0)

    Sam_Mdm_Socket_t *pSock = NULL;
    pSock = context;
    if (pSock == NULL)
        return;
    
    if (IS_LINE("echo "))
    {
        printf("Received ==echo== command\r\n");
        Sam_Mdm_Socket_Send(pSock, &buff[5], length-5);
    }
    else if (IS_LINE("sendfrom "))
    {
        printf("Received ==sendfrom== command\r\n");
        uint32_t sockid = 0;
        sscanf((const char *)buff,"sendfrom %d", &sockid);
        if (socket[sockid] != NULL)
        {
            Sam_Mdm_Socket_Send(socket[sockid] , &buff[11], length-11);
        }
    }
    else if (IS_LINE("close"))
    {
        printf("Received ==close== command\r\n");
        uint32_t index = 0;
        if (IS_LINE("close socket "))
        {
            sscanf((const char *)buff,"close socket %d", &index);
            if (index > 9) return;
            Sam_Mdm_Socket_Close(socket[index]);
        }
        else if (IS_LINE("close server "))
        {
            sscanf((const char *)buff,"close server %d", &index);
            if (index > 3) return;
            Sam_Mdm_Socket_Close(tcpServer[index]);
        }
        else
            Sam_Mdm_Socket_Close(pSock);
    }
    else if (IS_LINE("destroy"))
    {
        printf("Received ==destroy== command\r\n");
        uint32_t index = 0;
        if (IS_LINE("destroy socket "))
        {
            sscanf((const char *)buff,"destroy socket %d", &index);
            if (index > 9) return;
            Sam_Mdm_Socket_Destroy(socket[index]);
            socket[index] = NULL;
        }
        else if (IS_LINE("destroy server "))
        {
            sscanf((const char *)buff,"destroy server %d", &index);
            if (index > 3) return;
            Sam_Mdm_Socket_Destroy(tcpServer[index]);
            tcpServer[index] = NULL;
        }
        else 
        {
            Sam_Mdm_Socket_Destroy(pSock);
            socket[socketId] = NULL;
        }
    }
    else if (IS_LINE("new client"))
    {
        printf("Received ==new client== command\r\n");
        uint32_t sockid = 0;
        char hostip[64] = {0};
        uint32_t port = 0;
        int ret  = 0;
        
        ret = sscanf((const char *)buff,"new client %d %s %d", &sockid, hostip, &port);
        if (ret != 3)
        {
            printf("sscanf result: %d\r\n", ret);
            printf("new client %d %s %d\r\n", sockid, hostip, port);
            return;
        }
        else 
        {
            printf("new client %d %s %d\r\n", sockid, hostip, port);
        }
        newTcpClient(sockid, hostip, port);
    }
    else if (IS_LINE("new server"))
    {
        printf("Received ==new server== command\r\n");
        uint32_t srvindex = 0;
        uint32_t port = 0;
        
        sscanf((const char *)buff,"new server %u %u", &srvindex, &port);
        printf("new server %d %d\r\n", srvindex, port);
        newTcpServer(srvindex, port);
    }
}

/**
 * @brief Create a new socket.
 */
void newSocket(void)
{
    memset((void *)&sock, 0x00, sizeof(Sam_Mdm_Socket_t));
    
    // Initialize the socket with the default configuration
    Sam_Mdm_Socket_init(&sock, defcfgstr);    
    // Set the callback functions
    Sam_Mdm_Socket_setCallback(&sock, SocketEventCallback, SocketDataCallback, (void *)&sock);      
}

/**
 * @brief Create a new TCP client socket.
 * @param socketId Socket ID for the new client.
 * @param hostIP IP address of the server.
 * @param port Port number of the server.
 */
void newTcpClient(uint8_t socketId, char *hostIP, uint16_t port)
{
    char cfgstr[64] = {0};

    if (socket[socketId] != NULL)
    {
        printf("TCP Client[%d] not NULL\r\n", socketId);
        return;
    }
    
    socket[socketId] = Sam_Mdm_Socket_Create( NULL);
    if(socket[socketId] == NULL)
    {
        printf("Failed to create socket[%d]\r\n", socketId);
        return;
    }    
    
    // Generate the configuration string
    sprintf(cfgstr, "\vCFGSCT_M1\t0\tA\t%u\t0\t0\t1\t%s\t%u\t0\v", socketId, hostIP, port);
    // Initialize the socket with the generated configuration
    Sam_Mdm_Socket_init(socket[socketId], cfgstr);
    // Set the callback functions
    Sam_Mdm_Socket_setCallback(socket[socketId], SocketEventCallback, SocketDataCallback, (void *)socket[socketId]);  

    test_msclk = SamGetMsCnt(0);
    
}

void testTcpClient(void)
{    
    uint32_t clk = 0;
    clk = SamGetMsCnt(test_msclk);
    while(clk >= 1000)
    {
    	test_msclk +=1000;
    	test_sclk += 1;
    	clk -= 1000;
    }
    
    if ((socket[0] != NULL) && (Sam_Mdm_Socket_getState(socket[0]) == SAM_MDM_SOCKET_STATE_CONNECTED) && (test_sclk > 5))
    {
        Sam_Mdm_Socket_Send(socket[0] , (uint8_t *)"Hello World!\r\n", 14);
        test_sclk = 0;
    }
}

/**
 * @brief Create a new TCP server socket.
 * @param srvIndex Server index.
 * @param port Port number for the server to listen on.
 */
void newTcpServer(uint8_t srvIndex, uint16_t port)
{
    char cfgstr[64] = {0};
    printf("newTcpServer srvIndex:%d listerning port:%d\r\n", srvIndex, port);

    if (tcpServer[srvIndex] != NULL)
    {
        printf("TCP Server[%d] not NULL\r\n", srvIndex);
        return;
    }
    
    // Create a new socket instance for the server
    tcpServer[srvIndex] = Sam_Mdm_Socket_Create( NULL);
    if(tcpServer[srvIndex] == NULL)
    {
        printf("Failed to create TCP server[%d]\r\n", srvIndex);
        return;
    }    
    
    // Generate the configuration string for the server
    sprintf(cfgstr, "\vCFGSCT_M1\t0\tA\t%u\t0\t3\t1\t0.0.0.0\t0\t%u\v", srvIndex, port);
    // Initialize the server socket with the generated configuration
    Sam_Mdm_Socket_init(tcpServer[srvIndex], cfgstr);
    // Set the callback functions for the server socket
    Sam_Mdm_Socket_setCallback(tcpServer[srvIndex], SocketEventCallback, SocketDataCallback, (void *)tcpServer[srvIndex]);      
}

/**
 * @brief Accept a new TCP client socket.
 * @param socketId Socket ID for the new client.
 * @param pClient Pointer to the client socket structure.
 */
void acceptTcpClient(uint8_t socketId, Sam_Mdm_Socket_t *pClient)
{
    if (pClient == NULL)
    {
        return;
    }
    
    // Create a new socket instance for the client
    socket[socketId] = Sam_Mdm_Socket_Create(NULL);
//    socket[socketId] = malloc(sizeof(Sam_Mdm_Socket_t));

    if (socket[socketId] == NULL)
    {
        return;
    }

    // Copy the client socket configuration
    memcpy(socket[socketId], pClient, sizeof(Sam_Mdm_Socket_t));
    // Set the callback functions for the client socket
    Sam_Mdm_Socket_setCallback(socket[socketId], SocketEventCallback, SocketDataCallback, (void *)socket[socketId]); 
}
