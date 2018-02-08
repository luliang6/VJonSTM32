/**
  ******************************************************************************
  * @file    mod_ethernet.c 
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   This file contains all Ethernet Module functions.
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
#include "mod_ethernet.h"
#include "mod_system.h"
#include "mod_audio.h"
#include "gl_ethernet_res.c"
#include "gl_mgr.h"
#include "mod_ethernet_config.h"
#include "eth_bsp.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ETHERNET_MAIN_PAGE               MOD_ETHERNET_UID + 0
#define ETHERNET_WEBSERVER_PAGE          MOD_ETHERNET_UID + 1
#define ETHERNET_DISTANTCONTROL_PAGE     MOD_ETHERNET_UID + 2
#define ETHERNET_SETTINGS                MOD_ETHERNET_UID + 3
#define ETHERNET_WARNING_PAGE            MOD_ETHERNET_UID + 4
#define ETHERNET_BCKMSG_PAGE             MOD_ETHERNET_UID + 5

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void ETHERNET_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex);
static void ETHERNET_CreatePage(uint8_t Page);
static void ETHERNET_Startup (void);
static void ETHERNET_Cleanup(void);

static void return_to_menu(void);
static void return_from_webserver(void);
static void return_from_settings(void);
static void return_from_distantcontrol(void);
static void goto_webserver(void);
static void goto_EthernetSettingss(void);
static void goto_distantcontrol(void);
static void InitEthernet(void);
static void return_from_warning (void);
static void disable_distant_control (void);
static void UpdateConnectionStatus (uint8_t status);
static void return_bkgnd_from_errorpage(void);
static void wUpdateConnectionStatus (void);

/* Private variables ---------------------------------------------------------*/
static GL_Page_TypeDef *EthernetMainPage;
static GL_Page_TypeDef *EthernetWebServerPage;
static GL_Page_TypeDef *EthernetDistantControlPage;
static GL_Page_TypeDef *EthernetSettingssPage;
static GL_Page_TypeDef *EthernetCurrentPage;
static GL_Page_TypeDef *EthernetWarningPage;

xTaskHandle  HTTP_Task_Handle = NULL;
xTaskHandle  TCPIP_Task_Handle = NULL;
xTaskHandle  ETH_Task_Handle = NULL;
xTaskHandle  DHCP_Task_Handle = NULL;
xSemaphoreHandle Ethernet_xSemaphore = NULL;

extern struct netif xnetif;
extern __IO uint32_t  EthStatus, EthSpeed;
extern __IO uint8_t EthLinkStatus;
extern xTaskHandle Task_Handle;
extern IPCAM_ImageBuffer_TypeDef IPCAM_ImageBuffer;

uint8_t *DHCPStatus[] =
{
  (uint8_t *)"Waiting for DHCP server reply",
  (uint8_t *)"Waiting for DHCP server reply.",
  (uint8_t *)"Waiting for DHCP server reply..",
  (uint8_t *)"Waiting for DHCP server reply...",
  (uint8_t *)"IP address assigned by DHCP server:",
  (uint8_t *)"DHCP timeout, Static IP address:",
  (uint8_t *)"Static IP address:",
};
uint8_t *EthInitStatus[] =
{
  (uint8_t *)"Ethernet initialization",
  (uint8_t *)"Ethernet initialization.",
  (uint8_t *)"Ethernet initialization..",
  (uint8_t *)"Ethernet initialization...",
};
MOD_InitTypeDef  mod_ethernet =
{
  MOD_ETHERNET_UID,
  MOD_ETHERNET_VER,
  (uint8_t *)"Ethernet",
  (uint8_t *)ethernet_icon,
  ETHERNET_Startup,
  NULL, 
  NULL,
};
ETHERNET_SETTINGS_TypeDef  EthCfg;
__IO uint8_t DHCPStatusIndex = 0;
uint8_t IPAddressAssigned[20];
uint8_t ETHERNET_CableConnectionStatus = 0;
static uint8_t LastInitMessageIndex = 0;
ETHERNET_SettingsTypeDef  EthernetSettings;

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Handle the Module startup action and display the main menu
  * @param  None
  * @retval None
  */
static void ETHERNET_Startup (void)
{
  vTaskSuspend(Core_Time_Task_Handle);
  
  if(EthernetSettings.BackgroundEnabled)
  {
    ETHERNET_SwitchPage(GL_HomePage, ETHERNET_WARNING_PAGE);
  }
  else
  {
    EthernetSettings.BackgroundEnabled = 0;
    EthernetSettings.DistantControlEnabled = 0;
    EthernetSettings.WebserverEnabled = 0;
    EthernetSettings.InitDone = 0;
    ETHERNET_SwitchPage(GL_HomePage, ETHERNET_MAIN_PAGE);
  }
}

