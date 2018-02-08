/**
  ******************************************************************************
  * @file    mod_usbd.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Graphical window and events implementation of the USB Device module
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
#include "mod_usbd.h"
#include "app_usbd.h"
#include "gl_usbd_res.c"
#include "gl_mgr.h"

/* Private typedef -----------------------------------------------------------*/
typedef union _USBD_CONFIG_TypeDef
{
  uint32_t d32;
  struct
  {
    uint32_t media_size :
      4;
      
    uint32_t hid_polling :
      4;
      
    uint32_t Reserved:
      24;
  }
  b;
} USBD_CONFIG_TypeDef ;
/* Private define ------------------------------------------------------------*/
#define USB_MAIN_PAGE           MOD_USBD_UID + 0
#define USB_MSC_PAGE            MOD_USBD_UID + 1
#define USB_HID_PAGE            MOD_USBD_UID + 2
#define USB_SETTINGS_PAGE       MOD_USBD_UID + 3

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/*Graphical draw process */
static void USB_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex);
static void USB_CreatePage(uint8_t Page);
static void USB_Startup (void);
static void USB_Background(void);
/* Control Actions */
static void return_to_menu (void);
static void goto_to_msc (void);
static void goto_to_hid (void);
static void goto_to_settings (void);

static void return_from_msc (void);
static void return_from_hid (void);
static void return_from_settings (void);

static void USB_UpdateMSDState(uint8_t conn_state);
static void USB_UpdateMSDCapacityState(uint32_t capacity);
static void DrawTouchPadBackgound (void);
/* Private variables ---------------------------------------------------------*/
static GL_Page_TypeDef *USBMainPage;
static GL_Page_TypeDef *USB_MSC_Page;
static GL_Page_TypeDef *USB_HID_Page;
static GL_Page_TypeDef *USB_Settings_Page;
static GL_Page_TypeDef *Current_Page;
static void usbd_msc_background(void);
static void usbd_hid_background(void);
extern SD_CardInfo    SDCardInfo;
static uint8_t previous_state = 0 ;
static uint32_t previous_capacity = 0;

USBD_CONFIG_TypeDef cfg;

MOD_InitTypeDef  mod_usbd =
{
  MOD_USBD_UID,
  MOD_USBD_VER,
  (uint8_t *)"USB",
  (uint8_t *)usb_icon,
  USB_Startup,
  USB_Background,
  NULL,
};

/* Private functions ---------------------------------------------------------*/

/**
* @brief  Handle the Module startup action and display the main menu
* @param  None
* @retval None
*/

static void USB_Startup (void)
{
  vTaskSuspend(Core_Time_Task_Handle);
  USB_SwitchPage(GL_HomePage, USB_MAIN_PAGE);
}

/**
* @brief  Close parent page and display a child sub-page
* @param  pParent : parent page
* @param  PageIndex : sub-page index
* @retval None
*/

static void USB_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex)
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

  USB_CreatePage(PageIndex);

  switch (PageIndex)
  {
  case USB_MAIN_PAGE:
    NextPage = USBMainPage;
    break;

  case USB_MSC_PAGE:
    NextPage = USB_MSC_Page;
    break;

  case USB_HID_PAGE:
    NextPage = USB_HID_Page;
    break;

  case USB_SETTINGS_PAGE:
    NextPage = USB_Settings_Page;
    break;


  default:
    break;
  }
  Current_Page = NextPage;
  RefreshPage(NextPage);
}

/**
* @brief  Create a child sub-page
* @param  Page : Page handler
* @retval None
*/

