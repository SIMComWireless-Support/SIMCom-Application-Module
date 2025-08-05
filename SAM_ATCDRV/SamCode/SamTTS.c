/**
  * @file    : SamTTS.c
  * @brief   : Implement TTS related operations, including parameter settings,
  *                 playing TTS voice and other functions.
  * @author  : wangxiaochun@sunseaaiot.com
  * @version : 1.0.0
  * @date    : 2025-07-01
  * @license : MIT
  */

#include <stdio.h>
#include <stdlib.h>
#include "include.h"
#include "SamAtc.h"
#include "SamTTS.h"

#define	TTS_WRITE_BUF_LEN 512
#define	TTS_SAVE_WAVE_FILE_NAME_LEN 64

typedef struct{
    uint8 sta;
    uint8 step;
    uint8 dcnt;
    uint8 runlink; //for run link in atclink  
    uint32 msclk;  //for recode sysclk
    uint8 stim;    //second timer for user  
    uint8 localOrRomote;
    uint8 ttsSysVolSetting;
    TTS_PLAYING_DATA_FORMAT_E dataFormat;
    HdsAtcTag* phatc;
    char writeBuf[TTS_WRITE_BUF_LEN];
    uint16 writeCount;
    char fileName[TTS_SAVE_WAVE_FILE_NAME_LEN];
    sam_tts_callback ttsCallback;
    sam_tts_urc_callback ttsURCCallback;
    TTS_param_T ttsParams;
}TTS_Tag_T;


TTS_Tag_T mTTSTag;
extern HdsAtcTag * pAtcBusArray[];
static unsigned char sam_tts_proc(void *pTTSTag);
static unsigned char sam_tts_urc_cb(void *pTTS, char *urcStr);

/**
 * @brief Get the current status of TTS, whether it is stopped or playing TTS voice.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
int sam_tts_get_status(void) {
    TTS_Tag_T *pTTS = &mTTSTag;

    if (pTTS->sta == TTS_GET_STATUS){
        return 0;
    } else if (pTTS->sta != TTS_IDLE) {
        return -1;
    }
    pTTS->sta = TTS_GET_STATUS;
    pTTS->step = 0;
    pTTS->stim = 0;
    return 0;
}

/**
 * @brief Stop the speech play.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
int sam_tts_stop_playing(void) {
    TTS_Tag_T *pTTS = &mTTSTag;
    if(pTTS->sta == TTS_STOP_PLAYING) {
        return 0;
    } else if(pTTS->sta != TTS_IDLE) {
        return -1;
    }
    
    pTTS->sta = TTS_STOP_PLAYING;
    pTTS->step = 0;
    pTTS->stim = 0;
    return 0;
}

/**
 * @brief Start the speech play.
 * @param pData is a data pointer.  the <text> is in ASCII coding format which is
 *   synthetized to speed to be played, maximum data length is 512
 *   bytes.(including "").And <text> is in UCS2 coding format,
 *   maximum data length is 510 bytes. (including ""),because every
 *   four characters correspond to one Chinese character.
 * @param dataSize is the length of data owned by pData.
 * @param format is the format of the data owned by pData.
 * @return Returning 0 indicates successful execution and returning -1 indicates failure,
 *               Please check the input parameters correctly.
 */
int sam_tts_play(uint8 *pData,uint16 dataSize,TTS_PLAYING_DATA_FORMAT_E format) {
    return sam_tts_play_and_save_wav(pData,dataSize,NULL,format);
}

/**
 * @brief Start the speech play.
 * @param pData is a data pointer.  <text> maximum data length is 50 bytes because of the
 *  memory. For TOUNGTONE TTS Chinese polyphonic characters,
 *  pronunciation can be specified.For IFLY TTS Chinese polyphonic characters, pronunciation
 *  can be specified.
 * @param dataSize is the length of data owned by pData.
 * @param fileName if no path is added, save in C: by default.
 *  Maximum filename length is 60 bytes.
 *  Currently only .wav format file storage is supported.
 * @param format is the format of the data owned by pData.
 * @return Returning 0 indicates successful execution and returning -1 indicates failure,
 *               Please check the input parameters correctly.
 */