/**
  * @brief  Close parent page and display a child sub-page
  * @param  pParent : parent page
  * @param  PageIndex : sub-page index
  * @retval None
  */
static void ETHERNET_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex)
{
  /* Switch to new page, and free previous one */
  GL_Page_TypeDef* NextPage = NULL;

  (*pParent).ShowPage(pParent, GL_FALSE);
  DestroyPage(pParent);
  free(pParent);
  pParent = NULL;

  if(PageIndex == PAGE_MENU)
  {
    GL_ShowMainMenu();
    return;
  }

  ETHERNET_CreatePage(PageIndex);

  switch (PageIndex)
  {
  case ETHERNET_MAIN_PAGE:
    NextPage = EthernetMainPage;
    break;

  case ETHERNET_WEBSERVER_PAGE:
    NextPage = EthernetWebServerPage;
    break;

  case ETHERNET_DISTANTCONTROL_PAGE:
    NextPage = EthernetDistantControlPage;
    break;

  case ETHERNET_SETTINGS:
    NextPage = EthernetSettingssPage;
    break;

  case ETHERNET_WARNING_PAGE:
  case ETHERNET_BCKMSG_PAGE:
    NextPage = EthernetWarningPage;
    break;

  default:
    break;
  }
  EthernetWebServerPage = NextPage;
  RefreshPage(NextPage);
}

/**
   * @brief  Create a child sub-page
   * @param  Page: Page handler
   * @retval None
  */
