
#include "SamTTSSrv.h"

static int test = 0;

void sam_audio_test_callback(Audio_Status_E audioStatus,char *result){
    char *pData = result;
    switch(audioStatus){
        case AUDIO_PLAY:
            //0:Audio play successfully.
            //-1:Error occurred during execution.
            DebugTrace("AUDIO_PLAY result:%d\r\n",*pData);
            break;
        case AUDIO_STOP_PLAYING:
            //0:Audio stoping successfully.
            //-1:Error occurred during execution.
            DebugTrace("AUDIO_STOP_PLAYING result:%d\r\n",*pData);
            break;
        case AUDIO_RECORDING_STATUS:
            //0:free.
            //1:busy.
            //-1:Error occurred during execution.
            DebugTrace("AUDIO_RECORDING_STATUS result:%d\r\n",*pData);
            break;
        case AUDIO_RECORD_START:
            //0:Audio recording successfully.
            //-1:Error occurred during execution.
            //-2:memory full.
            DebugTrace("AUDIO_RECORD_START result:%d\r\n",*pData);
            break;
        case AUDIO_RECORD_STOP:
            //0:Audio stop recording successfully.
            //-1:Error occurred during execution.
            DebugTrace("AUDIO_RECORD_STOP result:%d\r\n",*pData);
            break;
        default:
            break;
    }
}

char sam_audio_urc_test_callback(char *urcStr){
    uint8 ret;
    ret = StrsCmp(urcStr,"+AUDIOSTATE: audio play\r\n\t+AUDIOSTATE: audio play stop\r\n");
    //ret: 1 Audio start to play file.
    //ret: 2 Audio playback completed or the stop play function has been executed.
    if(ret ==1) {
    } else if(ret == 2){
        test = 0;
    }
    if (ret == 1 || ret == 2) {
        DebugTrace("Audio play urcStr:%s\r\n",urcStr);
        return 1;
    }
    ret = StrsCmp(urcStr,"+CREC: crec stop\r\n\t+CREC: file full\r\n");
    //ret: 1 stop recording.
    //ret: 2 When the file is recoding full, Response "+CREC: file full " is displayed.
    //Maximum size of wave file is 768KB and maximum size of amr file is 512KB.When the filesystem
    //free size is less than the maximum size of recording file,the maximum size of recording file is file
    //system free size.
    if(ret ==1) {
    } else if(ret == 2){
    }
    if (ret == 1 || ret == 2) {
        DebugTrace("Audio recording urcStr:%s\r\n",urcStr);
        return 1;
    }
	return 0;
}

/**
 * @brief  Register Audio module.
 */
void sam_demo_audio_init(void){
    sam_audio_init(0,sam_audio_test_callback,sam_audio_urc_test_callback);
}

/**
 * @brief Example of Audio module execution.
 */
void sam_demo_audio_proc(void){
    if(test == 1){
        return;
    }
    test = 1;
    //sam_audio_get_record_status();
    //sam_audio_record_start("c:/recording.amr",1);
    //sam_audio_record_stop();
    sam_audio_play("c:/recording.amr",0,0);
    //sam_audio_stop_playing();
}


