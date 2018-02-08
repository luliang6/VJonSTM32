/**
  ******************************************************************************
  * @file    mod_recorder.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief    Graphical window and events implementation of the recorder module  
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
#include "mod_core.h"
#include "mod_recorder.h"
#include "mod_audio.h"
#include "mod_filemgr.h"
#include "mod_system.h"
#include "app_recorder.h"
#include "gl_recorder_res.c"
#include "app_audio.h"
#include "gl_mgr.h"
#include "app_rtc.h"

/* Private typedef -----------------------------------------------------------*/

typedef union _RECORDER_CONFIG_TypeDef
{
  uint32_t d32;
  struct
  {
    uint32_t SampleRate :
      8;
    uint32_t Format :
      8;
    uint32_t Reserved :
      30;
  }
  b;
} RECORDER_CONFIG_TypeDef ;

/* Private define ------------------------------------------------------------*/
#define RECORDER_MAIN_PAGE           MOD_RECORDER_UID + 0
#define RECORDER_RECORD_PAGE         MOD_RECORDER_UID + 1
#define RECORDER_SETTINGS_PAGE       MOD_RECORDER_UID + 2
#define RECORDER_ERROR_PAGE          MOD_RECORDER_UID + 3

#define REC_MAINICON_ID              1
#define REC_RECEXIT_ID               2

#define RECID                        0x10
#define REC_DST_PATH                BKPSRAM_BASE  + 0x200
#define REC_MICLOGO_ID               RECID + 1
#define REC_TIME_ID                  RECID + 2
#define REC_FILNAME_ID               RECID + 3
#define REC_STRT_ID                  RECID + 4
#define REC_PLAY_ID                  RECID + 5
#define REC_STOP_ID                  RECID + 6
#define REC_PAUSE_ID                 RECID + 7
#define REC_CANCEL_ID                RECID + 8
#define REC_COMBO1_ID                RECID + 9
#define REC_FMTLBL_ID                RECID + 10
#define REC_SPLLBL_ID                RECID + 11
#define REC_COMBO2_ID                RECID + 12
#define REC_SETEXIT_ID               RECID + 13


#define REC_GL_IDLE                  0
#define REC_GL_RECORDING             1
#define REC_DST_PATH_LENGTH          19
#define MAX_RECORDING_TIME           (5 * 60) /* 5 Min */
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/*Graphical draw process */
static void RECORDER_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex);
static void RECORDER_CreatePage(uint8_t Page);
static void RECORDER_Startup (void);
static void RECORDER_background(void);
static void RECORDER_GetDefaultDirectory (void);

/* Control Actions */
static void return_to_menu (void);
static void go_to_record_page (void);
static void go_to_settings_page (void);
static void RECORDER_Cleanup (uint8_t disconnect_source);
static void return_from_settings(void);
static void set_encoder_wav(void);
static void return_from_errorpage (void);
static void Check_DefaultPath (char *path);
static void Sel_folder (void);
static void Update_folder (void);
static void Exit_recorder (void);

/* Recorder functional API */
static void RECORDER_Start  (void);
static void RECORDER_Play   (void);
static void RECORDER_Stop   (void);
static void RECORDER_Pause  (void);
static void RECORDER_Cancel (void);


/* Private variables ---------------------------------------------------------*/
static GL_Page_TypeDef *RecorderMainPage;
static GL_Page_TypeDef *RecordPage;
static GL_Page_TypeDef *SettingsPage;
static GL_Page_TypeDef *RecorderErrorPage;

__IO uint32_t RecorderSampleRate = 8000;
static uint32_t RecoveryRecorderCounter = 0;
static uint32_t REC_GL_State = REC_GL_IDLE;


uint8_t RecFileName[25]; 

uint8_t      REC_DefaultPath[129];
RECORDER_CONFIG_TypeDef RecCfg;
uint8_t      Recorder_UsedStorage = 0xFF;

MOD_InitTypeDef  mod_recorder =
{
  MOD_RECORDER_UID,
  MOD_RECORDER_VER,
  (uint8_t *)"Recorder", 
  (uint8_t *)recorder_icon,
  RECORDER_Startup,
  RECORDER_background,
  RECORDER_Cleanup,
};

/* Private functions ---------------------------------------------------------*/