static void ETHERNET_CreatePage(uint8_t Page)
{
  GL_PageControls_TypeDef* item;
  GL_PageControls_TypeDef* BackButton;
  GL_PageControls_TypeDef* ConfirmButton;
  GL_PageControls_TypeDef* Message;
  GL_ComboBoxGrp_TypeDef* pTmp;
  uint8_t str_address[64];

  GL_SetBackColor( GL_White );
  GL_SetTextColor( GL_Blue );

  switch (Page)
  {
    case ETHERNET_MAIN_PAGE:
    {
      EthernetMainPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( EthernetMainPage, ETHERNET_MAIN_PAGE );
      item = NewIcon (5, ethernet_icon,50,50,MOD_NullFunc);
      AddPageControlObj(185,40,item,EthernetMainPage);

      if((Global_Config.b.DistantControlEnabled != 0) && (Global_Config.b.BackgroundModeEnabled != 0))
      {
        GL_SetMenuItem(EthernetMainPage, (uint8_t *)"Web Server",0,goto_webserver );
        GL_SetMenuItem(EthernetMainPage, (uint8_t *)"Distant Control",1,goto_distantcontrol);
        GL_SetMenuItem(EthernetMainPage, (uint8_t *)"Settings",2,goto_EthernetSettingss );
        GL_SetMenuItem(EthernetMainPage, (uint8_t *)"Return",3,return_to_menu );
      }
      else
      {
        GL_SetMenuItem(EthernetMainPage, (uint8_t *)"Web Server",0,goto_webserver );
        GL_SetMenuItem(EthernetMainPage, (uint8_t *)"Settings",1,goto_EthernetSettingss );
        GL_SetMenuItem(EthernetMainPage, (uint8_t *)"Return",2,return_to_menu );
      }
      GL_SetPageHeader(EthernetMainPage , (uint8_t *)"Ethernet Menu");
    }
    break;

    case ETHERNET_WEBSERVER_PAGE:
    {
      EthernetWebServerPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( EthernetWebServerPage, ETHERNET_WEBSERVER_PAGE );

      GL_SetPageHeader(EthernetWebServerPage , (uint8_t *)"Web Server");

      item = NewIcon (1,ethernet_disconn_icon,50,50,MOD_NullFunc);
      AddPageControlObj(185,40,item,EthernetWebServerPage);

      BackButton= NewButton(2, (uint8_t *)" Return ",return_from_webserver);
      AddPageControlObj(195, 212, BackButton, EthernetWebServerPage);

      Message  = NewLabel(3, (uint8_t *)" ",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black,GL_FALSE);
      AddPageControlObj(290,113,Message,EthernetWebServerPage);

      Message  = NewLabel(4, (uint8_t *)" ",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black,GL_FALSE);
      AddPageControlObj(290,126,Message,EthernetWebServerPage);

      Message  = NewLabel(5, (uint8_t *)" ",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black,GL_FALSE);
      AddPageControlObj(290,139,Message,EthernetWebServerPage);

      Message  = NewLabel(6, (uint8_t *)" ",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black,GL_FALSE);
      AddPageControlObj(290,160,Message,EthernetWebServerPage);

      Message  = NewLabel(7, (uint8_t *)" ",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black,GL_FALSE);
      AddPageControlObj(290,173,Message,EthernetWebServerPage);

      Message  = NewLabel(8, (uint8_t *)" ",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black,GL_FALSE);
      AddPageControlObj(290,186,Message,EthernetWebServerPage);

      EthernetWebServerPage->CustomPostDraw = InitEthernet;
    }
    break;

    case ETHERNET_DISTANTCONTROL_PAGE:
    {
      EthernetDistantControlPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( EthernetDistantControlPage, ETHERNET_DISTANTCONTROL_PAGE );

      GL_SetPageHeader(EthernetDistantControlPage , (uint8_t *)"Distant Control");

      item = NewIcon (1,ethernet_conn_icon,50,50,MOD_NullFunc);
      AddPageControlObj(185,40,item,EthernetDistantControlPage);

      BackButton= NewButton(2, (uint8_t *)" Return ",return_from_distantcontrol);
      AddPageControlObj(195, 212, BackButton, EthernetDistantControlPage);

      Message  = NewLabel(3, (uint8_t *)" ",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black,GL_FALSE);
      AddPageControlObj(290,113,Message,EthernetDistantControlPage);

      Message  = NewLabel(4, (uint8_t *)" ",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black,GL_FALSE);
      AddPageControlObj(290,126,Message,EthernetDistantControlPage);

      Message  = NewLabel(5, (uint8_t *)" ",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black,GL_FALSE);
      AddPageControlObj(290,139,Message,EthernetDistantControlPage);

      Message  = NewLabel(6, (uint8_t *)" ",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black,GL_FALSE);
      AddPageControlObj(290,160,Message,EthernetDistantControlPage);

      EthernetDistantControlPage->CustomPostDraw = InitEthernet;
    }
    break;

    case ETHERNET_SETTINGS:
    {
      EthernetSettingssPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( EthernetSettingssPage, ETHERNET_SETTINGS );
      item = NewIcon (1,ethernet_settings_icon,50,50,MOD_NullFunc);
      AddPageControlObj(185,40,item,EthernetSettingssPage);
      BackButton= NewButton(2, (uint8_t *)" Return ",return_from_settings);
      AddPageControlObj(195, 212, BackButton, EthernetSettingssPage);

      item= NewCheckbox(3 , (uint8_t *)"Enable DHCP Client", GL_TRUE, MOD_NullFunc);
      AddPageControlObj(260, 105, item, EthernetSettingssPage);

      item  = NewLabel(4, (uint8_t *)"Image Format:",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black,GL_FALSE);
      AddPageControlObj(260,145,item,EthernetSettingssPage);

      item = NewComboBoxGrp(5);
      AddComboOption (item->objPTR, (uint8_t *)"BMP QQVGA", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"BMP QVGA", MOD_NullFunc);
      AddPageControlObj( 260, 160, item, EthernetSettingssPage);

      GL_SetPageHeader(EthernetSettingssPage , (uint8_t *)"Ethernet Settings Menu");

      MOD_GetParam(3, &EthCfg.d32);
      
      if(EthCfg.b.DHCPEnable == 1)
      {
        GL_SetChecked(EthernetSettingssPage, 3, EthCfg.b.DHCPEnable);
      }

      pTmp = (GL_ComboBoxGrp_TypeDef*)(item->objPTR);
      if((EthCfg.b.SelectedImageFormat > 0) && (EthCfg.b.SelectedImageFormat < 3))
      {
        pTmp->ComboOptions[0]->IsActive = GL_FALSE;
        pTmp->ComboOptions[EthCfg.b.SelectedImageFormat -1]->IsActive = GL_TRUE;
      }
    }
    break;
  case ETHERNET_WARNING_PAGE:
    {
      EthernetWarningPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( EthernetWarningPage, ETHERNET_WARNING_PAGE );


      BackButton= NewButton(2, (uint8_t *)" Cancel ",return_from_warning);
      AddPageControlObj(230, 212,BackButton,EthernetWarningPage);

      ConfirmButton= NewButton(3, (uint8_t *)"Disable",disable_distant_control);
      AddPageControlObj(150, 212,ConfirmButton,EthernetWarningPage);

      item = NewIcon (4,error_icon,50,50,MOD_NullFunc);
      AddPageControlObj(185, 40, item, EthernetWarningPage);

      GL_SetPageHeader(EthernetWarningPage , (uint8_t *)"Ethernet : warning");
      item  = NewLabel(5, (uint8_t *)"The Distant control feature is currently",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,100,item,EthernetWarningPage);

      item  = NewLabel(6, (uint8_t *)"enabled. You need to disable the used",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,113,item,EthernetWarningPage);

      item  = NewLabel(7, (uint8_t *)"processes and try later. do you want to",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,126,item,EthernetWarningPage);

      item  = NewLabel(8, (uint8_t *)"disable the Distant control feature?",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,139,item,EthernetWarningPage);

      if(ETHERNET_CableConnectionStatus == 0)
      {
        sprintf((char *)str_address, "[IP Address : %s]",IPAddressAssigned);

        item  = NewLabel(9,str_address,GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_FALSE);
      }
      else
      {
         item  = NewLabel(9, (uint8_t *)"Network Cable is unplugged !!!",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_FALSE);
      }
      AddPageControlObj(310,160,item,EthernetWarningPage); 

      EthernetWarningPage->CustomPostDraw = wUpdateConnectionStatus;

    }
    break;

  case ETHERNET_BCKMSG_PAGE:
    {
      EthernetWarningPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( EthernetWarningPage, ETHERNET_BCKMSG_PAGE );

      item= NewButton(2, (uint8_t *)"  OK  ",return_bkgnd_from_errorpage);
      AddPageControlObj(185,212,item,EthernetWarningPage);

      item = NewIcon (3,error_icon,50,50,MOD_NullFunc);
      AddPageControlObj(185, 40, item, EthernetWarningPage);

      GL_SetPageHeader(EthernetWarningPage , (uint8_t *)"System : Warning");
      item  = NewLabel(4, (uint8_t *)"A Background process is already running, (uint8_t *)",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,100,item,EthernetWarningPage);

      item  = NewLabel(5, (uint8_t *)"Background mode can not be disabled.",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,113,item,EthernetWarningPage);
    }
    default:
    break;
  }
}

