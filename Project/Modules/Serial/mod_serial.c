/**
  ******************************************************************************
  * @file    mod_serial.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   This file contains all Serial Module functions.
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
#include "gl_serial_res.c"
#include "mod_serial.h"
#include "mod_audio.h"
#include "mod_filemgr.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SERIAL_INFO_PAGE               MOD_SERIAL_UID + 0  
#define SERIAL_MAIN_PAGE               MOD_SERIAL_UID + 1
#define SERIAL_PAGE                    MOD_SERIAL_UID + 2
#define SERIAL_SETTINGS                MOD_SERIAL_UID + 3

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SERIAL_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex);
static void SERIAL_CreatePage(uint8_t Page);
static void SERIAL_Startup (void);
static void return_to_menu(void);
static void return_from_serial(void);
static void return_from_settings(void);
static void return_from_warning (void);
static void goto_serial(void);
static void goto_serialsettings(void);
static void goto_main (void);

static void Serial_PostDraw (void);
static void Serial_DisplayStringLine(uint16_t Line, uint8_t *ptr);

/* Private variables ---------------------------------------------------------*/
static GL_Page_TypeDef *SerialMainPage;
static GL_Page_TypeDef *SerialPage;
static GL_Page_TypeDef *SerialInfoPage;
static GL_Page_TypeDef *SerialSettingsPage;
MOD_InitTypeDef  mod_serial =
{
  MOD_SERIAL_UID,
  MOD_SERIAL_VER,
  (uint8_t *)"Serial",
  (uint8_t *)serial_icon,
  SERIAL_Startup,
  NULL, 
  NULL,
};
SERIAL_SETTINGS_TypeDef  SerialCfg;

__IO uint8_t SerialTxBuffer[MAXBUFFERSIZE] = {0};
__IO uint8_t  MajFlag = 0;
__IO uint32_t TxBufferIndex = 0;

const uint32_t SERIAL_BaudRate[6] = {4800, 9600, 19200, 38400, 57600, 115200};
const uint16_t SERIAL_DataLength[2] = {USART_WordLength_8b, USART_WordLength_9b};
const uint16_t SERIAL_StopBits[4] = {USART_StopBits_0_5, USART_StopBits_1, 
                                     USART_StopBits_1_5, USART_StopBits_2};
const uint16_t SERIAL_Parity[4] = {USART_Parity_No, USART_Parity_Even, USART_Parity_Odd};
const uint16_t SERIAL_Mode[2] = {USART_Mode_Rx, USART_Mode_Tx};
const uint16_t SERIAL_HardwareFlowControl[4] = {USART_HardwareFlowControl_None, 
                                                USART_HardwareFlowControl_RTS,
                                                USART_HardwareFlowControl_CTS,
                                                USART_HardwareFlowControl_RTS_CTS};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Handle the Module startup action and display the main menu
  * @param  None
  * @retval None
  */
static void SERIAL_Startup (void)
{
  vTaskSuspend(Core_Time_Task_Handle);
  SERIAL_SwitchPage(GL_HomePage, SERIAL_INFO_PAGE);
}

/**
  * @brief  Close parent page and display a child sub-page
  * @param  pParent : parent page
  * @param  PageIndex : sub-page index
  * @retval None
  */
static void SERIAL_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex)
{
  /* Switch to new page, and free previous one */
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
  
  SERIAL_CreatePage(PageIndex);
  
  switch (PageIndex)
  {
  case SERIAL_INFO_PAGE:
    NextPage = SerialInfoPage;
    break;
    
  case SERIAL_MAIN_PAGE:
    NextPage = SerialMainPage;
    break;
    
  case SERIAL_PAGE:
    NextPage = SerialPage;
    break;
    
  case SERIAL_SETTINGS:
    NextPage = SerialSettingsPage;
    break;
     
  default:
    break;
  }
  RefreshPage(NextPage);
}

/**
  * @brief  Create a child sub-page
  * @param  Page: Page handler
  * @retval None
  */
