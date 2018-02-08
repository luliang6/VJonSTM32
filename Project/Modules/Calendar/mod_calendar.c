/**
  ******************************************************************************
  * @file    mod_calendar.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Graphical window and events implementation of the calendar module
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
#include "mod_calendar.h"
#include "time_utils.h"
#include "gl_calendar_res.c"
#include "gl_mgr.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CALENDAR_MAIN_PAGE           MOD_CALENDAR_UID + 0
#define CALENDAR_CLOCK               MOD_CALENDAR_UID + 1
#define CALENDAR_DATE                MOD_CALENDAR_UID + 2
#define CALENDAR_TIME_SETTINGS       MOD_CALENDAR_UID + 3
#define CALENDAR_DATE_SETTINGS       MOD_CALENDAR_UID + 4
#define CALENDAR_ALARM_SETTINGS      MOD_CALENDAR_UID + 5

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/*Graphical draw process */
static void CALENDAR_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex);
static void CALENDAR_CreatePage(uint8_t Page);
static void CALENDAR_Startup (void);
static void CALENDAR_Background(void);


/* Control Actions */
static void goto_calendar (void);
static void return_from_calendar (void);
static void return_to_menu (void);


static void goto_next_month (void);
static void goto_previous_month (void);

static void goto_clock (void);
static void return_from_clock (void);

static void goto_time_settings (void);
static void goto_date_settings (void);
static void return_from_time_settings (void);
static void return_from_date_settings (void);

static void goto_alarm_settings (void);
static void return_from_alarm_settings (void);
static void save_settings (void);
static void alarm_background(void);

static void IncSec (void);
static void DecSec (void);

static void IncMin (void);
static void DecMin (void);

static void IncHour (void);
static void DecHour(void);


static void IncaSec (void);
static void DecaSec (void);

static void IncaMin (void);
static void DecaMin (void);

static void IncaHour (void);
static void DecaHour(void);


static void IncDay (void);
static void DecDay (void);

static void IncMonth (void);
static void DecMonth (void);

static void IncYear (void);
static void DecYear(void);

static void calendar_background(void);

/* Private variables ---------------------------------------------------------*/

static GL_Page_TypeDef *CalendarMainPage;
static GL_Page_TypeDef *ClockPage;
static GL_Page_TypeDef *CalendarPage;
static GL_Page_TypeDef *TimeSettingsPage;
static GL_Page_TypeDef *DateSettingsPage;
static GL_Page_TypeDef *AlarmSettingsPage;
static GL_Page_TypeDef *Current_Page;

static int8_t sec, min, hour;
static int8_t asec, amin, ahour;
static int8_t day, month;
static int16_t  year;

RTC_TimeTypeDef   RTC_TimeStructure;
RTC_DateTypeDef   RTC_DateStructure;
RTC_AlarmTypeDef  RTC_AlarmStructure;

MOD_InitTypeDef  mod_calendar =
{
  MOD_CALENDAR_UID,
  MOD_CALENDAR_VER,
  (uint8_t *)"Calendar",
  (uint8_t *)calendar_icon,
  CALENDAR_Startup,
  CALENDAR_Background,
  NULL,
};

/* Private functions ---------------------------------------------------------*/

/**
* @brief  Handles the Module startup action and display the main menu
* @param  None
* @retval None
*/
static void CALENDAR_Startup (void)
{
  vTaskSuspend(Core_Time_Task_Handle);
  CALENDAR_SwitchPage(GL_HomePage, CALENDAR_MAIN_PAGE);
}

/**
* @brief  Closes parent page and display a child sub-page
* @param  pParent : parent page
* @param  PageIndex : sub-page index
* @retval None
*/
static void CALENDAR_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex)
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
  
  CALENDAR_CreatePage(PageIndex);

  switch (PageIndex)
  {
  case CALENDAR_MAIN_PAGE:
    NextPage = CalendarMainPage;
    break;

  case CALENDAR_CLOCK:
    NextPage = ClockPage;
    break;

  case CALENDAR_DATE:
    NextPage = CalendarPage;
    break;

  case CALENDAR_TIME_SETTINGS:
    NextPage = TimeSettingsPage;
    break;

  case CALENDAR_DATE_SETTINGS:
    NextPage = DateSettingsPage;
    break;

  case CALENDAR_ALARM_SETTINGS:
    NextPage = AlarmSettingsPage;
    break;

  default:
    break;
  }
  Current_Page = NextPage;
  RefreshPage(NextPage);
}