/**
  * @brief  Draw the Ethernet page backgound
  * @param  None
  * @retval None
  */
static void InitEthernet(void)
{
  uint8_t temp[45];
  uint8_t buf[30];

  if( EthernetSettings.InitDone == 0)
  {
  MOD_GetParam(3 , &EthCfg.d32);
  EthernetSettings.InitDone  = 1;
  if(EthernetSettings.WebserverEnabled == 1)
  {
    EthernetCurrentPage = EthernetWebServerPage;
  }
  else if(EthernetSettings.DistantControlEnabled == 1)
  {
    EthernetCurrentPage = EthernetDistantControlPage;
  }

  if(TCPIP_Task_Handle != NULL)
  {
    if(xTaskIsTaskSuspended(TCPIP_Task_Handle) == pdTRUE)
    {
      vTaskResume(TCPIP_Task_Handle);
    }
  }

  if (EthernetSettings.BackgroundEnabled == 0)
  {
    memset (temp, ' ', 45);

    if(EthStatus == 0)
    {
      ETH_BSP_Config();

      if(EthStatus == 1) /* Network cable is connected */
      {
        EthLinkStatus = 0;
        RefreshLabel(EthernetCurrentPage, 3, temp);  
        RefreshLabel(EthernetCurrentPage, 3, (uint8_t *)"Ethernet initialization..OK");
        LastInitMessageIndex = 1;
      }
      else /* Network cable is unplugged */
      {
        RefreshLabel(EthernetCurrentPage, 3, temp);
        RefreshLabel(EthernetCurrentPage, 4, temp);
        RefreshLabel(EthernetCurrentPage, 5, temp);
        RefreshLabel(EthernetCurrentPage, 3, (uint8_t *)"Ethernet initialization failed, Please");
        RefreshLabel(EthernetCurrentPage, 4, (uint8_t *)"verify if the Ethernet cable is connected");
        RefreshLabel(EthernetCurrentPage, 5, (uint8_t *)"and try again.");
        LastInitMessageIndex = 2;
        return;
      }
    }
    if(EthLinkStatus == 0) /* Network Cable is connected */
    {
      ETHERNET_UpdateIcon(ethernet_conn_icon);
      sprintf((char*)buf, "Ethernet Speed: %d.0Mbps", (int)EthSpeed);
      RefreshLabel(EthernetCurrentPage, 3, temp);
      RefreshLabel(EthernetCurrentPage, 3, buf);
      LastInitMessageIndex = 3;
    }
    else
    {
      ETHERNET_UpdateIcon(ethernet_disconn_icon);
      RefreshLabel(EthernetCurrentPage, 4, temp);
      RefreshLabel(EthernetCurrentPage, 5, temp);
      RefreshLabel(EthernetCurrentPage, 4, (uint8_t *)"Please verify if the Ethernet cable ");
      RefreshLabel(EthernetCurrentPage, 5, (uint8_t *)"is connected and try again. ");
      LastInitMessageIndex = 4;
      return;
    }
    LwIP_Init();
    http_init();
    if(EthCfg.b.DHCPEnable == 1)
    {
      if(DHCP_Task_Handle == NULL)
      {
        xTaskCreate(LwIP_DHCP_task, (signed char const*)"DHCP_P", DHCP_THREAD_STACK_SIZE, NULL, DHCP_THREAD_PRIO, &DHCP_Task_Handle);
      }
    }
    else
    {
      vTaskPrioritySet(Task_Handle, (configMAX_PRIORITIES - 7));//5//7
    }

    if(EthernetSettings.WebserverEnabled == 1)
    {
      if(EthCfg.b.SelectedImageFormat == 1)
      {
        IPCAM_ImageBuffer.ImageFormat = BMP_QQVGA;
      }
      else if(EthCfg.b.SelectedImageFormat == 2)
      {
        IPCAM_ImageBuffer.ImageFormat = BMP_QVGA;
      }
      else
      {
        IPCAM_ImageBuffer.ImageFormat = BMP_QQVGA;
      }
      IPCAM_ImageCaptureInit();
    }
  }
  else
  {
    LastInitMessageIndex = 5;
    RefreshLabel(EthernetCurrentPage, 4, temp);
    RefreshLabel(EthernetCurrentPage, 4, DHCPStatus[DHCPStatusIndex]);
    RefreshLabel(EthernetCurrentPage, 5, temp);
    RefreshLabel(EthernetCurrentPage, 5, IPAddressAssigned);
    if (EthernetSettings.DistantControlEnabled == 1)
    {
      vTaskDelay(1000);
      EthernetSettings.BackgroundEnabled = 1;
      ETHERNET_SwitchPage(EthernetDistantControlPage, PAGE_MENU);
      EthernetDistantControlPage = NULL;
    }
  }
 }
 else
 {
   switch(LastInitMessageIndex)
   {
   case 1:
     RefreshLabel(EthernetCurrentPage, 3, (uint8_t *)"                                        ");  
     RefreshLabel(EthernetCurrentPage, 3, (uint8_t *)"Ethernet initialization..OK");
     break;
   case 2:
     RefreshLabel(EthernetCurrentPage, 3, (uint8_t *)"                                        ");
     RefreshLabel(EthernetCurrentPage, 4, (uint8_t *)"                                        ");
     RefreshLabel(EthernetCurrentPage, 5, (uint8_t *)"                                        ");
     RefreshLabel(EthernetCurrentPage, 3, (uint8_t *)"Ethernet initialization failed, Please");
     RefreshLabel(EthernetCurrentPage, 4, (uint8_t *)"verify if the Ethernet cable is connected");
     RefreshLabel(EthernetCurrentPage, 5, (uint8_t *)"and try again.");
     break;
   case 3: 
     ETHERNET_UpdateIcon(ethernet_conn_icon);
     sprintf((char*)buf, "Ethernet Speed: %d.0Mbps", (int)EthSpeed);
     RefreshLabel(EthernetCurrentPage, 3, (uint8_t *)"                                        ");
     RefreshLabel(EthernetCurrentPage, 3, buf);
     break;
   case 4: 
     ETHERNET_UpdateIcon(ethernet_disconn_icon);
     RefreshLabel(EthernetCurrentPage, 4, (uint8_t *)"                                        ");
     RefreshLabel(EthernetCurrentPage, 5, (uint8_t *)"                                        ");
     RefreshLabel(EthernetCurrentPage, 4, (uint8_t *)"Please verify if the Ethernet cable     ");
     RefreshLabel(EthernetCurrentPage, 5, (uint8_t *)"is connected and try again.             ");
     break;
   case 5:
     RefreshLabel(EthernetCurrentPage, 4, (uint8_t *)"                                        ");
     RefreshLabel(EthernetCurrentPage, 4, DHCPStatus[DHCPStatusIndex]);
     RefreshLabel(EthernetCurrentPage, 5, (uint8_t *)"                                        ");
     RefreshLabel(EthernetCurrentPage, 5, IPAddressAssigned);

     if(IPaddress !=0)
     {
       sprintf((char*)temp, "%s",IPAddressAssigned);
       ETHERNET_UpdateDHCPState(4,temp);
       ETHERNET_UpdateIcon(ethernet_conn_icon);
     }
     break;

   case 6:
     RefreshLabel(EthernetCurrentPage ,6 , (uint8_t *)"Network Cable is unplugged !!!");
     break;

   case 7:
     RefreshLabel(EthernetCurrentPage, 6, (uint8_t *)"Network Cable is now connected");     
     break;
   }
 }
}