static void SERIAL_CreatePage(uint8_t Page)
{
  GL_PageControls_TypeDef* item;
  GL_PageControls_TypeDef* BackButton;
  GL_PageControls_TypeDef* ConfirmButton;  
  GL_ComboBoxGrp_TypeDef* pTmp;
  USART_InitTypeDef USART_InitStructure;

  GL_SetBackColor( GL_White );
  GL_SetTextColor( GL_Blue );

  switch (Page)
  {
  case SERIAL_INFO_PAGE: 
    {
      SerialInfoPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( SerialInfoPage, SERIAL_INFO_PAGE ); 

      BackButton= NewButton(2, (uint8_t *)" Cancel ",return_from_warning);
      AddPageControlObj(230, 212,BackButton,SerialInfoPage);

      ConfirmButton= NewButton(3, (uint8_t *)"Continue",goto_main);
      AddPageControlObj(150, 212,ConfirmButton,SerialInfoPage);

      item = NewIcon (4,error_icon,50,50,MOD_NullFunc);
      AddPageControlObj(185, 40, item, SerialInfoPage);

      GL_SetPageHeader(SerialInfoPage , (uint8_t *)"Serial Terminal : warning");
      item  = NewLabel(5, (uint8_t *)"The Serial module share some H/W resources",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,100,item,SerialInfoPage);

      item  = NewLabel(6, (uint8_t *)"with the SD card, launching the terminal",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,113,item,SerialInfoPage);

      item  = NewLabel(7, (uint8_t *)"will close all the  processes using the SD",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,126,item,SerialInfoPage);

      item  = NewLabel(8, (uint8_t *)"card, do you want to continue anyway?",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,139,item,SerialInfoPage);
    }
    break;
  case SERIAL_MAIN_PAGE:
    {
      SerialMainPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( SerialMainPage, SERIAL_MAIN_PAGE );
      item = NewIcon (5, serial_icon, 50, 50, MOD_NullFunc);
      AddPageControlObj(185, 40, item, SerialMainPage);

      GL_SetMenuItem(SerialMainPage, (uint8_t *)"Serial Terminal", 0, goto_serial );
      GL_SetMenuItem(SerialMainPage, (uint8_t *)"Settings", 1, goto_serialsettings );
      GL_SetMenuItem(SerialMainPage, (uint8_t *)"Return", 2, return_to_menu );
      GL_SetPageHeader(SerialMainPage , (uint8_t *)"Serial Terminal");
    }
    break;
    
  case SERIAL_PAGE:
    {
      MOD_GetParam(SERIAL_SETTINGS_MEM, &SerialCfg.d32);
      /* Enable USART2 clock */
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
      if (SerialCfg.d32 != 0)
      {
        /* USART Configuration */
        USART_InitStructure.USART_BaudRate = SERIAL_BaudRate[SerialCfg.b.BaudRate -1];
        USART_InitStructure.USART_WordLength = SERIAL_DataLength[1];
        USART_InitStructure.USART_StopBits = SERIAL_StopBits[SerialCfg.b.StopBits -1];
        USART_InitStructure.USART_Parity = SERIAL_Parity[SerialCfg.b.Parity -1];
        USART_InitStructure.USART_HardwareFlowControl = SERIAL_HardwareFlowControl[SerialCfg.b.FlowControl -1];
        USART_InitStructure.USART_Mode = SERIAL_Mode[1];
      }
      else
      {
        /* USART Configuration */
        USART_InitStructure.USART_BaudRate = SERIAL_BaudRate[0];
        USART_InitStructure.USART_WordLength = SERIAL_DataLength[1];
        USART_InitStructure.USART_StopBits = SERIAL_StopBits[0];
        USART_InitStructure.USART_Parity = SERIAL_Parity[0];
        USART_InitStructure.USART_HardwareFlowControl = SERIAL_HardwareFlowControl[0];
        USART_InitStructure.USART_Mode = SERIAL_Mode[1];
      }
      STM_EVAL_COMInit(COM1, &USART_InitStructure);

      SerialPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( SerialPage, SERIAL_PAGE );
      GL_SetPageHeader(SerialPage , (uint8_t *)"Serial Terminal");

      item = NewIcon (40,serial_close_icon,24,24,return_from_serial);
      AddPageControlObj(25,1,item,SerialPage);

      LCD_SetTextColor(GL_Grey);
      GL_LCD_FillRect(33, 314, 72, 311);

      LCD_SetTextColor(GL_Black);
      GL_LCD_DrawRect(31, 316, 75, 315);

      /* Line 1 */
      item = NewIcon (1,charQ_icon,30,30,charQ_function);
      AddPageControlObj(318,110,item,SerialPage);

      item = NewIcon (2,charW_icon,30,30,charW_function);
      AddPageControlObj(286,110,item,SerialPage);

      item = NewIcon (3,charE_icon,30,30,charE_function);
      AddPageControlObj(254,110,item,SerialPage);

      item = NewIcon (4,charR_icon,30,30,charR_function);
      AddPageControlObj(222,110,item,SerialPage);

      item = NewIcon (5,charT_icon,30,30,charT_function);
      AddPageControlObj(190,110,item,SerialPage);

      item = NewIcon (6,charY_icon,30,30,charY_function);
      AddPageControlObj(158,110,item,SerialPage);

      item = NewIcon (7,charU_icon,30,30,charU_function);
      AddPageControlObj(126,110,item,SerialPage);

      item = NewIcon (8,charI_icon,30,30,charI_function);
      AddPageControlObj(94,110,item,SerialPage);

      item = NewIcon (9,charO_icon,30,30,charO_function);
      AddPageControlObj(62,110,item,SerialPage);

      item = NewIcon (10,charP_icon,30,30,charP_function);
      AddPageControlObj(30,110,item,SerialPage);

      /* Line 2 */
      item = NewIcon (11,charA_icon,30,30,charA_function);
      AddPageControlObj(303,142,item,SerialPage);

      item = NewIcon (12,charS_icon,30,30,charS_function);
      AddPageControlObj(271,142,item,SerialPage);

      item = NewIcon (13,charD_icon,30,30,charD_function);
      AddPageControlObj(239,142,item,SerialPage);

      item = NewIcon (14,charF_icon,30,30,charF_function);
      AddPageControlObj(207,142,item,SerialPage);

      item = NewIcon (15,charG_icon,30,30,charG_function);
      AddPageControlObj(175,142,item,SerialPage);

      item = NewIcon (16,charH_icon,30,30,charH_function);
      AddPageControlObj(143,142,item,SerialPage);

      item = NewIcon (17,charJ_icon,30,30,charJ_function);
      AddPageControlObj(111,142,item,SerialPage);

      item = NewIcon (18,charK_icon,30,30,charK_function);
      AddPageControlObj(79,142,item,SerialPage);

      item = NewIcon (19,charL_icon,30,30,charL_function);
      AddPageControlObj(47,142,item,SerialPage);

      /* Line 3 */
      if(MajFlag != 0)
      {
        item = NewIcon (21,charMajY_icon,30,30,charMaj_function);
      }
      else
      {
        item = NewIcon (21,charMaj_icon,30,30,charMaj_function);
      }   
      AddPageControlObj(318,174,item,SerialPage);

      item = NewIcon (22,charZ_icon,30,30,charZ_function);
      AddPageControlObj(286,174,item,SerialPage);

      item = NewIcon (23,charX_icon,30,30,charX_function);
      AddPageControlObj(254,174,item,SerialPage);

      item = NewIcon (24,charC_icon,30,30,charC_function);
      AddPageControlObj(222,174,item,SerialPage);

      item = NewIcon (25,charV_icon,30,30,charV_function);
      AddPageControlObj(190,174,item,SerialPage);

      item = NewIcon (26,charB_icon,30,30,charB_function);
      AddPageControlObj(158,174,item,SerialPage);

      item = NewIcon (27,charN_icon,30,30,charN_function);
      AddPageControlObj(126,174,item,SerialPage);

      item = NewIcon (28,charM_icon,30,30,charM_function);
      AddPageControlObj(94,174,item,SerialPage);

      item = NewIcon (29,charInterComma_icon,30,30,charInterComma_function);
      AddPageControlObj(62,174,item,SerialPage);

      item = NewIcon (30,charPntSemicolon_icon,30,30,charPntSemicolon_function);
      AddPageControlObj(30,174,item,SerialPage);

      /* Line 4 */
      item = NewIcon (31,char123_icon,48,30,char123_function);
      AddPageControlObj(318,206,item,SerialPage);

      item = NewIcon (32,charSpace_icon,154,30,charSpace_function);
      AddPageControlObj(268,206,item,SerialPage);

      item = NewIcon (33,charErase_icon,48,30,charErase_function);
      AddPageControlObj(112,206,item,SerialPage);

      item = NewIcon (34,charSendY_icon,62,30,send_function);
      AddPageControlObj(62,206,item,SerialPage);

      SerialPage->CustomPostDraw = Serial_PostDraw;
    }
    break;

  case SERIAL_SETTINGS:
    {
      MOD_GetParam(SERIAL_SETTINGS_MEM, &SerialCfg.d32);

      SerialSettingsPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( SerialSettingsPage, SERIAL_SETTINGS );
      item = NewIcon (1, serial_settings_icon, 50, 50, MOD_NullFunc);
      AddPageControlObj(185, 40, item, SerialSettingsPage);
      BackButton = NewButton(2, (uint8_t *)" Return ", return_from_settings);
      AddPageControlObj(195, 212, BackButton, SerialSettingsPage);
      GL_SetPageHeader(SerialSettingsPage , (uint8_t *)"Serial Settings Menu");

      item  = NewLabel(3, (uint8_t *)"Bits per second:",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black,GL_FALSE);
      AddPageControlObj(315,105,item,SerialSettingsPage);
      item = NewComboBoxGrp(4);
      AddComboOption (item->objPTR, (uint8_t *)"4800", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"9600", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"19200", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"38400", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"57600", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"115200", MOD_NullFunc);
      AddPageControlObj(198, 100, item, SerialSettingsPage);

      pTmp = (GL_ComboBoxGrp_TypeDef*)(item->objPTR);
      if((SerialCfg.b.BaudRate > 0) && (SerialCfg.b.BaudRate < 7))
      {
        pTmp->ComboOptions[0]->IsActive = GL_FALSE;
        pTmp->ComboOptions[SerialCfg.b.BaudRate -1]->IsActive = GL_TRUE; 
      }

      item  = NewLabel(5, (uint8_t *)"Parity:",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black,GL_FALSE);
      AddPageControlObj(252,130,item,SerialSettingsPage);
      item = NewComboBoxGrp(6);
      AddComboOption (item->objPTR, (uint8_t *)"No", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"Even", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"Odd", MOD_NullFunc);
      AddPageControlObj( 198, 125, item, SerialSettingsPage);

      pTmp = (GL_ComboBoxGrp_TypeDef*)(item->objPTR);
      if((SerialCfg.b.Parity > 0) && (SerialCfg.b.Parity < 4))
      {
        pTmp->ComboOptions[0]->IsActive = GL_FALSE;
        pTmp->ComboOptions[SerialCfg.b.Parity -1]->IsActive = GL_TRUE; 
      }

      item  = NewLabel(7, (uint8_t *)"Stop bits:",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black,GL_FALSE);
      AddPageControlObj(273,155,item,SerialSettingsPage);
      item = NewComboBoxGrp(8);
      AddComboOption (item->objPTR, (uint8_t *)"0.5", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"1.0", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"1.5", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"2.0", MOD_NullFunc);
      AddPageControlObj( 198, 150, item, SerialSettingsPage);

      pTmp = (GL_ComboBoxGrp_TypeDef*)(item->objPTR);
      if((SerialCfg.b.StopBits > 0) && (SerialCfg.b.StopBits < 5))
      {
        pTmp->ComboOptions[0]->IsActive = GL_FALSE;
        pTmp->ComboOptions[SerialCfg.b.StopBits -1]->IsActive = GL_TRUE; 
      }

      item  = NewLabel(9, (uint8_t *)"Flow control:",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black,GL_FALSE);
      AddPageControlObj(294,180,item,SerialSettingsPage);
      item = NewComboBoxGrp(10);
      AddComboOption (item->objPTR, (uint8_t *)"None", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"RTS", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"CTS", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"RTS/CTS", MOD_NullFunc);
      AddPageControlObj( 198, 175, item, SerialSettingsPage);

      pTmp = (GL_ComboBoxGrp_TypeDef*)(item->objPTR);
      if((SerialCfg.b.FlowControl > 0) && (SerialCfg.b.FlowControl < 5))
      {
        pTmp->ComboOptions[0]->IsActive = GL_FALSE;
        pTmp->ComboOptions[SerialCfg.b.FlowControl -1]->IsActive = GL_TRUE; 
      }
    }
    break;
    
  default:
    break;
  }
}

