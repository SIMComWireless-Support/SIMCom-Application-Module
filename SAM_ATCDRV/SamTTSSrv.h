/**
  * @file    : SamTTSSrv.h
  * @brief   : Implement TTS related operations, including parameter settings,
  *                 playing TTS voice and other functions.
  * @author  : wangxiaochun@sunseaaiot.com
  * @version : 1.0.0
  * @date    : 2025-07-01
  * @license : MIT
  */

#ifndef SAM_TTS_SRV_H
#define SAM_TTS_SRV_H

#include "include.h"

#include "SamTTS.h"

/**
 * @brief  Register TTS module.
 */
extern void sam_demo_tts_init(void);

/**
 * @brief Example of TTS module execution.
 */
extern void sam_demo_tts_proc(void);
#endif /* SAM_TTS_SRV_H */