
#include "include.h"
#include "SamFotaSrv.h"

// Global variables
Sam_Fota_t *fota = NULL;
const char *fotacfgstr = "\vCFGSCT_M1\t0\tA\t0\t1\thttp://117.131.85.142:60058/FTPDATA/system_patch_22_23.bin\tsimcom\tsimcom\v";

/**
 * @brief FOTA progress callback function type.
 * @param progress Current progress (0-100).
 * @param context User context.
 */
void fotaProcessCallback(uint8_t progress, void* context);

/**
 * @brief FOTA status callback function type.
 * @param state Current FOTA state.
 * @param error Error code, if any.
 * @param context User context.
 */
void fotaStateCallback(Sam_Fota_State_t state, Sam_Fota_Error_t error, void* context);

/**
 * @brief FOTA progress callback function type.
 * @param progress Current progress (0-100).
 * @param context User context.
 */
void fotaProcessCallback(uint8_t progress, void* context)
{
    Sam_Fota_t *pFota = (Sam_Fota_t *)context;
    if (pFota == NULL)
    {
        return;
    }

    Sam_Fota_State_t state = Sam_Fota_GetState(pFota);
    if (state == FOTA_STATE_DOWNLOADING)
    {
        printf("FOTA Downloading: %u/100 \r\n", progress);
    }
    else if (state == FOTA_STATE_INSTALLING)
    {
        printf("FOTA Update: %u/100 \r\n", progress);
    }
}


/**
 * @brief FOTA status callback function type.
 * @param state Current FOTA state.
 * @param error Error code, if any.
 * @param context User context.
 */
void fotaStateCallback(Sam_Fota_State_t state, Sam_Fota_Error_t error, void* context)
{
    Sam_Fota_t *pFota = (Sam_Fota_t *)context;
    if (pFota == NULL)
    {
        return;
    }

    if (state == FOTA_STATE_SUCCESS)
    {
        printf("FOTA Completed...\r\n");
        Sam_Fota_Destroy(pFota);
        fota = NULL;
        // or
//        Sam_Fota_Deinit(pFota);
//        free(fota);
//        fota = NULL;
    }
    else if (state == FOTA_STATE_FAILED)
    {
        printf("FOTA Failed error: %d ...\r\n", error);
        Sam_Fota_Destroy(pFota);
        fota = NULL;
        // or
//        Sam_Fota_Deinit(pFota);
//        free(fota);
//        fota = NULL;
    }
}



/**
 * @brief Start FOTA process.
 */
void fotaStart(void)
{
    // create fota instance using create or malloc
    fota = Sam_Fota_Create(NULL);
//    fota = malloc(sizeof(Sam_Fota_t));
    if (fota == NULL)
    {
        printf("Failed to create fota\r\n");
        return;
    }

    Sam_Fota_Init(fota, fotacfgstr);
    Sam_Fota_SetCallbacks(fota, fotaProcessCallback, fotaStateCallback, (void *)fota);
}

/**
 * @brief Start FOTA process function1.
 * @param [mode] download mode ftp or http.
 * @param [url] fota package url address.
 * @param [username] username for ftp access.
 * @param [password] password for ftp access.
 */
void fotaStart1(Sam_Fota_Mode_t mode, char *url, char *username, char *password)
{
    Sam_Fota_Config_t config = {0};
    config.atChannelId = 0;
    config.channel = 0;
    config.mode = mode;
    strcpy(config.serverUrl, url);
    strcpy(config.username, username);
    strcpy(config.password, password);

    
    // create fota instance using create or malloc
    fota = Sam_Fota_Create(&config);
    if (fota == NULL)
    {
        printf("Failed to create fota\r\n");
        return;
    }

    Sam_Fota_SetCallbacks(fota, fotaProcessCallback, fotaStateCallback, (void *)fota);
}
