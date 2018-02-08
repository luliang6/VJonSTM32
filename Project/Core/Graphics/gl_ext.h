/**
  ******************************************************************************
  * @file    gl_ext.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Header for gl_ext.c module
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
#ifndef __GL_EXT_H
#define __GL_EXT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "lcdHal.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define MAX_CAPTION_SIZE 40

#define BF_TYPE            0x4D42  /* "MB" */
#define BF_XSIZE           50
#define BF_YSIZE           50

#define BI_RGB       0             /* No compression - straight BGR data */
#define BI_RLE8      1             /* 8-bit run-length compression */
#define BI_RLE4      2             /* 4-bit run-length compression */
#define BI_BITFIELDS 3             /* RGB bitmap with RGB masks */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */   
uint32_t GL_Handle_Inputs (void);
GL_PageControls_TypeDef *AddIconControlObj (GL_Page_TypeDef *pPage,
                                            uint8_t id, 
                                            uint8_t index,
                                            uint8_t *icon, 
                                            uint8_t *caption,
                                            uint8_t mode,
                                            void (*pEventHandler)(void));

void GL_SetMenuItem( GL_Page_TypeDef* page , 
                 uint8_t *caption,
                 uint8_t index,
                 void (*pEventHandler)(void));

void GL_RefreshControlNow (GL_PageControls_TypeDef* item);
GL_bool RefreshLabel (GL_Page_TypeDef* pPage , uint16_t id , uint8_t *str);
GL_bool ChangeLabelColor (GL_Page_TypeDef* pPage , uint16_t id , uint16_t color);

void GL_CenterCaption(uint8_t * dBuf, 
                          uint8_t * sBuf, 
                          uint32_t MaxLength);

void GL_SetPageHeader( GL_Page_TypeDef* page , 
                 uint8_t *caption);

void GL_SetChecked (GL_Page_TypeDef* pPage , 
                    uint16_t id , 
                    uint8_t state);

GL_bool GL_IsChecked (GL_Page_TypeDef* pPage , 
                      uint16_t id);
void GL_AddScroll (GL_Page_TypeDef *pPage,
                   uint16_t x,
                   uint16_t y, 
                   uint16_t yl,  
                   void (*pUpEventHandler)(void),
                   void (*pDwnEventHandler)(void),
                   uint8_t pos);

void GL_UpdateScrollPosition (GL_Page_TypeDef *pPage,
                   uint16_t x,
                   uint16_t y, 
                   uint16_t yl,
                   uint8_t position);

GL_PageControls_TypeDef *AddSmallIconControlObj (GL_Page_TypeDef *pPage,
                                            uint8_t id, 
                                            uint8_t *icon, 
                                            uint8_t *caption,
                                            uint8_t mode,
                                            void (*pSelectHandler)(void),
                                            void (*pEventHandler)(void));

uint16_t GL_AdjustColor (uint16_t color);

void GL_AddCalendar (GL_Page_TypeDef* pPage, 
                     uint16_t id, 
                     uint8_t day,
                     uint8_t month,
                     uint16_t year);

void GL_UpdateCalendar (GL_Page_TypeDef* pPage, 
                     uint16_t id, 
                     uint8_t day,
                     uint8_t month,
                     uint16_t year);

#ifdef __cplusplus
}
#endif

#endif /* __GL_EXT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
