/**
  * @file    : SamAudio.c
  * @brief   : Implement recording and playback related functions.
  * @author  : wangxiaochun@sunseaaiot.com
  * @version : 1.0.0
  * @date    : 2025-07-01
  * @license : MIT
  */

#include <stdio.h>
#include <stdlib.h>
#include "include.h"
#include "SamAtc.h"
#include "SamAudio.h"

#define	AUDIO_WRITE_BUF_LEN 128

typedef struct{
    uint8 sta;
    uint8 step;
    uint8 dcnt;
    uint8 runlink; //for run link in atclink  
    uint32 msclk;  //for recode sysclk
    uint8 stim;    //second timer for user  
    HdsAtcTag* phatc;
    char writeBuf[AUDIO_WRITE_BUF_LEN];
    uint16 writeCount;
    sam_audio_callback audioCallback;
    sam_audio_urc_callback audioURCCallback;
}Audio_Tag_T;


Audio_Tag_T mAudioTag;

extern HdsAtcTag * pAtcBusArray[];
static unsigned char sam_audio_proc(void *pAudioTag);
static unsigned char sam_audio_urc_cb(void *pAudio, char *urcStr);


/**
 * @brief Play an audio file, where the file format is amr,wav,mp3 or pcm.
 *  PCM file must have a header to play, otherwise playback is invalid.
 * @param fileName: The name of audio file.Enter path and filename, if no path is added, save in C: by default.
 *  Maximum filename length is 60 bytes.
 * @param playPath (0:local path;1:remote path (just support voice call);2:local path and remote path).
 * @param repeat (0:don't play repeat.play only once;1..255:play repeat times. E.g. <repeat>=1, audio will play twice.).
 * @return Returning 0 indicates successful execution and returning -1 indicates failure,
 *               Please check the input parameters correctly. 
 */
int sam_audio_play(char *fileName,uint8 playPath,uint8 repeat) {
    Audio_Tag_T *pAudioTag = &mAudioTag;
    if(fileName == NULL || playPath > 2) {
        return -1;
    }
    if (pAudioTag->sta == AUDIO_PLAY){
        return 0;
    } else if (pAudioTag->sta != AUDIO_IDLE) {
        return -1;
    }
    pAudioTag->sta = AUDIO_PLAY;
    pAudioTag->step = 0;
    pAudioTag->stim = 0;
    memset(pAudioTag->writeBuf,0,sizeof(pAudioTag->writeBuf));
    sprintf(pAudioTag->writeBuf,"\"%s\",%d,%d\r\n",fileName,playPath,repeat);
    pAudioTag->writeCount = strlen(pAudioTag->writeBuf);
    return 0;
}

/**
 * @brief The function is used to stop playing audio file.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
int sam_audio_stop_playing(void) {
    Audio_Tag_T *pAudioTag = &mAudioTag;
    if(pAudioTag->sta == AUDIO_STOP_PLAYING) {
        return 0;
    } else if(pAudioTag->sta != AUDIO_IDLE) {
        return -1;
    }

    pAudioTag->sta = AUDIO_STOP_PLAYING;
    pAudioTag->step = 0;
    pAudioTag->stim = 0;
    return 0;
}

/**
 * @brief This function is used to stop recording.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
int sam_audio_get_record_status(void){
    Audio_Tag_T *pAudioTag = &mAudioTag;
    if(pAudioTag->sta == AUDIO_RECORDING_STATUS) {
        return 0;
    } else if(pAudioTag->sta != AUDIO_IDLE) {
        return -1;
    }

    pAudioTag->sta = AUDIO_RECORDING_STATUS;
    pAudioTag->step = 0;
    pAudioTag->stim = 0;
    return 0;
}

/**
 * @brief This function is used to record a wav/amr audio file.
 *  It can record wav/amr file during a call or not, the
 *  record file should be put into the "c:/". The supported file format is WAV and AMR. Only SD card support
 *  Non-ASCII characters in file path.
 * @param fileName:The name of wav/amr audio file.(MAX is 60 bytes)
 *  Enter path and filename, if no path is added, save in C: by default.
 * @param recordPath:(1:local path;2:remote path (get voice from cs call);3:mixd (local and remote)).
 * @return Returning 0 indicates successful execution and returning -1 indicates failure,
 *               Please check the input parameters correctly. 
 */
