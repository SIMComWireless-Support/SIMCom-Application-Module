/**
 * @file SamFotaSrv.h
 * @brief FOTA server handling module. This header file defines the function prototypes
 *        related to FOTA server operations in the project. It provides a set of functions
 *        for FOTA process.
 * @version 1.0
 * @date [Date]
 * @author [Author]
 * (c) Copyright [Year Range], [Company Email]
 */

#ifndef SAM_FOTA_SRV_H
#define SAM_FOTA_SRV_H

#include "SamFota.h"

/**
 * @brief Start FOTA process.
 */
void fotaStart(void);

/**
 * @brief Start FOTA process function1.
 * @param [mode] download mode ftp or http.
 * @param [url] fota package url address.
 * @param [username] username for ftp access.
 * @param [password] password for ftp access.
 */
void fotaStart1(Sam_Fota_Mode_t mode, char *url, char *username, char *password);

#endif /* SAM_FOTA_SRV_H */