int sam_tts_play_and_save_wav(uint8 *pData,uint16 dataSize,char *fileName,TTS_PLAYING_DATA_FORMAT_E format) {
    TTS_Tag_T *pTTS = &mTTSTag;

    if (pTTS->sta != TTS_IDLE || pData == NULL) {
        return -1;
    }
    if (pTTS == NULL) {
        return -1;
    } else if (pTTS->sta == TTS_PLAY) {
        return 0;
    } else if(pTTS->sta != TTS_IDLE) {
        return -1;
    }
    if(fileName != NULL) {
        if(strlen(fileName) > 60) {
            return -1;
        }
        if(dataSize > 50) {
            return -1;
        }
        memcpy(pTTS->fileName,fileName,strlen(fileName));
    } else {
        if (format == TTS_PLAYING_UCS2_FORMAT || \
            format == TTS_PLAYING_ASCII_AND_GBK_OR_ONLY_GBK_FORMAT) {
            if(dataSize > 510) {
                return -1;
            }
        } else if (format == TTS_PLAYING_ONLY_ASCII_FORMAT) {
            if(dataSize > 512) {
                return -1;
            }
        } else {
            return -1;
        }
    }
    memcpy(pTTS->writeBuf,pData,dataSize);
    pTTS->writeCount = dataSize;
    pTTS->dataFormat = format;
    pTTS->sta = TTS_PLAY;
    pTTS->step = 0;
    pTTS->stim = 0;
    return 0;
}

/**
 * @brief Register TTS module.
 * @param atcIndex Which ATC channel should be used. Usually set to 0.
 * @param ttsCallback After the AT instruction is executed, the callback function returns the execution result.
 * @param urcTTSCallback Report the result after the asynchronous command is executed.
 * @return Returning 0 indicates successful execution and returning -1 indicates failure,
 *               Please check the input parameters correctly. 
 */
int sam_tts_init(uint8 atcIndex,sam_tts_callback ttsCallback,sam_tts_urc_callback urcTTSCallback) {
    TTS_Tag_T *pTTS = &mTTSTag;

    if(atcIndex >= ATCBUS_CHMAX) {
        return -1;
    }
    pTTS->phatc = pAtcBusArray[atcIndex];
    pTTS->sta = TTS_IDLE;
    pTTS->step = 0;
    pTTS->dcnt = 0;
    pTTS->stim = 0;
    pTTS->writeCount = 0;
    pTTS->dataFormat = TTS_PLAYING_NONE_FORMAT;
    memset(pTTS->ttsParams.params,0,sizeof(pTTS->ttsParams.params));
    pTTS->msclk = SamGetMsCnt(0);
    pTTS->runlink =	SamAtcFunLink(pTTS->phatc, pTTS, sam_tts_proc, sam_tts_urc_cb);
    pTTS->ttsCallback = ttsCallback;
    pTTS->ttsURCCallback = urcTTSCallback;
    return 0;
}

/**
 * @brief Deregister TTS module.
 */
void sam_tts_deinit(void)
{
	TTS_Tag_T *pTTS = &mTTSTag;
	SamAtcFunUnlink(pTTS->phatc, pTTS->runlink);
}

/**
 * @brief Set YOUNGTONE related parameters.
 * @param pParam Which is a TTS_param_T structure pointer.
 *  The array inside the structure represents six parameters.
 *  index 0:TTS lib volume,range (0,1,2),default:1.
 *  index 1:system volume,range (0,1,2,3),default:3.
 *  index 2:digitmode,range (0,1,2),default:0.
 *  index 3:pitch,range (0,1,2),default:1.
 *  index 4:speed,rough speed regulation,range (0,1,2),default:1;precision speed regulation,range [10...30].
 *  index 5:digitreading,range (0,1),default:0.
 * @return Returning 0 indicates successful execution and returning -1 indicates failure,
 *               Please check the input parameters correctly. 
 */
int sam_tts_set_YOUNGTONE_param(TTS_param_T *pParam){
    TTS_Tag_T *pTTS = &mTTSTag;

    if(pParam == NULL) {
        return -1;
    }
    if(pTTS->sta == TTS_SET_YOUNGTONE_PARAM) {
        return 0;
    } else if(pTTS->sta != TTS_IDLE) {
        return -1;
    }
    pTTS->sta = TTS_SET_YOUNGTONE_PARAM;
    pTTS->step = 0;
    pTTS->stim = 0;
    memcpy(pTTS->ttsParams.params,pParam->params,sizeof(pParam->params));
    return 0;
}

