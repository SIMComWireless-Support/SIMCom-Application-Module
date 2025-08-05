/**
  * @file    : SamTTS.h
  * @brief   : Implement TTS related operations, including parameter settings,
  *                 playing TTS voice and other functions.
  * @author  : wangxiaochun@sunseaaiot.com
  * @version : 1.0.0
  * @date    : 2025-07-01
  * @license : MIT
  */


#ifndef SAMTTS_H_
#define SAMTTS_H_

typedef enum {
    TTS_PLAYING_NONE_FORMAT = 0,
    TTS_PLAYING_ONLY_ASCII_FORMAT,
    TTS_PLAYING_UCS2_FORMAT,
    TTS_PLAYING_ASCII_AND_GBK_OR_ONLY_GBK_FORMAT,
}TTS_PLAYING_DATA_FORMAT_E;

typedef enum{
    TTS_NONE,
    TTS_IDLE,
    TTS_INIT,
    TTS_DEINIT,
	TTS_GET_STATUS,
	TTS_PLAY,
	TTS_PLAY_AND_SAVE_TO_FILE,
	TTS_STOP_PLAYING,
	TTS_GET_YOUNGTONE_PARAM,
	TTS_SET_YOUNGTONE_PARAM,
	TTS_GET_IFLY_PARAM,
	TTS_SET_IFLY_PARAM,
	TTS_GET_LOCAL_OR_REMOTE_PLAY,
	TTS_SET_LOCAL_OR_REMOTE_PLAY,
	TTS_GET_SYS_VOLUME_SETTING,
	TTS_SET_SYS_VOLUME_SETTING,
	TTS_FAILED,
}TTS_Status_E;

typedef enum {
    TTS_VOL = 0,
    TTS_SYS_VOL,
    TTS_DIGIT_MODE,
    TTS_PITCH,
    TTS_SPEED,
    TTS_DIGIT_READING_FOR_YOUNGTONE_OR_TTSLIB_FOR_IFLY,
}TTS_ATTRIBUTE_SETTING;

typedef struct {
    uint8 params[6];
}TTS_param_T;

typedef void (* sam_tts_callback)(TTS_Status_E ttsStatus,char *result);
typedef char (* sam_tts_urc_callback)(char *urcStr);


/**
 * @brief Register TTS module.
 * @param atcIndex Which ATC channel should be used. Usually set to 0.
 * @param ttsCallback After the AT instruction is executed, the callback function returns the execution result.
 * @param urcTTSCallback Report the result after the asynchronous command is executed.
 * @return Returning 0 indicates successful execution and returning -1 indicates failure,
 *               Please check the input parameters correctly. 
 */
extern int sam_tts_init(uint8 atcIndex,sam_tts_callback ttsCallback,sam_tts_urc_callback urcTTSCallback);

/**
 * @brief Deregister TTS module.
 */
extern void sam_tts_deinit(void);

/**
 * @brief Get the current status of TTS, whether it is stopped or playing TTS voice.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
extern int sam_tts_get_status(void);

/**
 * @brief Stop the speech play.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
extern int sam_tts_stop_playing(void);

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
extern int sam_tts_play(uint8 *pData,uint16 dataSize,TTS_PLAYING_DATA_FORMAT_E format);

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
extern int sam_tts_play_and_save_wav(uint8 *pData,uint16 dataSize,char *fileName,TTS_PLAYING_DATA_FORMAT_E format);

/**
 * @brief Get YOUNGTONE related parameters.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
extern int sam_tts_get_YOUNGTONE_param(void);

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
extern int sam_tts_set_YOUNGTONE_param(TTS_param_T *pParam);

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
extern int sam_tts_set_IFLY_param(TTS_param_T *pParam);

/**
 * @brief Get IFLY related parameters.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
extern int sam_tts_get_IFLY_param(void);

/**
 * @brief The read command is used to get Local or Remote Audio Play.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
extern int sam_tts_get_local_or_remote_status(void);

/**
 * @brief The write command is used to Set Local or Remote Audio Play.
 * @param localOrRemote 0:Local Path,1:Remote Path.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
extern int sam_tts_set_local_or_remote_status(uint8 localOrRemote);

/**
 * @brief The read command is used to get the TTS system volume setting.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
extern int sam_tts_get_sys_vol_setting_status(void);

/**
 * @brief The write command is used to Disable the TTS system volume setting.
 * @param sysVolSetting 0:volume setting is valid,1:volume setting is invalid.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
extern int sam_tts_set_sys_vol_setting(uint8 sysVolSetting);
#endif /* SAMTTS_H_ */
