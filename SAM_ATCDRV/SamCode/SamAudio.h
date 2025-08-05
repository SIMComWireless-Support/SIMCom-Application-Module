/**
  * @file    : SamAudio.h
  * @brief   : Implement recording and playback related functions.
  * @author  : wangxiaochun@sunseaaiot.com
  * @version : 1.0.0
  * @date    : 2025-07-01
  * @license : MIT
  */

#ifndef SAMAUDIO_H_
#define SAMAUDIO_H_

typedef enum{
    AUDIO_NONE,
    AUDIO_IDLE,
    AUDIO_INIT,
    AUDIO_DEINIT,
	AUDIO_PLAY,
	AUDIO_STOP_PLAYING,
	AUDIO_RECORDING_STATUS,
	AUDIO_RECORD_START,
    AUDIO_RECORD_STOP,
	AUDIO_FAILED,
}Audio_Status_E;

typedef void (* sam_audio_callback)(Audio_Status_E audioStatus,char *result);
typedef char (* sam_audio_urc_callback)(char *urcStr);


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
extern int sam_audio_play(char *fileName,uint8 playPath,uint8 repeat);

/**
 * @brief The function is used to stop playing audio file.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
extern int sam_audio_stop_playing(void);

/**
 * @brief This function is used to get recording status.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
extern int sam_audio_get_record_status(void);

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
extern int sam_audio_record_start(const char *fileName,uint8 recordPath);

/**
 * @brief This function is used to stop a recording.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The Audio module is executing other tasks,please try late.
 */
extern int sam_audio_record_stop(void);

/**
 * @brief Register Audio module.
 * @param atcIndex Which ATC channel should be used. Usually set to 0.
 * @param audioCallback After the AT instruction is executed, the callback function returns the execution result.
 * @param urcCallback Report the result after the asynchronous command is executed.
 * @return Returning 0 indicates successful execution and returning -1 indicates failure,
 *               Please check the input parameters correctly. 
 */
extern int sam_audio_init(uint8 atcIndex,sam_audio_callback audioCallback,sam_audio_urc_callback urcCallback);

/**
 * @brief Deregister Audio module.
 * @return Returning 0 indicates successful execution and returning -1 indicates that 
 *  The TTS module is executing other tasks,please try late.
 */
extern int sam_audio_deinit(void);

#endif /* SAMAUDIO_H_ */

