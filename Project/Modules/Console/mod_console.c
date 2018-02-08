/**
  ******************************************************************************
  * @file    mod_console.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Graphical window and events implementation of the console module
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
#include "mod_console.h"
#include "app_console.h"
#include "gl_console_res.c"
#include "gl_mgr.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CONSOLE_MAIN_PAGE           MOD_CONSOLE_UID + 0
#define CONSOLE_MAX_LINES           12

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/*Graphical draw process */
void CONSOLE_Refresh (void);
static void CONSOLE_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex);
static void CONSOLE_CreatePage(uint8_t Page);
static void CONSOLE_Startup (void);
/* Control Actions */
static void ScrollUp (void);
static void ScrollDown (void);
static void return_to_menu (void);
/* Misc */
static void ClearTextZone (void);
static void DrawConsoleBackground (void);

/* Private variables ---------------------------------------------------------*/
uint8_t ptr_back;
uint8_t scroll_active = RESET;
uint16_t scroll_step = 0;
uint8_t set_scroll = RESET;
GL_Page_TypeDef* ConsoleMainPage;
GL_PageControls_TypeDef* Label0;
GL_PageControls_TypeDef* Label1;
GL_PageControls_TypeDef* Label2;
GL_PageControls_TypeDef* Label3;
GL_PageControls_TypeDef* Label4;
GL_PageControls_TypeDef* Label5;
GL_PageControls_TypeDef* Label6;
GL_PageControls_TypeDef* Label7;
GL_PageControls_TypeDef* Label8;
GL_PageControls_TypeDef* Label9;
GL_PageControls_TypeDef* Label9;
GL_PageControls_TypeDef* Label10;
GL_PageControls_TypeDef* Label11;

MOD_InitTypeDef  mod_console =
{
  MOD_CONSOLE_UID,
  MOD_CONSOLE_VER,
  (uint8_t *)"Log",
  (uint8_t *)console_icon,
  CONSOLE_Startup,
  NULL,  
  NULL,
};

/* Private functions ---------------------------------------------------------*/

/**
* @brief  Handle the Module startup action and display the main menu
* @param  None
* @retval None
*/
static void CONSOLE_Startup (void)
{
  vTaskSuspend(Core_Time_Task_Handle);
  CONSOLE_SwitchPage(GL_HomePage, CONSOLE_MAIN_PAGE);
  
}

