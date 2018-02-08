/**
  ******************************************************************************
  * @file    messages.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Graphical window and events implementation of under construction message  
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

/* Includes ------------------------------------------------------------------*/
#include "gl_mgr.h"
#include "mod_core.h"
#include "app_image.h"
#include "image_bmp.h"

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/*Graphical draw process */
static void CreatePage(uint8_t msg_id);
static void return_from_direct(void);
/* Private variables ---------------------------------------------------------*/


static GL_Page_TypeDef *MsgDirectPage;
static GL_Page_TypeDef *MsgBackPage;
static uint16_t        prev_ID = 0;

static uint8_t MESSAGE_Active = 0;
/* Private functions ---------------------------------------------------------*/

/**
* @brief  Launch directly the image viewer from command line
* @param  pParent : caller page
* @param  msg_id : message index
* @retval None
*/
void MESSAGES_DirectEx(GL_Page_TypeDef* pParent, uint8_t msg_id)
{

  (*pParent).ShowPage(pParent, GL_FALSE);
  prev_ID = pParent->Page_ID;
  pParent->Page_ID = 0xFFFE;
  MsgBackPage = pParent;
  CreatePage(msg_id);
  RefreshPage(MsgDirectPage);
  MESSAGE_Active = 1;
}

/**
* @brief  Create a child sub-page 
* @param  msg_id : message index
* @retval None
*/
static void CreatePage( uint8_t msg_id)
{
  uint8_t TempStr[128];
  GL_PageControls_TypeDef *ExitButton;
  GL_PageControls_TypeDef *ErrorIcon;
  GL_PageControls_TypeDef *Messages;
  RTC_AlarmTypeDef  RTC_AlarmStructure;


  GL_SetBackColor( GL_White );
  GL_SetTextColor( GL_Blue );

  MsgDirectPage = malloc(sizeof(GL_Page_TypeDef));
  Create_PageObj( MsgDirectPage , 0xFFFE);

  ExitButton= NewButton(2, (uint8_t *)"  OK  ",return_from_direct);
  AddPageControlObj(185,212,ExitButton,MsgDirectPage);

  if(msg_id == MESSAGE_UNDER_CONSTRUCTION)
  {
    ErrorIcon = NewIcon (3,under_construction_icon, 100,100,MOD_NullFunc);
    AddPageControlObj(210, 40, ErrorIcon, MsgDirectPage);
    
    GL_SetPageHeader(MsgDirectPage , (uint8_t *)"Information"); 
    Messages  = NewLabel(4, (uint8_t *)"!!! This feature is under construction !!!",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_FALSE);
    AddPageControlObj(305,160,Messages,MsgDirectPage);
  }
  else if (msg_id == MESSAGE_ALARM_RING)
  {
    ErrorIcon = NewIcon (3, alarm_ring_icon, 100,100,MOD_NullFunc);
    AddPageControlObj(210, 40, ErrorIcon, MsgDirectPage);

    RTC_GetAlarm(RTC_Format_BIN,RTC_Alarm_A, &RTC_AlarmStructure);
    sprintf((char *)TempStr, "%02d:%02d:%02d",
            RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours,
            RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes,
            RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds);

    GL_SetPageHeader(MsgDirectPage , (uint8_t *)"Alarm");
    Messages  = NewLabel(4,TempStr,GL_HORIZONTAL,GL_FONT_BIG,GL_Red, GL_FALSE);
    AddPageControlObj(220,150,Messages,MsgDirectPage); 
  }
}

/**
* @brief  return from the direct Message frame
* @param  None
* @retval None
*/
static void return_from_direct (void)
{
  LCD_WindowModeDisable();
  (*MsgDirectPage).ShowPage(MsgDirectPage, GL_FALSE);
  DestroyPage(MsgDirectPage);
  free(MsgDirectPage);
  MsgDirectPage->Page_ID = 0;
  MsgDirectPage = NULL;
  MsgBackPage->Page_ID = prev_ID;
  RefreshPage(MsgBackPage);
  MESSAGE_Active = 0;
}

/**
* @brief  return from the direct Message from external event
* @param  None
* @retval None
*/
void MESSAGES_CloseEx (void)
{
  if(MESSAGE_Active)
  {
    vTaskSuspendAll ();
    return_from_direct();
    xTaskResumeAll ();
  }
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
