/**
  * @file    : SamAudio.h
  * @brief   : Implement recording and playback related functions.
  * @author  : wangxiaochun@sunseaaiot.com
  * @version : 1.0.0
  * @date    : 2025-07-01
  * @license : MIT
  */

#ifndef SAM_AUDIO_SRV_H
#define SAM_AUDIO_SRV_H

#include "include.h"

#include "SamAudio.h"

/**
 * @brief  Register Audio module.
 */
extern void sam_demo_audio_init(void);

/**
 * @brief Example of Audio module execution.
 */
extern void sam_demo_audio_proc(void);

#endif /* SAM_AUDIO_SRV_H */