/**
  ******************************************************************************
  * @file    mod_template.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   This file contains all the Modules functions.
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
#include "mod_template.h"
#include "gl_template_res.c"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define TEMPLATE_MAIN_PAGE           0x00
#define TEMPLATE_PAGE1               0x01
#define TEMPLATE_PAGE2               0x02
#define TEMPLATE_PAGE3               0x03

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/*Gui mgmt */
static void TEMPLATE_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex);
static void TEMPLATE_CreatePage(uint8_t Page);
static void TEMPLATE_Startup (void);
/* Actions */
static void return_to_menu (void);

/* Private variables ---------------------------------------------------------*/
static GL_Page_TypeDef *TemplateMainPage;


MOD_InitTypeDef  mod_template =
{
  MOD_TEMPLATE_UID,
  MOD_TEMPLATE_VER,
  "Template",
  (uint8_t *)template_icon,
  TEMPLATE_Startup,
  NULL,
  NULL,
};

/* Private functions ---------------------------------------------------------*/

/**
* @brief  Handles the Module startup action and display the main menu
* @param  None
* @retval None
*/
static void TEMPLATE_Startup (void)
{
  TEMPLATE_SwitchPage(GL_HomePage, TEMPLATE_MAIN_PAGE);
}

/**
* @brief  Close parent page and display a child sub-page
* @param  pParent : parent page
* @param  PageIndex : sub-page index
* @retval None
*/

static void TEMPLATE_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex)
{
  /* Switch to new page, and free previous one. */
  GL_Page_TypeDef* NextPage = NULL;

  (*pParent).ShowPage(pParent, GL_FALSE);
  DestroyPage(pParent);
  vPortFree(pParent);
  pParent = NULL;

  if (PageIndex == PAGE_MENU)
  {
    GL_ShowMainMenu();
    return;
  }

  TEMPLATE_CreatePage(PageIndex);

  switch (PageIndex)
  {
    case TEMPLATE_MAIN_PAGE:
      NextPage = TemplateMainPage;
      break;

    case TEMPLATE_PAGE1:
    case TEMPLATE_PAGE2:
    case TEMPLATE_PAGE3:


    default:
      break;
  }
  (*NextPage).ShowPage(NextPage, GL_TRUE);
}

/**
* @brief  Creates a child sub-page
* @param  Page : Page handler
* @retval None
*/
static void TEMPLATE_CreatePage(uint8_t Page)
{

  GL_PageControls_TypeDef* item;

  GL_SetBackColor( GL_White );
  GL_SetTextColor( GL_Blue );

  switch (Page)
  {
    case TEMPLATE_MAIN_PAGE:
    {
      TemplateMainPage = pvPortMalloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( TemplateMainPage,TEMPLATE_MAIN_PAGE );
      item = NewIcon (4, template_icon, 50, 50, MOD_NullFunc);
      AddPageControlObj(185, 40, item, TemplateMainPage);

      GL_SetMenuItem(TemplateMainPage, "menu1", 0, MOD_NullFunc );
      GL_SetMenuItem(TemplateMainPage, "menu2", 1, MOD_NullFunc );
      GL_SetMenuItem(TemplateMainPage, "menu3", 2, MOD_NullFunc );
      GL_SetMenuItem(TemplateMainPage, "Return", 3, return_to_menu );
      GL_SetPageHeader(TemplateMainPage , "Template Menu");
    }
    break;

    default:
      break;
  }
}

/**
  * @brief  Return to module nmain menu
  * @param  None
  * @retval None
  */
static void return_to_menu (void)
{
  TEMPLATE_SwitchPage(TemplateMainPage, PAGE_MENU);
  TemplateMainPage = NULL;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