/**
  * @brief  Display Serial Terminal page
  * @param  None
  * @retval None
  */
static void goto_serial(void)
{
  TxBufferIndex = 0;
  MajFlag = 0;
  /*Force Audio process running to stop if it is already running */
  AUDIO_ForceStop(MSD_MEDIA_STORAGE);
  
  SERIAL_SwitchPage(SerialMainPage, SERIAL_PAGE);
  SerialMainPage = NULL;
}


/**
  * @brief  Display Settings page
  * @param  None
  * @retval None
  */
static void goto_serialsettings(void)
{
  SERIAL_SwitchPage(SerialMainPage, SERIAL_SETTINGS);
  SerialMainPage = NULL;
}

/**
  * @brief  Return to global main menu action
  * @param  None
  * @retval None
  */
static void return_to_menu(void)
{
  SERIAL_SwitchPage(SerialMainPage, PAGE_MENU);
  SerialMainPage = NULL;
}

/**
  * @brief  Return to global main menu action
  * @param  None
  * @retval None
  */
static void return_from_warning (void)
{
  SERIAL_SwitchPage(SerialInfoPage, PAGE_MENU);
  SerialInfoPage = NULL;
}

/**
  * @brief  Open the main menu
  * @param  None
  * @retval None
  */
static void goto_main (void)
{
  SERIAL_SwitchPage(SerialInfoPage, SERIAL_MAIN_PAGE);
  SerialInfoPage = NULL;
}
/**
  * @brief  Return the module main menu from Serial Terminal page
  * @param  None
  * @retval None
  */
