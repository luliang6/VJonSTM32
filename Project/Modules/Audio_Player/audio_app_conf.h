/**
  ******************************************************************************
  * @file    audio_app_conf.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Audio modules configuration file
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
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_APP_CONF__H__
#define __AUDIO_APP_CONF__H__

/* Includes ------------------------------------------------------------------*/
#ifdef STM32F2XX
 #include "stm32f2xx.h"
#elif defined(STM32F4XX)
 #include "stm32f4xx.h"
#elif defined(STM32F10X_CL)
 #include "stm32f10x.h"
#endif /* STM32F2XX */

/** @defgroup AUDIO_APP_CONF_Exported_Defines
  * @{
  */

/* Uncomment these defines to disable the Equalizer and the loudness modules */
/* #define __DISABLE_EQUALIZER_CTRL */
/* #define __DISABLE_LOUDNESS_CTRL */

/* Uncomment these defines to set the protection of critical sections */
/* #define   CRITICAL_SECTION_ON()     vPortEnterCritical(); */
/* #define   CRITICAL_SECTION_OFF()    vPortExitCritical();  */

/* settings Memory */
#define AUDIO_SETTINGS_MEM         6
#define AUDIO_EQU1_MEM             8
#define AUDIO_EQU2_MEM             9
#define AUDIO_EQU3_MEM             10
#define AUDIO_EQU4_MEM             11
#define AUDIO_LDNS_MEM             12


/* Select which format should be supported */
#define AUDIO_PLAYER_FILE_MASK                    (MASK_WAV)

/* Number of sub-packets in the audio transfer buffer. You can modify this value but always make sure
  that it is an even number and higher than 3 */
#define MAX_OUT_PACKET_SZE                        (uint32_t)(2048*2*2) /* Maximum usage for AUDIO decoder */

#define MAX_NUM_BUFF                              (uint32_t)(3)
/* Number of sub-packets in the audio transfer buffer. You can modify this value but always make sure
  that it is an even number and higher than 3 */
#define MAX_IN_PACKET_SZE                        ( 512 )

#define WAVIN_PACKET_SZE                          (uint32_t)(512*2*2) /* 512 samples stereo 16-bit */

#define MAX_AUDIO_HEADER_SIZE                     200


/* Default audio frequency in Hz. If SUPPORTED_FREQ_NBR > 1, make sure that the default frequency is
   always the higher one in all supported frequencies (because the allocation of data buffer is done
   based on this frequency). */
#define DEFAULT_OUT_AUDIO_FREQ                    48000


#define DEFAULT_VOLUME                            70    /* Default volume in % (Mute=0%, Max = 100%) in Logarithmic values
                                                  To get accurate volume variations, it is possible to use a logarithmic
                                                  conversion table to convert from percentage to logarithmic law.
                                                  In order to keep this example code simple, this conversion is not used.*/

/* Default audio frequency in Hz. If SUPPORTED_FREQ_NBR > 1, make sure that the default frequency is
   always the higher one in all supported frequencies (because the allocation of data buffer is done
   based on this frequency). */
#define DEFAULT_IN_AUDIO_FREQ                    8000

/*
  Interrupt's priorities configuration:
  All application interrupt's priorities configuration are gathered in this section
  to make it easier to control and update
   - Interrupt priority grouping is set to 2: 2 bits for Preemption priority and 2 bits for Sub-Priority
   - Select the preemption priority level(0 is the highest, 3 is the lowest)
   - Select the sub-priority level (0 is the highest, 3 is the lowest)
  Make sure you configure correctly the interrupt priorities
  because bad configuration may easilly lead to application not
  working or not working correctly
*/
/*-----------------------------------------------------------------------------
   Defines from stm322xg_audio_codec.h, stm32f4_discovery_audio_codec.h and
   stm3210c_audio_codec.h files 
------------------------------------------------------------------------------*/
#define EVAL_AUDIO_IRQ_PREPRIO                  12
#define EVAL_AUDIO_IRQ_SUBRIO                    0

/*-----------------------------------------------------------------------------
   Defines from stm32_audio_recorder.h file 
------------------------------------------------------------------------------*/
#define AUDIO_REC_TIM_PRE_PRIO                   13
#define AUDIO_REC_TIM_SUB_PRIO                   0

/*-----------------------------------------------------------------------------
   Defines from usbd_audio_correction.h file 
------------------------------------------------------------------------------*/
#define AUDIO_CORR_TIM_PRE_PRIO                  13
#define AUDIO_CORR_TIM_SUB_PRIO                  1

/*-----------------------------------------------------------------------------
   General configuration defines 
------------------------------------------------------------------------------*/
/* Uncomment this define to enable the ".wav" file format support on Player */
#define __WAV_DECODER__

/* Uncomment this define to enable the ".wav" file format support on Recorder */
#define __WAV_ENCODER__

/*-----------------------------------------------------------------------------
   Defines from stm322xg_audio_codec.h, stm32f4_discovery_audio_codec.h and
   stm3210c_audio_codec.h files 
------------------------------------------------------------------------------*/
/* Uncomment this define to use the dynamic I2S intialization
   This define must be enaled if the number of supported frequencies in usbd_conf.h is higher than 1
   "#define SUPPORTED_FREQ_NBR" in usbd_conf.h. */
#define USE_I2S_DYNAMIC_INIT

/* Audio Transfer mode (DMA, Interrupt or Polling) */
#define AUDIO_MAL_MODE_NORMAL    /* Uncomment this line to enable the audio
                                         Transfer using DMA */
/* #define AUDIO_MAL_MODE_CIRCULAR */ /* Uncomment this line to enable the audio
                                         Transfer using DMA */

/* For the DMA modes select the interrupt that will be used */
#define AUDIO_MAL_DMA_IT_TC_EN   /* Uncomment this line to enable DMA Transfer Complete interrupt */
/* #define AUDIO_MAL_DMA_IT_HT_EN */  /* Uncomment this line to enable DMA Half Transfer Complete interrupt */
/* #define AUDIO_MAL_DMA_IT_TE_EN */  /* Uncomment this line to enable DMA Transfer Error interrupt */

/* #define AUDIO_USE_MACROS */

extern uint8_t  stm32_audio_BuffXferCplt (uint8_t Direction, uint8_t** pbuf, uint32_t* pSize);

/**
  * @}
  */

/** @defgroup AUDIO_APP_CONF_Exported_Types
  * @{
  */
#define EXTERNATED_PLAYER_SETTINGS
typedef struct _PLAYER_SettingsTypeDef
{
  __IO uint32_t RepeatCtrl;
  __IO uint32_t BackgroundEnabled;
  __IO uint32_t GuiEnabled;
  __IO uint32_t CurrVolume;
  __IO uint32_t MuteState;
   uint32_t CurrFileListSze;
  __IO uint32_t FileMask;
  __IO uint32_t FileIdx;
  __IO uint32_t isEqDisabled;
  __IO uint32_t isLdnDisabled;
}PLAYER_SettingsTypeDef;
/**
  * @}
  */


/** @defgroup AUDIO_APP_CONF_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup AUDIO_APP_CONF_Exported_Variables
  * @{
  */
/**
  * @}
  */

/** @defgroup AUDIO_APP_CONF_Exported_FunctionsPrototype
  * @{
  */
/**
  * @}
  */


#endif /* __AUDIO_APP_CONF__H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