/**
* @brief  Close parent page and display a child sub-page
* @param  pParent : parent page
* @param  PageIndex : sub-page index
* @retval None
*/
static void CONSOLE_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex)
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

  CONSOLE_CreatePage(PageIndex);

  switch (PageIndex)
  {
  case CONSOLE_MAIN_PAGE:
    NextPage = ConsoleMainPage;
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
static void CONSOLE_CreatePage(uint8_t Page)
{

  GL_PageControls_TypeDef *ExitButton;

  switch (Page)
  {
  case CONSOLE_MAIN_PAGE:
    {
      ConsoleMainPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( ConsoleMainPage, CONSOLE_MAIN_PAGE );

      ExitButton = NewButton(1, (uint8_t *)" Return ", return_to_menu);
      AddPageControlObj(195, 212, ExitButton, ConsoleMainPage);
      GL_SetPageHeader(ConsoleMainPage , (uint8_t *)"Events Log");

      Label0 = NewLabel(2, (uint8_t *)"", GL_HORIZONTAL, GL_FONT_SMALL, GL_Green, GL_TRUE);
      AddPageControlObj(307, 40, Label0, ConsoleMainPage);

      Label1 = NewLabel(3,(uint8_t *) "", GL_HORIZONTAL, GL_FONT_SMALL, GL_Green, GL_TRUE);
      AddPageControlObj(307, 52, Label1, ConsoleMainPage);

      Label2 = NewLabel(4, (uint8_t *)"", GL_HORIZONTAL, GL_FONT_SMALL, GL_Green, GL_TRUE);
      AddPageControlObj(307, 64, Label2, ConsoleMainPage);

      Label3 = NewLabel(5,(uint8_t *)"", GL_HORIZONTAL, GL_FONT_SMALL, GL_Green, GL_TRUE);
      AddPageControlObj(307, 76, Label3, ConsoleMainPage);

      Label4 = NewLabel(6, (uint8_t *)"", GL_HORIZONTAL, GL_FONT_SMALL, GL_Green, GL_TRUE);
      AddPageControlObj(307, 88, Label4, ConsoleMainPage);

      Label5 = NewLabel(7, (uint8_t *)"", GL_HORIZONTAL, GL_FONT_SMALL, GL_Green, GL_TRUE);
      AddPageControlObj(307, 100, Label5, ConsoleMainPage);

      Label6 = NewLabel(8, (uint8_t *)"", GL_HORIZONTAL, GL_FONT_SMALL, GL_Green, GL_TRUE);
      AddPageControlObj(307, 112, Label6, ConsoleMainPage);

      Label7 = NewLabel(9, (uint8_t *)"", GL_HORIZONTAL, GL_FONT_SMALL, GL_Green, GL_TRUE);
      AddPageControlObj(307, 124, Label7, ConsoleMainPage);

      Label8 = NewLabel(10, (uint8_t *)"", GL_HORIZONTAL, GL_FONT_SMALL, GL_Green, GL_TRUE);
      AddPageControlObj(307, 136, Label8, ConsoleMainPage);

      Label9 = NewLabel(11, (uint8_t *)"", GL_HORIZONTAL, GL_FONT_SMALL, GL_Green, GL_TRUE);
      AddPageControlObj(307, 148, Label9, ConsoleMainPage);

      Label10 = NewLabel(12, (uint8_t *)"", GL_HORIZONTAL, GL_FONT_SMALL, GL_Green, GL_TRUE);
      AddPageControlObj(307, 160, Label10, ConsoleMainPage);

      Label11 = NewLabel(13, (uint8_t *)"", GL_HORIZONTAL, GL_FONT_SMALL, GL_Green, GL_TRUE);
      AddPageControlObj(307, 172, Label11, ConsoleMainPage);
      ConsoleMainPage->CustomPreDraw = DrawConsoleBackground;
      ConsoleMainPage->CustomPostDraw = CONSOLE_Refresh;
      ptr_back =   Console_Msg.ptr ;
    }
    break;

  default:
    break;
  }
}

/**
* @brief  return to global main menu action
* @param  None
* @retval None
*/
static void return_to_menu (void)
{
  CONSOLE_SwitchPage(ConsoleMainPage, PAGE_MENU);
  ConsoleMainPage = NULL;
  set_scroll = RESET;
  CONSOLE_ScrollReset();
}

/**
* @brief  Refresh the console window
* @param  None
* @retval None
*/
void CONSOLE_Refresh (void)
{
  uint16_t ptr , idx;

  if (ConsoleMainPage != NULL)
  {
    if (ConsoleMainPage->Page_ID == CONSOLE_MAIN_PAGE)
    {

      ClearTextZone();
      /* copy msg table to labels */
      if (Console_Msg.full == 0)
      {
        idx = 0;
        ptr = 0;

        if ( Console_Msg.ptr > CONSOLE_MAX_LINES)
        {
          ptr = Console_Msg.ptr - CONSOLE_MAX_LINES ;
          if (set_scroll == RESET)
          {
            GL_AddScroll (ConsoleMainPage, 293, 30, 140, ScrollUp, ScrollDown, 100);
            set_scroll = SET;
          }
        }

        while ( idx  < CONSOLE_MAX_LINES )
        {
          RefreshLabel (ConsoleMainPage, Label0->ID + idx, Console_Msg.msg[ptr + idx]);
          idx ++;
        }
      }
      else /* Message buffer full, new data are in the top */
      {

        if (set_scroll == RESET)
        {
          GL_AddScroll (ConsoleMainPage, 293, 30, 140, ScrollUp, ScrollDown, 100);
          set_scroll = SET;
        }

        idx = 0;
        ptr = Console_Msg.ptr;

        while ( (idx < CONSOLE_MAX_LINES) && (ptr > 0))
        {
          RefreshLabel (ConsoleMainPage, Label11->ID - idx, Console_Msg.msg[ptr - 1]);
          idx ++;
          ptr --;
        }

        ptr = CONSOLE_DEPDTH - 1;
        while (idx < CONSOLE_MAX_LINES)
        {
          RefreshLabel (ConsoleMainPage, Label11->ID - idx, Console_Msg.msg[ptr]);
          idx ++;
          ptr --;
        }

      }
      GL_SetBackColor (GL_White);
    }
  }
}

/**
* @brief  Move up in the console window
* @param  None
* @retval None
*/
void CONSOLE_UpdateCursor(void)
{
  if(ConsoleMainPage != NULL)
  {
    if (ConsoleMainPage->Page_ID == CONSOLE_MAIN_PAGE)
    {
      GL_UpdateScrollPosition (NULL, 293, 30, 140, 100) ;
    }
  }
}

/**
* @brief  Move up in the console window
* @param  None
* @retval None
*/
void ScrollUp (void)
{
  static uint8_t max_step_up;
  
  if ( scroll_active == RESET)
  {
    ptr_back = Console_Msg.ptr;
    scroll_active = SET;

    if (Console_Msg.full == 0)
    {
      max_step_up = ptr_back - CONSOLE_MAX_LINES;
    }
    else
    {
      max_step_up = CONSOLE_DEPDTH;
    }
  }
  
  if (scroll_step < max_step_up)
  {
    scroll_step++;

    Console_Msg.ptr--;

    if ((Console_Msg.ptr == 0) && (Console_Msg.full == 1))
    {
      Console_Msg.ptr = CONSOLE_DEPDTH;
    }

    CONSOLE_Refresh();

    GL_UpdateScrollPosition (NULL, 293, 30, 140, ((Console_Msg.ptr- CONSOLE_MAX_LINES) * 100)/(ptr_back - CONSOLE_MAX_LINES))  ;
  }
}

/**
* @brief  Move down in the console window
* @param  None
* @retval None
*/
void ScrollDown (void)
{
  if (scroll_step != 0)
  {
    if ( scroll_active == RESET)
    {
      ptr_back =  Console_Msg.ptr;
      scroll_active = SET;
    }
    
    scroll_step--;

    Console_Msg.ptr++;

    if (Console_Msg.ptr == CONSOLE_DEPDTH)
    {
      Console_Msg.ptr = 0;
    }
    CONSOLE_Refresh();
    GL_UpdateScrollPosition (NULL, 293, 30, 140, ((Console_Msg.ptr- CONSOLE_MAX_LINES) * 100)/(ptr_back - CONSOLE_MAX_LINES)) ;
  }
}


/**
* @brief  Clear log Text zone
* @param  None
* @retval None
*/
static void ClearTextZone (void)
{
  LCD_SetTextColor(GL_Black);
  GL_LCD_FillRect(33, 319 - 6, 158, 286);
}

/**
* @brief  Clear log Text zone
* @param  None
* @retval None
*/
static void DrawConsoleBackground (void)
{
  LCD_SetTextColor(GL_Grey);
  GL_LCD_DrawRect(32, 319 - 5, 160, 310);
  LCD_SetTextColor(GL_Black);
  GL_LCD_FillRect(33, 319 - 6, 158, 308);

  if (set_scroll == SET)
  {
    DestroyPageControl (ConsoleMainPage, 0xFE);
    DestroyPageControl (ConsoleMainPage, 0xFF);
    GL_AddScroll (ConsoleMainPage, 293, 30, 140, ScrollUp, ScrollDown, 100);
  }
}
/**
* @brief  restore the scroll flags to default
* @param  None
* @retval None
*/
void CONSOLE_ScrollReset (void)
{
  Console_Msg.ptr = ptr_back;
  scroll_active = 0;
  scroll_step = 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
