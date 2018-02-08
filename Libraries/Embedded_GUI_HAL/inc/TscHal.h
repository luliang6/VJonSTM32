/**
  ******************************************************************************
  * @file    TscHal.h
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    31-July-2012
  * @brief   This file contains all the TouchScreen functions whose
  *          implementation depends on the TSC Controller used in your Design.
  *          You only need to change these functions implementations
  *          in order to reuse this code with other TSC Controller 
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
#ifndef __TSC_HAL_H
#define __TSC_HAL_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes --------------------------------------------------------------------*/
#include "global_includes.h"
#include "graphicObject.h"

/** @addtogroup Embedded_GUI_Library
  * @{
  */

/** @addtogroup TscHal
  * @{
  */ 

/** @defgroup TscHal_Exported_Types
  * @{
  */

/**
  * @}
  */

/** @defgroup TscHal_Private_Defines
  * @{
  */
   
/**
  * @}
  */

/** @defgroup tsc_type_Exported_Types
  * @{
  */
 /**
  * @}
  */
   
/** @defgroup TscHal_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup TscHal_Exported_Functions
  * @{
  */
uint32_t GL_TSC_Init(void);
void GL_TSC_GetCoordinate(void);

#ifdef __cplusplus
}
#endif

#endif /*__TSC_HAL_H */

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