static void return_from_serial(void)
{
  SERIAL_SwitchPage(SerialPage, SERIAL_MAIN_PAGE);
  SerialPage = NULL;

 /* The Serial module share some H/W resources with the SD card, 
  so reconfigure the sdcard when leaving the terminal page */
  SD_Init();
}

/**
  * @brief  Return the module main menu from Settings page
  * @param  None
  * @retval None
  */
static void return_from_settings(void)
{
  SerialCfg.b.BaudRate = GetComboOptionActive(SerialSettingsPage, 4);
  SerialCfg.b.Parity = GetComboOptionActive(SerialSettingsPage, 6);
  SerialCfg.b.StopBits = GetComboOptionActive(SerialSettingsPage, 8);
  SerialCfg.b.FlowControl = GetComboOptionActive(SerialSettingsPage, 10);
  MOD_SetParam(SERIAL_SETTINGS_MEM , &SerialCfg.d32);

  SERIAL_SwitchPage(SerialSettingsPage, SERIAL_MAIN_PAGE);
  SerialSettingsPage = NULL;
}

/**
  * @brief  Shift key callback function
  * @param  None
  * @retval None
  */
void charMaj_function(void)
{
  GL_PageControls_TypeDef* item;

  MajFlag = ~MajFlag;
  if(MajFlag != 0)
  {
    DestroyPageControl(SerialPage, 20);
    RefreshPageControl(SerialPage, 20);
    item = NewIcon (20,charMajY_icon,30,30,charMaj_function);
    AddPageControlObj(318,174,item,SerialPage);
    RefreshPageControl(SerialPage, 20);
  }
  else
  {
    DestroyPageControl(SerialPage, 20);
    RefreshPageControl(SerialPage, 20);
    item = NewIcon (20,charMaj_icon,30,30,charMaj_function);
    AddPageControlObj(318,174,item,SerialPage);
    RefreshPageControl(SerialPage, 20);
  }
}

/**
  * @brief  A key callback function
  * @param  None
  * @retval None
  */
void charA_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x41;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x61;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  B key callback function
  * @param  None
  * @retval None
  */
void charB_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x42;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x62;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  C key callback function
  * @param  None
  * @retval None
  */
void charC_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x43;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x63;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  D key callback function
  * @param  None
  * @retval None
  */
void charD_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x44;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x64;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  E key callback function
  * @param  None
  * @retval None
  */
void charE_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x45;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x65;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  F key callback function
  * @param  None
  * @retval None
  */
void charF_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x46;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x66;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  G key callback function
  * @param  None
  * @retval None
  */
void charG_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x47;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x67;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  H key callback function
  * @param  None
  * @retval None
  */
void charH_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x48;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x68;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  I key callback function
  * @param  None
  * @retval None
  */
void charI_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x49;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x69;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  J key callback function
  * @param  None
  * @retval None
  */
void charJ_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x4A;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x6A;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  K key callback function
  * @param  None
  * @retval None
  */
void charK_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x4B;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x6B;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  L key callback function
  * @param  None
  * @retval None
  */
void charL_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x4C;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x6C;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  M key callback function
  * @param  None
  * @retval None
  */
void charM_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x4D;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x6D;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  N key callback function
  * @param  None
  * @retval None
  */
void charN_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x4E;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x6E;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  O key callback function
  * @param  None
  * @retval None
  */
void charO_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x4F;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x6F;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  P key callback function
  * @param  None
  * @retval None
  */
