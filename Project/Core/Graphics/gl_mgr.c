/**
  ******************************************************************************
  * @file    gl_mgr.c 
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   This file contains graphical aspect of the kernel
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
#include "mod_audio.h"
#include "mod_ethernet.h"
#include "gl_mgr.h"
#include "bsp.h"
#include "cursor.h"
#include "mod_core.h"
#include "gl_res.c"
#include "time_utils.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define Time_Task_PRIO    ( tskIDLE_PRIORITY + 3 )
#define HOME_PAGE_ID      0xFFFF
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GL_Page_TypeDef  *GL_HomePage;
uint8_t GL_Group = UTILITIES_GROUP;
xTaskHandle      Core_Time_Task_Handle;

/* Private function prototypes -----------------------------------------------*/
static void GL_ShowConnectivityGroup (void);
static void GL_ShowMultimediaGroup (void);
static void GL_ShowUtilitiesGroup (void);
static void GL_UpdateGroupSelectionLabel (uint8_t group);
static void Time_Task(void * pvParameters);
static void DrawBackgroundZone (void);
/* Private functions ---------------------------------------------------------*/

/**
* @brief  Initialize the screen and display the main menu
* @param  None
* @retval None
*/
uint32_t GL_Init (void)
{
  /* Clear the screen */
  GL_Clear(GL_White);

  xTaskCreate(Time_Task, 
              (signed char const*)"TIME_B", 
              configMINIMAL_STACK_SIZE, 
              NULL, 
              Time_Task_PRIO, 
              &Core_Time_Task_Handle);

  /* Menu Initialisation*/
  GL_ShowMainMenu();

  return 0;
}

/**
* @brief  Handle the HMI events.
* @param  None
* @retval Status
*/
uint32_t GL_Handle_Inputs (void)
{
  GL_TSC_GetCoordinate();
  MOD_fClickHandler();
  return 0;
}

/**
* @brief  Display the main menu
* @param  None
* @retval None
*/
void GL_ShowMainMenu(void)
{
  uint8_t TempStr[25];
  RTC_TimeTypeDef   RTC_TimeStructure;
  RTC_DateTypeDef   RTC_DateStructure;

  GL_PageControls_TypeDef  *Icon , *LabelTime , *LabelDate, *LabelBkgnd;
  uint8_t sec, min, hour, day, month;
  uint16_t year;


  GL_Clear(White);

  GL_HomePage = malloc(sizeof(GL_Page_TypeDef));
  Create_PageObj( GL_HomePage, HOME_PAGE_ID );

  /* Add group icons */

  Icon = NewIcon(2 , Group_Connectivity_icon, BF_XSIZE, BF_YSIZE ,GL_ShowConnectivityGroup );
  AddPageControlObj(305, 40, Icon, GL_HomePage );

  Icon = NewIcon(3 , Group_Multimedia_icon, BF_XSIZE, BF_YSIZE ,GL_ShowMultimediaGroup );
  AddPageControlObj(305, 103, Icon, GL_HomePage );

  Icon = NewIcon(4 , Group_System_icon, BF_XSIZE, BF_YSIZE ,GL_ShowUtilitiesGroup );
  AddPageControlObj(305, 166, Icon, GL_HomePage );


  Icon = NewIcon(32 , STLogo, 40, 22 ,MOD_NullFunc );
  AddPageControlObj(240, 216, Icon, GL_HomePage );

  LabelBkgnd  = NewLabel(33, (uint8_t *)"",GL_HORIZONTAL,GL_FONT_SMALL,GL_Red, GL_FALSE);
  AddPageControlObj(195, 227, LabelBkgnd, GL_HomePage);

  /* add modules */
  switch(GL_Group)
  {
  case CONNECTIVITY_GROUP :
    GL_ShowConnectivityGroup();
    break;

  case MULTIMEDIA_GROUP : 
    GL_ShowMultimediaGroup();
    break;

  case UTILITIES_GROUP : 
    GL_ShowUtilitiesGroup();
    break;

  }

  GL_HomePage->CustomPreDraw = DrawBackgroundZone;
  /* time section */

  /* Get info from RTC here */
  RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);

  sec    =  RTC_TimeStructure.RTC_Seconds;
  min    =  RTC_TimeStructure.RTC_Minutes;
  hour   =  RTC_TimeStructure.RTC_Hours;

  sprintf((char *)TempStr, "%02d:%02d:%02d", hour , min, sec);

  LabelTime  = NewLabel(30,TempStr,GL_HORIZONTAL,GL_FONT_SMALL,0x1253, GL_FALSE);
  AddPageControlObj(60, 215, LabelTime, GL_HomePage);


  RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);

  year =  RTC_DateStructure.RTC_Year + 2000;
  month =  RTC_DateStructure.RTC_Month;
  day =  RTC_DateStructure.RTC_Date;

  sprintf((char *)TempStr, "%02d %s %04d", day , strMonth[month-1], year);
  LabelDate  = NewLabel(31,TempStr,GL_HORIZONTAL,GL_FONT_SMALL,0x1253, GL_FALSE);
  AddPageControlObj(88, 227, LabelDate, GL_HomePage); 


  RefreshPage(GL_HomePage);
  vTaskResume(Core_Time_Task_Handle); 

  Global_Config.b.Force_Background_Refresh = 1;

}

