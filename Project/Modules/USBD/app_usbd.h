/**
  ******************************************************************************
  * @file    app_usbd.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   header file for the app_usbd.c file
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
#ifndef __APP_USBD_H
#define __APP_USBD_H

/* Includes ------------------------------------------------------------------*/
#include "usbd_core.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define CURSOR_STEP      20

/* Exported variables --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
extern int8_t HID_Buffer [4];

/* Exported functions ------------------------------------------------------- */
void  USBD_MSC_AppInit (void);
void  USBD_HID_AppInit (void);
void  USBD_StopDeviceApp (void);
void USBD_HID_AppSendReport(void);

#endif /*__APP_USBD_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
