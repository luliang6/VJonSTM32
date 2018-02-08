/**
  ******************************************************************************
  * @file    TscHal.c
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

/* Includes ------------------------------------------------------------------*/
#include "TscHal.h"


/** @addtogroup Embedded_GUI_Library
  * @{
  */

/** @defgroup TscHal 
  * @brief TscHal main functions
  * @{
  */ 

/** @defgroup TscHal_Private_TypesDefinitions
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

/** @defgroup TscHal_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup TscHal_Private_Variables
  * @{
  */
__IO uint32_t u32_TSXCoordinate = 0x0000;
__IO uint32_t u32_TSYCoordinate = 0x0000;



/**
  * @brief  Initializes the IO Expander registers.
  * @param  None
  * @retval - 0: if all initializations are OK.
*/
uint32_t GL_TSC_Init(void)
{
  portENTER_CRITICAL();  
  IOE_Config();
  portEXIT_CRITICAL();    
  return 0; /* Configuration is OK */
}

/**
  * @brief  Read the coordinate of the point touched and assign their
  *         value to the variables u32_TSXCoordinate and u32_TSYCoordinate
  * @param  None
  * @retval None
  */
void GL_TSC_GetCoordinate(void)
{


  TS_STATE  *ts;
  uint32_t TSC_Value_X;
  uint32_t TSC_Value_Y;
  
  ts = IOE_TS_GetState();
  
  TSC_Value_X = ts->X;
  TSC_Value_Y = ts->Y;
  touch_done = (ts->TouchDetected == 0x80);
  
  u32_TSXCoordinate = 319 - TSC_Value_X,
  u32_TSYCoordinate = TSC_Value_Y;

}

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