/**
* @brief  Creates a child sub-page
* @param  Page : Page handler
* @retval None
*/
static void CALENDAR_CreatePage(uint8_t Page)
{

  GL_PageControls_TypeDef* BackButton;
  GL_PageControls_TypeDef* SaveSettings;
  GL_PageControls_TypeDef* Icon;
  GL_PageControls_TypeDef* SecP, *SecM, *MinP, *MinM, *HourP, *HourM, *LabelSec, *LabelMin, *LabelHour ;
  GL_PageControls_TypeDef* DayP, *DayM, *MonthM, *MonthP, *YearM, *YearP, *LabelDay, *LabelMonth, *LabelYear ;
  GL_PageControls_TypeDef* LabelTime;
  GL_PageControls_TypeDef* LabelDate;
  GL_PageControls_TypeDef* AlarmEnable;

  uint8_t TempStr[25];

  GL_SetBackColor( GL_White );
  GL_SetTextColor( GL_Blue );

  switch (Page)
  {
  case CALENDAR_MAIN_PAGE:
    {
      CalendarMainPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( CalendarMainPage ,CALENDAR_MAIN_PAGE);
      Icon = NewIcon (5, calendar_icon, 50, 50, MOD_NullFunc);
      AddPageControlObj(185, 40, Icon, CalendarMainPage);

      GL_SetMenuItem(CalendarMainPage, (uint8_t *)"Digital Clock", 0, goto_clock );
      GL_SetMenuItem(CalendarMainPage, (uint8_t *)"Calendar", 1, goto_calendar );
      GL_SetMenuItem(CalendarMainPage, (uint8_t *)"Settings", 2, goto_time_settings );
      GL_SetMenuItem(CalendarMainPage, (uint8_t *)"Return", 3, return_to_menu );
      GL_SetPageHeader(CalendarMainPage , (uint8_t *)"Calendar Menu");
    }
    break;
    
  case CALENDAR_CLOCK:
    {

      ClockPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( ClockPage ,CALENDAR_CLOCK);

      GL_SetPageHeader(ClockPage , (uint8_t *)"Digital Clock");

      BackButton = NewButton(1, (uint8_t *)" Return ", return_from_clock);
      AddPageControlObj(195, 212, BackButton, ClockPage);

      /* Draw Time */
      Icon = NewIcon (4, clock_icon, 50, 50, MOD_NullFunc);
      AddPageControlObj(185, 40, Icon, ClockPage);

      /* Get info from RTC here */
      RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);

      sec    =  RTC_TimeStructure.RTC_Seconds;
      min    =  RTC_TimeStructure.RTC_Minutes;
      hour   =  RTC_TimeStructure.RTC_Hours;

      sprintf((char *)TempStr, "%02d:%02d:%02d", hour , min, sec);
      /* ToDo: Create task to update bundle each 1s*/
      LabelTime  = NewLabel(5, TempStr, GL_HORIZONTAL, GL_FONT_BIG, GL_Black, GL_FALSE);
      AddPageControlObj(220, 105, LabelTime, ClockPage);


      RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);

      year =  RTC_DateStructure.RTC_Year + 2000;
      month =  RTC_DateStructure.RTC_Month;
      day =  RTC_DateStructure.RTC_Date;

      sprintf((char *)TempStr, "%02d %s %04d", day , strMonth[month-1], year);
      LabelDate  = NewLabel(6, TempStr, GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(202, 140, LabelDate, ClockPage);
    }
    break;

  case CALENDAR_DATE:
    {

      CalendarPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( CalendarPage , CALENDAR_DATE);

      GL_SetPageHeader(CalendarPage , (uint8_t *)"Calendar");

      BackButton = NewButton(1, (uint8_t *)" Return ", return_from_calendar);
      AddPageControlObj(195, 212, BackButton, CalendarPage);

      RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);

      sec    =  RTC_TimeStructure.RTC_Seconds;
      min    =  RTC_TimeStructure.RTC_Minutes;
      hour   =  RTC_TimeStructure.RTC_Hours;

      sprintf((char *)TempStr, "%02d:%02d:%02d" , hour, min, sec);
      LabelTime = NewLabel(2, TempStr, GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(65, 30, LabelTime, CalendarPage);

      /* Get info from RTC */
      RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);

      year =  RTC_DateStructure.RTC_Year + 2000;
      month =  RTC_DateStructure.RTC_Month;
      day =  RTC_DateStructure.RTC_Date;

      Icon = NewIcon (2, forward_icon, 30, 30, goto_next_month);
      AddPageControlObj(43, 57, Icon, CalendarPage);

      Icon = NewIcon (3, back_icon, 30, 30, goto_previous_month);
      AddPageControlObj(315, 57, Icon, CalendarPage);

      /* Display the current month calendar */
      GL_AddCalendar (CalendarPage,
                      5, /* id */
                      day, /* Current day*/
                      month,  /* Current month */
                      year);/* Current year */
    }
    break;

  case CALENDAR_TIME_SETTINGS:
    {

      RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);

      sec    =  RTC_TimeStructure.RTC_Seconds;
      min    =  RTC_TimeStructure.RTC_Minutes;
      hour   =  RTC_TimeStructure.RTC_Hours;

      TimeSettingsPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( TimeSettingsPage, CALENDAR_TIME_SETTINGS);

      GL_SetPageHeader(TimeSettingsPage , (uint8_t *)"Time Setting");

      BackButton = NewButton(1, (uint8_t *)" Next > ", goto_date_settings);
      AddPageControlObj(150, 212, BackButton, TimeSettingsPage);

      BackButton = NewButton(14, (uint8_t *)" Cancel ", return_from_time_settings);
      AddPageControlObj(230, 212, BackButton, TimeSettingsPage);

      HourP = NewButton(2, (uint8_t *)" + ", IncHour);
      AddPageControlObj(230,  60, HourP, TimeSettingsPage);

      HourM = NewButton(3, (uint8_t *)" - ", DecHour);
      AddPageControlObj(230, 140, HourM, TimeSettingsPage);

      MinP = NewButton(4, (uint8_t *)" + ", IncMin);
      AddPageControlObj(180,  60, MinP, TimeSettingsPage);

      MinM = NewButton(5, (uint8_t *)" - ", DecMin);
      AddPageControlObj(180, 140, MinM, TimeSettingsPage);

      SecP = NewButton(6, (uint8_t *)" + ", IncSec);
      AddPageControlObj(130,  60, SecP, TimeSettingsPage);

      SecM = NewButton(7, (uint8_t *)" - ", DecSec);
      AddPageControlObj(130, 140, SecM, TimeSettingsPage);


      sprintf((char *)TempStr, "%02d:" , hour);
      LabelHour  = NewLabel(8, TempStr, GL_HORIZONTAL, GL_FONT_BIG, GL_Black, GL_FALSE);
      AddPageControlObj(230, 105, LabelHour, TimeSettingsPage);

      sprintf((char *)TempStr, "%02d:" , min);
      LabelMin  = NewLabel(9, TempStr, GL_HORIZONTAL, GL_FONT_BIG, GL_Black, GL_FALSE);
      AddPageControlObj(180, 105, LabelMin, TimeSettingsPage);

      sprintf((char *)TempStr, "%02d" , sec);
      LabelSec  = NewLabel(10, TempStr, GL_HORIZONTAL, GL_FONT_BIG, GL_Black, GL_FALSE);
      AddPageControlObj(130, 105, LabelSec, TimeSettingsPage);


      LabelHour  = NewLabel(11, (uint8_t *)"Hour", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(227, 45, LabelHour, TimeSettingsPage);


      LabelMin  = NewLabel(12, (uint8_t *)"Min", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(176, 45, LabelMin, TimeSettingsPage);

      LabelSec  = NewLabel(13, (uint8_t *)"Sec", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(126, 45, LabelSec, TimeSettingsPage);
    }
    break;

  case CALENDAR_DATE_SETTINGS:
    {

      RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);

      year =  RTC_DateStructure.RTC_Year + 2000;
      month =  RTC_DateStructure.RTC_Month;
      day =  RTC_DateStructure.RTC_Date;


      DateSettingsPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( DateSettingsPage , CALENDAR_TIME_SETTINGS);

      GL_SetPageHeader(DateSettingsPage , (uint8_t *)"Date Setting");

      BackButton = NewButton(1, (uint8_t *)" Next > ", goto_alarm_settings);
      AddPageControlObj(150, 212, BackButton, DateSettingsPage);


      BackButton = NewButton(14, (uint8_t *)" Cancel ", return_from_date_settings);
      AddPageControlObj(230, 212, BackButton, DateSettingsPage);


      DayP = NewButton(2, (uint8_t *)" + ", IncDay);
      AddPageControlObj(240,  60, DayP, DateSettingsPage);

      DayM = NewButton(3, (uint8_t *)" - ", DecDay);
      AddPageControlObj(240, 140, DayM, DateSettingsPage);

      MonthP = NewButton(4, (uint8_t *)"  +  ", IncMonth);
      AddPageControlObj(190,  60, MonthP, DateSettingsPage);

      MonthM = NewButton(5, (uint8_t *)"  -  ", DecMonth);
      AddPageControlObj(190, 140, MonthM, DateSettingsPage);

      YearP = NewButton(6, (uint8_t *)"   +   ", IncYear);
      AddPageControlObj(125,  60, YearP, DateSettingsPage);

      YearM = NewButton(7, (uint8_t *)"   -   ", DecYear);
      AddPageControlObj(125, 140, YearM, DateSettingsPage);


      sprintf((char *)TempStr, "%02d" , day);
      LabelDay  = NewLabel(8, TempStr, GL_HORIZONTAL, GL_FONT_BIG, GL_Black, GL_FALSE);
      AddPageControlObj(240, 105, LabelDay, DateSettingsPage);

      sprintf((char *)TempStr, (char *)strMonth[month-1] , month);
      LabelMonth  = NewLabel(9, TempStr, GL_HORIZONTAL, GL_FONT_BIG, GL_Black, GL_FALSE);
      AddPageControlObj(190, 105, LabelMonth, DateSettingsPage);

      sprintf((char *)TempStr, "%04d" , year);
      LabelYear  = NewLabel(10, TempStr, GL_HORIZONTAL, GL_FONT_BIG, GL_Black, GL_FALSE);
      AddPageControlObj(125, 105, LabelYear, DateSettingsPage);


      LabelDay  = NewLabel(11, (uint8_t *)"Day", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(235, 45, LabelDay, DateSettingsPage);
      
      
      LabelMonth  = NewLabel(12, (uint8_t *)"Month", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(185, 45, LabelMonth, DateSettingsPage);
      
      LabelYear  = NewLabel(13, (uint8_t *)"  Year", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(121, 45, LabelYear, DateSettingsPage);
    }
    break;

  case CALENDAR_ALARM_SETTINGS:
    {

      RTC_GetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);

      asec  = RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds;
      amin  = RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes;
      ahour = RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours;

      AlarmSettingsPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( AlarmSettingsPage, CALENDAR_TIME_SETTINGS);

      GL_SetPageHeader(AlarmSettingsPage , (uint8_t *)"Alarm Setting");

      BackButton = NewButton(1, (uint8_t *)"   Return   ", return_from_alarm_settings);
      AddPageControlObj(260, 212, BackButton, AlarmSettingsPage);

      HourP = NewButton(2, (uint8_t *)" + ", IncaHour);
      AddPageControlObj(230,  60, HourP, AlarmSettingsPage);

      HourM = NewButton(3, (uint8_t *)" - ", DecaHour);
      AddPageControlObj(230, 140, HourM, AlarmSettingsPage);

      MinP = NewButton(4, (uint8_t *)" + ", IncaMin);
      AddPageControlObj(180,  60, MinP, AlarmSettingsPage);

      MinM = NewButton(5, (uint8_t *)" - ", DecaMin);
      AddPageControlObj(180, 140, MinM, AlarmSettingsPage);

      SecP = NewButton(6, (uint8_t *)" + ", IncaSec);
      AddPageControlObj(130,  60, SecP, AlarmSettingsPage);

      SecM = NewButton(7, (uint8_t *)" - ", DecaSec);
      AddPageControlObj(130, 140, SecM, AlarmSettingsPage);

      sprintf((char *)TempStr, "%02d:" , ahour);
      LabelHour  = NewLabel(8, TempStr, GL_HORIZONTAL, GL_FONT_BIG, GL_Black, GL_FALSE);
      AddPageControlObj(230, 105, LabelHour, AlarmSettingsPage);

      sprintf((char *)TempStr, "%02d:" , amin);
      LabelMin  = NewLabel(9, TempStr, GL_HORIZONTAL, GL_FONT_BIG, GL_Black, GL_FALSE);
      AddPageControlObj(180, 105, LabelMin, AlarmSettingsPage);

      sprintf((char *)TempStr, "%02d" , asec);
      LabelSec  = NewLabel(10, TempStr, GL_HORIZONTAL, GL_FONT_BIG, GL_Black, GL_FALSE);
      AddPageControlObj(130, 105, LabelSec, AlarmSettingsPage);

      LabelHour  = NewLabel(11, (uint8_t *)"Hour", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(227, 45, LabelHour, AlarmSettingsPage);

      LabelMin  = NewLabel(12, (uint8_t *)"Min", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(176, 45, LabelMin, AlarmSettingsPage);

      LabelSec  = NewLabel(13, (uint8_t *)"Sec", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(126, 45, LabelSec, AlarmSettingsPage);

      SaveSettings = NewButton(14, (uint8_t *)"Save & Quit ", save_settings);
      AddPageControlObj(160, 212, SaveSettings, AlarmSettingsPage);

      AlarmEnable = NewCheckbox(15, (uint8_t *)" Enable/Modify Alarm", GL_TRUE, MOD_NullFunc);
      AddPageControlObj(225, 175, AlarmEnable, AlarmSettingsPage);
    }
    break;
  default:
    break;
  }
}

/**
* @brief  Returns to the global main menu action
* @param  None
* @retval None
*/
static void return_to_menu (void)
{
  CALENDAR_SwitchPage(CalendarMainPage, PAGE_MENU);
  CalendarMainPage = NULL;
}


/**
* @brief  Displays the digital clock page
* @param  None
* @retval None
*/
static void goto_clock (void)
{
  CALENDAR_SwitchPage(CalendarMainPage, CALENDAR_CLOCK);
  CalendarMainPage = NULL;
}

/**
* @brief  Returns to module main menu action from the clock page
* @param  None
* @retval None
*/
static void return_from_clock (void)
{
  CALENDAR_SwitchPage(ClockPage, CALENDAR_MAIN_PAGE);
  ClockPage = NULL;
}

/**
* @brief  Displays the calendar page
* @param  None
* @retval None
*/
static void goto_calendar (void)
{
  CALENDAR_SwitchPage(CalendarMainPage, CALENDAR_DATE);
  CalendarMainPage = NULL;
}

/**
* @brief  returns to the module main menu from the calendar page
* @param  None
* @retval None
*/
static void return_from_calendar (void)
{
  CALENDAR_SwitchPage(CalendarPage, CALENDAR_MAIN_PAGE);
  CalendarPage = NULL;
}

/**
* @brief  Displays the time settings page
* @param  None
* @retval None
*/
static void goto_time_settings (void)
{
  CALENDAR_SwitchPage(CalendarMainPage, CALENDAR_TIME_SETTINGS);
  CalendarMainPage = NULL;
}

/**
* @brief  Displays the date settings page
* @param  None
* @retval None
*/
static void goto_date_settings (void)
{
  CALENDAR_SwitchPage(TimeSettingsPage, CALENDAR_DATE_SETTINGS);
  TimeSettingsPage = NULL;
}

/**
* @brief  Displays the alarm settings page
* @param  None
* @retval None
*/
static void goto_alarm_settings (void)
{
  CALENDAR_SwitchPage(DateSettingsPage, CALENDAR_ALARM_SETTINGS);
  DateSettingsPage = NULL;
}
/**
* @brief  Returns to the module main menu from the alarm settings page
* @param  None
* @retval None
*/
static void return_from_alarm_settings (void)
{
  CALENDAR_SwitchPage(AlarmSettingsPage, CALENDAR_MAIN_PAGE);
  AlarmSettingsPage = NULL;
}


/**
* @brief  Returns the module main menu from the timr settings page
* @param  None
* @retval None
*/
static void return_from_time_settings (void)
{
  CALENDAR_SwitchPage(TimeSettingsPage, CALENDAR_MAIN_PAGE);
  TimeSettingsPage = NULL;
}

/**
* @brief  Returns the module main menu from the date settings page
* @param  None
* @retval None
*/
static void return_from_date_settings (void)
{
  CALENDAR_SwitchPage(DateSettingsPage, CALENDAR_MAIN_PAGE);
  DateSettingsPage = NULL;
}

/**
* @brief  Saves the calendar settings into the backup memory
* @param  None
* @retval None
*/
static void save_settings (void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;

  if ( RTC_Error == 0)
  {
    RTC_TimeStructure.RTC_Seconds = sec;
    RTC_TimeStructure.RTC_Minutes = min;
    RTC_TimeStructure.RTC_Hours   = hour;

    RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);

    RTC_DateStructure.RTC_Year  = year - 2000;
    RTC_DateStructure.RTC_Month = month;
    RTC_DateStructure.RTC_Date  = day;
    RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);

    if (GL_IsChecked (AlarmSettingsPage , 15) == GL_TRUE)
    {
      /* EXTI configuration */
      EXTI_ClearITPendingBit(EXTI_Line17);
      EXTI_InitStructure.EXTI_Line = EXTI_Line17;
      EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
      EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
      EXTI_InitStructure.EXTI_LineCmd = ENABLE;
      EXTI_Init(&EXTI_InitStructure);

      /* Enable the RTC Alarm Interrupt */
      NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
      NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
      NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
      NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
      NVIC_Init(&NVIC_InitStructure);

      /* Enable the alarm */
      RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;
      RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = asec;
      RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = amin;
      RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = ahour;

      RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
      RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);

      RTC_ITConfig(RTC_IT_ALRA, ENABLE);
      RTC_AlarmCmd(RTC_Alarm_A, ENABLE);



    }
  }
  return_from_alarm_settings();
}

/**
* @brief  Increment time seconds
* @param  None
* @retval None
*/
static void IncSec (void)
{
  uint8_t TempStr[25];

  if (++sec > 59)
  {
    sec = 0;
  }
  sprintf((char *)TempStr, "%02d" , sec);
  RefreshLabel(Current_Page , 10 , TempStr);
}

/**
* @brief  Decrement time seconds
* @param  None
* @retval None
*/
static void DecSec (void)
{
  uint8_t TempStr[20];

  if (--sec < 0)
  {
    sec = 59;
  }
  sprintf((char *)TempStr, "%02d" , sec);
  RefreshLabel(Current_Page , 10 , TempStr);
}

/**
* @brief  Increment time minutes
* @param  None
* @retval None
*/
static void IncMin (void)
{
  uint8_t TempStr[20];

  if (++min > 59)
  {
    min = 0;
  }
  sprintf((char *)TempStr, "%02d:" , min);
  RefreshLabel(Current_Page , 9 , TempStr);
}

/**
* @brief  Decrement time minutes
* @param  None
* @retval None
*/
static void DecMin (void)
{
  uint8_t TempStr[20];

  if (--min < 0)
  {
    min = 59;
  }
  sprintf((char *)TempStr, "%02d:" , min);
  RefreshLabel(Current_Page , 9 , TempStr);
}

/**
* @brief  Increment time hours
* @param  None
* @retval None
*/
static void IncHour (void)
{
  uint8_t TempStr[20];

  if (++hour > 23)
  {
    hour = 0;
  }
  sprintf((char *)TempStr, "%02d:" , hour);
  RefreshLabel(Current_Page , 8 , TempStr);

}

/**
* @brief  Decrement time hours
* @param  None
* @retval None
*/
static void DecHour(void)
{
  uint8_t TempStr[20];

  if (--hour < 0)
  {
    hour = 23;
  }
  sprintf((char *)TempStr, "%02d:" , hour);
  RefreshLabel(Current_Page , 8 , TempStr);
}


/**
* @brief  Increment alarm seconds
* @param  None
* @retval None
*/
static void IncaSec (void)
{
  uint8_t TempStr[20];

  if (++asec > 59)
  {
    asec = 0;
  }
  sprintf((char *)TempStr, "%02d" , asec);
  RefreshLabel(Current_Page , 10 , TempStr);
}

/**
* @brief  Decrement alarm seconds
* @param  None
* @retval None
*/
static void DecaSec (void)
{
  uint8_t TempStr[20];

  if (--asec < 0)
  {
    asec = 59;
  }
  sprintf((char *)TempStr, "%02d" , asec);
  RefreshLabel(Current_Page , 10 , TempStr);
}

/**
* @brief  Increment alarm minutes
* @param  None
* @retval None
*/
static void IncaMin (void)
{
  uint8_t TempStr[20];

  if (++amin > 59)
  {
    amin = 0;
  }
  sprintf((char *)TempStr, "%02d:" , amin);
  RefreshLabel(Current_Page , 9 , TempStr);
}

/**
* @brief  Decrement alarm minutes
* @param  None
* @retval None
*/
static void DecaMin (void)
{
  uint8_t TempStr[20];

  if (--amin < 0)
  {
    amin = 59;
  }
  sprintf((char *)TempStr, "%02d:" , amin);
  RefreshLabel(Current_Page , 9 , TempStr);
}

/**
* @brief  Increment alarm hours
* @param  None
* @retval None
*/
static void IncaHour (void)
{
  uint8_t TempStr[20];

  if (++ahour > 23)
  {
    ahour = 0;
  }
  sprintf((char *)TempStr, "%02d:" , ahour);
  RefreshLabel(Current_Page , 8 , TempStr);

}

/**
* @brief  Derement alarm hours
* @param  None
* @retval None
*/
static void DecaHour(void)
{
  uint8_t TempStr[20];

  if (--ahour < 0)
  {
    ahour = 23;
  }
  sprintf((char *)TempStr, "%02d:" , ahour);
  RefreshLabel(Current_Page , 8 , TempStr);
}

/**
* @brief  Increment date days
* @param  None
* @retval None
*/
static void IncDay (void)
{
  uint8_t TempStr[20];

  if (++day > GetMaxDays(month, year))
  {
    day = 1;
  }
  sprintf((char *)TempStr, "%02d" , day);
  RefreshLabel(Current_Page , 8 , TempStr);
}

/**
* @brief  Decrement date days
* @param  None
* @retval None
*/
static void DecDay (void)
{
  uint8_t TempStr[20];

  if (--day < 1)
  {
    day = GetMaxDays(month, year);
  }
  sprintf((char *)TempStr, "%02d" , day);
  RefreshLabel(Current_Page , 8 , TempStr);
}

/**
* @brief  Increments date months
* @param  None
* @retval None
*/
static void IncMonth (void)
{
  uint8_t TempStr[20];
  if (++month > 12)
  {
    month = 1;
  }
  
  if (day > GetMaxDays(month, year))
  {
    day = MonLen[month-1] - 1;
    sprintf((char *)TempStr, "%02d" , day);
    RefreshLabel(Current_Page , 8 , TempStr);
  }
  RefreshLabel(Current_Page , 9 , strMonth[month-1]);
}

/**
* @brief  Decrement date months
* @param  None
* @retval None
*/
static void DecMonth (void)
{
  uint8_t TempStr[20];

  if (--month < 1)
  {
    month = 12;
  }

  if (day > GetMaxDays(month, year))
  {
    day = MonLen[month-1] - 1;
    sprintf((char *)TempStr, "%02d" , day);
    RefreshLabel(Current_Page , 8 , TempStr);
  }
  RefreshLabel(Current_Page , 9 , strMonth[month-1]);
}

/**
* @brief  Increment date years
* @param  None
* @retval None
*/
static void IncYear (void)
{
  uint8_t TempStr[20];

  if (year > 9999)
  {
    return;
  }
  year++;
  sprintf((char *)TempStr, "%04d" , year);
  RefreshLabel(Current_Page , 10 , TempStr);

}

/**
* @brief  Decrement date years
* @param  None
* @retval None
*/
static void DecYear(void)
{
  uint8_t TempStr[20];

  if (year == 2000)
  {
    return;
  }
  year--;
  sprintf((char *)TempStr, "%04d" , year);
  RefreshLabel(Current_Page , 10 , TempStr);
}
/**
* @brief  Time background process callback 
* @param  None
* @retval None
*/
static void calendar_background (void)
{
  uint8_t TempStr[20];

  if(( ClockPage != NULL )||( CalendarPage != NULL ))
  {
    if((ClockPage->Page_ID == CALENDAR_CLOCK) || (CalendarPage->Page_ID == CALENDAR_DATE))
    {

      RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
      sec    =  RTC_TimeStructure.RTC_Seconds;
      min    =  RTC_TimeStructure.RTC_Minutes;
      hour   =  RTC_TimeStructure.RTC_Hours;

      RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);

      sprintf((char *)TempStr, "%02d:%02d:%02d", hour , min, sec);

      if(ClockPage->Page_ID == CALENDAR_CLOCK)
      {

        RefreshLabel (ClockPage, 5, TempStr);

        if (( hour == 0 ) && (min == 0) && (sec == 0))
        {
          year =  RTC_DateStructure.RTC_Year + 2000;
          month =  RTC_DateStructure.RTC_Month;
          day =  RTC_DateStructure.RTC_Date;

          sprintf((char *)TempStr, "%02d %s %04d", day , strMonth[month-1], year);
          RefreshLabel(ClockPage, 6 , TempStr);
        }
      }

      if (CalendarPage->Page_ID == CALENDAR_DATE)
      {
        RefreshLabel (CalendarPage, 2, TempStr);

        if (( hour == 0 ) && (min == 0) && (sec == 0))
        {

          year =  RTC_DateStructure.RTC_Year + 2000;
          month =  RTC_DateStructure.RTC_Month;
          day =  RTC_DateStructure.RTC_Date;

          GL_UpdateCalendar (CalendarPage,
                             5, /* id  */
                             day, /* Current day */
                             month,  /* Current month */
                             year);/* Current year */
        }
      }
    }
  }
}