/**
* @brief  display the initialization page 
* @param  None
* @retval None
*/
void GL_Startup (void)
{
  uint8_t cnt = 0;

  GL_Clear(GL_White);

  GL_DrawButtonBMP( 210, 110, 148, (LCD_Height / 10)*2, (uint8_t*) STM32Logo );

  for ( ; cnt < 2; cnt ++)
  {
    GL_DisplayAdjStringLine(144, 250, (uint8_t *)"STM32 System Initializing.  ", GL_FALSE);
    GL_Delay(100);
    GL_DisplayAdjStringLine(144, 250, (uint8_t *)"STM32 System Initializing.. ", GL_FALSE);
    GL_Delay(100);
    GL_DisplayAdjStringLine(144, 250, (uint8_t *)"STM32 System Initializing...", GL_FALSE);
    GL_Delay(100);
  }
}

/**
* @brief  Add a message in the initialization page 
* @param  msg: pointer to the string to be displayed
* @retval None
*/
void GL_State_Message (uint8_t *msg)
{
  uint8_t tmpCaption[40];

  GL_CenterCaption (tmpCaption, msg, LCD_Width / GL_FONT_SMALL_WIDTH);
  GL_DisplayAdjStringLine(3 * (LCD_Height / 5), 300, tmpCaption, GL_FALSE);
  GL_Delay(300);
}


/**
* @brief  Display the Connectivity group
* @param  None
* @retval None
*/
static void GL_ShowConnectivityGroup (void)
{

  uint8_t idx = 0;
  
  if((GL_Group != CONNECTIVITY_GROUP)||(GL_HomePage->Page_Visible == GL_FALSE))
  {
    for ( idx = 0; idx < MAX_MODULES_NUM * 2; idx ++)
    {
      DestroyPageControl ( GL_HomePage, 5 + idx);
    }

    LCD_SetTextColor(GL_White);
    GL_LCD_FillRect(42, 240,160, 230);

    /* add modules */
    for ( idx = 0; idx < MOD_table[CONNECTIVITY_GROUP].counter; idx ++)
    {
      AddIconControlObj (GL_HomePage,
                         5 + 2 * idx,
                         idx,
                         MOD_table[CONNECTIVITY_GROUP].module[idx]->icon, 
                         MOD_table[CONNECTIVITY_GROUP].module[idx]->name,
                         0,
                         MOD_table[CONNECTIVITY_GROUP].module[idx]->startup);

      RefreshPageControl ( GL_HomePage, 5 + 2 * idx);
      RefreshPageControl ( GL_HomePage, 5 + 2 * idx + 1);

    }

    GL_Group = CONNECTIVITY_GROUP;
    GL_UpdateGroupSelectionLabel (CONNECTIVITY_GROUP);
  }
}