/**
* @brief    Handle the Module startup action and display the main menu
  * @param  None
  * @retval None
  */

static void RECORDER_Startup (void)
{
  vTaskSuspend(Core_Time_Task_Handle);

  Recorder_UsedStorage = 0xFF;

  if (AudioPlayerSettings.BackgroundEnabled == 0)
  {
    /* Switch to the recorder page */
    RECORDER_SwitchPage(GL_HomePage, RECORDER_MAIN_PAGE);
  }
  else
  {
    /* Switch to the error page */
    RECORDER_SwitchPage(GL_HomePage, RECORDER_ERROR_PAGE);
  }
}

/**
  * @brief  Close parent page and display a child sub-page 
  * @param  pParent : parent page
  * @param  PageIndex : sub-page index
  * @retval None
  */

static void RECORDER_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex)
{
 /* Switch to new page, and free previous one. */
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

  RECORDER_CreatePage(PageIndex);

  switch (PageIndex)
  {
    case RECORDER_MAIN_PAGE:
      NextPage = RecorderMainPage;
      break;

    case RECORDER_RECORD_PAGE:
      NextPage = RecordPage;
      break;

    case RECORDER_SETTINGS_PAGE:
      NextPage = SettingsPage;
      break;

  case RECORDER_ERROR_PAGE:
      NextPage = RecorderErrorPage;
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

static void RECORDER_CreatePage(uint8_t Page)
{

  GL_PageControls_TypeDef* item;
  GL_PageControls_TypeDef* TimeInfo;
  GL_PageControls_TypeDef* ExitEventButton;
  GL_ComboBoxGrp_TypeDef* pTmp;

  uint8_t path[129];

  GL_SetBackColor( GL_White );
  GL_SetTextColor( GL_Blue );

  switch (Page)
  {
  case RECORDER_MAIN_PAGE:
    {
      RecorderMainPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( RecorderMainPage, RECORDER_MAIN_PAGE ); 
      item = NewIcon (REC_MAINICON_ID, recorder_icon,50,50,MOD_NullFunc);
      AddPageControlObj(185,40,item,RecorderMainPage);
      
      GL_SetMenuItem(RecorderMainPage, (uint8_t *)"Start Recorder",0, go_to_record_page );
      GL_SetMenuItem(RecorderMainPage,(uint8_t *)"Settings",1, go_to_settings_page );
      GL_SetMenuItem(RecorderMainPage,(uint8_t *)"Return",2, return_to_menu );
      GL_SetPageHeader(RecorderMainPage , (uint8_t *)"Recorder Menu");
    }
    break;


  case RECORDER_RECORD_PAGE:
    {

      RECORDER_GetDefaultDirectory();

      /* Store selected settings */
      MOD_GetParam(RECORDER_SETTINGS_MEM , &RecCfg.d32);

      switch (RecCfg.b.SampleRate)
      {
      case 0:
      case 1:
        RecorderSampleRate = 8000;
        break;

      case 2:
        RecorderSampleRate = 16000;
        break;
      }
      
      /* Deinit */
      AudioRecorder_DeInit();

      /* Initialize the recorder */
      AudioRecorder_Init(RecorderSampleRate);

      RecordPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( RecordPage, RECORDER_RECORD_PAGE );

      GL_SetPageHeader(RecorderMainPage , (uint8_t *)"Recorder");

      /* Static display */
      item = NewIcon (REC_MICLOGO_ID , rec_mic_logo, 110, 180, MOD_NullFunc);
      AddPageControlObj(300, 30, item, RecordPage);

      /*------------- Control icons */
      ExitEventButton = NewButton(REC_RECEXIT_ID, (uint8_t *)" Return ", Exit_recorder);
      AddPageControlObj(195, 212, ExitEventButton, RecordPage);

      TimeInfo = NewLabel(REC_TIME_ID, (uint8_t *)"00:00", GL_HORIZONTAL, GL_FONT_BIG, GL_Red, GL_FALSE);
      AddPageControlObj(130,80, TimeInfo, RecordPage); 

      item= NewLabel(REC_FILNAME_ID,(uint8_t *)"Press Record to start",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_FALSE);
      AddPageControlObj(170,40, item, RecordPage);

      item = NewIcon (REC_STRT_ID, rec_start_icon, 40, 40, RECORDER_Start);
      AddPageControlObj(170,135,item, RecordPage);

      item = NewIcon (REC_PAUSE_ID, rec_pause_icon, 40, 40, MOD_NullFunc);
      AddPageControlObj(110,135,item, RecordPage);
      
      item = NewIcon (REC_PLAY_ID, rec_play_icon, 40, 40, RECORDER_Play);
      AddPageControlObj(50,135,item, RecordPage);
    }
    break;

  case RECORDER_SETTINGS_PAGE:
    {
      MOD_GetParam(RECORDER_SETTINGS_MEM , &RecCfg.d32);

      SettingsPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( SettingsPage, RECORDER_RECORD_PAGE );

      GL_SetPageHeader( SettingsPage , (uint8_t *)"Recorder Settings");

      /* Exit button */
      ExitEventButton = NewButton(1, (uint8_t *)"  OK  ", return_from_settings);
      AddPageControlObj(195, 212, ExitEventButton, SettingsPage);

      item  = NewLabel(2, (uint8_t *)"Storage :", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(260, 140, item, SettingsPage);

      item  = NewButton(3, (uint8_t *)"...", Sel_folder);
      AddPageControlObj(120 , 150, item, SettingsPage);

      Check_DefaultPath((char *)path);

      if(strlen((char *)path) > REC_DST_PATH_LENGTH)
      {
        path[REC_DST_PATH_LENGTH - 2] = '.';
        path[REC_DST_PATH_LENGTH - 1] = '.';
        path[REC_DST_PATH_LENGTH]     =  0;
      }

      item  = NewLabel(4, path, GL_HORIZONTAL, GL_FONT_SMALL, GL_Blue, GL_FALSE);
      AddPageControlObj(260, 155, item, SettingsPage);


      /* Format selection settings */
      item  = NewLabel(4, (uint8_t *)"Audio Format: ", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(260, 50, item, SettingsPage);

      item = NewComboBoxGrp(5);

      AddComboOption (item->objPTR, (uint8_t *)"  PCM Wav  ", set_encoder_wav);
      AddPageControlObj( 260, 65, item, SettingsPage);

      /* Sample Rate selection settings */
      item  = NewLabel(6, (uint8_t *)"Sample Rate:", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(260, 95, item, SettingsPage);

      item = NewComboBoxGrp(7);

      AddComboOption (item->objPTR, (uint8_t *)"  8000 Hz  ", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)" 16000 Hz  ", MOD_NullFunc);
      AddPageControlObj( 260, 110, item, SettingsPage);

      pTmp = (GL_ComboBoxGrp_TypeDef*)(item->objPTR);

      if ((RecCfg.b.SampleRate > 0) && (RecCfg.b.SampleRate < 3))
      {
        pTmp->ComboOptions[0]->IsActive = GL_FALSE;
        pTmp->ComboOptions[RecCfg.b.SampleRate -1]->IsActive = GL_TRUE;
      }

    }
    break;

  case RECORDER_ERROR_PAGE:
    {
      RecorderErrorPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( RecorderErrorPage, RECORDER_ERROR_PAGE );

      ExitEventButton= NewButton(2, (uint8_t *)"  OK  ",return_from_errorpage);
      AddPageControlObj(185,212,ExitEventButton,RecorderErrorPage);

      item = NewIcon (3,error_icon,50,50,MOD_NullFunc);
      AddPageControlObj(185, 40, item, RecorderErrorPage);

      GL_SetPageHeader(RecorderErrorPage , (uint8_t *)"Audio Recorder : Error");
      item  = NewLabel(4,(uint8_t *)"An Audio process is already running in ",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,100,item,RecorderErrorPage); 

      item  = NewLabel(5,(uint8_t *)"background, Please stop it and try again.",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,113,item,RecorderErrorPage);
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
static void go_to_record_page (void)
{
  memset (RecFileName, 0, sizeof(RecFileName));
  RECORDER_SwitchPage(RecorderMainPage, RECORDER_RECORD_PAGE);
  RecorderMainPage = NULL;
}

/**
  * @brief  Enter the recorder settings page
  * @param  None
  * @retval None
  */
static void go_to_settings_page(void)
{
  RECORDER_SwitchPage(RecorderMainPage, RECORDER_SETTINGS_PAGE);
  RecorderMainPage = NULL;
}

/**
  * @brief  return to global main menu action 
  * @param  Disconnect_source : SD or USB disk flash
  * @retval None
  */
static void RECORDER_Cleanup (uint8_t disconnect_source)
{
  if (Recorder_UsedStorage == disconnect_source)
  {
    if(RecordPage != NULL)
    {
      if (RecordPage->Page_ID == RECORDER_RECORD_PAGE)
      {
        Exit_recorder();
      }
    }
  }
}

/**
  * @brief  return to global main menu action 
  * @param  None
  * @retval None
  */
static void Exit_recorder (void)
{
  Recorder_UsedStorage = 0xFF;

  /* Stop recording */
  AudioRecorder_Stop();

  AudioRecorder_Close();

  /* Free resources allocated by the recorder application */
  AudioRecorder_DeInit();

  RECORDER_SwitchPage(RecordPage, RECORDER_MAIN_PAGE);
  RecordPage = NULL;
}
/**
  * @brief  return to global main menu action 
  * @param  None
  * @retval None
  */
static void return_to_menu (void)
{
  RECORDER_SwitchPage(RecorderMainPage, PAGE_MENU);
  RecorderMainPage = NULL;
}

/**
  * @brief  return to global main menu action 
  * @param  None
  * @retval None
  */
static void return_from_errorpage (void)
{
  RECORDER_SwitchPage(RecorderErrorPage, PAGE_MENU);
  RecorderErrorPage = NULL;
}

/**
  * @brief  Exit from settings menu and return to main recorder menu
  * @param  None
  * @retval None
  */
static void return_from_settings(void)
{

  MOD_GetParam(RECORDER_SETTINGS_MEM , &RecCfg.d32);
  RecCfg.b.SampleRate = GetComboOptionActive(SettingsPage, 7);
  MOD_SetParam(RECORDER_SETTINGS_MEM , &RecCfg.d32);

  RECORDER_SwitchPage(SettingsPage, RECORDER_MAIN_PAGE);
  SettingsPage = NULL;
}

/**
  * @brief  Set encoder format to pcm wav
  * @param  None
  * @retval None
  */
static void set_encoder_wav(void)
{
}


/**
  * @brief  Start recording
  * @param  None
  * @retval None
  */
static void RECORDER_Start (void)
{
  GL_PageControls_TypeDef* item;
  uint8_t time[6];
  RTC_TimeTypeDef   RTC_TimeStructure;
  RTC_DateTypeDef   RTC_DateStructure;

  /*Create file name */
  if ( RTC_Error == 0)
  {
    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);

    sprintf((char *)RecFileName, "Rec_%02d%02d%d%02d%02d%02d.wav", RTC_DateStructure.RTC_Date,
            RTC_DateStructure.RTC_Month,
            RTC_DateStructure.RTC_Year + 2000,
            RTC_TimeStructure.RTC_Hours,
            RTC_TimeStructure.RTC_Minutes,
            RTC_TimeStructure.RTC_Seconds);
  }
  else
  {
    /* Wait until one RNG number is ready */
    while(RNG_GetFlagStatus(RNG_FLAG_DRDY)== RESET)
    {
    }
    RecoveryRecorderCounter = RNG_GetRandomNumber() & 0x7FFFFFFF;
    sprintf((char *)RecFileName, "Rec_%014d.wav", (int)RecoveryRecorderCounter);
  }

  RECORDER_GetDefaultDirectory();
  strcat ((char *)REC_DefaultPath,"/");
  strcat((char *)REC_DefaultPath, (char *)RecFileName);
  RefreshLabel(RecordPage , REC_FILNAME_ID, (uint8_t *)"                                  ");


  if (AudioRecorder_Open(REC_DefaultPath))
  {
    memset (RecFileName, 0, sizeof(RecFileName));
    RefreshLabel(RecordPage , REC_FILNAME_ID, (uint8_t *)"Error Open File !");
    return;
  }
  else
  {
    RefreshLabel(RecordPage , REC_FILNAME_ID, RecFileName);
  }
  
  /* Graphical update */
  DestroyPageControl (RecordPage, REC_STRT_ID);
  RefreshPageControl(RecordPage, REC_STRT_ID);

  DestroyPageControl (RecordPage, REC_PLAY_ID);
  RefreshPageControl(RecordPage, REC_PLAY_ID);

  DestroyPageControl (RecordPage, REC_PAUSE_ID);
  RefreshPageControl(RecordPage, REC_PAUSE_ID);

  item = NewIcon (REC_STOP_ID, rec_stop_icon, 40, 40, RECORDER_Stop);
  AddPageControlObj(170,135,item, RecordPage);
  RefreshPageControl(RecordPage, REC_STOP_ID);

  item = NewIcon (REC_PAUSE_ID, rec_pause_icon, 40, 40, RECORDER_Pause);
  AddPageControlObj(110,135,item, RecordPage);
  RefreshPageControl(RecordPage, REC_PAUSE_ID);

  item = NewIcon (REC_CANCEL_ID, rec_cancel_icon, 40, 40, RECORDER_Cancel);
  AddPageControlObj(50,135,item, RecordPage);
  RefreshPageControl(RecordPage, REC_CANCEL_ID);

  sprintf((char *)time, "%02d:%02d", 0, 0);
  RefreshLabel(RecordPage, REC_TIME_ID, time);

  REC_GL_State = REC_GL_RECORDING;
  Recorder_UsedStorage = REC_DefaultPath[0];
  AudioRecorder_Start();
}

/**
  * @brief  Stop recording and save the recorded file
  * @param  None
  * @retval None
  */
static void RECORDER_Stop (void)
{
  GL_PageControls_TypeDef* item;


  Recorder_UsedStorage = 0xFF;

  if (REC_GL_State == REC_GL_RECORDING)
  {
    AudioRecorder_Stop();

    AudioRecorder_Close();

    DestroyPageControl (RecordPage, REC_STOP_ID);
    RefreshPageControl(RecordPage, REC_STOP_ID);

    DestroyPageControl (RecordPage, REC_PAUSE_ID);
    RefreshPageControl(RecordPage, REC_PAUSE_ID);

    DestroyPageControl (RecordPage, REC_STRT_ID);
    RefreshPageControl(RecordPage, REC_STRT_ID);

    DestroyPageControl (RecordPage, REC_CANCEL_ID);
    RefreshPageControl(RecordPage, REC_CANCEL_ID);

    /* Restore old icons */
    item = NewIcon (REC_STRT_ID, rec_start_icon, 40, 40, RECORDER_Start);
    AddPageControlObj(170,135,item, RecordPage);
    RefreshPageControl(RecordPage, REC_STRT_ID);

    item = NewIcon (REC_PAUSE_ID, rec_pause_icon, 40, 40, MOD_NullFunc);
    AddPageControlObj(110,135,item, RecordPage);
    RefreshPageControl(RecordPage, REC_PAUSE_ID);

    item = NewIcon (REC_PLAY_ID, rec_play_icon, 40, 40, RECORDER_Play);
    AddPageControlObj(50,135,item, RecordPage);
    RefreshPageControl(RecordPage, REC_PLAY_ID);
  }
}

/**
  * @brief  Pause recording
  * @param  None
  * @retval None
  */
static void RECORDER_Pause (void)
{
  GL_PageControls_TypeDef* item;

  /* Pause/Resume recording */
  AudioRecorder_PauseResume(REC_GL_State);

  /* Update GUI */
  if (REC_GL_State == REC_GL_RECORDING)
  {
    REC_GL_State = REC_GL_IDLE;

    DestroyPageControl (RecordPage, REC_PAUSE_ID);
    RefreshPageControl(RecordPage, REC_PAUSE_ID);

    item = NewIcon (REC_STRT_ID, rec_start_icon, 40, 40, RECORDER_Pause);
    AddPageControlObj(110,135,item, RecordPage);
    RefreshPageControl(RecordPage, REC_STRT_ID);
  }
  else
  {
    REC_GL_State = REC_GL_RECORDING;

    DestroyPageControl (RecordPage, REC_STRT_ID);
    RefreshPageControl(RecordPage, REC_STRT_ID);

    item = NewIcon (REC_PAUSE_ID, rec_pause_icon, 40, 40, RECORDER_Pause);
    AddPageControlObj(110,135,item, RecordPage);
    RefreshPageControl(RecordPage, REC_PAUSE_ID);
  }
}

/**
  * @brief  Cancel the recorded item
  * @param  None
  * @retval None
  */
static void RECORDER_Cancel (void)
{

  Recorder_UsedStorage = 0xFF;

  /* Stop recording */
  RECORDER_Stop();

  /* Delete the recorded file */
  f_unlink ((char const*)REC_DefaultPath);
  memset (RecFileName, 0, sizeof(RecFileName));
  RefreshLabel(RecordPage , REC_FILNAME_ID, (uint8_t *)"Press Record to start ");
}

/**
  * @brief  Start playing the recorded file
  * @param  None
  * @retval None
  */
static void RECORDER_Play (void)
{

  if (strstr((char *)RecFileName,"Rec_"))
  {
    AUDIO_DirectEx(RecordPage, REC_DefaultPath, MOD_AUDIO_CTX_DISABLED);
  }
  else
  {
    RefreshLabel(RecordPage , REC_FILNAME_ID, (uint8_t *)"No recorded file yet !");
    vTaskDelay(1000);
    RefreshLabel(RecordPage , REC_FILNAME_ID, (uint8_t *)"Press Record to start ");
  }
}

/**
  * @brief  Main task for updating recorder GUI
  * @param  pvParameters: pointer to task initialization parameters.
  * @retval None
  */
static void RECORDER_background(void)
{
  uint32_t tElapsed = 0;
  static uint32_t intElapsed = 0;
  uint8_t time[6];


  if (RecordPage->Page_ID == RECORDER_RECORD_PAGE)
  {
    /* Get the total and elapsed time */
    tElapsed = AudioRecorder_GetElapsedTime();

    if (intElapsed != tElapsed)
    {
      sprintf((char *)time, "%02d:%02d", (int )(tElapsed/60), (int )(tElapsed%60));
      RefreshLabel(RecordPage, REC_TIME_ID, time);
    }
    if (REC_GL_State == REC_GL_RECORDING)
    {
      /* Stop recording when Max recording time is reached */
      if(tElapsed >= MAX_RECORDING_TIME)
      {
        RECORDER_Stop();
      }
    }
  }
}

/**
* @brief  retreive Recorder Default folder
* @param  path : full recorder directory path
* @retval None
*/
static void Check_DefaultPath (char *path)
{
  if ((*((char *)(REC_DST_PATH )) == '0') || (*((char *)(REC_DST_PATH)) == '1'))
  {
    strncpy((char *)path, (char *)(REC_DST_PATH), 128);
  }
  else
  {
    strcpy((char *)path, "0:");
  }
}

/**
* @brief  Retrieve default storage path from settings
* @param  None
* @retval None
*/
static void RECORDER_GetDefaultDirectory (void)
{
  Check_DefaultPath((char *)REC_DefaultPath);
  strcpy(FILEMGR_CurrentDirectory,(char *)REC_DefaultPath);
}

/**
* @brief  Select Default folder
* @param  None
* @retval None
*/
static void Sel_folder (void)
{
  FILMGR_DirectEx(SettingsPage, MODE_SEL_DIR, MASK_NONE, Update_folder);
}

/**
* @brief  Update Default folder
* @param  None
* @retval None
*/
static void Update_folder (void)
{
  if(( FILEMGR_CurrentDirectory[0] == USB_MEDIA_STORAGE) ||
     ( FILEMGR_CurrentDirectory[0] == MSD_MEDIA_STORAGE)) 
  {


    strncpy((char *)REC_DefaultPath, FILEMGR_CurrentDirectory, 128);
    strncpy((char *)REC_DST_PATH, FILEMGR_CurrentDirectory, 128);

    if(strlen((char *)REC_DefaultPath) > REC_DST_PATH_LENGTH)
    {
     REC_DefaultPath[REC_DST_PATH_LENGTH - 2] = '.';
     REC_DefaultPath[REC_DST_PATH_LENGTH - 1] = '.';
     REC_DefaultPath[REC_DST_PATH_LENGTH]     =  0;
    }
    RefreshLabel(SettingsPage, 4, (uint8_t *)REC_DefaultPath);
  }
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