/**
  * @brief  Display Webserver page
  * @param  None
  * @retval None
  */
static void goto_webserver(void)
{
  EthernetSettings.WebserverEnabled = 1; 
  EthernetSettings.DisableDControlLater = 0; 
  EthernetSettings.DisableBackgroundLater = 0;
  EthernetSettings.InitDone = 0;
  ETHERNET_SwitchPage(EthernetMainPage, ETHERNET_WEBSERVER_PAGE);
  EthernetMainPage = NULL;
}

/**
  * @brief  Display Settings page
  * @param  None
  * @retval None
  */
static void goto_EthernetSettingss(void)
{
  ETHERNET_SwitchPage(EthernetMainPage, ETHERNET_SETTINGS);
  EthernetMainPage = NULL;
}


/**
  * @brief  Return from error page
  * @param  None
  * @retval None
  */
static void return_bkgnd_from_errorpage(void)
{
  ETHERNET_Cleanup();
  ETHERNET_SwitchPage(EthernetWarningPage, ETHERNET_MAIN_PAGE);
  EthernetWarningPage = NULL;
}

/**
  * @brief  Display Distant Control page
  * @param  None
  * @retval None
  */
static void goto_distantcontrol(void)
{
  EthernetSettings.DistantControlEnabled   = 1;
  ETHERNET_SwitchPage(EthernetMainPage, ETHERNET_DISTANTCONTROL_PAGE);
  EthernetMainPage = NULL;
}

