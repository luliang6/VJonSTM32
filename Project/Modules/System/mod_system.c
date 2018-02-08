/**
  ******************************************************************************
  * @file    mod_system.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Graphical window and events implementation of the system module
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
#include "mod_system.h"
#include "mod_ethernet.h"
#include "mod_audio.h"
#include "gl_mgr.h"
#include "gl_system_res.c"


/* Private typedef -----------------------------------------------------------*/
typedef  void (*pFunc)(void);

/* Private define ------------------------------------------------------------*/
#define SYSTEM_MAIN_PAGE                     MOD_SYSTEM_UID + 0
#define SYSTEM_INFO_PAGE                     MOD_SYSTEM_UID + 1
#define SETTINGS_PAGE                        MOD_SYSTEM_UID + 2
#define UPGRADE_PAGE                         MOD_SYSTEM_UID + 3
#define SYSTEM_BACKGROUND_ERROR_PAGE         MOD_SYSTEM_UID + 4
#define SYSTEM_DISTANTCONTROL_ERROR_PAGE     MOD_SYSTEM_UID + 5

#define BL_ADDRESS                0x1FFF0000
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/*Graphical draw process */
static void SYSTEM_INFO_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex);
static void SYSTEM_INFO_CreatePage(uint8_t Page);
static void SYSTEM_INFO_Startup (void);
/* Control Actions */
static void goto_system_info (void);
static void return_from_system_info (void);
static void goto_Settings (void);
static void return_from_settings (void);
static void goto_upgrade (void);
static void return_from_upgrade (void);
static void return_to_menu (void);
static void start_upgrade (void);
static void return_bkgnd_from_errorpage (void);
static void return_dcontrol_from_errorpage (void);
static void EnableDistantControl(void);
static void EnableBackgroundMode(void);
static void return_from_settings (void);
static void save_settings (void);

/* Private variables ---------------------------------------------------------*/
static GL_Page_TypeDef *SystemMainPage;
static GL_Page_TypeDef *SystemInfoPage;
static GL_Page_TypeDef *SettingsPage;
static GL_Page_TypeDef *UpgradePage;
static GL_Page_TypeDef *SystemErrorPage;


uint32_t JumpAddress;
uint8_t  BL_Enabled;
SYSTEM_CONFIG_TypeDef Global_Config;

