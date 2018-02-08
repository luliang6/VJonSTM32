/**
  ******************************************************************************
  * @file    app_audio.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Header for app_audio.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_AUDIO_H
#define __APP_AUDIO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "global_includes.h"
#include "mod_console.h"
#include <string.h>

#include "stm32_audio.h"
#include "stm32_audio_out_if.h"
#include "stm32_audio_correction.h"

#include "songutilities.h"
#include "stm32_audio_decoders.h"

#include "spiritEQ.h"
#include "spiritLdCtrl.h"

/** @addtogroup STM32_Audio_Utilities
  * @{
  */

/** @defgroup STM32_AUDIO_PLAYER
  * @brief This file is the header file for the stm32 audio player module
  * @{
  */


/** @defgroup STM32_AUDIO_PLAYER_Exported_Defines
  * @{
  */

/* Select carefully the player task stack size because it is used by the audio codec functions */
#define PLAYER_TASK_STACK_SZE                        ( 512 )
#define PLAYER_TASK_PRIO                             ( tskIDLE_PRIORITY + 2 ) /* Player Task priority */


/* Uncomment this define to enable using simple buffering instead of multibuffering */
/* #define USE_SIMPLE_BUFFER */




/* Buffering state definitions */
#define STATE_IDLE                                    0
#define STATE_BUFFERING                               1
#define STATE_READY                                   2
#define STATE_CLOSING                                 3
#define STATE_RUN                                     4
#define STATE_RUN_OVR                                 (STATE_RUN + 1)
#define STATE_RUN_UDR                                 (STATE_RUN + 2)
#define STATE_RUN_ERR                                 (STATE_RUN + 3)
#define STATE_ERROR                                   8


#define LED_DECODE                                    LED1
#define LED_I2S_XFERCPLT                              LED2
#define LED_SDCARD                                    LED3


#define AUDIO_FWD_RWD_STEP                             5
/**
  * @}
  */

/** @defgroup STM32_AUDIO_PLAYER_Exported_TypesDefinitions
  * @{
  */
typedef enum
{
  PLAYER_IDLE = 0,
  PLAYER_STOPPED ,
  PLAYER_PLAYING,
  PLAYER_PAUSED,
  PLAYER_ERROR,
}AudioPlayerState_TypeDef;


/**
  * @}
  */


/** @defgroup STM32_AUDIO_PLAYER_Exported_Macros
  * @{
  */ 
#define EQUI_DB_TO_PERC(x)       ((20 * x )/2  + 50)
#define PERC_TO_EQUI_DB(x)       (((int16_t)(x - 50) * 2) / 20)
#define PERC_TO_LDNS_DB(x)       (1 << (SPIRIT_LDCTRL_GAIN_Q_BITS + (int16_t)(50 - x) *  12 / 100 ))

/* Equalizer Band Configuration */
#define SET_BAND_PRMS(band, _fltType, _centerHz, _widthHz, _gainDb) \
  (band)->fltType = _fltType; \
  (band)->centerHz = _centerHz; \
  (band)->widthHz = _widthHz; \
  (band)->gainDb = _gainDb;
#define BlockedTaskTrigger()     (EXTI->SWIER = (uint32_t)((uint32_t)EXTI->SWIER | (uint32_t)EXTI_Line0))
/**
  * @}
  */ 

/** @defgroup STM32_AUDIO_PLAYER_Exported_Variables
  * @{
  */ 
extern TSpiritEQ_Band         EQ_Bands[];
extern TSpiritLdCtrl_Prms     LdCtrlInstanceParams;

/**
  * @}
  */ 

/** @defgroup STM32_AUDIO_PLAYER_Exported_FunctionsPrototype
  * @{
  */
uint32_t AudioPlayer_Init(void);
uint32_t AudioPlayer_DeInit(void);
uint32_t AudioPlayer_Open(uint8_t  *FilePath, uint16_t volume);
uint32_t AudioPlayer_Close(void);
uint32_t AudioPlayer_Play(uint8_t *FilePath, uint16_t volume);
uint32_t AudioPlayer_Pause(void);
uint32_t AudioPlayer_Stop(void);
uint32_t AudioPlayer_Forward(void);
uint32_t AudioPlayer_Rewind(void);
uint32_t AudioPlayer_VolumeCtrl(uint8_t Volume);

TAGS_TypeDef* AudioPlayer_GetFileInfo(void);
uint32_t AudioPlayer_GetTimeInfo(uint32_t* Length, uint32_t* Elapsed);
uint32_t AudioPlayer_SetPosition(uint32_t pos);
uint32_t AudioPlayer_GetFileLength(void);
AudioPlayerState_TypeDef AudioPlayer_GetState(void);
void AudioPlayer_SetState(AudioPlayerState_TypeDef state);

uint32_t Dec_ReadDataCallback(
                      void * pCompressedData,        /* [OUT] Bitbuffer */
                      uint32_t nDataSizeInChars, /* sizeof(Bitbuffer) */
                      void * pUserData               /* Application-supplied parameter */
                      );

void AudioPlayer_Task(void * pvParameters);


void App_Audio_SetEq(uint8_t BandNum, int16_t NewGainValue);
void App_Audio_SetLoudnessGain(int16_t NewGainValue);

/**
  * @}
  */


#ifdef __cplusplus
}
#endif

#endif /* __APP_AUDIO_H */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