/**
  * @brief  Update the Connection Status in th warning page
  * @param  None
  * @retval None
  */
static void wUpdateConnectionStatus (void)
{
  uint8_t str_address[64];

  RefreshLabel (EthernetWarningPage, 9 , (uint8_t *)"                              ") ;
  if(ETHERNET_CableConnectionStatus == 1)
  {
    RefreshLabel(EthernetCurrentPage ,9 , (uint8_t *)"Network Cable is unplugged !!!");
  }
  else
  {
    sprintf((char *)str_address, "[IP Address : %s]",IPAddressAssigned); 
    RefreshLabel(EthernetCurrentPage, 9, str_address);
  }
}

/**
  * @brief  Return to global main menu action
  * @param  None
  * @retval None
  */
static void return_to_menu(void)
{
  ETHERNET_SwitchPage(EthernetMainPage, PAGE_MENU);
  EthernetMainPage = NULL;
  EthStatus = 0;
}

/**
  * @brief  Return the module main menu from Webserver page
  * @param  None
  * @retval None
  */
static void return_from_webserver(void)
{
  EthernetSettings.WebserverEnabled = 0;
  EthernetSettings.InitDone = 0;
  ETH_Stop();

  vTaskPrioritySet(Task_Handle, (configMAX_PRIORITIES - 1));
  if(ETH_Task_Handle != NULL)
  {
    vTaskDelete(ETH_Task_Handle);
    ETH_Task_Handle = NULL;
  }

  if (Ethernet_xSemaphore != NULL)
  {
    vQueueDelete( Ethernet_xSemaphore );
    Ethernet_xSemaphore = NULL;
  }

  if(TCPIP_Task_Handle != NULL)
  {
    vTaskSuspend(TCPIP_Task_Handle);
  }

  if(HTTP_Task_Handle != NULL)
  {
    vTaskDelete(HTTP_Task_Handle);
    HTTP_Task_Handle = NULL;
  }
  if(DHCP_Task_Handle != NULL)
  {
    vTaskDelete(DHCP_Task_Handle);
    DHCP_Task_Handle = NULL;
  }

  DMA_Cmd(DMA2_Stream1, DISABLE);
  DCMI_Cmd(DISABLE);
  DCMI_CaptureCmd(DISABLE);

  netif_remove(&xnetif);

  ETHERNET_SwitchPage(EthernetWebServerPage, ETHERNET_MAIN_PAGE);
  EthernetWebServerPage = NULL;
}

/**
  * @brief  Return the module main menu from Distant Control page
  * @param  None
  * @retval None
  */
static void return_from_distantcontrol(void)
{
  ETHERNET_Cleanup();
  ETHERNET_SwitchPage(EthernetDistantControlPage, ETHERNET_MAIN_PAGE);
  EthernetDistantControlPage = NULL;
}

/**
  * @brief  Return the module main menu from Settings page
  * @param  None
  * @retval None
  */
static void return_from_settings(void)
{
  EthCfg.b.DHCPEnable  = GL_IsChecked (EthernetSettingssPage , 3);
  EthCfg.b.SelectedImageFormat = GetComboOptionActive(EthernetSettingssPage, 5);
  MOD_SetParam(3 , &EthCfg.d32);

  ETHERNET_SwitchPage(EthernetSettingssPage, ETHERNET_MAIN_PAGE);
  EthernetSettingssPage = NULL;
}

