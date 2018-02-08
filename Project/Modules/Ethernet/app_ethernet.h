/**
  ******************************************************************************
  * @file    app_ethernet.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012 
  * @brief   This file contains all the functions prototypes for the app_ethernet.c 
  *          file.
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
#ifndef __APP_ETHERNET_H
#define __APP_ETHERNET_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mod_ethernet_config.h"

/* Exported types ------------------------------------------------------------*/
typedef enum 
{ 
  DHCP_START = 0,
  DHCP_WAIT_ADDRESS,
  DHCP_ADDRESS_ASSIGNED,
  DHCP_TIMEOUT
} 
DHCP_State_TypeDef;

typedef enum {
  BUFFER_EMPTY    = 1,
  BUFFER_FILLED   = 2,
  BUFFER_ERROR    = 3
}IPCAM_ImageBufferStatus;

/* Image Buffer structure */
typedef struct
{
  uint32_t ImageHeaderLen;               /* Image header size */
  uint8_t *ImageHeader;                  /* Image header */
  uint32_t ImageLen;                     /* Image size */
  uint32_t MaxImageLen;                  /* Max image size */
  uint8_t *ImageData;                    /* Image buffer */
  Camera_TypeDef Camera;                 /* Camera devices */
  ImageFormat_TypeDef ImageFormat;       /* Image Format */
  IPCAM_ImageBufferStatus BufferStatus;  /* Image buffer status */
}IPCAM_ImageBuffer_TypeDef;

extern SYSTEM_CONFIG_TypeDef DC_Global_Config;
extern uint32_t IPaddress;;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void http_init(void);
void LwIP_Init(void);
void LwIP_DHCP_task(void * pvParameters);
void IPCAM_ImageCaptureInit(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_ETHERNET_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
