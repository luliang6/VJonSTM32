/**
  ******************************************************************************
  * @file    app_console.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   This file contains the console cache filling process
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
#include "mod_console.h"
#include "app_console.h"
#include "mem_utils.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern uint8_t scroll_active;
CONSOLE_MsgTypeDef Console_Msg;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Inititialize the console queue
  * @param  None
  * @retval None
  */
void CONSOLE_Init (void)
{
  Console_Msg.ptr  = 0;
  Console_Msg.full = 0;
}

/**
  * @brief  Add a message into the message queue
  * @param  None
  * @retval None
  */
void CONSOLE_AddMessage (uint8_t * msg)
{
  if (scroll_active == 1)
  {
    CONSOLE_ScrollReset ();
  }

  if (Console_Msg.ptr == CONSOLE_DEPDTH)
  {
    Console_Msg.ptr = 0;
    Console_Msg.full = 1;
  }

  strncpy((char *)Console_Msg.msg[Console_Msg.ptr], (char *)msg, CONSOLE_LINESIZE);
  Console_Msg.ptr++;
  CONSOLE_Refresh();
  CONSOLE_UpdateCursor();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