void charP_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x50;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x70;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Q key callback function
  * @param  None
  * @retval None
  */
void charQ_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x51;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x71;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  R key callback function
  * @param  None
  * @retval None
  */
void charR_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x52;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x72;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  S key callback function
  * @param  None
  * @retval None
  */
void charS_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x53;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x73;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  T key callback function
  * @param  None
  * @retval None
  */
void charT_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x54;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x74;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  U key callback function
  * @param  None
  * @retval None
  */
void charU_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x55;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x75;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  V key callback function
  * @param  None
  * @retval None
  */
void charV_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x56;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x76;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  W key callback function
  * @param  None
  * @retval None
  */
void charW_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x57;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x77;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  X key callback function
  * @param  None
  * @retval None
  */
void charX_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x58;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x78;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Y key callback function
  * @param  None
  * @retval None
  */
void charY_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x59;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x79;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Z key callback function
  * @param  None
  * @retval None
  */
void charZ_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x5A;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x7A;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Erase key callback function
  * @param  None
  * @retval None
  */
void charErase_function(void)
{
  if (TxBufferIndex == 0)
  {
    return;
  }
  else
  {
    TxBufferIndex--;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  InterComma key callback function
  * @param  None
  * @retval None
  */
void charInterComma_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x3F;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x2C;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  PntSemicolon key callback function
  * @param  None
  * @retval None
  */
void charPntSemicolon_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    if (MajFlag)
    {
      SerialTxBuffer[TxBufferIndex] = 0x2E;
    }
    else
    {
      SerialTxBuffer[TxBufferIndex] = 0x3B;
    }
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Space key callback function
  * @param  None
  * @retval None
  */
void charSpace_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x20;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Send key callback function
  * @param  None
  * @retval None
  */
void send_function(void)
{
  uint32_t index = 0;

  for(index = 0; index < TxBufferIndex; index++)
  {
    USART_SendData(EVAL_COM1, (uint8_t) SerialTxBuffer[index]);
    /* Loop until the end of transmission */
    while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TC) == RESET)
    {}
  }
  TxBufferIndex = 0;
  DisplayMsg(TxBufferIndex);
}

/**
  * @brief  0 key callback function
  * @param  None
  * @retval None
  */
void char0_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x30;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  1 key callback function
  * @param  None
  * @retval None
  */
void char1_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x31;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  2 key callback function
  * @param  None
  * @retval None
  */
void char2_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x32;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  3 key callback function
  * @param  None
  * @retval None
  */
void char3_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x33;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  4 key callback function
  * @param  None
  * @retval None
  */
void char4_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x34;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  5 key callback function
  * @param  None
  * @retval None
  */
void char5_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x35;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  6 key callback function
  * @param  None
  * @retval None
  */
void char6_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x36;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  7 key callback function
  * @param  None
  * @retval None
  */
void char7_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x37;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  8 key callback function
  * @param  None
  * @retval None
  */
void char8_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x38;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  9 key callback function
  * @param  None
  * @retval None
  */
void char9_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x39;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Slash key callback function
  * @param  None
  * @retval None
  */
void charSlash_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x2F;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Asterisk key callback function
  * @param  None
  * @retval None
  */
void charAsterisk_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x2A;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  LeftParen key callback function
  * @param  None
  * @retval None
  */
void charLeftParen_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x28;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  RightParen key callback function
  * @param  None
  * @retval None
  */
void charRightParen_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x29;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Dollar key callback function
  * @param  None
  * @retval None
  */
void charDollar_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x24;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Ampersand key callback function
  * @param  None
  * @retval None
  */
void charAmpersand_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x26;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  @ key callback function
  * @param  None
  * @retval None
  */
void charAt_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x40;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Hash key callback function
  * @param  None
  * @retval None
  */
void charHash_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x23;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  LeftSquare key callback function
  * @param  None
  * @retval None
  */
void charLeftSquare_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x5B;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  RightSquare key callback function
  * @param  None
  * @retval None
  */
void charRightSquare_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x5D;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Plus key callback function
  * @param  None
  * @retval None
  */
void charPlus_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x2B;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Minus key callback function
  * @param  None
  * @retval None
  */
void charMinus_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x2D;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Point key callback function
  * @param  None
  * @retval None
  */
void charPoint_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x2E;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Comma key callback function
  * @param  None
  * @retval None
  */
void charComma_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x2C;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Colon key callback function
  * @param  None
  * @retval None
  */
void charColon_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x3A;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  SemiColon key callback function
  * @param  None
  * @retval None
  */
void charSemiColon_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x3B;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Inter key callback function
  * @param  None
  * @retval None
  */
void charInter_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x3F;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Excl key callback function
  * @param  None
  * @retval None
  */
void charExcl_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x21;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Percent key callback function
  * @param  None
  * @retval None
  */
void charPercent_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x25;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  Equal key callback function
  * @param  None
  * @retval None
  */
void charEqual_function(void)
{
  if (TxBufferIndex == MAXBUFFERSIZE)
  {
    /* Error Message */
    DisplayErrorMsg();
  }
  else
  {
    SerialTxBuffer[TxBufferIndex] = 0x3D;
    TxBufferIndex++;
    DisplayMsg(TxBufferIndex);
  }
}

/**
  * @brief  ABC key callback function
  * @param  None
  * @retval None
  */