int sam_audio_record_start(const char *fileName,uint8 recordPath){
    Audio_Tag_T *pAudioTag = &mAudioTag;
    if(pAudioTag->sta == AUDIO_RECORD_START) {
        return 0;
    } else if(pAudioTag->sta != AUDIO_IDLE) {
        return -1;
    }

    pAudioTag->sta = AUDIO_RECORD_START;
    pAudioTag->step = 0;
    pAudioTag->stim = 0;
    memset(pAudioTag->writeBuf,0,sizeof(pAudioTag->writeBuf));
    sprintf(pAudioTag->writeBuf,"%d,\"%s\"\r\n",recordPath,fileName);
    pAudioTag->writeCount = strlen(pAudioTag->writeBuf);
    return 0;
}

/**
 * @brief This function is used to stop a recording.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
int sam_audio_record_stop(void){
    Audio_Tag_T *pAudioTag = &mAudioTag;
    if(pAudioTag->sta == AUDIO_RECORD_STOP) {
        return 0;
    } else if(pAudioTag->sta != AUDIO_IDLE) {
        return -1;
    }

    pAudioTag->sta = AUDIO_RECORD_STOP;
    pAudioTag->step = 0;
    pAudioTag->stim = 0;
    return 0;
}



/**
 * @brief Register Audio module.
 * @param atcIndex Which ATC channel should be used. Usually set to 0.
 * @param audioCallback After the AT instruction is executed, the callback function returns the execution result.
 * @param urcCallback Report the result after the asynchronous command is executed.
 * @return Returning 0 indicates successful execution and returning -1 indicates failure,
 *               Please check the input parameters correctly. 
 */
int sam_audio_init(uint8 atcIndex,sam_audio_callback audioCallback,sam_audio_urc_callback urcCallback) {
    Audio_Tag_T *pAudioTag = &mAudioTag;
    if(atcIndex >= ATCBUS_CHMAX) {
        return -1;
    }
    pAudioTag->phatc = pAtcBusArray[atcIndex];
    pAudioTag->sta = AUDIO_IDLE;
    pAudioTag->step = 0;
    pAudioTag->dcnt = 0;
    pAudioTag->stim = 0;
    pAudioTag->writeCount = 0;
    pAudioTag->msclk = SamGetMsCnt(0);
    pAudioTag->runlink =	SamAtcFunLink(pAudioTag->phatc, pAudioTag, sam_audio_proc, sam_audio_urc_cb);
    pAudioTag->audioCallback = audioCallback;
    pAudioTag->audioURCCallback = urcCallback;
    return 0;
}

