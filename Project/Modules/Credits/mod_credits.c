/**
  ******************************************************************************
  * @file    mod_credits.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Graphical window and events implementation of the credits module
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
#include "mod_credits.h"
#include "gl_mgr.h"
#include "gl_credits_res.c"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CREDITS_PAGE1          MOD_CREDIT_UID + 0
#define CREDITS_PAGE2          MOD_CREDIT_UID + 1

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/*Graphical draw process */
static void CREDITS_INFO_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex);
static void CREDITS_INFO_CreatePage(uint8_t Page);
static void CREDITS_INFO_Startup (void);
/* Control Actions */
static void goto_credits_page1 (void);
static void goto_credits_page2 (void);
static void return_from_credits_page1 (void);
static void return_from_credits_page2 (void);

/* Private variables ---------------------------------------------------------*/
static GL_Page_TypeDef *CreditsPage1;
static GL_Page_TypeDef *CreditsPage2;

MOD_InitTypeDef  mod_credits =
{
  MOD_CREDIT_UID,
  MOD_CREDIT_VER,
  (uint8_t *)"Credits",
  (uint8_t *)credits_icon,
  CREDITS_INFO_Startup,
  NULL,
  NULL
};

/* Private functions ---------------------------------------------------------*/

/**
* @brief  Handle the Module startup action and display the main menu
* @param  None
* @retval None
*/
static void CREDITS_INFO_Startup (void)
{
  vTaskSuspend(Core_Time_Task_Handle);
  CREDITS_INFO_SwitchPage(GL_HomePage, CREDITS_PAGE1);
}

/**
* @brief  Close parent page and display a child sub-page
* @param  pParent : parent page
* @param  PageIndex : sub-page index
* @retval None
*/
static void CREDITS_INFO_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex)
{
  /* Switch to new page, and free previous one. */
  GL_Page_TypeDef* NextPage = NULL;

  (*pParent).ShowPage(pParent, GL_FALSE);
  DestroyPage(pParent);
  free(pParent);
  pParent = NULL;

  if (PageIndex == PAGE_MENU)
  {
    GL_ShowMainMenu();
    return;
  }

  CREDITS_INFO_CreatePage(PageIndex);

  switch (PageIndex)
  {
  case CREDITS_PAGE1:
    NextPage = CreditsPage1;
    break;

  case CREDITS_PAGE2:
    NextPage = CreditsPage2;
    break;

  default:
    break;
  }
  RefreshPage(NextPage);
}

/**
* @brief  Create a child sub-page
* @param  Page : Page handler
* @retval None
*/