void charABC_function(void)
{
  uint32_t index = 0;
  GL_PageControls_TypeDef* item;
  
  for(index = 1; index < 32; index++)
  {
    DestroyPageControl(SerialPage, index);
  }

  /* Line 1 */
  item = NewIcon (1,charQ_icon,30,30,charQ_function);
  AddPageControlObj(318,110,item,SerialPage);
  RefreshPageControl(SerialPage, 1);

  item = NewIcon (2,charW_icon,30,30,charW_function);
  AddPageControlObj(286,110,item,SerialPage);
  RefreshPageControl(SerialPage, 2);

  item = NewIcon (3,charE_icon,30,30,charE_function);
  AddPageControlObj(254,110,item,SerialPage);
  RefreshPageControl(SerialPage, 3);
  
  item = NewIcon (4,charR_icon,30,30,charR_function);
  AddPageControlObj(222,110,item,SerialPage);
  RefreshPageControl(SerialPage, 4);

  item = NewIcon (5,charT_icon,30,30,charT_function);
  AddPageControlObj(190,110,item,SerialPage);
  RefreshPageControl(SerialPage, 5);

  item = NewIcon (6,charY_icon,30,30,charY_function);
  AddPageControlObj(158,110,item,SerialPage);
  RefreshPageControl(SerialPage, 6);

  item = NewIcon (7,charU_icon,30,30,charU_function);
  AddPageControlObj(126,110,item,SerialPage);
  RefreshPageControl(SerialPage, 7);

  item = NewIcon (8,charI_icon,30,30,charI_function);
  AddPageControlObj(94,110,item,SerialPage);
  RefreshPageControl(SerialPage, 8);

  item = NewIcon (9,charO_icon,30,30,charO_function);
  AddPageControlObj(62,110,item,SerialPage);
  RefreshPageControl(SerialPage, 9);

  item = NewIcon (10,charP_icon,30,30,charP_function);
  AddPageControlObj(30,110,item,SerialPage);
  RefreshPageControl(SerialPage, 10);

  /* Line 2 */
  item = NewIcon (11,charA_icon,30,30,charA_function);
  AddPageControlObj(303,142,item,SerialPage);
  RefreshPageControl(SerialPage, 11);

  item = NewIcon (12,charS_icon,30,30,charS_function);
  AddPageControlObj(271,142,item,SerialPage);
  RefreshPageControl(SerialPage, 12);

  item = NewIcon (13,charD_icon,30,30,charD_function);
  AddPageControlObj(239,142,item,SerialPage);
  RefreshPageControl(SerialPage, 13);

  item = NewIcon (14,charF_icon,30,30,charF_function);
  AddPageControlObj(207,142,item,SerialPage);
  RefreshPageControl(SerialPage, 14);

  item = NewIcon (15,charG_icon,30,30,charG_function);
  AddPageControlObj(175,142,item,SerialPage);
  RefreshPageControl(SerialPage, 15);

  item = NewIcon (16,charH_icon,30,30,charH_function);
  AddPageControlObj(143,142,item,SerialPage);
  RefreshPageControl(SerialPage, 16);

  item = NewIcon (17,charJ_icon,30,30,charJ_function);
  AddPageControlObj(111,142,item,SerialPage);
  RefreshPageControl(SerialPage, 17);

  item = NewIcon (18,charK_icon,30,30,charK_function);
  AddPageControlObj(79,142,item,SerialPage);
  RefreshPageControl(SerialPage, 18);

  item = NewIcon (19,charL_icon,30,30,charL_function);
  AddPageControlObj(47,142,item,SerialPage);
  RefreshPageControl(SerialPage, 19);

  /* Line 3 */
  if(MajFlag != 0)
  {
    item = NewIcon (21,charMajY_icon,30,30,charMaj_function);
  }
  else
  {
    item = NewIcon (21,charMaj_icon,30,30,charMaj_function);
  }
  AddPageControlObj(318,174,item,SerialPage);
  RefreshPageControl(SerialPage, 20);

  item = NewIcon (22,charZ_icon,30,30,charZ_function);
  AddPageControlObj(286,174,item,SerialPage);
  RefreshPageControl(SerialPage, 21);

  item = NewIcon (23,charX_icon,30,30,charX_function);
  AddPageControlObj(254,174,item,SerialPage);
  RefreshPageControl(SerialPage, 22);

  item = NewIcon (24,charC_icon,30,30,charC_function);
  AddPageControlObj(222,174,item,SerialPage);
  RefreshPageControl(SerialPage, 23);

  item = NewIcon (25,charV_icon,30,30,charV_function);
  AddPageControlObj(190,174,item,SerialPage);
  RefreshPageControl(SerialPage, 24);

  item = NewIcon (26,charB_icon,30,30,charB_function);
  AddPageControlObj(158,174,item,SerialPage);
  RefreshPageControl(SerialPage, 25);

  item = NewIcon (27,charN_icon,30,30,charN_function);
  AddPageControlObj(126,174,item,SerialPage);
  RefreshPageControl(SerialPage, 26);

  item = NewIcon (28,charM_icon,30,30,charM_function);
  AddPageControlObj(94,174,item,SerialPage);
  RefreshPageControl(SerialPage, 27);

  item = NewIcon (29,charInterComma_icon,30,30,charInterComma_function);
  AddPageControlObj(62,174,item,SerialPage);
  RefreshPageControl(SerialPage, 28);

  item = NewIcon (30,charPntSemicolon_icon,30,30,charPntSemicolon_function);
  AddPageControlObj(30,174,item,SerialPage);
  RefreshPageControl(SerialPage, 29);

  /* Line 4 */
  item = NewIcon (31,char123_icon,48,30,char123_function);
  AddPageControlObj(318,206,item,SerialPage);
  RefreshPageControl(SerialPage, 30);

  RefreshPage(SerialPage);

  LCD_SetTextColor(GL_Grey);
  GL_LCD_FillRect(33, 314, 72, 311);
  LCD_SetTextColor(GL_Black);
  GL_LCD_DrawRect(31, 316, 75, 315);
  DisplayMsg(TxBufferIndex);

}