static void USB_CreatePage(uint8_t Page)
{

  GL_PageControls_TypeDef* item;
  GL_PageControls_TypeDef* BackButton;
  GL_ComboBoxGrp_TypeDef* pTmp;

  GL_SetBackColor( GL_White );
  GL_SetTextColor( GL_Blue );

  switch (Page)
  {
  case USB_MAIN_PAGE:
    {
      USBMainPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( USBMainPage, USB_MAIN_PAGE );
      item = NewIcon (5, usb_icon, 50, 50, MOD_NullFunc);
      AddPageControlObj(185, 40, item, USBMainPage);

      GL_SetMenuItem(USBMainPage, (uint8_t *)"Mass storage", 0, goto_to_msc );
      GL_SetMenuItem(USBMainPage, (uint8_t *)"Touch Pad", 1, goto_to_hid );
      GL_SetMenuItem(USBMainPage, (uint8_t *)"Settings", 2, goto_to_settings);
      GL_SetMenuItem(USBMainPage, (uint8_t *)"Return", 3, return_to_menu );
      GL_SetPageHeader(USBMainPage , (uint8_t *)"USB Menu");
    }
    break;

  case USB_MSC_PAGE:
    {
      USB_MSC_Page = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( USB_MSC_Page, USB_MSC_PAGE );

      item = NewIcon (1, usb_msc_unmounted, 50, 50, MOD_NullFunc);
      AddPageControlObj(185, 80, item, USB_MSC_Page);

      BackButton = NewButton(2, (uint8_t *)" Return ", return_from_msc);
      AddPageControlObj(195, 212, BackButton, USB_MSC_Page);
      GL_SetPageHeader(USB_MSC_Page , (uint8_t *)"USB Mass Storage Device");

      item  = NewLabel(3, (uint8_t *)"MicroSD: CARD Not Connected.", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(280, 152, item, USB_MSC_Page);

      item  = NewLabel(4, (uint8_t *)"CAPACITY: Unknown.", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(280, 166, item, USB_MSC_Page);

      item  = NewLabel(6, (uint8_t *)"STATE: USB Device Not Connected.", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(280, 180, item, USB_MSC_Page);

      previous_state = 0 ;
      previous_capacity = 0;

    }
    break;


  case USB_HID_PAGE:
    {
      USB_HID_Page = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( USB_HID_Page, USB_HID_PAGE );
      BackButton = NewButton(1, (uint8_t *)" Return ", return_from_hid);
      AddPageControlObj(195, 212, BackButton, USB_HID_Page);
      GL_SetPageHeader(USB_HID_Page , (uint8_t *)"USB HID Device");

      item  = NewLabel(6, (uint8_t *)"STATE: USB Device Not Connected.", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(280, 180, item, USB_HID_Page);

      USB_HID_Page->CustomPreDraw = DrawTouchPadBackgound;
    }
    break;

  case USB_SETTINGS_PAGE:
    {
      USB_Settings_Page = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( USB_Settings_Page, USB_SETTINGS_PAGE );
      BackButton = NewButton(1, (uint8_t *)" Return ", return_from_settings);
      AddPageControlObj(195, 212, BackButton, USB_Settings_Page);

      item  = NewLabel(4, (uint8_t *)"MSC media buffer size :", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(260, 50 + 20, item, USB_Settings_Page);

      MOD_GetParam(USBD_SETTINGS_MEM , &cfg.d32);

      item = NewComboBoxGrp(5);
      AddComboOption (item->objPTR, (uint8_t *)"512 Bytes", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"1  KBytes", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"2  KBytes", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"4  KBytes", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"8  KBytes", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"16 KBytes", MOD_NullFunc);
      AddPageControlObj( 260, 65 + 20, item, USB_Settings_Page);

      pTmp = (GL_ComboBoxGrp_TypeDef*)(item->objPTR);
      
      if ((cfg.b.media_size > 0) && (cfg.b.media_size < 7))
      {
        pTmp->ComboOptions[0]->IsActive = GL_FALSE;
        pTmp->ComboOptions[cfg.b.media_size -1]->IsActive = GL_TRUE;
      }

      item  = NewLabel(6, (uint8_t *)"USB HID hid polling time:", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(260, 95 + 30, item, USB_Settings_Page);

      item = NewComboBoxGrp(7);
      AddComboOption (item->objPTR, (uint8_t *)"05 msec", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"10 msec", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"15 msec", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"20 msec", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"25 msec", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"30 msec", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"35 msec", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"40 msec", MOD_NullFunc);
      AddPageControlObj( 260, 110 + 30, item, USB_Settings_Page);

      pTmp = (GL_ComboBoxGrp_TypeDef*)(item->objPTR);

      if ((cfg.b.hid_polling > 0) && (cfg.b.hid_polling < 9))
      {
        pTmp->ComboOptions[0]->IsActive = GL_FALSE;
        pTmp->ComboOptions[cfg.b.hid_polling -1]->IsActive = GL_TRUE;
      }


      GL_SetPageHeader(USB_Settings_Page , (uint8_t *)"USB Settings");
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
  USB_SwitchPage(USBMainPage, PAGE_MENU);
  USBMainPage = Current_Page = NULL;
}

/**
* @brief  Display the USB Mass storage page
* @param  None
* @retval None
*/
static void goto_to_msc (void)
{
  USBD_MSC_AppInit();
  USB_SwitchPage(USBMainPage, USB_MSC_PAGE);

}

/**
* @brief  Display the USB Mass storage page
* @param  None
* @retval None
*/
static void goto_to_settings (void)
{
  USB_SwitchPage(USBMainPage, USB_SETTINGS_PAGE);
}
/**
* @brief  Display the USB HID page
* @param  None
* @retval None
*/
static void goto_to_hid (void)
{
  USBD_HID_AppInit();
  USB_SwitchPage(USBMainPage, USB_HID_PAGE);

}

/**
* @brief  return to the module main menu from the MSC page
* @param  None
* @retval None
*/
static void return_from_msc (void)
{
  USB_SwitchPage(USB_MSC_Page, USB_MAIN_PAGE);
  USBD_StopDeviceApp();
  Current_Page = NULL;
}

/**
* @brief  return to the module main menu from the HID page
* @param  None
* @retval None
*/
static void return_from_hid (void)
{
  USB_SwitchPage(USB_HID_Page, USB_MAIN_PAGE);
  USBD_StopDeviceApp();
  Current_Page = NULL;
}

/**
* @brief  return to the module main menu from the Audio page
* @param  None
* @retval None
*/
static void return_from_settings (void)
{

  cfg.d32 = 0;
  cfg.b.media_size     = GetComboOptionActive(USB_Settings_Page, 5);
  cfg.b.hid_polling    = GetComboOptionActive(USB_Settings_Page, 7);

  MOD_SetParam(USBD_SETTINGS_MEM , &cfg.d32);

  USB_SwitchPage(USB_Settings_Page, USB_MAIN_PAGE);
  Current_Page = NULL;
}

/**
* @brief  Update the USB state message in the current page
* @param  msg : pointer to the new message string
* @retval None
*/
void USBD_SetStateMsg(uint8_t *msg)
{
  uint8_t temp[40];
  if ( Current_Page != NULL)
  {
    memset (temp, ' ', 40);
    RefreshLabel (Current_Page , 6 , temp);
    RefreshLabel (Current_Page , 6 , msg);
  }
}


/**
* @brief  Update the microSD card connection state in the MSC page
* @param  state : connection state
* @retval None
*/
static void USB_UpdateMSDState(uint8_t conn_state)
{
  uint8_t temp[40];

  GL_PageControls_TypeDef* item;

  if (previous_state != conn_state)
  {
    previous_state = conn_state;

    memset (temp, ' ', 40);
    RefreshLabel (USB_MSC_Page , 3 , temp);
    if (conn_state)
    {
      RefreshLabel (USB_MSC_Page , 3 , (uint8_t *)"MicroSD: Card connected");
      DestroyPageControl ( USB_MSC_Page, 1);
      item = NewIcon (1, usb_msc_mounted, 50, 50, MOD_NullFunc);
      AddPageControlObj(185, 80, item, USB_MSC_Page);
      RefreshPageControl ( USB_MSC_Page, 1);
    }
    else
    {
      RefreshLabel (USB_MSC_Page , 3 , (uint8_t *)"MicroSD: Card NOT connected");
      DestroyPageControl ( USB_MSC_Page, 1);
      item = NewIcon (1, usb_msc_unmounted, 50, 50, MOD_NullFunc);
      AddPageControlObj(185, 80, item, USB_MSC_Page);
      RefreshPageControl ( USB_MSC_Page, 1);
    }
  }
}

/**
* @brief  Update the microSD capacity in the MSC page
* @param  Capacity : new microSD card capacity
* @retval None
*/
static void USB_UpdateMSDCapacityState(uint32_t capacity)
{
  uint8_t temp[40];

  if (previous_capacity != capacity)
  {
    previous_capacity = capacity;

    memset (temp, ' ', 40);
    RefreshLabel (USB_MSC_Page , 4 , temp);

    if (capacity == 0)
      RefreshLabel (USB_MSC_Page , 4 , (uint8_t *)"CAPACITY: Unknown");
    else
    {
      sprintf ((char *)temp, "CAPACITY: %d MB", (int)(capacity / 1024 / 1024));
      RefreshLabel (USB_MSC_Page , 4 , temp);
    }
  }
}

/**
* @brief  USB MSC device background process callback
* @param  None
* @retval None
*/
static void usbd_msc_background(void)
{
  static uint8_t counter = 0;

  if(USB_MSC_Page != NULL)
  {
    if (USB_MSC_Page->Page_ID == USB_MSC_PAGE)
    {
      if(counter++ > 10)
      {
        counter = 0;
        if (SD_GetStatus() != 0)
        {
          SD_Init();
          USB_UpdateMSDState(0);
          USB_UpdateMSDCapacityState(0);

        }
        else
        {
          USB_UpdateMSDState(1);
          USB_UpdateMSDCapacityState(SDCardInfo.CardCapacity);
        }
      }
    }
  }
}


/**
* @brief  USB HID device background process callback
* @param  None
* @retval None
*/
static void usbd_hid_background(void)
{
  __IO TS_STATE  * ts;
  int16_t TSC_Value_X ;
  int16_t TSC_Value_Y;
  int16_t diff;
  __IO uint8_t  touch_done;

  static uint16_t prev_x = 0, prev_y = 0;
  static uint16_t prev_x2 = 0, prev_y2 = 0;

  if(USB_HID_Page != NULL)
  {
    if (USB_HID_Page->Page_ID == USB_HID_PAGE)
    {
      portENTER_CRITICAL();
      ts = IOE_TS_GetState();
      portEXIT_CRITICAL();
      prev_x = ts->X;
      prev_y = ts->Y;
      touch_done = (ts->TouchDetected == 0x80);

      vTaskDelay(10);

      portENTER_CRITICAL();
      ts = IOE_TS_GetState();
      portEXIT_CRITICAL();
      TSC_Value_X = ts->X;
      TSC_Value_Y = ts->Y;

      if((TSC_Value_Y > 45) && (TSC_Value_X < 268) && (TSC_Value_Y < (33 + 130)) && (TSC_Value_X > 40))
      {

        LCD_SetTextColor(GL_Grey);
        GL_DisplayAdjStringLine(prev_y2, 319- prev_x2, (uint8_t *)"X", GL_TRUE);
        LCD_SetTextColor(GL_Black);
        GL_DisplayAdjStringLine(TSC_Value_Y, 319- TSC_Value_X, (uint8_t *)"X", GL_TRUE);
        prev_y2 = TSC_Value_Y;
        prev_x2 = TSC_Value_X;

        if((touch_done == 1)&&((prev_x * TSC_Value_X * prev_y * TSC_Value_Y) != 0))
        {
          diff = TSC_Value_X - prev_x;

          if(diff > 0)
            diff += 5;

          if(diff < 0)
            diff -= 5; 

          if(abs(diff) > 20)
          {
            diff *= 2;
          }
          HID_Buffer[1] = diff ;

          diff = TSC_Value_Y - prev_y;

          if(diff > 0)
            diff += 5;

          if(diff < 0)
            diff -= 5;

          if(abs(diff) > 20)
          {
            diff *= 2;
          }
          HID_Buffer[2] = diff;

          USBD_HID_AppSendReport();

        }
        else
        {
          HID_Buffer[0] = 0;
          HID_Buffer[1] = 0;
          HID_Buffer[2] = 0;
          HID_Buffer[3] = 0;
        }
      }
    }
  }
}

/**
* @brief  Main USB  device background process callback
* @param  None
* @retval None
*/
static void USB_Background (void)

{
  if( USB_MSC_Page->Page_ID  == USB_MSC_PAGE)
  {
    usbd_msc_background();
  }
  else  if( USB_HID_Page->Page_ID  == USB_HID_PAGE)
  {
    usbd_hid_background();
  }
  
}
/**
* @brief  Draw the touch pad background zone
* @param  None
* @retval None
*/
static void DrawTouchPadBackgound (void)
{
  LCD_SetTextColor(GL_Grey);
  GL_LCD_FillRect(45, 280, 130, 240);

  LCD_SetTextColor(GL_Black);
  GL_LCD_DrawRect(43, 282, 133, 244);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