static void CREDITS_INFO_CreatePage(uint8_t Page)
{
  GL_PageControls_TypeDef* ExitEventButton;
  GL_PageControls_TypeDef* NextEventButton;
  GL_PageControls_TypeDef* PrevEventButton;
  GL_PageControls_TypeDef* item;

  GL_SetBackColor( GL_White );
  GL_SetTextColor( GL_Blue );

  switch (Page)
  {
  case CREDITS_PAGE1:
    {

      CreditsPage1 = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( CreditsPage1, CREDITS_PAGE1 );


      NextEventButton = NewButton(1, (uint8_t *)" Next > ", goto_credits_page2);
      AddPageControlObj(150, 212, NextEventButton, CreditsPage1);

      ExitEventButton = NewButton(2, (uint8_t *)" Return ", return_from_credits_page1);
      AddPageControlObj(230, 212, ExitEventButton, CreditsPage1);

      item = NewLabel(3, (uint8_t *)"- Audio Player Equalizer, Loudness controls ", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 84 - 50 , item, CreditsPage1);

      item = NewLabel(4, (uint8_t *)"  are licensed from SPIRIT DSP Copyright (c) ", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 100 - 50 , item, CreditsPage1);

      item = NewLabel(5, (uint8_t *)"  1995-2011.", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 116 - 50 , item, CreditsPage1); 

      item = NewLabel(6, (uint8_t *)"- lwIP is freely available under a BSD ", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 132 - 50 , item, CreditsPage1);

      item = NewLabel(7, (uint8_t *)"  license.", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 148 - 50 , item, CreditsPage1);

      item = NewLabel(8, (uint8_t *)"- The FatFs module is a free software, All", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 164 - 50 , item, CreditsPage1);

      item = NewLabel(9, (uint8_t *)"  rights reserved to ChaN.", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 180 - 50 , item, CreditsPage1);


      item = NewLabel(10, (uint8_t *)"- LibJpeg is a free software, All Rights", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 196 - 50 , item, CreditsPage1);

      item = NewLabel(11, (uint8_t *)"  reserved to Thomas G. Lane, Guido ", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 212 - 50 , item, CreditsPage1); 

      item = NewLabel(12, (uint8_t *)"  Vollbeding, For more details refer to ", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 228 - 50 , item, CreditsPage1);

      item = NewLabel(13, (uint8_t *)"  the ReadMe in the LibJpeg sources.", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 244 - 50 , item, CreditsPage1);

      GL_SetPageHeader( CreditsPage1 , (uint8_t *)"Info & Credits 1/2");
    }
    break;

  case CREDITS_PAGE2:
    {

      CreditsPage2 = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( CreditsPage1, CREDITS_PAGE2 );

      PrevEventButton = NewButton(1, (uint8_t *)" < Prev ", goto_credits_page1);
      AddPageControlObj(230, 212, PrevEventButton, CreditsPage2);

      ExitEventButton = NewButton(2, (uint8_t *)" Return ", return_from_credits_page2);
      AddPageControlObj(150, 212, ExitEventButton, CreditsPage2); 

      item = NewLabel(3, (uint8_t *)"- FreeRTOS is free software; you can ", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 84 - 50 , item, CreditsPage2);

      item = NewLabel(4, (uint8_t *)"  redistribute it and/or modify it under ", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 100 - 50 , item, CreditsPage2);

      item = NewLabel(5, (uint8_t *)"  the terms of the GNU GPL (version 2) as", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 116 - 50 , item, CreditsPage2);

      item = NewLabel(6, (uint8_t *)"  published by the  FreeRTOS exception.", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 132 - 50 , item, CreditsPage2);

      item = NewLabel(7, (uint8_t *)"  See the GNU General Public License for ", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 148 - 50 , item, CreditsPage2);

      item = NewLabel(8, (uint8_t *)"  more details.", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 164 - 50 , item, CreditsPage2);

      item = NewLabel(9, (uint8_t *)"- All other firmware modules are copyrighted ", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 180 - 50 , item, CreditsPage2);

      item = NewLabel(10, (uint8_t *)"  by STMicroelectronics, To learn more about", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 196 - 50 , item, CreditsPage2);

      item = NewLabel(11, (uint8_t *)"  this solution and to order, please contact", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 212 - 50 , item, CreditsPage2);

      item = NewLabel(12, (uint8_t *)"  your local ST Sales representative.", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 228 - 50 , item, CreditsPage2);

      item = NewLabel(13, (uint8_t *)"- Icons from http://commons.wikimedia.org", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(316, 244 - 50 , item, CreditsPage2);

      GL_SetPageHeader( CreditsPage2 , (uint8_t *)"Info & Credits 2/2");
    }
    break;
  default:
    break;
  }
}

/**
* @brief  go to page info 1
* @param  None
* @retval None
*/
static void goto_credits_page1 (void)
{
  CREDITS_INFO_SwitchPage(CreditsPage2, CREDITS_PAGE1);
  CreditsPage2 = NULL;
}

/**
* @brief  go to page info 2
* @param  None
* @retval None
*/
static void goto_credits_page2 (void)
{
  CREDITS_INFO_SwitchPage(CreditsPage1, CREDITS_PAGE2);
  CreditsPage1 = NULL;
}

/**
* @brief  return to global main menu action from info page 1
* @param  None
* @retval None
*/
static void return_from_credits_page1 (void)
{
  CREDITS_INFO_SwitchPage(CreditsPage1,  PAGE_MENU);
}

/**
* @brief  return to global main menu action from info page 2
* @param  None
* @retval None
*/
static void return_from_credits_page2 (void)
{
  CREDITS_INFO_SwitchPage(CreditsPage2,  PAGE_MENU);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