/**
  * @brief  123 key callback function
  * @param  None
  * @retval None
  */
void char123_function(void)
{
  uint32_t index = 0;
  GL_PageControls_TypeDef* item;
  
  for(index = 1; index < 32; index++)
  {
    DestroyPageControl(SerialPage, index);
  }

  /* Line 1 */
  item = NewIcon (1,char1_icon,30,30,char1_function);
  AddPageControlObj(318,110,item,SerialPage);
  RefreshPageControl(SerialPage, 1);

  item = NewIcon (2,char2_icon,30,30,char2_function);
  AddPageControlObj(286,110,item,SerialPage);
  RefreshPageControl(SerialPage, 2);

  item = NewIcon (3,char3_icon,30,30,char3_function);
  AddPageControlObj(254,110,item,SerialPage);
  RefreshPageControl(SerialPage, 3);

  item = NewIcon (4,char4_icon,30,30,char4_function);
  AddPageControlObj(222,110,item,SerialPage);
  RefreshPageControl(SerialPage, 4);

  item = NewIcon (5,char5_icon,30,30,char5_function);
  AddPageControlObj(190,110,item,SerialPage);
  RefreshPageControl(SerialPage, 5);

  item = NewIcon (6,char6_icon,30,30,char6_function);
  AddPageControlObj(158,110,item,SerialPage);
  RefreshPageControl(SerialPage, 6);

  item = NewIcon (7,char7_icon,30,30,char7_function);
  AddPageControlObj(126,110,item,SerialPage);
  RefreshPageControl(SerialPage, 7);

  item = NewIcon (8,char8_icon,30,30,char8_function);
  AddPageControlObj(94,110,item,SerialPage);
  RefreshPageControl(SerialPage, 8);

  item = NewIcon (9,char9_icon,30,30,char9_function);
  AddPageControlObj(62,110,item,SerialPage);
  RefreshPageControl(SerialPage, 9);

  item = NewIcon (10,char0_icon,30,30,char0_function);
  AddPageControlObj(30,110,item,SerialPage);
  RefreshPageControl(SerialPage, 10);

  /* Line 2 */
  item = NewIcon (11,charSlash_icon,30,30,charSlash_function);
  AddPageControlObj(318,142,item,SerialPage);
  RefreshPageControl(SerialPage, 11);

  item = NewIcon (12,charAsterisk_icon,30,30,charAsterisk_function);
  AddPageControlObj(286,142,item,SerialPage);
  RefreshPageControl(SerialPage, 12);

  item = NewIcon (13,charLeftParen_icon,30,30,charLeftParen_function);
  AddPageControlObj(254,142,item,SerialPage);
  RefreshPageControl(SerialPage, 13);

  item = NewIcon (14,charRightParen_icon,30,30,charRightParen_function);
  AddPageControlObj(222,142,item,SerialPage);
  RefreshPageControl(SerialPage, 14);

  item = NewIcon (15,charDollar_icon,30,30,charDollar_function);
  AddPageControlObj(190,142,item,SerialPage);
  RefreshPageControl(SerialPage, 15);

  item = NewIcon (16,charAmpersand_icon,30,30,charAmpersand_function);
  AddPageControlObj(158,142,item,SerialPage);
  RefreshPageControl(SerialPage, 16);

  item = NewIcon (17,charAt_icon,30,30,charAt_function);
  AddPageControlObj(126,142,item,SerialPage);
  RefreshPageControl(SerialPage, 17);

  item = NewIcon (18,charHash_icon,30,30,charHash_function);
  AddPageControlObj(94,142,item,SerialPage);
  RefreshPageControl(SerialPage, 18);

  item = NewIcon (19,charLeftSquare_icon,30,30,charLeftSquare_function);
  AddPageControlObj(62,142,item,SerialPage);
  RefreshPageControl(SerialPage, 19);

  item = NewIcon (20,charRightSquare_icon,30,30,charRightSquare_function);
  AddPageControlObj(30,142,item,SerialPage);
  RefreshPageControl(SerialPage, 20);

  /* Line 3 */
  item = NewIcon (21,charPlus_icon,30,30,charPlus_function);
  AddPageControlObj(318,174,item,SerialPage);
  RefreshPageControl(SerialPage, 21);

  item = NewIcon (22,charMinus_icon,30,30,charMinus_function);
  AddPageControlObj(286,174,item,SerialPage);
  RefreshPageControl(SerialPage, 22);

  item = NewIcon (23,charPoint_icon,30,30,charPoint_function);
  AddPageControlObj(254,174,item,SerialPage);
  RefreshPageControl(SerialPage, 23);

  item = NewIcon (24,charComma_icon,30,30,charComma_function);
  AddPageControlObj(222,174,item,SerialPage);
  RefreshPageControl(SerialPage, 24);

  item = NewIcon (25,charColon_icon,30,30,charColon_function);
  AddPageControlObj(190,174,item,SerialPage);
  RefreshPageControl(SerialPage, 25);

  item = NewIcon (26,charSemiColon_icon,30,30,charSemiColon_function);
  AddPageControlObj(158,174,item,SerialPage);
  RefreshPageControl(SerialPage, 26);

  item = NewIcon (27,charInter_icon,30,30,charInter_function);
  AddPageControlObj(126,174,item,SerialPage);
  RefreshPageControl(SerialPage, 27);

  item = NewIcon (28,charExcl_icon,30,30,charExcl_function);
  AddPageControlObj(94,174,item,SerialPage);
  RefreshPageControl(SerialPage, 28);

  item = NewIcon (29,charPercent_icon,30,30,charPercent_function);
  AddPageControlObj(62,174,item,SerialPage);
  RefreshPageControl(SerialPage, 29);

  item = NewIcon (30,charEqual_icon,30,30,charEqual_function);
  AddPageControlObj(30,174,item,SerialPage);
  RefreshPageControl(SerialPage, 30);

  /* Line 4 */
  item = NewIcon (31,charABC_icon,48,30,charABC_function);
  AddPageControlObj(318,206,item,SerialPage);
  RefreshPageControl(SerialPage, 31);

  RefreshPage(SerialPage);

  LCD_SetTextColor(GL_Grey);
  GL_LCD_FillRect(33, 314, 72, 311);
  LCD_SetTextColor(GL_Black);
  GL_LCD_DrawRect(31, 316, 75, 315);
  DisplayMsg(TxBufferIndex);

}

