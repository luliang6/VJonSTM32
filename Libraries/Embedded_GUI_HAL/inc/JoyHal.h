/**
  ******************************************************************************
  * @file    JoyHal.h
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    31-July-2012
  * @brief   This file contains all the Joystick functions whose implementation
  *          depends on the Joystick Controller used in your Design.
  *          You only need to change these functions implementations in order to
  *          reuse this code with other Joystick Controller 
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

/* Define to prevent recursive inclusion ---------------------------------------*/
#ifndef __JOY_HAL_H
#define __JOY_HAL_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes --------------------------------------------------------------------*/
#include "global_includes.h"  
  

/** @addtogroup Embedded_GUI_Library
  * @{
  */

/** @addtogroup JoyHal
  * @{
  */ 

/** @defgroup JoyHal_Exported_Types
  * @{
  */

/**
  * @}
  */

/** @defgroup JoyHal_Private_Defines
  * @{
  */
   
#define GL_JOY_NONE     0x00
#define GL_JOY_CENTER   0x01
#define GL_JOY_DOWN     5//0x02
#define GL_JOY_LEFT     4//0x03
#define GL_JOY_RIGHT    3//0x04
#define GL_JOY_UP       2//0x05

/**
  * @}
  */
   
/** @defgroup JoyHal_Private_TypesDefinitions
  * @{
  */
/** 
  * @brief  JOY_ReadMode enumeration definition  
  */

/**
  * @}
  */ 

/** @defgroup JoyHal_Exported_variables
  * @{
  */
/**
  * @}
  */
   
/** @defgroup JoyHal_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup JoyHal_Exported_Functions
  * @{
  */
uint32_t GL_JoyStickState(void);
uint32_t GL_JOY_Init(void);

#ifdef __cplusplus
}
#endif

#endif /*__JOY_HAL_H */

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