/**
* @brief  Display next month in the calendar page
* @param  None
* @retval None
*/
static void goto_next_month (void)
{
  if (month < 12)
  {
    month ++;

  }
  else
  {
    month = 1;

    if (year < 10000)
    {
      year ++;
    }
    else
    {
      return;
    }

  }
  /* Display the current month calendar */
  GL_UpdateCalendar (CalendarPage,
                     5, /* id  */
                     day, /* Current day */
                     month,  /* Current month */
                     year);/* Current year */
}

/**
* @brief  Display previous month in the calendar page
* @param  None
* @retval None
*/
static void goto_previous_month (void)
{

  if (month > 1)
  {
    month --;
  }
  else
  {
    month = 12;


    if (year > 2000)
    {
      year --;
    }
    else
    {
      return;
    }
  }

  /* Display the current month calendar */
  GL_UpdateCalendar (CalendarPage,
                     5, /* id */
                     day, /* Current day */
                     month,  /* Current month */
                     year);/* Current year */
}

/**
* @brief  display the alarm ring
* @param  None
* @retval None
*/
static void alarm_background(void)
{
  GL_Page_TypeDef* cParent;

  if(RTC_HandlerFlag == 1)
  {
    RTC_HandlerFlag = 0;


    if (PagesList[0]->Page_Visible == GL_TRUE)
    {
      cParent = PagesList[0];
    }
    else
    {
      cParent = PagesList[1];
    }

    MESSAGES_DirectEx(cParent, MESSAGE_ALARM_RING);
  }
}

/**
* @brief  Calendar module background process
* @param  None
* @retval None
*/
void CALENDAR_Background (void)
{
  alarm_background();
  calendar_background();

}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
