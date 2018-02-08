/**
  ******************************************************************************
  * @file    images.h
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    31-July-2012
  * @brief   Hex dumps of the 16-color 565 RGB images used by Graphics Library.
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
#ifndef __IMAGES_H
#define __IMAGES_H

#ifdef __cplusplus
 extern "C" {
#endif
   
/* Includes ------------------------------------------------------------------*/
#include "global_includes.h"

   
/** @addtogroup Embedded_GUI_Library
  * @{
  */

/** @addtogroup Images
  * @{
  */ 

/** @defgroup Images_Exported_Constants
  * @{
  */
#define Red_BG    0xFD4F
#define Green_BG  0xBF22
#define Blue_BG   0x65BF
#define Blue_BG2  0x95FF

extern const uint8_t RButtonA[866];
extern const uint8_t RButtonB[866];
extern const uint8_t CheckBox1[866];
extern const uint8_t CheckBox2[866];
extern const uint8_t ArrowUpTouched[1034];
extern const uint8_t ArrowUpUntouched[1034];
extern const uint8_t ArrowDownTouched[1034];
extern const uint8_t ArrowDownUntouched[1034];
extern const uint8_t BtnNormalLeft[482];
extern const uint8_t BtnNormalRight[482];
extern const uint8_t BtnNormal[482];
extern const uint8_t BtnPressedLeft[482];
extern const uint8_t BtnPressedRight[482];
extern const uint8_t BtnPressed[482];
extern const uint8_t SwitchNormalLeft[482];
extern const uint8_t SwitchNormalRight[482];
extern const uint8_t SwitchNormal[482];
extern const uint8_t SwitchPressedLeft[482];
extern const uint8_t SwitchPressedRight[482];
extern const uint8_t SwitchPressed[482];
extern const uint8_t SlidebarLeft[210];
extern const uint8_t SlidebarRight[210];
extern const uint8_t SlidebarCentral[210];
extern const uint8_t SlidebarCursor[306];

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /*__IMAGES_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
