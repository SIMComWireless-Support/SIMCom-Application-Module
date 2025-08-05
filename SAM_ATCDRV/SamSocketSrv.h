/**
 * @file SamSocketSrv.h
 * @brief Socket server handling module. This header file defines the function prototypes
 *        related to socket server operations in the project. It provides a set of functions
 *        for creating new sockets, clients, and servers, as well as accepting new clients.
 * @version 1.0
 * @date [Date]
 * @author [Author]
 * (c) Copyright [Year Range], [Company Email]
 */

#ifndef SAM_MDM_SOCKET_SRV_H
#define SAM_MDM_SOCKET_SRV_H

#include "SamSocket.h"


void testTcpClient(void);

/**
 * @brief Create a new socket.
 */
void newSocket(void);

/**
 * @brief Create a new TCP client socket.
 * @param socketId Socket ID for the new client.
 * @param hostIP IP address of the server.
 * @param port Port number of the server.
 */
void newTcpClient(uint8_t socketId, char *hostIP, uint16_t port);

/**
 * @brief Create a new TCP server socket.
 * @param srvIndex Server index.
 * @param port Port number for the server to listen on.
 */
void newTcpServer(uint8_t srvIndex, uint16_t port);

/**
 * @brief Accept a new TCP client socket.
 * @param socketId Socket ID for the new client.
 * @param pClient Pointer to the client socket structure.
 */
void acceptTcpClient(uint8_t socketId, Sam_Mdm_Socket_t *pClient);

#endif /* SAM_MDM_SOCKET_SRV_H */