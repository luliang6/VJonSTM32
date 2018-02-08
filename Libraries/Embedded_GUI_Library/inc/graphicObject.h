/**
  ******************************************************************************
  * @file    graphicObject.h
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    31-July-2012
  * @brief   This file contains the methods to create the objects that
  *          can be printed on the LCD, and the TouchScreen calibration.
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
#ifndef __GRAPHIC_OBJECT_H
#define __GRAPHIC_OBJECT_H

#ifdef __cplusplus
 extern "C" {
#endif
   
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graphicObjectTypes.h"

/** @addtogroup Embedded_GUI_Library
  * @{
  */

/** @addtogroup graphicObject 
  * @{
  */ 

/** @defgroup graphicObject_Exported_Types
  * @{
  */

/**
  * @}
  */

/** @defgroup graphicObject_Exported_Constants
  * @{
  */
#define GL_NULL  0x00
/**
  * @}
  */

/** @defgroup graphicObject_Exported_variables
  * @{
  */
extern GL_Page_TypeDef* PagesList[];
extern __IO uint8_t touch_done;
extern __IO uint8_t joy_done;
extern __IO uint8_t calibration_done;
extern __IO uint8_t vu8_gPowerSaveOption;

extern uint16_t LCD_Height;
extern uint16_t LCD_Width;
/**
  * @}
  */

/** @defgroup graphicObject_Exported_Macros
  * @{
  */
#define RADIO_BUTTON_ADD(RadioButtonGrpName, Label, Function) \
 RadioButtonGrpName->AddRadioOption( RadioButtonGrpName, Label, Function );


#define min(x,y)                    ((x<y)? x:y)
#define p_strncpy(oBuf, iBuf, Len)  strncpy((char*)oBuf, (char*)iBuf, (int)Len)
#define p_strlen(iBuf)              strlen((char*)iBuf)
#define p_strcmp(str1,str2)         strcmp((char*)str1,(char*)str2)
#define p_strcpy(str1,str2)         strcpy((char*)str1,(char*)str2)
/**
  * @}
  */

/** @defgroup graphicObject_Exported_Functions
  * @{
  */
/*----- High layer function -----*/
void GL_Cross(uint16_t Ypos, uint16_t Xpos);
void GL_DrawButtonBMP(uint16_t maxX, uint16_t minX, uint16_t maxY, uint16_t minY, uint8_t* ptrBitmap);
GL_PageControls_TypeDef* NewLabel (uint16_t ID, uint8_t* label, GL_Direction direction, __IO uint8_t FontSize, __IO uint16_t Colour, GL_bool Transaprency);
GL_PageControls_TypeDef* NewButton (uint16_t ID, const uint8_t* label, void (*pEventHandler)(void));
GL_PageControls_TypeDef* NewCheckbox (uint16_t ID, const uint8_t* label, GL_bool ReadOnly, void (*pEventHandler)(void));
GL_PageControls_TypeDef* NewSwitch (uint16_t ID, const uint8_t* label_1, const uint8_t* label_2, void (*pEventHandler)(void));
GL_PageControls_TypeDef* NewIcon (uint16_t ID, const uint8_t* Image_PTR, uint16_t Width, uint8_t Height, void (*pEventHandler)(void));
GL_RadioButtonGrp_TypeDef* NewRadioButtonGrp (uint16_t ID);
GL_PageControls_TypeDef* AddRadioOption (GL_RadioButtonGrp_TypeDef* pThis, const uint8_t* label, void (*pEventHandler)(void));
GL_PageControls_TypeDef* NewComboBoxGrp (uint16_t ID);
GL_ErrStatus AddComboOption (GL_ComboBoxGrp_TypeDef* pThis, const uint8_t* label, void (*pEventHandler)(void));
GL_PageControls_TypeDef* NewSlidebar (uint16_t ID, const uint8_t* label, GL_Direction direction, GL_bool ReadOnly, void (*pEventHandler)(void));
GL_PageControls_TypeDef* NewHistogram (uint16_t ID, const uint8_t* labelX, const uint8_t* labelY, int16_t data_points[], uint8_t n_points);
GL_PageControls_TypeDef* NewGraphChart (uint16_t ID, const uint8_t* labelX, const uint8_t* labelY, int16_t data_points[], uint8_t n_points, GL_bool Background );
GL_ErrStatus Create_PageObj (GL_Page_TypeDef* pThis, uint16_t ID);
GL_ErrStatus AddPageControlObj (uint16_t PosX, uint16_t PosY, GL_PageControls_TypeDef* objPTR, GL_Page_TypeDef* pagePTR);
GL_ErrStatus DestroyPageControl ( GL_Page_TypeDef* pPage, uint16_t ID );
GL_ErrStatus DestroyPage (GL_Page_TypeDef *pThis);
GL_ErrStatus Set_Label ( GL_Page_TypeDef* pPage, uint16_t ID, const uint8_t* label );
GL_ErrStatus Get_Label ( GL_Page_TypeDef* pPage, uint16_t ID, uint8_t* label );
GL_ErrStatus Set_LabelColor( GL_Page_TypeDef* pPage, uint16_t ID, uint16_t Color );
uint8_t Get_SlidebarValue ( GL_Page_TypeDef* pPage, uint16_t ID );
uint8_t Set_SlidebarValue ( GL_Page_TypeDef* pPage, uint16_t ID, uint8_t position );
GL_ErrStatus SetHistogramPoints( GL_Page_TypeDef* pPage, uint16_t ID, int16_t data_points[], uint8_t n_points );
GL_ErrStatus SetGraphChartPoints( GL_Page_TypeDef* pPage, uint16_t ID, int16_t data_points[], uint8_t n_points );
GL_bool GetObjStatus(GL_Page_TypeDef* pThis, uint16_t ID);
uint8_t GetComboOptionActive(GL_Page_TypeDef* pPage, uint16_t ID);
GL_ErrStatus ResetComboOptionActive(GL_Page_TypeDef* pPage, uint16_t ID);
const uint8_t* GetComboOptionLabel(GL_Page_TypeDef* pPage, uint16_t ID);
GL_ErrStatus SetComboOptionLabel(GL_Page_TypeDef* pPage, uint16_t ID, const uint8_t* label);
GL_ErrStatus SetIconImage(GL_Page_TypeDef* pPage, uint16_t ID, const uint8_t* pImage, uint16_t Width, uint8_t Height);
GL_ErrStatus ShowPage(GL_Page_TypeDef* pThis, GL_bool bVal);
void RefreshPage(GL_Page_TypeDef* pThis);
GL_ErrStatus RefreshPageControl( GL_Page_TypeDef* pPage, uint16_t ID);
void ChangePage(GL_Page_TypeDef* pPageOld, GL_Page_TypeDef* pPageNew);
void NullFunc(void);
void GL_Background_Handler(void);
void Set_LCD_Resolution( uint16_t Lcd_Width, uint16_t Lcd_Height );
uint8_t CompareCoordinates(uint16_t u16_XMax, uint16_t u16_XMin, uint16_t u16_YMax, uint16_t u16_YMin);
uint8_t CompareJoyCoordinates(uint16_t u16_XMax, uint16_t u16_XMin, uint16_t u16_YMax, uint16_t u16_YMin);
void GL_LowPower(void);
void GL_Delay(uint32_t nCount);
uint16_t GL_GetLastEvent (void);
uint8_t GetRadioOptionChecked(GL_Page_TypeDef* pPage, uint16_t ID);

#ifdef __cplusplus
}
#endif


#endif /*__GRAPHIC_OBJECT_H */
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
