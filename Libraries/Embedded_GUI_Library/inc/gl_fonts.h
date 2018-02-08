/**
  ******************************************************************************
  * @file    gl_fonts.h
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    31-July-2012
  * @brief   Header for gl_fonts.c
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
#ifndef __GL_FONTS_H
#define __GL_FONTS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/** @addtogroup Embedded_GUI_Library
  * @{
  */
   
/** @addtogroup GL_FONTS
  * @{
  */ 

/** @defgroup GL_FONTS_Exported_Types
  * @{
  */ 
typedef struct tFont
{    
  const uint16_t *table;
  uint16_t Width;
  uint16_t Height;
}gl_sFONT;

extern gl_sFONT GL_Font16x24;
extern gl_sFONT GL_Font12x12;
extern gl_sFONT GL_Font8x12;
extern gl_sFONT GL_Font8x12_bold;
extern gl_sFONT GL_Font8x8;

/**
  * @}
  */

/** @defgroup GL_FONTS_Exported_Constants
  * @{
  */
/**
  * @}
  */

/** @defgroup GL_FONTS_Exported_Macros
  * @{
  */
#define LINE(x) ((x) * (((sFONT *)LCD_GetFont())->Height))

/**
  * @}
  */

/** @defgroup GL_FONTS_Exported_Functions
  * @{
  */
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __GL_FONTS_H */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
