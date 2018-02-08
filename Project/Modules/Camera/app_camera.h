/**
  ******************************************************************************
  * @file    app_camera.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Header for app_camera.c module
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
#ifndef __APP_CAMERA_H
#define __APP_CAMERA_H

#ifdef __cplusplus
extern "C"
{
#endif

  /* Includes ------------------------------------------------------------------*/
#include "dcmi_ov9655.h"
#include "dcmi_ov2640.h"
#include <string.h>

  /* Exported types ------------------------------------------------------------*/
  typedef struct
  {
    uint32_t HeaderLen;              /* Header size */
    uint8_t *ImageHeader;            /* Image header */
    uint32_t ImageLen;               /* Image size */
    uint8_t *RawData;                /* Image buffer*/
    uint8_t *SrcData;              /* Image buffer*/
    uint8_t *DestData;              /* Image buffer*/
    uint8_t ImageName[256];
  }
  ImageBuffer_TypeDef;

  /* Exported constants --------------------------------------------------------*/
#define RGB_HEADER_SIZE      54
#define IMAGE_BUFFER_SIZE   (320*240*2)     /* Size of RGB16 image */
#define MAX_IMAGE_SIZE      (320*240*3)      /* Size of RGB24 image  */
#define IMAGE_COLUMN_SIZE   240
#define IMAGE_LINE_SIZE     320

#define FSMC_SRAM_ADDRESS   0x68000000			//0x64000000
  /* Exported macro ------------------------------------------------------------*/
#define BMP_PIXEL16_TO_R(pixel)       ((pixel & 0x1F) << 3)
#define BMP_PIXEL16_TO_G(pixel)       (((pixel >> 5) & 0x3F) << 2)
#define BMP_PIXEL16_TO_B(pixel)       (((pixel >> 11) & 0x1F) << 3)
  /* Exported functions ------------------------------------------------------- */
uint8_t Save_Image_To_File (uint8_t *path , uint8_t *file_name, uint8_t image_format);
void Apply_camera_native_effects (uint8_t ieffect);
#ifdef __cplusplus
}
#endif

#endif /* __APP_CAMERA_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
