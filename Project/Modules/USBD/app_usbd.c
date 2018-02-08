/**
  ******************************************************************************
  * @file    app_usbd.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   This file contains the USB Device applications startup process
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

/* Includes ------------------------------------------------------------------*/
#include "app_usbd.h"
#include "usbd_msc_core.h"
#include "usbd_hid_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define USBD_MSC_APP                     0
#define USBD_HID_APP                     1

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma data_alignment=4
#endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

__ALIGN_BEGIN USB_OTG_CORE_HANDLE     USB_OTG_HS_dev __ALIGN_END ;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma data_alignment=4
#endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

__ALIGN_BEGIN  int8_t HID_Buffer [4] __ALIGN_END ;


uint8_t USBD_APP_Id ;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialize the mass storage application
  * @param  None
  * @retval None
  */
void  USBD_MSC_AppInit (void)
{

  USBD_APP_Id = USBD_MSC_APP;
  USBD_Init(&USB_OTG_HS_dev,
            USB_OTG_HS_CORE_ID,
            &USR_desc,
            &USBD_MSC_cb,
            &USR_cb);
}

/**
  * @brief  Initialize the HID application
  * @param  None
  * @retval None
  */
void  USBD_HID_AppInit (void)
{

  USBD_APP_Id = USBD_HID_APP;
  USBD_Init(&USB_OTG_HS_dev,
            USB_OTG_HS_CORE_ID,
            &USR_desc,
            &USBD_HID_cb,
            &USR_cb);
}

/**
  * @brief  Disconnect and stop the USB device core
  * @param  None
  * @retval None
  */
void  USBD_StopDeviceApp (void)
{
  DCD_DevDisconnect (&USB_OTG_HS_dev);
  USB_OTG_StopDevice (&USB_OTG_HS_dev);
}

/**
  * @brief  Send the HID reports
  * @param  None
  * @retval None
  */
void USBD_HID_AppSendReport(void)
{

  if ((HID_Buffer[1] != 0) || (HID_Buffer[2] != 0))
  {
    USBD_HID_SendReport (&USB_OTG_HS_dev,
                         (uint8_t *)HID_Buffer,
                         4);
  }
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
