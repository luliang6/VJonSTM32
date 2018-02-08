/**
  ******************************************************************************
  * @file    app_image.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Header for app_image.c module
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
#ifndef __APP_IMAGE_H
#define __APP_IMAGE_H

/* Includes ------------------------------------------------------------------*/
#include "global_includes.h"  
#include  "LcdHal.h"
#include  "image_bmp.h"
#include  "image_jpeg.h"



/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int32_t Image_Decode (FIL *file, char *file_name, uint8_t effect);
void Image_SetDefaultSize(void);
void Image_Enlarge(void);
uint8_t  Image_IsSmall(void);

#endif /* __APP_IMAGE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
