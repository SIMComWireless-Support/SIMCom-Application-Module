
#include "include.h"
#include "SamTTSSrv.h"


static int test = 0;


void sam_tts_test_callback(TTS_Status_E ttsStatus,char *result){
    char *pData = result;
    switch(ttsStatus){
        case TTS_GET_STATUS:
            //0:TTS no working.
            //1:TTS working.
            //-1:Error occurred during execution.
            DebugTrace("TTS_GET_STATUS result:%d\r\n",*pData);
            break;
        case TTS_STOP_PLAYING:
            //0:TTS stoping success.
            //-1:Error occurred during execution.
            DebugTrace("TTS_STOP_PLAYING result:%d\r\n",*pData);
            break;
        case TTS_PLAY:
        case TTS_PLAY_AND_SAVE_TO_FILE:
            //0:TTS playing success.
            //-1:Error occurred during execution.
            DebugTrace("TTS_PLAY result:%d\r\n",*pData);
            break;
        case TTS_SET_YOUNGTONE_PARAM:
        case TTS_SET_IFLY_PARAM:
            //0:Set YOUNGTONE/IFLY params success.
            //-1:Error occurred during execution.
            DebugTrace("TTS_SET_YOUNGTONE/IFLY_PARAM result:%d\r\n",*pData);
            break;
        case TTS_GET_YOUNGTONE_PARAM:
        case TTS_GET_IFLY_PARAM:
            //0:Get YOUNGTONE/IFLY params success.
            //if pData[0] value is -1:Error occurred during execution.
            //0:TTS lib volume;1:system volume;2:digitmode;3:pitch;4:speed;5:digitreading for YOUNGTONE,ttslib for IFLY.
            DebugTrace("TTS_GET_YOUNGTONE_OR_IFLY_PARAM:%d,%d,%d,%d,%d,%d\r\n", \
                    pData[0], \
                    pData[1], \
                    pData[2], \
                    pData[3], \
                    pData[4], \
                    pData[5]);
            break;
        case TTS_GET_LOCAL_OR_REMOTE_PLAY:
            //0:Local Player.
            //1:Remote Player.
            //-1:Error occurred during execution.
            DebugTrace("TTS_GET_LOCAL_OR_REMOTE_PLAY result:%d\r\n",*pData);
            break;
        case TTS_SET_LOCAL_OR_REMOTE_PLAY:
            //0:Instruction execution successful.
            //-1:Error occurred during execution.
            DebugTrace("TTS_SET_LOCAL_OR_REMOTE_PLAY result:%d\r\n",*pData);
            break;
        case TTS_GET_SYS_VOLUME_SETTING:
            //0:volume setting is valid.
            //1:volume setting is invalid.
            //-1:Error occurred during execution.
            DebugTrace("TTS_GET_SYS_VOLUME_SETTING result:%d\r\n",*pData);
            break;
        case TTS_SET_SYS_VOLUME_SETTING:
            //0:Instruction execution successful.
            //-1:Error occurred during execution.
            DebugTrace("TTS_SET_SYS_VOLUME_SETTING result:%d\r\n",*pData);
            break;
        default:
            break;
    }
}

char sam_tts_urc_test_callback(char *urcStr){
    uint8 ret;
    ret = StrsCmp(urcStr,"+CTTS: 0\r\n\t+CTTS: 1\r\n\t+CTTS: 2\r\n");
    //ret: 1 TTS playback completed.
    //ret: 2 TTS data malloc failed.
    //ret: 3 FS size not enough.
    if(ret ==1) {
        test = 0;
    } else if(ret == 2){
    } else if(ret == 3){
    }
    if(ret >= 1 && ret <= 3) {
        DebugTrace("TTS urcStr:%s\r\n",urcStr);
        return 1;
    }
	return 0;
}

/**
 * @brief  Register TTS module.
 */
void sam_demo_tts_init(void){
    sam_tts_init(0,sam_tts_test_callback,sam_tts_urc_test_callback);
}

/**
 * @brief Example of TTS module execution.
 */
void sam_demo_tts_proc(void){
    if(test == 1){
        return;
    }
    test = 1;
    //sam_tts_get_status();
    //sam_tts_stop_playing();
    //uint8 *pData = "\"6B228FCE4F7F75288BED97F3540862107CFB7EDF\"";
    //sam_tts_play(pData,strlen(pData),TTS_PLAYING_UCS2_FORMAT);
    //char  * pData = "\"È¥³¯<pinyin=chao2>Ñô£¬¿´³¯<pinyin=zhao1>Ñô\"";
    //sam_tts_play((uint8 *)pData,strlen(pData),TTS_PLAYING_ASCII_AND_GBK_OR_ONLY_GBK_FORMAT);
    //uint8 *pData = "\"hello world\"";
    //char *fileName = "\"C:/12.wav\"";
    //sam_tts_play_and_save_wav(pData,strlen(pData),fileName,TTS_PLAYING_ONLY_ASCII_FORMAT);
    //sam_tts_get_YOUNGTONE_param();
    /*TTS_param_T param;
    param.params[TTS_VOL] = 2;
    param.params[TTS_SYS_VOL] = 3;
    param.params[TTS_DIGIT_MODE] = 0;
    param.params[TTS_PITCH] = 1;
    param.params[TTS_SPEED] = 1;
    param.params[TTS_DIGIT_READING_FOR_YOUNGTONE_OR_TTSLIB_FOR_IFLY] = 1;
    sam_tts_set_YOUNGTONE_param(&param);*/
    //sam_tts_get_IFLY_param();
    /*TTS_param_T param;
    param.params[TTS_VOL] = 2;
    param.params[TTS_SYS_VOL] = 3;
    param.params[TTS_DIGIT_MODE] = 0;
    param.params[TTS_PITCH] = 1;
    param.params[TTS_SPEED] = 1;
    param.params[TTS_DIGIT_READING_FOR_YOUNGTONE_OR_TTSLIB_FOR_IFLY] = 1;
    sam_tts_set_IFLY_param(&mTTSTag,&param);*/
    //sam_tts_get_local_or_remote_status();
    //sam_tts_set_local_or_remote_status(0);
    //sam_tts_get_sys_vol_setting_status();
    //sam_tts_set_sys_vol_setting(0);
}
