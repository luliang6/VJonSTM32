/**
  ******************************************************************************
  * @file    JoyHal.c
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    31-July-2012
  * @brief   This file contains all the Joystick functions whose
  *          implementation depends on the Controller used in your Design.
  *          You only need to change these functions implementations
  *          in order to reuse this code with other Controller 
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

/* Includes ---------------------------------------------------------------------------*/
#include "JoyHal.h"
#include "graphicObjectTypes.h"

#ifdef USE_STM3210C_EVAL
   #include "stm3210c_eval_ioe.h"
#elif USE_STM322xG_EVAL
   #include "stm322xg_eval_ioe.h"
#endif

/** @addtogroup Embedded_GUI_Library
  * @{
  */

/** @defgroup JoyHal 
  * @brief JoyHal main functions
  * @{
  */

/** @defgroup JoyHal_Private_Defines
  * @{
  */

/**
  * @}
  */

/** @defgroup JoyHal_Private_Variables
  * @{
  */
/**
  * @}
  */ 

/** @defgroup JoyHal_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup JoyHal_Private_FunctionPrototypes
  * @{
  */

/**
  * @}
  */

/** @defgroup JoyHal_Private_Functions
  * @{
  */

/**
  * @brief  Initializes the IO Expander registers.
  * @param  None
  * @retval - 0: if all initializations are OK.
  */
uint32_t GL_JOY_Init(void)
{

  return 0; /* Configuration is OK */
}


/**
  * @brief  Return the Joystick status.
  * @param  None
  * @retval None
  */
uint32_t GL_JoyStickState(void)
{
  uint8_t ret;
  ret = IOE_JoyStickGetState();
  return ret;  
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
