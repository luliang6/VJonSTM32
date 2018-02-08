/**
  ******************************************************************************
  * @file    gl_mgr.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Header for gl_mgr.c module
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
#ifndef __GL_MGR_H
#define __GL_MGR_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "global_includes.h "
#include "lcdHal.h"
#include "gl_ext.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define PAGE_MENU        0xFF

#define BF_TYPE          0x4D42  /* "MB" */
#define BF_XSIZE         50
#define BF_YSIZE         50

#define BI_RGB           0 /* No compression - straight RGB data */
#define BI_RLE8          1 /* 8-bit run-length compression */
#define BI_RLE4          2 /* 4-bit run-length compression */
#define BI_BITFIELDS     3 /* RGB bitmap with RGB masks */

#define CONNECTIVITY_GROUP    0
#define MULTIMEDIA_GROUP      1
#define UTILITIES_GROUP       2

#define MESSAGE_UNDER_CONSTRUCTION 0
#define MESSAGE_ALARM_RING         1
#define MESSAGE_ALARM_RING2        2
#define MESSAGE_3                  3
#define MESSAGE_4                  4

/* Exported variables --------------------------------------------------------*/
extern GL_Page_TypeDef *GL_HomePage;
extern xTaskHandle      Core_Time_Task_Handle;

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void GL_Startup (void);
uint32_t GL_Init( void );
void  GL_ShowMainMenu(void);
uint32_t GL_Handle_Inputs (void);
void GL_State_Message (uint8_t *msg);
void GL_HandleSystemSettingsChange (void);
void GL_UpdateBackgroundProcessStatus (void);
void MESSAGES_DirectEx(GL_Page_TypeDef* pParent, uint8_t msg_id);
void MESSAGES_CloseEx(void);

#ifdef __cplusplus
}
#endif

#endif /* __GL_MGR_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