/**
 * @brief Get YOUNGTONE related parameters.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
int sam_tts_get_YOUNGTONE_param(void){
    TTS_Tag_T *pTTS = &mTTSTag;
    if(pTTS->sta == TTS_GET_YOUNGTONE_PARAM) {
        return 0;
    } else if(pTTS->sta != TTS_IDLE) {
        return -1;
    }
    pTTS->sta = TTS_GET_YOUNGTONE_PARAM;
    pTTS->step = 0;
    pTTS->stim = 0;
    return 0;
}

/**
 * @brief Set IFLY related parameters.
 * @param pParam Which is a TTS_param_T structure pointer.
 *  The array inside the structure represents six parameters.
 *  index 0:TTS lib volume,range (0,1,2),default:2.
 *  index 1:system volume,range (0...7),default:4.
 *  index 2:digitmode,range (0,1,2),default:0.
 *  index 3:pitch,range (0,1,2),default:1.
 *  index 4:speed,rough speed regulation,range (0,1,2),default:1;precision speed regulation,range [10...30].
 *  index 5:ttslib,range (0,1),default:0.
 * @return Returning 0 indicates successful execution and returning -1 indicates failure,
 *               Please check the input parameters correctly. 
 */
int sam_tts_set_IFLY_param(TTS_param_T *pParam){
    TTS_Tag_T *pTTS = &mTTSTag;

    if(pParam == NULL) {
        return -1;
    }
    if(pTTS->sta == TTS_SET_IFLY_PARAM) {
        return 0;
    } else if(pTTS->sta != TTS_IDLE) {
        return -1;
    }
    pTTS->sta = TTS_SET_IFLY_PARAM;
    pTTS->step = 0;
    pTTS->stim = 0;
    memcpy(pTTS->ttsParams.params,pParam->params,sizeof(pParam->params));
    return 0;
}