/**
* @brief  Display the Utilities group menu
* @param  None
* @retval None
*/
static void GL_ShowUtilitiesGroup (void)
{

  uint8_t idx = 0;

  if((GL_Group != UTILITIES_GROUP)||(GL_HomePage->Page_Visible == GL_FALSE))
  {
    for ( idx = 0; idx < MAX_MODULES_NUM * 2; idx ++)
    {
      DestroyPageControl ( GL_HomePage, 5 + idx);
    }

    LCD_SetTextColor(GL_White);
    GL_LCD_FillRect(42, 240,160, 230);

    /* add modules */
    for ( idx = 0; idx < MOD_table[UTILITIES_GROUP].counter; idx ++)
    { 
      AddIconControlObj (GL_HomePage,
                         5 + 2 * idx,
                         idx,
                         MOD_table[UTILITIES_GROUP].module[idx]->icon,
                         MOD_table[UTILITIES_GROUP].module[idx]->name,
                         0,
                         MOD_table[UTILITIES_GROUP].module[idx]->startup);

      RefreshPageControl ( GL_HomePage, 5 + 2 * idx);
      RefreshPageControl ( GL_HomePage, 5 + 2 * idx + 1);

    }

    GL_Group = UTILITIES_GROUP;
    GL_UpdateGroupSelectionLabel (UTILITIES_GROUP);
  }
}

/**
* @brief  Display the Multimedia group menu
* @param  None
* @retval None
*/
static void GL_ShowMultimediaGroup (void)
{

  uint8_t idx = 0;

  if((GL_Group != MULTIMEDIA_GROUP)||(GL_HomePage->Page_Visible == GL_FALSE))
  {
    for ( idx = 0; idx < MAX_MODULES_NUM * 2; idx ++)
    {
      DestroyPageControl ( GL_HomePage, 5 + idx);
    }

    LCD_SetTextColor(GL_White);
    GL_LCD_FillRect(42, 240,160, 230);

    /* add modules */
    for ( idx = 0; idx < MOD_table[MULTIMEDIA_GROUP].counter; idx ++)
    {
      AddIconControlObj (GL_HomePage,
                         5 + 2 * idx,
                         idx,
                         MOD_table[MULTIMEDIA_GROUP].module[idx]->icon,
                         MOD_table[MULTIMEDIA_GROUP].module[idx]->name,
                         0,
                         MOD_table[MULTIMEDIA_GROUP].module[idx]->startup);

      RefreshPageControl ( GL_HomePage, 5 + 2 * idx);
      RefreshPageControl ( GL_HomePage, 5 + 2 * idx + 1);
    }

    GL_Group = MULTIMEDIA_GROUP;
    GL_UpdateGroupSelectionLabel (MULTIMEDIA_GROUP);
  }
}

/**
* @brief  update the graphics of the selected group window
* @param  group: selected group 
* @retval None
*/
static void GL_UpdateGroupSelectionLabel (uint8_t group)
{

  GL_SetFont(GL_FONT_SMALL);

  switch ( group)
  {
  case CONNECTIVITY_GROUP :
    GL_SetTextColor(GL_Yellow);
    GL_DisplayAdjStringLine(40 + BF_XSIZE - 1, 310,  (uint8_t *)" Connect.", GL_TRUE);
    GL_SetTextColor(GL_White);
    GL_DisplayAdjStringLine(103 + BF_XSIZE - 1, 316,  (uint8_t *)"Multimedia", GL_TRUE);
    GL_DisplayAdjStringLine(166 + BF_XSIZE - 1, 310,  (uint8_t *)"Utilities", GL_TRUE);
    break;

  case MULTIMEDIA_GROUP :
    GL_SetTextColor(GL_Yellow);
    GL_DisplayAdjStringLine(103 + BF_XSIZE - 1, 316,  (uint8_t *)"Multimedia", GL_TRUE);
    GL_SetTextColor(GL_White);
    GL_DisplayAdjStringLine(40 + BF_XSIZE - 1, 310,  (uint8_t *)" Connect.", GL_TRUE);
    GL_DisplayAdjStringLine(166 + BF_XSIZE - 1, 310,  (uint8_t *)"Utilities", GL_TRUE);
    break;

  case UTILITIES_GROUP :
    GL_SetTextColor(GL_Yellow);
    GL_DisplayAdjStringLine(166 + BF_XSIZE - 1, 310,  (uint8_t *)"Utilities", GL_TRUE);
    GL_SetTextColor(GL_White);
    GL_DisplayAdjStringLine(103 + BF_XSIZE - 1, 316,  (uint8_t *)"Multimedia", GL_TRUE);
    GL_DisplayAdjStringLine(40 + BF_XSIZE - 1, 310,  (uint8_t *)" Connect.", GL_TRUE);
    break;

  default :
    break;
  }
}

/**
  * @brief  Time main task
  * @param  pvParameters : task parameters
  * @retval None
  */
