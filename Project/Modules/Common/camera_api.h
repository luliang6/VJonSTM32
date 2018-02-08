/**
  ******************************************************************************
  * @file    camera_api.h 
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Header for camera_api.c module
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
#ifndef __CAMERA_API_H
#define __CAMERA_API_H

/* Includes ------------------------------------------------------------------*/
#include "global_includes.h"

/* Exported constants --------------------------------------------------------*/
#define DCMI_DR_ADDRESS       0x50050028
#define FSMC_SRAM_ADDRESS     0x68000000		//0x64000000

/* Exported types ------------------------------------------------------------*/
/* Camera devices enumeration */
typedef enum
{
  OV9655_CAMERA         =   0x01,      /* Use OV9655 Camera */
  OV2640_CAMERA         =   0x02       /* Use OV2640 Camera */
}Camera_TypeDef;

/* Image Sizes enumeration */
typedef enum
{
  BMP_QQVGA             =   0x01,	/* BMP Image QQVGA 160x120 Size */
  BMP_QVGA              =   0x02, /* BMP Image QVGA 320x240 Size */
  JPEG_160x120          =   0x03,	/* JPEG Image 160x120 Size */
  JPEG_176x144          =   0x04,	/* JPEG Image 176x144 Size */
  JPEG_320x240          =   0x05,	/* JPEG Image 320x240 Size */
  JPEG_352x288          =   0x06	/* JPEG Image 352x288 Size */
}ImageFormat_TypeDef;
/* Exported variables ---------------------------------------------------------*/
extern Camera_TypeDef Camera;
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void  Camera_Config(ImageFormat_TypeDef ImageFormat);
void  Camera_SetBrighnessValue(int8_t brightness_value);
#endif /* __CAMERA_API_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