/**
 * @brief Get IFLY related parameters.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
int sam_tts_get_IFLY_param(void){
    TTS_Tag_T *pTTS = &mTTSTag;
    if(pTTS->sta == TTS_GET_IFLY_PARAM) {
        return 0;
    } else if(pTTS->sta != TTS_IDLE) {
        return -1;
    }
    pTTS->sta = TTS_GET_IFLY_PARAM;
    pTTS->step = 0;
    pTTS->stim = 0;
    return 0;
}

/**
 * @brief The read command is used to get Local or Remote Audio Play.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
int sam_tts_get_local_or_remote_status(void) {
    TTS_Tag_T *pTTS = &mTTSTag;
    if (pTTS->sta == TTS_GET_LOCAL_OR_REMOTE_PLAY) {
        return 0;
    } else if(pTTS->sta != TTS_IDLE){
        return -1;
    }
    pTTS->sta = TTS_GET_LOCAL_OR_REMOTE_PLAY;
    pTTS->step = 0;
    pTTS->stim = 0;
    return 0;
}

/**
 * @brief The write command is used to Set Local or Remote Audio Play.
 * @param localOrRemote 0:Local Path,1:Remote Path.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
int sam_tts_set_local_or_remote_status(uint8 localOrRemote) {
    TTS_Tag_T *pTTS = &mTTSTag;
    if (pTTS->sta == TTS_SET_LOCAL_OR_REMOTE_PLAY) {
        return 0;
    } else if(pTTS->sta != TTS_IDLE){
        return -1;
    }
    pTTS->localOrRomote = localOrRemote;
    pTTS->sta = TTS_SET_LOCAL_OR_REMOTE_PLAY;
    pTTS->step = 0;
    pTTS->stim = 0;
    return 0;
}

/**
 * @brief The read command is used to get the TTS system volume setting.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
int sam_tts_get_sys_vol_setting_status(void) {
    TTS_Tag_T *pTTS = &mTTSTag;

    if (pTTS->sta == TTS_GET_SYS_VOLUME_SETTING) {
        return 0;
    } else if(pTTS->sta != TTS_IDLE){
        return -1;
    }
    pTTS->sta = TTS_GET_SYS_VOLUME_SETTING;
    pTTS->step = 0;
    pTTS->stim = 0;
    return 0;
}

/**
 * @brief The write command is used to Disable the TTS system volume setting.
 * @param sysVolSetting 0:volume setting is valid,1:volume setting is invalid.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
int sam_tts_set_sys_vol_setting(uint8 sysVolSetting) {
    TTS_Tag_T *pTTS = &mTTSTag;

    if (pTTS->sta == TTS_SET_SYS_VOLUME_SETTING) {
        return 0;
    } else if(pTTS->sta != TTS_IDLE){
        return -1;
    }
    pTTS->ttsSysVolSetting = sysVolSetting;
    pTTS->sta = TTS_SET_SYS_VOLUME_SETTING;
    pTTS->step = 0;
    pTTS->stim = 0;
    return 0;
}

unsigned char sam_tts_proc(void *pTTSTag)
{
	uint8 ratcret;
	uint32 clk;
    char value[6];
	TTS_Tag_T * pTTS = NULL;
	HdsAtcTag * phatc = NULL;

	pTTS = pTTSTag;
	if (pTTS == NULL) {
        return ('E'+1);
    }
	phatc = pTTS->phatc;
	if (phatc == NULL) {
        return ('E'+2);
    }

	clk = SamGetMsCnt(pTTS->msclk);
	while(clk >= 1000)
	{
		pTTS->msclk +=1000;
		pTTS->stim += 1;
		clk -= 1000;
	}
	
	switch(pTTS->sta)
	{
		case TTS_IDLE:
			return RETCHAR_FREE;
		case TTS_GET_STATUS:
			if(pTTS->step == 0)
			{
			    phatc->type = CRLF_HATCTYP;
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) {
                    phatc->retbufp = 0;
                }
                SamSendAtCmd(pTTS->phatc, "AT+CTTS?\r\n", CRLF_HATCTYP, 80);
                pTTS->step += 1;
				pTTS->stim = 0;
				return RETCHAR_KEEP;
			}
			else if(pTTS->step == 1)
			{
			    pTTS->phatc->type = CRLF_HATCTYP;
				ratcret = SamChkAtcRet(pTTS->phatc, "+CTTS: 0\r\n\t+CTTS: 1\r\n\tOK\r\n\tERROR\r\n");
				if(ratcret == NOSTRRET_ATCRET) {
                	return RETCHAR_KEEP;
				}
				else if(ratcret == OVERTIME_ATCRET || ratcret == 4) {
                    if(pTTS->ttsCallback != NULL) {
                        value[0] = -1;
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				}
				else if(ratcret == 1) {
                    if(pTTS->ttsCallback != NULL) {
                        value[0] = 0;
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				}
				else if(ratcret == 2) {
                    if(pTTS->ttsCallback != NULL) {
                        value[0] = 1;
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				}
				pTTS->sta = TTS_IDLE;
				pTTS->stim = 0;
                pTTS->step = 0;
				phatc->retbufp = 0;
                return RETCHAR_FREE;
			}
			break;
		case TTS_PLAY:
			if(pTTS->step == 0)
			{
			    uint8 format;
                uint8 fileNameLen = 0;
                if(pTTS->fileName != NULL) {
                    fileNameLen = strlen(pTTS->fileName);
                }
                uint16 dataLen = 0;
			    char data[544] = {0};
			    phatc->type = CRLF_HATCTYP;
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) {
                    phatc->retbufp = 0;
                }
                if (pTTS->dataFormat == TTS_PLAYING_UCS2_FORMAT) {
                    if(fileNameLen > 0) {
                        format = 4;
                    } else {
                        format = 1;
                    }
                } else if(pTTS->dataFormat == TTS_PLAYING_ONLY_ASCII_FORMAT || \
                          pTTS->dataFormat == TTS_PLAYING_ASCII_AND_GBK_OR_ONLY_GBK_FORMAT) {
                    if(fileNameLen > 0) {
                        format = 3;
                    } else {
                        format = 2;
                    }
                }
                sprintf(data,"AT+CTTS=%u,",format);
                dataLen = strlen(data);
                memcpy(data+dataLen,pTTS->writeBuf,pTTS->writeCount);
                dataLen += pTTS->writeCount;
                if (fileNameLen > 0) {
                    memcpy(data+dataLen,",",1);
                    dataLen++;
                    memcpy(data+dataLen,pTTS->fileName,fileNameLen);
                    dataLen += fileNameLen;
                }
                memcpy(data+dataLen,"\r\n\0",3);
                SamSendAtCmd(phatc, data, CRLF_HATCTYP, 80);
                pTTS->step += 1;
				pTTS->stim = 0;
				return RETCHAR_KEEP;
			}
			else if(pTTS->step == 1)
			{
			    uint8 status = TTS_PLAY;
                if(pTTS->fileName != NULL && strlen(pTTS->fileName) > 0) {
                    status = TTS_PLAY_AND_SAVE_TO_FILE;
                }
			    pTTS->phatc->type = CRLF_HATCTYP;
				ratcret = SamChkAtcRet(pTTS->phatc, "+CTTS:\r\n\tOK\r\n\tERROR\r\n");
				if(ratcret == NOSTRRET_ATCRET) {
                	return RETCHAR_KEEP;
				} else if(ratcret == OVERTIME_ATCRET || ratcret == 3) {
                    if(pTTS->ttsCallback != NULL) {
                        value[0] = -1;
                        pTTS->ttsCallback(status,value);
                    }
				} else if(ratcret == 2) {
                    if(pTTS->ttsCallback != NULL) {
                        value[0] = 0;
                        pTTS->ttsCallback(status,value);
                    }
				} else if(ratcret == 1) {
    				phatc->retbufp = 0;
				    return RETCHAR_KEEP;
				}
				pTTS->sta = TTS_IDLE;
				pTTS->stim = 0;
                pTTS->step = 0;
				phatc->retbufp = 0;
                return RETCHAR_FREE;
			}
			break;
        case TTS_STOP_PLAYING:
			if(pTTS->step == 0)
			{
			    phatc->type = CRLF_HATCTYP;
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) {
                    phatc->retbufp = 0;
                }
                SamSendAtCmd(phatc, "AT+CTTS=0\r\n", CRLF_HATCTYP, 80);
                pTTS->step += 1;
				pTTS->stim = 0;
				return RETCHAR_KEEP;
			}
			else if(pTTS->step == 1)
			{
			    pTTS->phatc->type = CRLF_HATCTYP;
				ratcret = SamChkAtcRet(pTTS->phatc, "+CTTS: 0\r\n\tOK\r\n\tERROR\r\n");
				if(ratcret == NOSTRRET_ATCRET) {
                	return RETCHAR_KEEP;
				}
				else if(ratcret == OVERTIME_ATCRET || ratcret == 3) {
                    if(pTTS->ttsCallback != NULL) {
                        value[0] = -1;
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				}
				else if(ratcret == 2) {
                    if(pTTS->ttsCallback != NULL) {
                        value[0] = 0;
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				} else if(ratcret == 1) {
				    phatc->retbufp = 0;
				    return RETCHAR_KEEP;
				}
				pTTS->sta = TTS_IDLE;
				pTTS->stim = 0;
                pTTS->step = 0;
				phatc->retbufp = 0;
                return RETCHAR_FREE;
			}
			break;
        case TTS_SET_YOUNGTONE_PARAM:
        case TTS_SET_IFLY_PARAM:
            if(pTTS->step == 0)
			{
			    char data[32] = {0};
			    phatc->type = CRLF_HATCTYP;
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) {
                    phatc->retbufp = 0;
                }
                sprintf(data,"AT+CTTSPARAM=%u,%u,%u,%u,%u,%u\r\n" \
                           ,pTTS->ttsParams.params[TTS_VOL] \
                           ,pTTS->ttsParams.params[TTS_SYS_VOL] \
                           ,pTTS->ttsParams.params[TTS_DIGIT_MODE] \
                           ,pTTS->ttsParams.params[TTS_PITCH] \
                           ,pTTS->ttsParams.params[TTS_SPEED] \
                           ,pTTS->ttsParams.params[TTS_DIGIT_READING_FOR_YOUNGTONE_OR_TTSLIB_FOR_IFLY]);
                SamSendAtCmd(phatc, data, CRLF_HATCTYP, 80);
                pTTS->step += 1;
				pTTS->stim = 0;
				return RETCHAR_KEEP;
			}
			else if(pTTS->step == 1)
			{
			    pTTS->phatc->type = CRLF_HATCTYP;
				ratcret = SamChkAtcRet(pTTS->phatc, "OK\r\n\tERROR\r\n");
				if(ratcret == NOSTRRET_ATCRET) {
                	return RETCHAR_KEEP;
				} else if(ratcret == OVERTIME_ATCRET || ratcret == 2) {
				    if(pTTS->ttsCallback != NULL) {
                        value[0] = -1;
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				} else if(ratcret == 1) {
                    if(pTTS->ttsCallback != NULL) {
                        value[0] = 0;
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				}
				pTTS->sta = TTS_IDLE;
				pTTS->stim = 0;
                pTTS->step = 0;
				phatc->retbufp = 0;
                return RETCHAR_FREE;
			}
            break;
        case TTS_GET_YOUNGTONE_PARAM:
        case TTS_GET_IFLY_PARAM:
            if(pTTS->step == 0)
			{
			    phatc->type = CRLF_HATCTYP;
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) {
                    phatc->retbufp = 0;
                }
                SamSendAtCmd(phatc, "AT+CTTSPARAM?\r\n", CRLF_HATCTYP, 80);
                pTTS->step += 1;
				pTTS->stim = 0;
				return RETCHAR_KEEP;
			}
			else if(pTTS->step == 1)
			{
			    pTTS->phatc->type = CRLF_HATCTYP;
				ratcret = SamChkAtcRet(pTTS->phatc, "+CTTSPARAM:\tOK\r\n\tERROR\r\n");
				if(ratcret == NOSTRRET_ATCRET) {
                	return RETCHAR_KEEP;
				} else if(ratcret == OVERTIME_ATCRET || ratcret == 3) {
				    if(pTTS->ttsCallback != NULL) {
                        value[0] = -1;
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				} else if(ratcret == 2) {
				    phatc->retbufp = 0;
                    return RETCHAR_KEEP;
				} else if(ratcret == 1) {
                    char *p = pTTS->phatc->retbuf + 11;
                    for (int i=0;i<6;i++) {
                        p++;
                        value[i] = atoi(p++);
                    }
                    if(pTTS->sta == TTS_GET_IFLY_PARAM) {
                        uint8 tmp;
                        tmp = value[0];
                        value[0] = value[1];
                        value[1] = tmp;
                    }
                    if(pTTS->ttsCallback != NULL) {
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				}
				pTTS->sta = TTS_IDLE;
				pTTS->stim = 0;
                pTTS->step = 0;
				phatc->retbufp = 0;
                return RETCHAR_FREE;
			}
            break;
        case TTS_GET_LOCAL_OR_REMOTE_PLAY:
            if(pTTS->step == 0)
			{
			    phatc->type = CRLF_HATCTYP;
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) {
                    phatc->retbufp = 0;
                }
                SamSendAtCmd(pTTS->phatc, "AT+CDTAM?\r\n", CRLF_HATCTYP, 80);
                pTTS->step += 1;
				pTTS->stim = 0;
				return RETCHAR_KEEP;
			}
			else if(pTTS->step == 1)
			{
			    pTTS->phatc->type = CRLF_HATCTYP;
				ratcret = SamChkAtcRet(pTTS->phatc, "+CDTAM: 0\r\n\t+CDTAM: 1\r\n\tOK\r\n\tERROR\r\n");
				if(ratcret == NOSTRRET_ATCRET) {
                	return RETCHAR_KEEP;
				}
				else if(ratcret == OVERTIME_ATCRET || ratcret == 4) {
                    if(pTTS->ttsCallback != NULL) {
                        value[0]=-1;
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				}
				else if(ratcret == 1) {
                    if(pTTS->ttsCallback != NULL) {
                        value[0]=0;
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				}
				else if(ratcret == 2) {
                    if(pTTS->ttsCallback != NULL) {
                        value[0]=1;
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				}
				pTTS->sta = TTS_IDLE;
				pTTS->stim = 0;
                pTTS->step = 0;
				phatc->retbufp = 0;
                return RETCHAR_FREE;
			}
            break;
        case TTS_SET_LOCAL_OR_REMOTE_PLAY:
            if(pTTS->step == 0)
			{
			    char data[16] = {0};
			    phatc->type = CRLF_HATCTYP;
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) {
                    phatc->retbufp = 0;
                }
                sprintf(data,"AT+CDTAM=%d\r\n",pTTS->localOrRomote);
                SamSendAtCmd(phatc, data, CRLF_HATCTYP, 80);
                pTTS->step += 1;
				pTTS->stim = 0;
				return RETCHAR_KEEP;
			}
			else if(pTTS->step == 1)
			{
			    pTTS->phatc->type = CRLF_HATCTYP;
				ratcret = SamChkAtcRet(pTTS->phatc, "OK\r\n\tERROR\r\n");
				if(ratcret == NOSTRRET_ATCRET) {
                	return RETCHAR_KEEP;
				}
				else if(ratcret == OVERTIME_ATCRET || ratcret == 2) {
                    if(pTTS->ttsCallback != NULL) {
                        value[0]=-1;
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				}
				else if(ratcret == 1) {
                    if(pTTS->ttsCallback != NULL) {
                        value[0]=0;
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				}
				pTTS->sta = TTS_IDLE;
				pTTS->stim = 0;
                pTTS->step = 0;
				phatc->retbufp = 0;
                return RETCHAR_FREE;
			}
            break;
        case TTS_GET_SYS_VOLUME_SETTING:
            if(pTTS->step == 0)
			{
			    phatc->type = CRLF_HATCTYP;
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) {
                    phatc->retbufp = 0;
                }
                SamSendAtCmd(pTTS->phatc, "AT+CTTSVOLINV?\r\n", CRLF_HATCTYP, 80);
                pTTS->step += 1;
				pTTS->stim = 0;
				return RETCHAR_KEEP;
			}
			else if(pTTS->step == 1)
			{
			    pTTS->phatc->type = CRLF_HATCTYP;
				ratcret = SamChkAtcRet(pTTS->phatc, "+CTTSVOLINV: 0\r\n\t+CTTSVOLINV: 1\r\n\tOK\r\n\tERROR\r\n");
				if(ratcret == NOSTRRET_ATCRET) {
                	return RETCHAR_KEEP;
				}
				else if(ratcret == OVERTIME_ATCRET || ratcret == 4) {
                    if(pTTS->ttsCallback != NULL) {
                        value[0] = -1;
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				}
				else if(ratcret == 1) {
                    if(pTTS->ttsCallback != NULL) {
                        value[0] = 0;
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				}
				else if(ratcret == 2) {
                    if(pTTS->ttsCallback != NULL) {
                        value[0] = 2;
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				}
				pTTS->sta = TTS_IDLE;
				pTTS->stim = 0;
                pTTS->step = 0;
				phatc->retbufp = 0;
                return RETCHAR_FREE;
			}
            break;
        case TTS_SET_SYS_VOLUME_SETTING:
            if(pTTS->step == 0)
			{
			    char data[32] = {0};
			    phatc->type = CRLF_HATCTYP;
				while(SamChkAtcRet(phatc, "OK\r\n\tERROR\r\n") != NOSTRRET_ATCRET) {
                    phatc->retbufp = 0;
                }
                sprintf(data,"AT+CTTSVOLINV=%d\r\n",pTTS->ttsSysVolSetting);
                SamSendAtCmd(phatc, data, CRLF_HATCTYP, 80);
                pTTS->step += 1;
				pTTS->stim = 0;
				return RETCHAR_KEEP;
			}
			else if(pTTS->step == 1)
			{
			    pTTS->phatc->type = CRLF_HATCTYP;
				ratcret = SamChkAtcRet(pTTS->phatc, "OK\r\n\tERROR\r\n");
				if(ratcret == NOSTRRET_ATCRET) {
                	return RETCHAR_KEEP;
				}
				else if(ratcret == OVERTIME_ATCRET || ratcret == 2) {
                    if(pTTS->ttsCallback != NULL) {
                        value[0] = -1;
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				}
				else if(ratcret == 1) {
                    if(pTTS->ttsCallback != NULL) {
                        value[0] = 0;
                        pTTS->ttsCallback(pTTS->sta,value);
                    }
				}
				pTTS->sta = TTS_IDLE;
				pTTS->stim = 0;
                pTTS->step = 0;
				phatc->retbufp = 0;
                return RETCHAR_FREE;
			}
            break;
		case TTS_FAILED:
		default :
			pTTS->sta = TTS_IDLE;
			break;
	}
	return RETCHAR_KEEP;
}

unsigned char sam_tts_urc_cb(void *pTTS, char *urcStr)
{
    TTS_Tag_T *pTmpTTS = (TTS_Tag_T *)pTTS;
    return pTmpTTS->ttsURCCallback(urcStr);
}
