#define __SAMTESTER_C

#include "include.h"


//Fuction Demo test 
#define SAM_MQTT_TEST
#define SAM_SOCKET_TEST
//#define SAM_TTS_TEST
//#define SAM_AUDIO_TEST
//#define SAM_FOTA_TEST
//#define SAM_SMS_TEST
void TesterInit(void)
{
//Start Modem Service
	SamMdmSrvStart( );
	
#ifdef SAM_SOCKET_TEST
    newSocket( );
#endif /* SAM_SOCKET_TEST */

#ifdef SAM_MQTT_TEST
    mqtt_demo_init();
#endif
#ifdef SAM_SMS_TEST
	sms_demo_init();
#endif
#ifdef SAM_TTS_TEST
    sam_demo_tts_init();
#endif

#ifdef SAM_AUDIO_TEST
    sam_demo_audio_init();
#endif


#ifdef SAM_FOTA_TEST
//    fotaStart1(1, "47.109.101.196:5050/SIMTEST/hjy/test2.bin", "SIMCOM", "simcom");
#endif /* SAM_FOTA_TEST */

}

void TesterProc(void)
{    
//Modem Service 
    SamMdmSrvRun( );

#ifdef SAM_SOCKET_TEST
    testTcpClient();
#endif /* SAM_SOCKET_TEST */

#ifdef SAM_MQTT_TEST
    mqtt_demo_client_run();
#endif

#ifdef SAM_SMS_TEST
    sms_demo_run();
#endif


#ifdef SAM_TTS_TEST
    sam_demo_tts_proc();
#endif
#ifdef SAM_AUDIO_TEST
    sam_demo_audio_proc();
#endif

}
