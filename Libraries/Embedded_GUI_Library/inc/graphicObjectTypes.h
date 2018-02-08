/**
  ******************************************************************************
  * @file    graphicObjectTypes.h
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    31-July-2012
  * @brief   This file contains all the specific Graphics Object types
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
  
/* Define to prevent recursive inclusion ---------------------------------------*/
#ifndef __GRAPHIC_OBJECT_TYPES_H
#define __GRAPHIC_OBJECT_TYPES_H

#ifdef __cplusplus
 extern "C" {
#endif
   
/* Includes ------------------------------------------------------------------*/
#include "global_includes.h"  
/** @addtogroup Embedded_GUI_Library
  * @{
  */

/** @defgroup graphicObjectTypes 
  * @{
  */ 

/** @defgroup graphicObjectTypes_Private_Defines
  * @{
  */
#define MAX_CTRL_X_PAGE 0xFF
#define MAX_NAME_LENGTH 22
#define MAX_LABEL_LENGTH 46
#define MAX_BUTTON_LABEL_LENGTH 40
#define MAX_CHECKBOX_LABEL_LENGTH 25
#define MAX_RADIO_OPTION_LABEL_LENGTH 20
#define MAX_RADIO_OPTIONS 10
#define MAX_COMBO_OPTIONS 10
#define MAX_COMBO_LABEL_LENGTH 18
#define MAX_SWITCH_LABEL_LENGTH 16
#define MAX_SLIDE_LABEL_LENGTH 16
#define MAX_HIST_LABEL_LENGTH 7
#define MAX_GRAPH_LABEL_LENGTH 16
#define MAX_HIST_POINTS 50
#define MAX_GRAPH_POINTS 100
#define HIST_MARGIN_LENGTH 20
#define GRAPH_MARGIN_LENGTH 30
#define ABS(X)  ((X) > 0 ? (X) : -(X))    
/**
  * @}
  */

/** @defgroup graphicObjectTypes_Exported_Types
  * @{
  */

/** 
  * @brief  GL_FunctionalState enumeration definition  
  */
typedef enum {GL_DISABLE = 0, GL_ENABLE = !GL_DISABLE} GL_FunctionalState;

/** 
  * @brief  GL_bool enumeration definition  
  */
typedef enum {GL_FALSE = 0, GL_TRUE = !GL_FALSE} GL_bool;

/** 
  * @brief  GL_ErrStatus enumeration definition  
  */
typedef enum {
    GL_ERROR = 0, 
    GL_OK = !GL_ERROR
}GL_ErrStatus;

/** 
  * @brief  GL_Direction enumeration definition  
  */
typedef enum {
    GL_HORIZONTAL = 0, 
    GL_LEFT_VERTICAL,
    GL_RIGHT_VERTICAL
}GL_Direction;

/** 
  * @brief  GL_ButtonStatus enumeration definition  
  */
typedef enum {
    UNSELECTED = 0, 
    SELECTED = 1, 
    GL_UNKNOWN = 2
}GL_ButtonStatus;

/** 
  * @brief  GL_ObjType enumeration definition  
  */
typedef enum{
    GL_BUTTON = 1,
    GL_RADIO_BUTTON = 2,
    GL_CHECKBOX = 3,
    GL_LABEL = 4,
    GL_SWITCH = 5,
    GL_ICON = 6,
    GL_COMBOBOX = 7,
    GL_SLIDEBAR = 8,
    GL_HISTOGRAM = 9,
    GL_GRAPH_CHART = 10
}GL_ObjType;

typedef struct   
{ uint16_t MaxX;
  uint16_t MinX;
  uint16_t MaxY;
  uint16_t MinY;
}GL_Coordinate_TypeDef;

typedef struct GL_PageControlsObj GL_PageControls_TypeDef;  /* Forward declaration for circular typedefs */
struct GL_PageControlsObj
{
  uint16_t               ID;
  void*                  objPTR;
  GL_Coordinate_TypeDef  objCoordinates;
  GL_ObjType             objType;
  GL_ErrStatus           (*SetObjVisible)(GL_PageControls_TypeDef* pThis, GL_Coordinate_TypeDef objCoordinates);
};

typedef struct GL_LabelObj GL_Label_TypeDef;  /* Forward declaration for circular typedefs */
struct GL_LabelObj
{
  uint16_t          ID;
  uint8_t           label[MAX_LABEL_LENGTH];
  __IO uint8_t      FontSize;
  GL_bool           Control_Visible;
  uint16_t          Colour;
  GL_bool           Transparency;  
  GL_Direction      Direction;
};

typedef struct GL_ButtonObj GL_Button_TypeDef;  /* Forward declaration for circular typedefs */
struct GL_ButtonObj
{
  uint16_t          ID;
#ifndef USE_2D_OBJECTS
  uint8_t*          ImageClickedPTR;
  uint8_t*          ImageUnClickedPTR;
#endif
  uint8_t           label[MAX_BUTTON_LABEL_LENGTH];
  GL_bool           isObjectTouched;
  GL_bool           Control_Visible;
  void              (*EventHandler)(void);
};

typedef struct GL_CheckboxObj GL_Checkbox_TypeDef;  /* Forward declaration for circular typedefs */
struct GL_CheckboxObj
{
  uint16_t          ID;
#ifndef USE_2D_OBJECTS
  uint8_t*          ImageCheckedPTR;
  uint8_t*          ImageUnCheckedPTR;
#endif
  uint8_t           label[MAX_CHECKBOX_LABEL_LENGTH];
  GL_bool           ReadOnly; 
  GL_bool           IsChecked;
  GL_bool           Control_Visible;
  void              (*EventHandler)(void);
};

typedef struct GL_SwitchObj GL_Switch_TypeDef;  /* Forward declaration for circular typedefs */
struct GL_SwitchObj
{
  uint16_t          ID;
#ifndef USE_2D_OBJECTS
  uint8_t*          ImageClickedPTR;
  uint8_t*          ImageUnClickedPTR;
#endif
  uint8_t           label_1[MAX_SWITCH_LABEL_LENGTH];
  uint8_t           label_2[MAX_SWITCH_LABEL_LENGTH];
  GL_bool           isObjectTouched;
  GL_bool           Control_Visible;
  void              (*EventHandler)(void);
};

typedef struct GL_RadioButtonOptionObj GL_RadioOption_TypeDef;

typedef struct GL_RadioButtonGrp GL_RadioButtonGrp_TypeDef;  /* Forward declaration for circular typedefs */
struct GL_RadioButtonGrp
{
  uint16_t                 ID;
  uint16_t                 RadioOptionCount;
  GL_PageControls_TypeDef* RadioOptions[MAX_RADIO_OPTIONS];
  GL_PageControls_TypeDef* (*AddRadioOption)( GL_RadioButtonGrp_TypeDef* pThis, const uint8_t* label, void (*pEventHandler)(void) );
};

struct GL_RadioButtonOptionObj
{
  GL_RadioButtonGrp_TypeDef* RadioButtonGrp;
#ifndef USE_2D_OBJECTS
  uint8_t*                   ImageCheckedPTR;
  uint8_t*                   ImageUnCheckedPTR;
#endif
  uint8_t                    label[MAX_RADIO_OPTION_LABEL_LENGTH];
  GL_bool                    IsChecked;
  GL_bool                    Control_Visible;
  void                       (*EventHandler)(void);
};

typedef struct GL_ComboOptionObj GL_ComboOption_TypeDef;  /* Forward declaration for circular typedefs */
typedef struct GL_ComboBoxGrp    GL_ComboBoxGrp_TypeDef;  /* Forward declaration for circular typedefs */
struct GL_ComboBoxGrp
{
#ifndef USE_2D_OBJECTS
  uint8_t*                  ImageClickedPTR;
  uint8_t*                  ImageUnClickedPTR;
#endif
  GL_bool                   Secondary;
  GL_PageControls_TypeDef*  PrimaryComboOpt;
  uint16_t                  ID;
  uint8_t                   objName[MAX_NAME_LENGTH];
  GL_bool                   Control_Visible;
  uint16_t                  ComboOptionCount;
  GL_ComboOption_TypeDef*   ComboOptions[MAX_COMBO_OPTIONS];
  GL_ErrStatus              (*AddComboOption)( GL_ComboBoxGrp_TypeDef* pThis, const uint8_t* label, void (*pEventHandler)(void) );
};

struct GL_ComboOptionObj
{
  __IO uint16_t             BackColor;
  __IO uint16_t             TextColor;
  GL_ComboBoxGrp_TypeDef*   ComboBoxGrp;
  uint8_t                   label[MAX_COMBO_LABEL_LENGTH];
  GL_bool                   IsActive;
  void                      (*EventHandler)(void);
};

typedef struct GL_IconObj GL_Icon_TypeDef;  /* Forward declaration for circular typedefs */
struct GL_IconObj
{
  uint16_t             ID;
  const uint8_t*       ImagePTR;
  uint16_t             ImageWidth;
  uint8_t              ImageHeight;
  GL_bool              isObjectTouched;
  GL_bool              Control_Visible;
  void                 (*EventHandler)(void);
};

typedef struct GL_SlidebarObj GL_Slidebar_TypeDef;  /* Forward declaration for circular typedefs */
struct GL_SlidebarObj
{
  uint16_t           ID;
#ifndef USE_2D_OBJECTS
  uint8_t*           ImageCursorPTR;
  uint8_t*           ImageBackgroundPTR;
#endif
  uint8_t            label[MAX_SLIDE_LABEL_LENGTH];
  GL_Direction       Direction;
  uint8_t            Width;
  uint8_t            Length;
  int8_t             CurrentValue;
  uint8_t            PrevValue;
  uint8_t            MinValue;
  uint8_t            MaxValue;
  uint16_t*          BehindCursor;
  GL_bool            Control_Visible;
  GL_bool            ReadOnly;    
  void               (*EventHandler)(void);
};

typedef struct GL_HistogramObj GL_Histogram_TypeDef;  /* Forward declaration for circular typedefs */
struct GL_HistogramObj
{
  uint16_t      ID;
  uint8_t       label_X[MAX_HIST_LABEL_LENGTH];
  uint8_t       label_Y[MAX_HIST_LABEL_LENGTH];
  int16_t       points[MAX_HIST_POINTS];
  uint8_t       n_points;
  GL_bool       Control_Visible;
};

typedef struct GL_GraphChartObj GL_GraphChart_TypeDef;  /* Forward declaration for circular typedefs */
struct GL_GraphChartObj
{
  uint16_t      ID;
  uint8_t       label_X[MAX_GRAPH_LABEL_LENGTH];
  uint8_t       label_Y[MAX_GRAPH_LABEL_LENGTH];
  int16_t       points[MAX_GRAPH_POINTS];
  uint8_t       n_points;
  GL_bool       Background;
  GL_bool       Control_Visible;
};

typedef struct   
{ uint16_t Height;
  uint16_t Length;
}GL_ObjDimensions_TypeDef;

typedef struct GL_PageObj GL_Page_TypeDef;  /* Forward declaration for circular typedefs */
struct GL_PageObj
{
  uint8_t                  objName[MAX_NAME_LENGTH];
  uint16_t                 Page_ID;
  GL_bool                  Page_Active;
  GL_bool                  Page_Visible;
  uint16_t                 ControlCount;
  GL_ErrStatus             (*ShowPage)(GL_Page_TypeDef* pThis, GL_bool bVal);
  GL_PageControls_TypeDef* PageControls[MAX_CTRL_X_PAGE]; 
  GL_ErrStatus             (*SetPage)(GL_Page_TypeDef* pThis, GL_bool bVal);
  GL_bool                  (*GetObjStatus)(GL_Page_TypeDef* pThis, uint16_t ID);
  GL_Coordinate_TypeDef    (*GetObjCoordinates)(GL_Page_TypeDef* pThis, uint16_t ID);
  void                     (*CustomPreDraw) (void);
  void                     (*CustomPostDraw) (void);  
};

/**
  * @}
  */
   
/** @defgroup graphicObjectTypes_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup graphicObjectTypes_Exported_Functions
  * @{
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /*__GRAPHIC_OBJECT_TYPES_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