MOD_InitTypeDef  mod_system =
{
  MOD_SYSTEM_UID,
  MOD_SYSTEM_VER,
  (uint8_t *)"System",
  (uint8_t *)hardware_icon,
  SYSTEM_INFO_Startup,
  NULL,  
  NULL,
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Handle the Module startup action and display the main menu
  * @param  None
  * @retval None
  */
static void SYSTEM_INFO_Startup (void)
{
  vTaskSuspend(Core_Time_Task_Handle);
  SYSTEM_INFO_SwitchPage(GL_HomePage, SYSTEM_MAIN_PAGE);
  BL_Enabled = 0;
}

/**
  * @brief  Close parent page and display a child sub-page
  * @param  pParent : parent page
  * @param  PageIndex : sub-page index
  * @retval None
  */
static void SYSTEM_INFO_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex)
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

  SYSTEM_INFO_CreatePage(PageIndex);

  switch (PageIndex)
  {
  case SYSTEM_MAIN_PAGE:
    NextPage = SystemMainPage;
    break;
  case SYSTEM_INFO_PAGE:
    NextPage = SystemInfoPage;
    break;
  case SETTINGS_PAGE:
    NextPage = SettingsPage;
    break;
  case UPGRADE_PAGE:
    NextPage = UpgradePage;
    break;
  case SYSTEM_BACKGROUND_ERROR_PAGE:
  case SYSTEM_DISTANTCONTROL_ERROR_PAGE:
    NextPage = SystemErrorPage;
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

static void SYSTEM_INFO_CreatePage(uint8_t Page)
{
  GL_PageControls_TypeDef* ExitEventButton;
  GL_PageControls_TypeDef* system_info;
  GL_PageControls_TypeDef* BoardRevLabel;
  GL_PageControls_TypeDef* CoreLabel;
  GL_PageControls_TypeDef* DeviceLabel;
  GL_PageControls_TypeDef* FwRevLabel;
  GL_PageControls_TypeDef* CPUSpeedLabel;
  GL_PageControls_TypeDef* FlashLabel;
  GL_PageControls_TypeDef* CopyrightLabel;
  GL_PageControls_TypeDef* WebLabel;
  GL_PageControls_TypeDef* item;


  char temp[100];

  GL_SetBackColor( GL_White );
  GL_SetTextColor( GL_Blue );

  switch (Page)
  {
  case SYSTEM_MAIN_PAGE:
    {
      SystemMainPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( SystemMainPage, SYSTEM_MAIN_PAGE );
      item = NewIcon (5, hardware_icon, 50, 50, MOD_NullFunc);
      AddPageControlObj(185, 40, item, SystemMainPage);

      GL_SetMenuItem(SystemMainPage, (uint8_t *)"System Info", 0, goto_system_info );
      GL_SetMenuItem(SystemMainPage, (uint8_t *)"Firmware Upgrade", 1, goto_upgrade );
      GL_SetMenuItem(SystemMainPage, (uint8_t *)"Settings", 2, goto_Settings );
      GL_SetMenuItem(SystemMainPage, (uint8_t *)"Return", 3, return_to_menu );
      GL_SetPageHeader(SystemMainPage , (uint8_t *)"System Menu");
    }
    break;

  case SYSTEM_INFO_PAGE:
    {
      ExitEventButton = NewButton(1, (uint8_t *)" Return ", return_from_system_info);
      system_info = NewIcon (3, system_icon, 50, 50, MOD_NullFunc);

      BoardRevLabel = NewLabel(4, (uint8_t *)DEMO_BOARD, GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      CoreLabel = NewLabel(5, (uint8_t *)DEMO_CORE, GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);


      DeviceLabel = NewLabel(6, (uint8_t *)DEMO_DEVICE, GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);

      sprintf(temp, "Firmware revision  : %s", DEMO_REV);
      FwRevLabel = NewLabel(7, (uint8_t *)temp, GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);

      CPUSpeedLabel = NewLabel(8, (uint8_t *)DEMO_CPU_SPEED, GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      FlashLabel = NewLabel(9, (uint8_t *)DEMO_MEMORY_RESOURCE, GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      CopyrightLabel = NewLabel(10, (uint8_t *)"Copyright 2012 STMicroelectronics", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      WebLabel = NewLabel(11,      (uint8_t *)"           www.st.com/stm32", GL_HORIZONTAL, GL_FONT_SMALL, GL_Blue, GL_FALSE);

      SystemInfoPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( SystemInfoPage, SYSTEM_INFO_PAGE );

      AddPageControlObj(307, 43, system_info, SystemInfoPage);

      AddPageControlObj(236, 60, BoardRevLabel, SystemInfoPage);
      AddPageControlObj(236, 76, CoreLabel, SystemInfoPage);
      AddPageControlObj(307, 100, DeviceLabel, SystemInfoPage);
      AddPageControlObj(307, 116, FwRevLabel, SystemInfoPage);
      AddPageControlObj(307, 132, CPUSpeedLabel, SystemInfoPage);
      AddPageControlObj(307, 148, FlashLabel, SystemInfoPage);
      AddPageControlObj(307, 164, CopyrightLabel, SystemInfoPage);
      AddPageControlObj(307, 184, WebLabel, SystemInfoPage);

      AddPageControlObj(195, 212, ExitEventButton, SystemInfoPage);

      GL_SetPageHeader( SystemInfoPage , (uint8_t *)"System Information");
    }
    break;

  case SETTINGS_PAGE:
    {
      MOD_GetParam(GLOBAL_SETTINGS_MEM , &Global_Config.d32);
      ExitEventButton = NewButton(1, (uint8_t *)"  OK  ", return_from_settings);
      SettingsPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( SettingsPage, SETTINGS_PAGE );

      AddPageControlObj(185, 212, ExitEventButton, SettingsPage);

      item = NewIcon (2, settings_icon, 50, 50, MOD_NullFunc);
      AddPageControlObj(185, 34, item, SettingsPage);

      item = NewCheckbox(3, (uint8_t *)"Enable Distant Control", GL_TRUE, EnableDistantControl);
      AddPageControlObj(260, 90, item , SettingsPage);
      GL_SetChecked(SettingsPage , 3 , Global_Config.b.DistantControlEnabled);

      if(GL_IsChecked (SettingsPage , 3))
      {
        item = NewCheckbox(4, (uint8_t *)"Enable Background mode", GL_FALSE, EnableBackgroundMode);
        AddPageControlObj(260, 112, item , SettingsPage);
        GL_SetChecked(SettingsPage , 4 , 1); 
        Global_Config.b.BackgroundModeEnabled = 1;
        MOD_SetParam(GLOBAL_SETTINGS_MEM , &Global_Config.d32);
      }
      else
      {
        item = NewCheckbox(4, (uint8_t *)"Enable Background mode", GL_TRUE, EnableBackgroundMode);
        AddPageControlObj(260, 112, item , SettingsPage);
        GL_SetChecked(SettingsPage , 4 , Global_Config.b.BackgroundModeEnabled);
      }

      item = NewCheckbox(5, (uint8_t *)"Enable low power mode", GL_FALSE, MOD_NullFunc);
      AddPageControlObj(260, 134, item , SettingsPage);
      GL_SetChecked(SettingsPage , 5 , Global_Config.b.LowPowerModeEnabled);

      item = NewCheckbox(6, (uint8_t *)"Enable LCD Power saving", GL_TRUE, MOD_NullFunc);
      AddPageControlObj(260, 156, item , SettingsPage);
      GL_SetChecked(SettingsPage , 6 , Global_Config.b.LCDPowerSavingEnabled);

      GL_SetPageHeader( SettingsPage , (uint8_t *)"Settings");
    }
    break;

  case UPGRADE_PAGE:
    {

      UpgradePage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( UpgradePage, UPGRADE_PAGE );

      ExitEventButton = NewButton(1, (uint8_t *)" Return ", return_from_upgrade);
      AddPageControlObj(130, 212, ExitEventButton, UpgradePage);

      item = NewButton(2, (uint8_t *)" Upgrade ", start_upgrade);
      AddPageControlObj(250, 212, item, UpgradePage);

      item = NewIcon (3, upgrade_icon, 50, 50, MOD_NullFunc);
      AddPageControlObj(185, 34, item, UpgradePage);

      item = NewIcon (4, error_icon, 50, 50, MOD_NullFunc);
      AddPageControlObj(310, 140, item, UpgradePage);

      item  = NewLabel(5, (uint8_t *)"Image source:", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(300, 107, item, UpgradePage);

      item  = NewLabel(6, (uint8_t *)"WARNING: Bootloader is about to be", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(258, 158, item, UpgradePage);

      item  = NewLabel(7, (uint8_t *)" launched, STM32 Demo will freeze.", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(258, 170, item, UpgradePage);

      item = NewComboBoxGrp(8);

      AddComboOption (item->objPTR, (uint8_t *)" Bootloader ", MOD_NullFunc);

#ifdef UPGRADE_FROM_MEDIA
      AddComboOption (item->objPTR, (uint8_t *)"USB Disk Flash", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"micro SD Flash", MOD_NullFunc);
#endif

      AddPageControlObj( 200, 100, item, UpgradePage);

      GL_SetPageHeader( UpgradePage , (uint8_t *)"Upgrade");
    }
    break;
    
 case SYSTEM_BACKGROUND_ERROR_PAGE:
    {
      SystemErrorPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( SystemErrorPage, SYSTEM_BACKGROUND_ERROR_PAGE );

      ExitEventButton= NewButton(2, (uint8_t *)"  OK  ",return_bkgnd_from_errorpage);
      AddPageControlObj(185,212,ExitEventButton,SystemErrorPage);

      item = NewIcon (3,error_icon,50,50,MOD_NullFunc);
      AddPageControlObj(185, 40, item, SystemErrorPage);

      GL_SetPageHeader(SystemErrorPage , (uint8_t *)"System : Warning");
      item  = NewLabel(4, (uint8_t *)"A Background process is already running, (uint8_t *)",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,100,item,SystemErrorPage);

      item  = NewLabel(5, (uint8_t *)"Please stop it and try again.",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,113,item,SystemErrorPage);
    }
    break;

 case SYSTEM_DISTANTCONTROL_ERROR_PAGE:
    {
      SystemErrorPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( SystemErrorPage, SYSTEM_DISTANTCONTROL_ERROR_PAGE );

      ExitEventButton= NewButton(2, (uint8_t *)"  OK  ",return_dcontrol_from_errorpage);
      AddPageControlObj(185,212,ExitEventButton,SystemErrorPage);

      item = NewIcon (3,error_icon,50,50,MOD_NullFunc);
      AddPageControlObj(185, 40, item, SystemErrorPage);

      GL_SetPageHeader(SystemErrorPage , (uint8_t *)"System : Warning");
      item  = NewLabel(4, (uint8_t *)"The Distant control is already Activated, (uint8_t *)",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,100,item,SystemErrorPage);

      item  = NewLabel(5, (uint8_t *)"Please stop it and try again.",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,113,item,SystemErrorPage);
    }
    break;

  default:
    break;
  }
}


/**
  * @brief  Display the system info page
  * @param  None
  * @retval None
  */
static void goto_system_info (void)
{
  SYSTEM_INFO_SwitchPage(SystemMainPage, SYSTEM_INFO_PAGE);
}

/**
  * @brief  return to the module main menu from the system info page
  * @param  None
  * @retval None
  */
static void return_from_system_info (void)
{
  SYSTEM_INFO_SwitchPage(SystemInfoPage, SYSTEM_MAIN_PAGE);
}

/**
  * @brief  return from the background error page to the seetings one
  * @param  None
  * @retval None
  */
static void return_bkgnd_from_errorpage (void)
{
   SYSTEM_INFO_SwitchPage(SystemErrorPage, SETTINGS_PAGE);
}

/**
  * @brief  return from the Distant control error page to the seetings one
  * @param  None
  * @retval None
  */
static void return_dcontrol_from_errorpage (void)
{
   SYSTEM_INFO_SwitchPage(SystemErrorPage, SETTINGS_PAGE);
}
/**
  * @brief  Display the settings page
  * @param  None
  * @retval None
  */
static void goto_Settings (void)
{
  SYSTEM_INFO_SwitchPage(SystemMainPage, SETTINGS_PAGE);
}

/**
  * @brief  return to the module main menu from the sttings page
  * @param  None
  * @retval None
  */
static void return_from_settings (void)
{
  save_settings();
  SYSTEM_INFO_SwitchPage(SettingsPage, SYSTEM_MAIN_PAGE);
}

/**
  * @brief  Save global setting in backup RAM
  * @param  None
  * @retval None
  */
static void save_settings (void)
{
  SYSTEM_CONFIG_TypeDef Global_ConfigBk; 

  Global_ConfigBk.d32 = Global_Config.d32; 

  Global_Config.b.DistantControlEnabled      = GL_IsChecked (SettingsPage , 3);
  Global_Config.b.BackgroundModeEnabled      = GL_IsChecked (SettingsPage , 4);
  Global_Config.b.LowPowerModeEnabled        = GL_IsChecked (SettingsPage , 5);
  Global_Config.b.LCDPowerSavingEnabled      = GL_IsChecked (SettingsPage , 6);

  if(Global_ConfigBk.d32 != Global_Config.d32)
  {
    MOD_SetParam(GLOBAL_SETTINGS_MEM , &Global_Config.d32);
    Global_Config.b.Configuration_Changed = 1;    
  }
  else
  {
    Global_Config.b.Configuration_Changed = 0;
  }
}
/**
  * @brief  Enable Distant Control radio box action
  * @param  None
  * @retval None
  */
static void EnableDistantControl(void)
{
 GL_PageControls_TypeDef* item;

  if(GL_IsChecked (SettingsPage , 3))
  {
    DestroyPageControl (SettingsPage, 4);

    item = NewCheckbox(4, (uint8_t *)"Enable Background mode", GL_FALSE, EnableBackgroundMode);
    AddPageControlObj(260, 112, item , SettingsPage);
    Global_Config.b.BackgroundModeEnabled = 1;
    save_settings();
    GL_SetChecked(SettingsPage , 4 , 1);
    RefreshPageControl( SettingsPage, 4);
  }
  else
  {
    if((EthernetSettings.DistantControlEnabled))
    {
      GL_SetChecked(SettingsPage , 3 , 1);
      RefreshPageControl( SettingsPage, 3);
      save_settings();
      SYSTEM_INFO_SwitchPage(SettingsPage, SYSTEM_DISTANTCONTROL_ERROR_PAGE);
    }
    else
    {

      DestroyPageControl (SettingsPage, 4);

      item = NewCheckbox(4, (uint8_t *)"Enable Background mode", GL_TRUE, EnableBackgroundMode);
      AddPageControlObj(260, 112, item , SettingsPage);

      GL_SetChecked(SettingsPage , 4 , Global_Config.b.BackgroundModeEnabled);
      RefreshPageControl( SettingsPage, 4);
    }
  }
}

/**
  * @brief  Enable Background Mode radio box action
  * @param  None
  * @retval None
  */
static void EnableBackgroundMode(void)
{
  if(GL_IsChecked (SettingsPage , 4) == 0)
  {
    if((AudioPlayerSettings.BackgroundEnabled) || (EthernetSettings.BackgroundEnabled))
    {
      GL_SetChecked(SettingsPage , 4 , 1);
      RefreshPageControl( SettingsPage, 4);
      save_settings();
      SYSTEM_INFO_SwitchPage(SettingsPage, SYSTEM_BACKGROUND_ERROR_PAGE);
    }
  }
}

/**
  * @brief  Display the upgrade page
  * @param  None
  * @retval None
  */
static void goto_upgrade (void)
{
  SYSTEM_INFO_SwitchPage(SystemMainPage, UPGRADE_PAGE);
}

/**
  * @brief  return to module main menu from the upgrade page
  * @param  None
  * @retval None
  */
static void return_from_upgrade (void)
{
  SYSTEM_INFO_SwitchPage(UpgradePage, SYSTEM_MAIN_PAGE);
}

/**
  * @brief  return to global main menu action
  * @param  None
  * @retval None
  */
static void return_to_menu (void)
{
  SYSTEM_INFO_SwitchPage(SystemMainPage, PAGE_MENU);
  SystemMainPage = NULL;
}

/**
  * @brief  Refresh the settings controls
  * @param  None
  * @retval None
  */
void SYSTEM_RefreshSetting (void)
{

  if(SettingsPage->Page_ID == SETTINGS_PAGE)
  {
    GL_SetChecked(SettingsPage , 3 , Global_Config.b.DistantControlEnabled);
    GL_SetChecked(SettingsPage , 4 , Global_Config.b.BackgroundModeEnabled);
    GL_SetChecked(SettingsPage , 5 , Global_Config.b.LowPowerModeEnabled);
    GL_SetChecked(SettingsPage , 6 , Global_Config.b.LCDPowerSavingEnabled);
    RefreshPageControl( SettingsPage, 3);
    RefreshPageControl( SettingsPage, 4);
    RefreshPageControl( SettingsPage, 5);
    RefreshPageControl( SettingsPage, 6);
  }
}
/**
  * @brief  Free usb host rersources on the USB FS core
  * @param  None
  * @retval None
  */
static void Free_USB_OTG_FS_Resources (void)
{
  extern USB_OTG_CORE_HANDLE  USB_OTG_Core;

  /* Free Host resources */
  f_mount(0, NULL);
  USB_OTG_DisableGlobalInt(&USB_OTG_Core) ;
  USB_OTG_StopHost(&USB_OTG_Core);
  USB_OTG_DriveVbus(&USB_OTG_Core, 0);

}

/**
  * @brief  Start upgrade process
  * @param  None
  * @retval None
  */
static void start_upgrade (void)
{
  uint8_t cnt = 0;

  GL_Clear(GL_White);
  GL_SetTextColor(GL_Black);
  GL_SetFont(GL_FONT_SMALL);
  GL_DrawButtonBMP( 210, 110, 148, 48, (uint8_t*) STM32Logo );

  for ( cnt = 0 ; cnt < 3; cnt ++)
  {
    GL_DisplayAdjStringLine(144 , 250, (uint8_t *)"Freeing USB FS Resources.    ", GL_FALSE);
    GL_Delay(100);
    GL_DisplayAdjStringLine(144 , 250, (uint8_t *)"Freeing USB FS Resources..   ", GL_FALSE);
    GL_Delay(100);
    GL_DisplayAdjStringLine(144 , 250, (uint8_t *)"Freeing USB FS Resources...  ", GL_FALSE);
    GL_Delay(100);
  }

    for ( cnt = 0 ; cnt < 3; cnt ++)
  {
    GL_DisplayAdjStringLine(144 , 250, (uint8_t *)"  Bootloader starting.       ", GL_FALSE);
    GL_Delay(100);
    GL_DisplayAdjStringLine(144 , 250, (uint8_t *)"  Bootloader starting..      ", GL_FALSE);
    GL_Delay(100);
    GL_DisplayAdjStringLine(144 , 250, (uint8_t *)"  Bootloader starting...     ", GL_FALSE);
    GL_Delay(100);
  }
  Free_USB_OTG_FS_Resources();
  GL_DisplayAdjStringLine(144 , 250, (uint8_t *)"  Bootloader running...      ", GL_FALSE);

  BL_Enabled = 1;
}

/**
  * @brief  Jump to bootloder
  * @param  None
  * @retval None
  */
void SYSTEM_JumpToBootloader (void)
{
  /* !!!! IMPORTANT !!!! THIS SHOULD BE LAUNCHED IN SUPERVISOR MODE 
  In our case this function is called in the systick interrupt handler */

  /* Reinitialize the Stack pointer*/
  __set_MSP(*(__IO uint32_t*) BL_ADDRESS);
  /* jump to application address */  
  ((pFunc) (*(__IO uint32_t*) (BL_ADDRESS + 4)))();

}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