/**
  * @brief  disable distant control and return to ethernet menu
  * @param  None
  * @retval None
  */
static void disable_distant_control (void)
{

  if(EthernetSettings.DisableDControlLater == 1)
  {
    Global_Config.b.DistantControlEnabled = 0;
    EthernetSettings.DisableDControlLater = 0;
    MOD_SetParam(GLOBAL_SETTINGS_MEM , &Global_Config.d32);
    Global_Config.b.Configuration_Changed = 1;  
  }

  if(EthernetSettings.DisableBackgroundLater == 1)
  {

    if(AudioPlayerSettings.BackgroundEnabled)
    {
      ETHERNET_SwitchPage(EthernetWarningPage, ETHERNET_BCKMSG_PAGE);
      return;
    }
    else
    {
      Global_Config.b.DistantControlEnabled = 0;
      MOD_SetParam(GLOBAL_SETTINGS_MEM , &DC_Global_Config.d32);
      Global_Config.b.Configuration_Changed = 1;
    }

    EthernetSettings.DisableBackgroundLater = 0;
  }
  ETHERNET_Cleanup();
  ETHERNET_SwitchPage(EthernetWarningPage, ETHERNET_MAIN_PAGE);
  EthernetWarningPage = NULL;
}

/**
  * @brief  Clean up th ethernet services and free resources
  * @param  None
  * @retval None
  */
static void ETHERNET_Cleanup(void)
{
  EthernetSettings.DistantControlEnabled = 0;
  EthernetSettings.BackgroundEnabled = 0;
  EthernetSettings.InitDone = 0;
  ETH_Stop();

  vTaskPrioritySet(Task_Handle, (configMAX_PRIORITIES - 1));
  if(ETH_Task_Handle != NULL)
  {
    vTaskDelete(ETH_Task_Handle);
    ETH_Task_Handle = NULL;
  }

  vQueueDelete( Ethernet_xSemaphore );
  Ethernet_xSemaphore = NULL;

  if(TCPIP_Task_Handle != NULL)
  {
    vTaskSuspend(TCPIP_Task_Handle);
  }

  if(HTTP_Task_Handle != NULL)
  {
    vTaskDelete(HTTP_Task_Handle);
    HTTP_Task_Handle = NULL;
  }
  if(DHCP_Task_Handle != NULL)
  {
    vTaskDelete(DHCP_Task_Handle);
    DHCP_Task_Handle = NULL;
  }
  netif_remove(&xnetif);
}

/**
  * @brief  Update Ethernet connection state
  * @param  conn_state: New state
  * @retval None
*/
void ETHERNET_UpdateConnState(uint8_t conn_state)
{
  uint8_t temp[45];

  ETHERNET_CableConnectionStatus = conn_state;
  UpdateConnectionStatus(conn_state);

  if(conn_state == 1)
  {
    LastInitMessageIndex = 6;
  }
  else
  {
    LastInitMessageIndex = 7;
  }

  if(EthernetCurrentPage != NULL)
  {
    if((EthernetCurrentPage->Page_ID == ETHERNET_WEBSERVER_PAGE)||
       (EthernetCurrentPage->Page_ID == ETHERNET_DISTANTCONTROL_PAGE))
    {
      memset (temp, ' ', 45);
      RefreshLabel (EthernetCurrentPage ,6 , temp);
      if(conn_state == 1)
      {
        RefreshLabel(EthernetCurrentPage ,6 , (uint8_t *)"Network Cable is unplugged !!!");
      }
      else
      {
        RefreshLabel(EthernetCurrentPage, 6, (uint8_t *)"Network Cable is now connected");
      }
    }
  }
  
}

/**
  * @brief  Update DHCP process state.
  * @param  index: Index to DHCPStatus table
  * @param  ip_add: New IP address
  * @retval None
  */
void ETHERNET_UpdateDHCPState(uint8_t index, uint8_t *ip_add)
{
  uint8_t temp[45];

  if((EthernetCurrentPage->Page_ID == ETHERNET_WEBSERVER_PAGE)||
     (EthernetCurrentPage->Page_ID == ETHERNET_DISTANTCONTROL_PAGE))
  {
    memset (temp, ' ', 45);
    RefreshLabel(EthernetCurrentPage, 4, temp);
    RefreshLabel(EthernetCurrentPage, 4, DHCPStatus[index]);
    DHCPStatusIndex = index;

    if(ip_add != NULL)
    {
      memcpy(IPAddressAssigned, ip_add, strlen((const char*)ip_add)) ;
      RefreshLabel(EthernetCurrentPage, 5, temp);
      RefreshLabel(EthernetCurrentPage, 5, IPAddressAssigned);
      if (EthernetSettings.DistantControlEnabled == 1)
      {
        vTaskDelay(1000);
        EthernetSettings.BackgroundEnabled = 1;
        ETHERNET_SwitchPage(EthernetDistantControlPage, PAGE_MENU);
        EthernetDistantControlPage = NULL;
      }
    }
  }
}