/**
 * @brief Deregister Audio module.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
int sam_audio_deinit(void)
{
	Audio_Tag_T *pAudioTag = &mAudioTag;
	SamAtcFunUnlink(pAudioTag->phatc, pAudioTag->runlink);
	return 0;
}

unsigned char sam_audio_proc(void *pAudioTag)
{
	uint8 ratcret;
	uint32 clk;
    char value[6];
	Audio_Tag_T * pAudio = NULL;
	HdsAtcTag * phatc = NULL;

	pAudio = pAudioTag;
	if (pAudio == NULL) {
        return ('E'+1);
    }
	phatc = pAudio->phatc;
	if (phatc == NULL) {
        return ('E'+2);
    }

	clk = SamGetMsCnt(pAudio->msclk);
	while(clk >= 1000)
	{
		pAudio->msclk +=1000;
		pAudio->stim += 1;
		clk -= 1000;
	}
	
	switch(pAudio->sta)
	{
		case AUDIO_IDLE:
			return RETCHAR_FREE;
		case AUDIO_PLAY:
			if(pAudio->step == 0)
			{
			    char data[128] = {0};
                char *pStr = "AT+CCMXPLAY=";
			    phatc->type = CRLF_HATCTYP;
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) {
                    phatc->retbufp = 0;
                }
                memcpy(data,pStr,strlen(pStr));
                memcpy(data+strlen(pStr),pAudio->writeBuf,pAudio->writeCount);
                SamSendAtCmd(phatc, data, CRLF_HATCTYP, 80);
                pAudio->step += 1;
				pAudio->stim = 0;
				return RETCHAR_KEEP;
			}
			else if(pAudio->step == 1)
			{
			    pAudio->phatc->type = CRLF_HATCTYP;
				ratcret = SamChkAtcRet(pAudio->phatc, "+CCMXPLAY:\r\n\tOK\r\n\tERROR\r\n");
				if(ratcret == NOSTRRET_ATCRET) {
                	return RETCHAR_KEEP;
				} else if(ratcret == OVERTIME_ATCRET || ratcret == 3) {
                    if(pAudio->audioCallback != NULL) {
                        value[0] = -1;
                        pAudio->audioCallback(pAudio->sta,value);
                    }
				} else if(ratcret == 2) {
                    if(pAudio->audioCallback != NULL) {
                        value[0] = 0;
                        pAudio->audioCallback(pAudio->sta,value);
                    }
				} else if(ratcret == 1) {
    				phatc->retbufp = 0;
				    return RETCHAR_KEEP;
				}
				pAudio->sta = AUDIO_IDLE;
				pAudio->stim = 0;
                pAudio->step = 0;
				phatc->retbufp = 0;
                return RETCHAR_FREE;
			}
			break;
        case AUDIO_STOP_PLAYING:
			if(pAudio->step == 0)
			{
			    phatc->type = CRLF_HATCTYP;
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) {
                    phatc->retbufp = 0;
                }
                SamSendAtCmd(phatc, "AT+CCMXSTOP\r\n", CRLF_HATCTYP, 80);
                pAudio->step += 1;
				pAudio->stim = 0;
				return RETCHAR_KEEP;
			}
			else if(pAudio->step == 1)
			{
			    pAudio->phatc->type = CRLF_HATCTYP;
				ratcret = SamChkAtcRet(pAudio->phatc, "+CCMXSTOP:\r\n\tOK\r\n\tERROR\r\n");
				if(ratcret == NOSTRRET_ATCRET) {
                	return RETCHAR_KEEP;
				}
				else if(ratcret == OVERTIME_ATCRET || ratcret == 3) {
                    if(pAudio->audioCallback != NULL) {
                        value[0] = -1;
                        pAudio->audioCallback(pAudio->sta,value);
                    }
				}
				else if(ratcret == 2) {
                    if(pAudio->audioCallback != NULL) {
                        value[0] = 0;
                        pAudio->audioCallback(pAudio->sta,value);
                    }
				} else if(ratcret == 1) {
				    phatc->retbufp = 0;
				    return RETCHAR_KEEP;
				}
				pAudio->sta = AUDIO_IDLE;
				pAudio->stim = 0;
                pAudio->step = 0;
				phatc->retbufp = 0;
                return RETCHAR_FREE;
			}
			break;
        case AUDIO_RECORD_START:
			if(pAudio->step == 0)
			{
			    char data[128] = {0};
                char *pStr = "AT+CREC=";
			    phatc->type = CRLF_HATCTYP;
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) {
                    phatc->retbufp = 0;
                }
                memcpy(data,pStr,strlen(pStr));
                memcpy(data+strlen(pStr),pAudio->writeBuf,pAudio->writeCount);
                SamSendAtCmd(phatc, data, CRLF_HATCTYP, 80);
                pAudio->step += 1;
				pAudio->stim = 0;
				return RETCHAR_KEEP;
			}
			else if(pAudio->step == 1)
			{
			    pAudio->phatc->type = CRLF_HATCTYP;
				ratcret = SamChkAtcRet(pAudio->phatc, "+CREC: memory full\r\n\t+CREC:\tOK\r\n\tERROR\r\n");
				if(ratcret == NOSTRRET_ATCRET) {
                	return RETCHAR_KEEP;
				} else if(ratcret == OVERTIME_ATCRET || ratcret == 4) {
                    if(pAudio->audioCallback != NULL) {
                        value[0] = -1;
                        pAudio->audioCallback(pAudio->sta,value);
                    }
				} else if(ratcret == 1) {
                    if(pAudio->audioCallback != NULL) {
                        value[0] = -2;
                        pAudio->audioCallback(pAudio->sta,value);
                    }
				} else if(ratcret == 3) {
                    if(pAudio->audioCallback != NULL) {
                        value[0] = 0;
                        pAudio->audioCallback(pAudio->sta,value);
                    }
				} else if(ratcret == 2) {
    				phatc->retbufp = 0;
				    return RETCHAR_KEEP;
				}
				pAudio->sta = AUDIO_IDLE;
				pAudio->stim = 0;
                pAudio->step = 0;
				phatc->retbufp = 0;
                return RETCHAR_FREE;
			}
			break;
        case AUDIO_RECORD_STOP:
			if(pAudio->step == 0)
			{
			    phatc->type = CRLF_HATCTYP;
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) {
                    phatc->retbufp = 0;
                }
                SamSendAtCmd(phatc, "AT+CREC=0\r\n", CRLF_HATCTYP, 80);
                pAudio->step += 1;
				pAudio->stim = 0;
				return RETCHAR_KEEP;
			}
			else if(pAudio->step == 1)
			{
			    pAudio->phatc->type = CRLF_HATCTYP;
				ratcret = SamChkAtcRet(pAudio->phatc, "+CREC: 0\r\n\tOK\r\n\tERROR\r\n");
				if(ratcret == NOSTRRET_ATCRET) {
                	return RETCHAR_KEEP;
				} else if(ratcret == OVERTIME_ATCRET || ratcret == 3) {
                    if(pAudio->audioCallback != NULL) {
                        value[0] = -1;
                        pAudio->audioCallback(pAudio->sta,value);
                    }
				} else if(ratcret == 2) {
                    if(pAudio->audioCallback != NULL) {
                        value[0] = 0;
                        pAudio->audioCallback(pAudio->sta,value);
                    }
				} else if(ratcret == 1) {
				    phatc->retbufp = 0;
				    return RETCHAR_KEEP;
				}
				pAudio->sta = AUDIO_IDLE;
				pAudio->stim = 0;
                pAudio->step = 0;
				phatc->retbufp = 0;
                return RETCHAR_FREE;
			}
			break;
        case AUDIO_RECORDING_STATUS:
		if(pAudio->step == 0)
		{
		    phatc->type = CRLF_HATCTYP;
			while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) {
                phatc->retbufp = 0;
            }
            SamSendAtCmd(phatc, "AT+CREC?\r\n", CRLF_HATCTYP, 80);
            pAudio->step += 1;
			pAudio->stim = 0;
			return RETCHAR_KEEP;
		}
		else if(pAudio->step == 1)
		{
		    pAudio->phatc->type = CRLF_HATCTYP;
			ratcret = SamChkAtcRet(pAudio->phatc, "+CREC: 0\r\n\t+CREC: 1\r\n\tOK\r\n\tERROR\r\n");
			if(ratcret == NOSTRRET_ATCRET) {
            	return RETCHAR_KEEP;
			} else if(ratcret == OVERTIME_ATCRET || ratcret == 4) {
                if(pAudio->audioCallback != NULL) {
                    value[0] = -1;
                    pAudio->audioCallback(pAudio->sta,value);
                }
			} else if(ratcret == 1 || ratcret == 2) {
                if(pAudio->audioCallback != NULL) {
                    value[0] = ratcret - 1;
                    pAudio->audioCallback(pAudio->sta,value);
                }
			} else if(ratcret == 3) {
                if(pAudio->audioCallback != NULL) {
                    value[0] = 0;
                    pAudio->audioCallback(pAudio->sta,value);
                }
			} else {
			    phatc->retbufp = 0;
			    return RETCHAR_KEEP;
			}
			pAudio->sta = AUDIO_IDLE;
			pAudio->stim = 0;
            pAudio->step = 0;
			phatc->retbufp = 0;
            return RETCHAR_FREE;
		}
		break;
		case AUDIO_FAILED:
		default :
			pAudio->sta = AUDIO_IDLE;
			break;
	}
	return RETCHAR_KEEP;
}

unsigned char sam_audio_urc_cb(void *pAudio, char *urcStr)
{
    Audio_Tag_T * pTmpAudio = NULL;
    pTmpAudio = (Audio_Tag_T *)pAudio;
    return pTmpAudio->audioURCCallback(urcStr);
}