static void Time_Task(void * pvParameters)
{
  static uint8_t TempStr[25];
  RTC_TimeTypeDef   RTC_TimeStructure;
  RTC_DateTypeDef   RTC_DateStructure;
  uint8_t sec, min, hour, day, month;
  uint16_t year;

  while(1)
  {
    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
    sec    =  RTC_TimeStructure.RTC_Seconds;
    min    =  RTC_TimeStructure.RTC_Minutes;
    hour   =  RTC_TimeStructure.RTC_Hours;

    RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);

    sprintf((char *)TempStr, "%02d:%02d:%02d", hour , min, sec);

    if(GL_HomePage->Page_ID == HOME_PAGE_ID)
    {
      /* ToDo: Create task to update bundle each 1s  */

      RefreshLabel (GL_HomePage, 30,TempStr);

      if(( hour == 0 ) && (min == 0) && (sec == 0))
      {
        year =  RTC_DateStructure.RTC_Year + 2000;
        month =  RTC_DateStructure.RTC_Month;
        day =  RTC_DateStructure.RTC_Date;

        sprintf((char *)TempStr, "%02d %s %04d", day , strMonth[month-1], year);
        RefreshLabel(GL_HomePage, 31 , TempStr);
      }
    }
    vTaskDelay(1000);
  }
}

/**
  * @brief  Background main window custom draw callback
  * @param  None
  * @retval None
  */
static void DrawBackgroundZone (void)
{

  LCD_SetTextColor(0x1253);
  GL_LCD_FillRect(38, 319, 202, 75);
  GL_LCD_FillRect(0, 319, 38, 320);

  LCD_SetTextColor(GL_Black);

  LCD_DrawLine(40, 243,200,Vertical);
  LCD_DrawLine(39, 244, 245,Horizontal);
  LCD_DrawLine(39, 320, 75,Horizontal);


  LCD_SetTextColor(GL_Grey);
  LCD_DrawLine(39, 244,201,Vertical);
  LCD_DrawLine(38, 244, 245,Horizontal);
  LCD_DrawLine(38, 320, 75,Horizontal);

  GL_SetTextColor(GL_White);
  GL_SetFont(GL_FONT_SMALL);
  GL_UpdateGroupSelectionLabel (GL_Group);
  GL_DisplayAdjStringLine(10 , 250,  (uint8_t *)"STM32 Demonstration builder", GL_TRUE);
}

/**
  * @brief  Apply settings if they were changed
  * @param  None
  * @retval None
  */
void GL_HandleSystemSettingsChange (void)
{
  if(Global_Config.b.Configuration_Changed)
  {
    vu8_gPowerSaveOption = Global_Config.b.LCDPowerSavingEnabled;
    Global_Config.b.Configuration_Changed = 0;
  }
}

/**
  * @brief  Display processes running in background
  * @param  None
  * @retval None
  */
void GL_UpdateBackgroundProcessStatus (void)
{
  static uint8_t TempStr[25];
  static uint16_t prev_audio_status = 0;
  static uint16_t prev_ethernet_status = 0;

  if(GL_HomePage->Page_ID == HOME_PAGE_ID)
  {
    strcpy ((char *)TempStr, "");

    if((prev_audio_status != AudioPlayerSettings.BackgroundEnabled)||
       (Global_Config.b.Force_Background_Refresh == 1))
    {
      if(AudioPlayerSettings.BackgroundEnabled)
      {
        strcpy ((char *)TempStr, "[Audio]");
        RefreshLabel(GL_HomePage,33,TempStr);
      }
      else
      {
        strcpy ((char *)TempStr, "              ");
        RefreshLabel(GL_HomePage,33,TempStr);
        strcpy ((char *)TempStr, "");
      }

      prev_audio_status = AudioPlayerSettings.BackgroundEnabled;
    }

    if((prev_ethernet_status != EthernetSettings.BackgroundEnabled)||
       (Global_Config.b.Force_Background_Refresh == 1))
    {
      if(EthernetSettings.BackgroundEnabled)
      {
        strcat ((char *)TempStr, "[D.Ctl]");
        RefreshLabel(GL_HomePage,33,TempStr);
      }
      prev_ethernet_status = EthernetSettings.BackgroundEnabled;
    }
    if(Global_Config.b.Force_Background_Refresh == 1)
    {
      Global_Config.b.Force_Background_Refresh = 0;
    }
  }
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
