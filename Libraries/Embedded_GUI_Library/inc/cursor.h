/**
  ******************************************************************************
  * @file    cursor.h
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    31-July-2012
  * @brief   Application - cursor routines
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
#ifndef __cursor_H
#define __cursor_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes --------------------------------------------------------------------*/
//#include "JoyHal.h"
#include "graphicObject.h"
   
/** @addtogroup Embedded_GUI_Library
  * @{
  */

/** @addtogroup Cursor
  * @{
  */ 
   
/** @defgroup Cursor_Exported_Constants
  * @{
  */
#define CUR_DOWN 	      0x01
#define CUR_UP		      0x02
#define CUR_RIGHT	      0x03
#define CUR_LEFT	      0x04
#define CUR_PRESS 	      0x10
#define CUR_RELEASE	      0x20

#define CUR_STEP	      0x04

#define	CUR_DRAW_ALL 	      0x07
#define	CUR_DRAW_BEH	      0x01
#define	CUR_READ_BEH	      0x02
#define	CUR_READ_DRAW_CUR     0x06
#define	CUR_DRAW_CUR          0x08

#define	CUR_DRAW_BEH_MASK     0x01
#define	CUR_READ_BEH_MASK     0x02
#define CUR_DRAW_CUR_MASK     0x04

#define Cursor_Width          8
#define Cursor_Height         14

   
/**
  * @}
  */
   
/** @defgroup Cursor_Exported_Types
  * @{
  */
typedef struct{
uint8_t *   CursorPointer;
uint16_t    X;
uint16_t    Y;
uint8_t	    PressState;
uint16_t *  BehindCursor;
uint16_t    Color;
}Cursor_TypeDef;

typedef struct{
uint8_t	Width;
uint8_t	Height;
uint8_t	NumOfPix;
}CursorHeader_Typedef;

/**
  * @}
  */

/** @defgroup Cursor_Exported_variables
  * @{
  */
extern Cursor_TypeDef* Cursor;

/**
  * @}
  */

/** @defgroup Cursor_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup Cursor_Exported_Functions
  * @{
  */
void CursorDraw(uint16_t X, uint16_t Y, uint8_t DrawPhase);
ErrorStatus CursorInit(uint8_t *PointerMark);
void CursorStepMove(uint8_t CursorEvent);
void CursorReadJoystick(void);
void CursorPosition(uint16_t Xpos, uint16_t Ypos);
void CursorShow(uint16_t Xpos, uint16_t Ypos);

#ifdef __cplusplus
}
#endif

#endif /* __cursor_H */

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
