/**
  ******************************************************************************
  * @file    app_console.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Header for app_console.c module
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
#ifndef __APP_CONSOLE_H
#define __APP_CONSOLE_H

#ifdef __cplusplus
extern "C"
{
#endif

  /* Includes ------------------------------------------------------------------*/
#include "gl_mgr.h"
#include <string.h>

  /* Exported constants --------------------------------------------------------*/

#define CONSOLE_DEPDTH      100
#define CONSOLE_LINESIZE    40

/* Exported types ------------------------------------------------------------*/
typedef struct _CONSOLE_MsgTypeDef
{
  uint8_t     msg[CONSOLE_DEPDTH][CONSOLE_LINESIZE] ;
  uint16_t    ptr;
  uint8_t     full;

}
CONSOLE_MsgTypeDef;

/* Exported variables --------------------------------------------------------*/
extern CONSOLE_MsgTypeDef Console_Msg;

  /* Exported macro ------------------------------------------------------------*/
  /* Exported functions ------------------------------------------------------- */
void CONSOLE_Init (void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_CONSOLE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
