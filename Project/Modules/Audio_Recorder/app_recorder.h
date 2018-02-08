/**
  ******************************************************************************
  * @file    app_recorder.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Header for app_recorder.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_RECORDER_H
#define __APP_RECORDER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <string.h>
   
#include "stm32_audio.h"
#include "stm32_audio_in_if.h"
   
#include "songutilities.h"
#include "stm32_audio_decoders.h"

/** @addtogroup STM32_Audio_Utilities
  * @{
  */
  
/** @defgroup STM32_AUDIO_RECORDER
  * @brief This file is the header file for the stm32 audio player module 
  * @{
  */ 


/** @defgroup STM32_AUDIO_RECORDER_Exported_Defines
  * @{
  */ 
   
#define AudioRecorder_Task_PRIO                      ( tskIDLE_PRIORITY + 11 )
#define RECORDER_TASK_STACK_SZE                      ( 512 ) 

/**
  * @}
  */ 

/** @defgroup STM32_RECORDER_Exported_TypesDefinitions
  * @{
  */ 
typedef enum
{
  REC_STATE_IDLE = 0,
  REC_STATE_READY,
  REC_STATE_RECORDING,
  REC_STATE_PAUSED,
  REC_STATE_ERROR
}RecoderState_TypeDef;

/**
  * @}
  */ 


/** @defgroup STM32_RECORDER_Exported_Macros
  * @{
  */ 
  
/**
  * @}
  */ 

/** @defgroup STM32_RECORDER_Exported_Variables
  * @{
  */ 

/**
  * @}
  */ 

/** @defgroup STM32_RECORDER_Exported_FunctionsPrototype
  * @{
  */ 
uint32_t AudioRecorder_Init(uint32_t SamplingFreq);
uint32_t AudioRecorder_DeInit(void);
uint32_t AudioRecorder_Open(uint8_t  *FilePath);
uint32_t AudioRecorder_Close(void);
uint32_t AudioRecorder_Start(void);
uint32_t AudioRecorder_Stop(void);
uint32_t AudioRecorder_PauseResume(uint32_t Cmd);
void AudioRecorder_Task(void * pvParameters);

uint32_t AudioRecorder_SetPosition(uint32_t Pos);
uint32_t AudioRecorder_GetElapsedTime(void);

/**
  * @}
  */ 


#ifdef __cplusplus
}
#endif

#endif /* __APP_RECORDER_H */
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