/**
  * @brief  Display written message
  * @param  None
  * @retval None
  */
void DisplayMsg(uint32_t Index)
{
  uint32_t i = 0;
  uint8_t tmpbuffer[30] = {0};

  LCD_SetFont(&Font12x12);
  LCD_SetBackColor(GL_Grey);
  LCD_SetTextColor(GL_Black);

  if(Index == 0)
  {
    Serial_DisplayStringLine(Line3, (uint8_t *)"                         ");
    Serial_DisplayStringLine(Line4, (uint8_t *)"                         ");
    Serial_DisplayStringLine(Line5, (uint8_t *)"                         ");
  }
  Serial_DisplayStringLine(Line7, (uint8_t *)"                         ");

  if((Index > 0) && (Index < 25))
  {
    for(i = 0; i < Index; i++)
    {
      tmpbuffer[i] = SerialTxBuffer[i];
    }
    Serial_DisplayStringLine(Line3, (uint8_t *)"                         ");
    Serial_DisplayStringLine(Line3, tmpbuffer);
  }
  else if((Index >= 25) && (Index < 50))
  {
    for(i = 0; i < 25; i++)
    {
      tmpbuffer[i] = SerialTxBuffer[i];
    }
    Serial_DisplayStringLine(Line3, (uint8_t *)"                         ");
    Serial_DisplayStringLine(Line3, tmpbuffer);

    for(i = 25; i < Index; i++)
    {
      tmpbuffer[i - 25] = SerialTxBuffer[i];
    }
    tmpbuffer[i - 25] = 0;
    Serial_DisplayStringLine(Line4, (uint8_t *)"                         ");
    Serial_DisplayStringLine(Line4, tmpbuffer);
  }
  else if(Index >= 50)
  {
    for(i = 0; i < 25; i++)
    {
      tmpbuffer[i] = SerialTxBuffer[i];
    }
    Serial_DisplayStringLine(Line3, (uint8_t *)"                         ");
    Serial_DisplayStringLine(Line3, tmpbuffer);

    for(i = 25; i < 50; i++)
    {
      tmpbuffer[i - 25] = SerialTxBuffer[i];
    }
    tmpbuffer[i - 25] = 0;
    Serial_DisplayStringLine(Line4, (uint8_t *)"                         ");
    Serial_DisplayStringLine(Line4, tmpbuffer);

    for(i = 50; i < Index; i++)
    {
      tmpbuffer[i - 50] = SerialTxBuffer[i];
    }
    tmpbuffer[i - 50] = 0;
    Serial_DisplayStringLine(Line5, (uint8_t *)"                         ");
    Serial_DisplayStringLine(Line5, tmpbuffer);
  }
}

/**
  * @brief  Display error message
  * @param  None
  * @retval None
  */
void DisplayErrorMsg(void)
{
  LCD_SetFont(&Font12x12);
  LCD_SetTextColor(GL_Red);
  Serial_DisplayStringLine(Line7, (uint8_t *)"         Buffer is filled");
  LCD_SetTextColor(GL_Black);
}

/**
  * @brief  Draw rectangle
  * @param  None
  * @retval None
  */
void Serial_PostDraw(void)
{
  LCD_SetTextColor(GL_Grey);
  GL_LCD_FillRect(33, 314, 72, 311);
  LCD_SetTextColor(GL_Black);
  GL_LCD_DrawRect(31, 316, 75, 315);
}

/**
  * @brief  Displays a maximum of 25 char on the LCD.
  * @param  Line: the Line where to display the character shape .
  * @param  *ptr: pointer to string to display on LCD.
  * @retval None
  */
static void Serial_DisplayStringLine(uint16_t Line, uint8_t *ptr)
{
  uint16_t refcolumn = 310;

  /* Send the string character by character on lCD */
  while ((*ptr != 0) & (((refcolumn + 1) & 0xFFFF) >= 12))
  {
    /* Display one character on LCD */
    LCD_DisplayChar(Line, refcolumn, *ptr);
    /* Decrement the column position by 16 */
    refcolumn -= 12;
    /* Point on the next character */
    ptr++;
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