/**
  * @brief  DHCP process failed.
  * @param  None
  * @retval None
  */
void ETHERNET_DHCPFailState(void)
{
  uint8_t temp[45];
  memset (temp, ' ', 45);

  if((EthernetCurrentPage->Page_ID == ETHERNET_WEBSERVER_PAGE)||
     (EthernetCurrentPage->Page_ID == ETHERNET_DISTANTCONTROL_PAGE))
  {
    RefreshLabel(EthernetCurrentPage, 4, temp);
    RefreshLabel(EthernetCurrentPage, 5, temp);
    RefreshLabel(EthernetCurrentPage, 4, (uint8_t *)"DHCP service failed, Please connect");
    RefreshLabel(EthernetCurrentPage, 5, (uint8_t *)"the Ethernet cable and tray again. ");
  }
}

/**
  * @brief  Update module icon.
  * @param  new_icon: New icon
  * @retval None
  */
void ETHERNET_UpdateIcon(const uint8_t *new_icon)
{
  GL_PageControls_TypeDef* item;

  if((EthernetCurrentPage->Page_ID == ETHERNET_WEBSERVER_PAGE)||
     (EthernetCurrentPage->Page_ID == ETHERNET_DISTANTCONTROL_PAGE))
  {
    DestroyPageControl(EthernetCurrentPage, 1);
    item = NewIcon (1,new_icon,50,50,MOD_NullFunc);
    AddPageControlObj(185,40,item,EthernetCurrentPage);
    RefreshPageControl(EthernetCurrentPage, 1);
  }
}

/**
  * @brief  Update the Ethernet initialization progress status.
  * @param  None
  * @retval None
  */
void ETHERNET_UpdateInitDisplay (void)
{
  static uint8_t index = 0;
  uint8_t temp[45];

  if((EthernetCurrentPage->Page_ID == ETHERNET_WEBSERVER_PAGE)||
     (EthernetCurrentPage->Page_ID == ETHERNET_DISTANTCONTROL_PAGE))
  {
    memset (temp, ' ', 45);
    index++;
    if(index == 4)
    {
      index = 0;
    }
    RefreshLabel(EthernetCurrentPage, 3, temp);
    RefreshLabel(EthernetCurrentPage, 3, EthInitStatus[index]);
  }
  STM_EVAL_LEDToggle(LED1);
//   STM_EVAL_LEDToggle(LED2);
//   STM_EVAL_LEDToggle(LED3);
//   STM_EVAL_LEDToggle(LED4);
  vTaskDelay(200);
}

/**
  * @brief  Update the Camera initialization progress status.
  * @param  None
  * @retval None
  */
void ETHERNET_UpdateCameraInit (void)
{
  if(EthernetWebServerPage->Page_ID == ETHERNET_WEBSERVER_PAGE)
  {
    RefreshLabel(EthernetWebServerPage, 7, (uint8_t *)"Camera initialization failed, Please ");
    RefreshLabel(EthernetWebServerPage, 8, (uint8_t *)"check the camera HW and try again.");
  }
}

/**
  * @brief  Return to global main menu action
  * @param  None
  * @retval None
  */
static void return_from_warning (void)
{
  ETHERNET_SwitchPage(EthernetWarningPage, PAGE_MENU);
  EthernetWarningPage = NULL;
}

/**
  * @brief  Return to global main menu action
  * @param  None
* @retval None
*/
static void UpdateConnectionStatus (uint8_t status)
{
  uint8_t str_address[64];

  if(EthernetSettings.DistantControlEnabled)
  {

    if(EthernetWarningPage != NULL)
    {
      if(EthernetWarningPage->Page_ID == ETHERNET_WARNING_PAGE)
      {

        //1- Notification 
        if(status == 1)
        {
          RefreshLabel (EthernetWarningPage, 9 , (uint8_t *)"Network Cable is unplugged !!!") ;
        }
        else
        {
          RefreshLabel (EthernetWarningPage, 9 , (uint8_t *)"                              ") ;
          sprintf((char *)str_address, "[IP Address : %s]",IPAddressAssigned);
          RefreshLabel ( EthernetWarningPage,9 ,str_address);
        }
      }
    }
  }
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
