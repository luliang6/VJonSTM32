/**
  ******************************************************************************
  * @file    mod_audio.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   This file contains the GUI interface controller for Audio module.
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
#include "mod_audio.h"
#include "mod_filemgr.h"
#include "mod_system.h"
#include "gl_mgr.h"
#include "app_audio.h"
#include "gl_audio_res.c"
#include "str_utils.h"

/** @addtogroup Embedded_GUI_Example
* @{
*/

/** @defgroup Modules
* @brief Modules main functions
* @{
*/

/** @defgroup Modules_Private_TypesDefinitions
* @{
*/
#define AUDIO_MAIN_PAGE           MOD_AUDIO_UID + 0x00
#define AUDIO_PLAYER_PAGE         MOD_AUDIO_UID + 0x01
#define AUDIO_SETTINGS_PAGE       MOD_AUDIO_UID + 0x02
#define AUDIO_ERROR_PAGE          MOD_AUDIO_UID + 0x03

#define AUDIO_EQUALIZER_PAGE      MOD_AUDIO_UID + 0x04
#define AUDIO_PLAYLIST_PAGE       MOD_AUDIO_UID + 0x05



#define ID                        6

#define AudioManager_Task_PRIO    ( 3 )
#define AUDIO_MANAGER_STACK_SIZE  ( 256 )

#define REPEAT_NONE               0
#define REPEAT_SINGLE             1
#define REPEAT_ALL                2


#define FILES_DISPLAY_NUM         6

#define AUDIO_Q_MSG_LENGTH            10
#define AUDIO_Q_MSG_ALL                1
#define AUDIO_Q_MSG_FILEINFO           2
#define AUDIO_Q_MSG_ICONS              3
#define AUDIO_Q_MSG_PLAY              10
#define AUDIO_Q_MSG_PAUSE             11
#define AUDIO_Q_MSG_STOP              12
#define AUDIO_Q_MSG_REPT              13
#define AUDIO_Q_MSG_MUTE              14




#define AUDIO_FILENAME_SIZE            129
#define PLAYLIST_DEPDTH                 6
#define AUDIO_INFO_DISPLAY_MAX         26

#define AUDIO_BKGND_POLL               50

typedef struct _PLAYLIST_LineTypeDef
{
  uint8_t           type;
  uint8_t           line[AUDIO_FILENAME_SIZE];

}PLAYLIST_LineTypeDef;

typedef struct _PLAYLIST_FileTypeDef
{
  PLAYLIST_LineTypeDef  file[PLAYLIST_DEPDTH] ;
  uint16_t              ptr; 
  uint8_t               full;

}PLAYLIST_FileTypeDef;

static PLAYLIST_FileTypeDef* PlayList;

typedef union _PLAYER_CONFIG_TypeDef
{
  uint32_t d32;
  struct
  {
    uint32_t LoudnessDisabled :
      1;
      uint32_t EqualizerDisabled :
        1;
        uint32_t RepeatMask :
          2;
          uint32_t Volume:
            16;
            uint32_t Reserved :
              12;
  }
  b;
} PLAYER_CONFIG_TypeDef ;

/**
* @}
*/


/** @defgroup Modules_Private_Defines
* @{
*/


const uint16_t FileListIDs[FILES_DISPLAY_NUM] = {
  ID + 50 + 0,
  ID + 50 + 1,
  ID + 50 + 2,
  ID + 50 + 3,
  ID + 50 + 4,
  ID + 50 + 5,
};

/**
* @}
*/

/** @defgroup Modules_Private_Macros
* @{
*/
#define _SET_ALIGNED_POSITION(pos)  ((((AudioPlayer_GetFileLength()/100) * pos)& 0xFFFFFFFE)|(WaveFormatStruct.AudioStartAddr % 2))
/**
* @}
*/

/** @defgroup Modules_Private_Variables
* @{
*/

static GL_Page_TypeDef *AudioMainPage;
static GL_Page_TypeDef *AudioPlayerPage;
static GL_Page_TypeDef *AudioErrorPage;
static GL_Page_TypeDef *AudioEqualizerPage;
static GL_Page_TypeDef *AudioSettingsPage;

static GL_Page_TypeDef *AudioBackPage = NULL;
static GL_Page_TypeDef *Current_Page;


xTaskHandle AudioManager_Task_Handle= NULL;
static void AudioManager_Task(void * pvParameters);
xQueueHandle* pAudioPlayerGUIQueue = NULL;

uint8_t tmpInfo1[AUDIO_INFO_DISPLAY_MAX+1];
uint8_t tmpInfo2[AUDIO_INFO_DISPLAY_MAX+1];
uint8_t tmpInfo3[AUDIO_INFO_DISPLAY_MAX+1];
uint8_t tmpInfo4[AUDIO_INFO_DISPLAY_MAX+1];
uint8_t tmpInfo5[AUDIO_INFO_DISPLAY_MAX+1];
uint8_t tmpString[AUDIO_INFO_DISPLAY_MAX+5];

PLAYER_CONFIG_TypeDef PlayerCfg;

PLAYER_SettingsTypeDef AudioPlayerSettings = {
  REPEAT_NONE,             /* RepeatCtrl */
  0,                       /* BackgroundEnabled */
  0,                       /* GuiDisabled */
  DEFAULT_VOLUME,          /* CurrVolume */
  0,                       /* MuteState */
  0,                       /* CurrFileListSze */
  AUDIO_PLAYER_FILE_MASK,  /* FileMask */
  0xFFFFFFFF,              /* FileIdx */
  0,                       /* isEqDisabled */
  0,                       /* isLdnDisabled */
};



static uint32_t ChangeOccurred = 0;

static TAGS_TypeDef* TrackInfo = NULL;

static uint32_t WaitForFileOpen = 0;

__IO uint32_t PlayListViewOn = 0;

uint8_t AudioContextMenuOn = 0;

__IO uint32_t AUDIO_DirectExActive = 0;

uint8_t AUDIO_UsedStorage = 0xFF;
/**
* @}
*/

/** @defgroup Modules_Private_FunctionPrototypes
* @{
*/
/*Gui mgmt */
static void AUDIO_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex);
static void AUDIO_CreatePage(uint8_t Page);
static void AUDIO_Startup (void);
static void AUDIO_Background(void);
static void AUDIO_Media_changed (uint8_t disconnect_source);
static void AUDIO_Play(void);
static void AUDIO_Pause(void);
static void AUDIO_Stop(void);
static void AUDIO_Next(void);
static void AUDIO_Previous(void);
static void AUDIO_Repeat(void);
static void AUDIO_OpenFile(void);
static void AUDIO_VolumeCtrl(void);
static void AUDIO_ProgressCtrl(void);
static void AUDIO_MuteCtrl(void);
static void AUDIO_Forward(void);
static void AUDIO_Rewind(void);


static void AUDIO_UpdateTimeInformation(void);
static void AUDIO_UpdateFileListDisplay(uint32_t msg);
static void AUDIO_RefreshIcons(uint32_t msg);
static void AUDIO_AddFileToPlaylist (void);
static int8_t AUDIO_PlayFromPlayList(void);

#ifndef __DISABLE_EQUALIZER_CTRL 
static void AUDIO_EqualizerCtrlBand1(void);
static void AUDIO_EqualizerCtrlBand2(void);
static void AUDIO_EqualizerCtrlBand3(void);
static void AUDIO_EqualizerCtrlBand4(void);
#endif /* __DISABLE_EQUALIZER_CTRL  */
#ifndef __DISABLE_LOUDNESS_CTRL
static void AUDIO_LoudnessCtrl(void);
#endif /* __DISABLE_LOUDNESS_CTRL */


static void DrawAudioPlayerBackgound(void);
static void UpdateAudioPlayerPage(void);

/* Actions */

static void return_to_menu (void);
static void exit_player(void);
static void cancel_exit_player (void);
static void return_from_player (void);
static void goto_player (void);
static void goto_equalizer (void);
static void goto_background(void);
static void goto_playlist (void);
static void return_from_errorpage (void);
static void return_from_equalizer(void);
static void return_from_playlist (void);
static void goto_settings (void);
static void return_from_settings (void);

MOD_InitTypeDef  mod_audio =
{
  MOD_AUDIO_UID,
  MOD_AUDIO_VER,
  (uint8_t *)"A.Player", 
  (uint8_t *)audio_icon,
  AUDIO_Startup,
  AUDIO_Background,
  AUDIO_Media_changed,
};

/**
* @}
*/

/** @defgroup Modules_Private_Functions
* @{
*/

/**
* @brief  Startup initialization routine
* @param  None
* @retval None
*/

static void AUDIO_Startup (void)
{
  vTaskSuspend(Core_Time_Task_Handle); 

  if (AudioPlayerSettings.BackgroundEnabled)
  {
    AUDIO_SwitchPage(GL_HomePage, AUDIO_PLAYER_PAGE);
    Current_Page = AudioPlayerPage;

    if(AudioPlayer_GetState() == PLAYER_PLAYING)
    {
      ChangeOccurred = AUDIO_Q_MSG_PLAY;
      if (pAudioPlayerGUIQueue != NULL)
      {
        xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
      }
    }
    
    return;
  }

  AUDIO_UsedStorage = 0xFF;
  AUDIO_SwitchPage(GL_HomePage, AUDIO_MAIN_PAGE);

  /* Initialize the track information */
  TrackInfo = NULL;
}

/**
* @brief  Switch from parent page to target page
* @param  pParent: pointer to the parent page handler
* @param  PageIndex: Index of the new page
* @retval None
*/

static void AUDIO_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex)
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
  
  AUDIO_CreatePage(PageIndex);
  
  switch (PageIndex)
  {
  case AUDIO_MAIN_PAGE:
    NextPage = AudioMainPage;
    break;  

  case AUDIO_PLAYER_PAGE:
    NextPage = AudioPlayerPage;
    break;  

  case AUDIO_EQUALIZER_PAGE:
    NextPage = AudioEqualizerPage;
    break;  

  case AUDIO_SETTINGS_PAGE:
    NextPage = AudioSettingsPage;
    break;  

  case AUDIO_ERROR_PAGE:
    NextPage = AudioErrorPage;
    break;  

  default:
    break;
  }
  RefreshPage(NextPage);
}

/**
* @brief  Manages audio player pages creation
* @param  Page: index of the page to be created
* @retval None
*/
static void AUDIO_CreatePage(uint8_t Page)
{
  uint32_t i = 0;
  GL_PageControls_TypeDef* item, *File;
  __IO uint32_t ldness_perc_value;

  GL_PageControls_TypeDef* TrackInfo1;
  GL_PageControls_TypeDef* TrackInfo2;
  GL_PageControls_TypeDef* TrackInfo3;
  GL_PageControls_TypeDef* TrackInfo4;
  GL_PageControls_TypeDef* TrackInfo5;

  GL_PageControls_TypeDef* TimeInfo1;
  GL_PageControls_TypeDef* TimeInfo2;

  GL_PageControls_TypeDef *ExitButton;
  GL_PageControls_TypeDef* ErrorLablel;
  GL_PageControls_TypeDef* ErrorIcon;

  GL_SetBackColor( GL_White );
  GL_SetTextColor( GL_Blue );

  switch (Page)
  {
  case AUDIO_MAIN_PAGE:
    {
      AudioMainPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( AudioMainPage, AUDIO_MAIN_PAGE);

      Current_Page = AudioMainPage;

      item = NewIcon (5,audio_icon,50,50,MOD_NullFunc);
      AddPageControlObj(185,40,item,AudioMainPage);

      GL_SetMenuItem(AudioMainPage, (uint8_t *)"Audio Player",0,goto_player );
      GL_SetMenuItem(AudioMainPage, (uint8_t *)"Settings",1,goto_settings );
      GL_SetMenuItem(AudioMainPage, (uint8_t *)"Return",2,return_to_menu );
      GL_SetPageHeader(AudioMainPage , (uint8_t *)"Audio Menu");
    }
    break;

  case AUDIO_PLAYER_PAGE:
    {
      AudioPlayerPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( AudioPlayerPage, AUDIO_PLAYER_PAGE );

      Current_Page = AudioPlayerPage;

      AudioPlayerPage->CustomPreDraw = DrawAudioPlayerBackgound;
      AudioPlayerPage->CustomPostDraw = UpdateAudioPlayerPage;


      /*--------- Playback Icons Group */
      item = NewIcon (ID + 1, previous_icon, 30, 30, AUDIO_Previous);
      AddPageControlObj(317 - 80,185, item, AudioPlayerPage);

      item = NewIcon (ID + 2, play_icon, 30, 30, AUDIO_Play);

      AddPageControlObj(287- 80,185,item, AudioPlayerPage);
      item = NewIcon (ID + 4, next_icon, 30, 30, AUDIO_Next);
      AddPageControlObj(257- 80,185,item, AudioPlayerPage);
      item = NewIcon (ID + 3, stop_icon, 30, 30, AUDIO_Stop);
      AddPageControlObj(198- 80,185,item, AudioPlayerPage);
      if (AudioPlayerSettings.MuteState)
      {
        item = NewIcon (ID + 5, mute_on_icon, 30, 30, AUDIO_MuteCtrl);
      }
      else
      {
        item = NewIcon (ID + 5, mute_off_icon, 30, 30, AUDIO_MuteCtrl);
      }
      AddPageControlObj(168- 80,185,item, AudioPlayerPage);

      item = NewIcon (ID + 35, audio_forward_icon, 30, 30, AUDIO_Forward);
      AddPageControlObj(228- 80,185,item,AudioPlayerPage);  

      item = NewIcon (ID + 36, rewind_icon, 30, 30, AUDIO_Rewind);
      AddPageControlObj(347- 80,185,item, AudioPlayerPage);

      /*------------- Control icons */
      item = NewIcon (ID + 9, exitblue_icon, 30, 30, exit_player);
      AddPageControlObj(35,30,item, AudioPlayerPage);
      item = NewIcon (ID + 6, equalizer_icon, 30, 30, goto_equalizer);
      AddPageControlObj(35,30 + 31,item, AudioPlayerPage);
      item = NewIcon (ID + 6, openfile_icon, 30, 30, AUDIO_OpenFile);
      AddPageControlObj(35,30 + 31*2,item, AudioPlayerPage);
      item = NewIcon (ID + 8, playlist_icon, 30, 30, goto_playlist);
      AddPageControlObj(35,30 + 31*3,item, AudioPlayerPage);

      switch (AudioPlayerSettings.RepeatCtrl)
      {
      case REPEAT_NONE:
        item = NewIcon (ID + 7, repeat_off_icon, 30, 30, AUDIO_Repeat);
        break;

      case REPEAT_SINGLE:
        item = NewIcon (ID + 7, repeat_single_icon, 30, 30, AUDIO_Repeat);
        break;

      case REPEAT_ALL:
        item = NewIcon (ID + 7, repeat_all_icon, 30, 30, AUDIO_Repeat);
        break;

      default:
        item = NewIcon (ID + 7, repeat_off_icon, 30, 30, AUDIO_Repeat);
        break;
      }
      AddPageControlObj(35,30+31*4,item, AudioPlayerPage);

      LCD_SetTextColor(GL_Black);
      LCD_SetBackColor(GL_White);

      /*------------- Progress Slide Bar */
      item = NewSlidebar (ID + 27, (uint8_t *)" ", GL_HORIZONTAL, GL_TRUE, AUDIO_ProgressCtrl);
      AddPageControlObj(220, 150, item, AudioPlayerPage);
      Set_SlidebarValue ( AudioPlayerPage, ID + 27, 0 );

      LCD_SetTextColor(GL_Black);
      LCD_SetBackColor(GL_White);

      /*------------ Time Information text */
      TimeInfo1 = NewLabel(ID + 12, (uint8_t *)"00:00",GL_HORIZONTAL,GL_FONT_SMALL,GL_Blue, GL_FALSE);
      AddPageControlObj(269,153,TimeInfo1,AudioPlayerPage);
      TimeInfo2 = NewLabel(ID + 15, (uint8_t *)"00:00",GL_HORIZONTAL,GL_FONT_SMALL,GL_Blue, GL_FALSE);
      AddPageControlObj(95,153,TimeInfo2,AudioPlayerPage);

      /*------------- Volume Slide Bar */
      item = NewSlidebar (ID + 13, (uint8_t *)" ", GL_LEFT_VERTICAL, GL_TRUE,  AUDIO_VolumeCtrl);
      AddPageControlObj(305,35,item,AudioPlayerPage);
     ((GL_Slidebar_TypeDef *)(item->objPTR))->CurrentValue = (uint8_t)AudioPlayerSettings.CurrVolume;

      item= NewLabel(ID + 40, (uint8_t *)"Vol",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(305,153 ,item,AudioPlayerPage);

      item= NewLabel(ID + 41, (uint8_t *)"+",GL_HORIZONTAL,GL_FONT_SMALL,GL_Grey, GL_TRUE);
      AddPageControlObj(315,35 ,item,AudioPlayerPage);

      item= NewLabel(ID + 42, (uint8_t *)"-",GL_HORIZONTAL,GL_FONT_SMALL,GL_Grey, GL_TRUE);
      AddPageControlObj(315,142 ,item,AudioPlayerPage);


      item = NewIcon (ID + 16, speaker_image, 40, 50, MOD_NullFunc);
      AddPageControlObj(316,190,item, AudioPlayerPage);

      item = NewIcon (ID + 39, speaker_image, 40, 50, MOD_NullFunc);
      AddPageControlObj(44,190,item, AudioPlayerPage);

      LCD_SetTextColor(GL_Black);
      LCD_SetBackColor(GL_White);

      /*------------- File Information Frame */
      LCD_SetTextColor(GL_Black);
      LCD_SetBackColor(GL_Black);

      for (i = 0; i < FILES_DISPLAY_NUM; i++)
      {
        File = NewLabel(FileListIDs[i], (uint8_t *)"           ", GL_HORIZONTAL, GL_FONT_SMALL, GL_Cyan, GL_TRUE);
        AddPageControlObj(265, 41 + (15 * i), File, Current_Page);
      }

      TrackInfo1= NewLabel(ID + 10, (uint8_t *)"Title:  ",GL_HORIZONTAL,GL_FONT_SMALL,GL_Cyan, GL_TRUE);
      TrackInfo2= NewLabel(ID + 11, (uint8_t *)"Album:  ",GL_HORIZONTAL,GL_FONT_SMALL,GL_Cyan, GL_TRUE);
      TrackInfo3= NewLabel(ID + 43, (uint8_t *)"Artist: ",GL_HORIZONTAL,GL_FONT_SMALL,GL_Cyan, GL_TRUE);
      TrackInfo4= NewLabel(ID + 44, (uint8_t *)"Genre:  ",GL_HORIZONTAL,GL_FONT_SMALL,GL_Cyan, GL_TRUE);
      TrackInfo5= NewLabel(ID + 45, (uint8_t *)"Sample Rate:  ",GL_HORIZONTAL,GL_FONT_SMALL,GL_Green, GL_TRUE);

      AddPageControlObj(265, 40, TrackInfo1, AudioPlayerPage);
      AddPageControlObj(265, 58, TrackInfo2, AudioPlayerPage);
      AddPageControlObj(265, 76, TrackInfo3, AudioPlayerPage);
      AddPageControlObj(265, 94, TrackInfo4, AudioPlayerPage);
      AddPageControlObj(265,112, TrackInfo5, AudioPlayerPage);

      LCD_SetTextColor(GL_Black);
      LCD_SetBackColor(GL_White);
      GL_SetPageHeader(AudioPlayerPage , (uint8_t *)"Audio Player");

      /*------------- End of Audio Player Page drawing */
      ChangeOccurred = AUDIO_Q_MSG_FILEINFO;
      if (pAudioPlayerGUIQueue != NULL)
      {
        xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
      }

      if (AudioPlayerSettings.BackgroundEnabled)
      {
        AudioPlayerSettings.BackgroundEnabled = 0;
        break;
      }
    }
    break;


  case AUDIO_EQUALIZER_PAGE:
    {
      MOD_GetParam(AUDIO_SETTINGS_MEM , &PlayerCfg.d32);

      AudioEqualizerPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( AudioEqualizerPage, AUDIO_EQUALIZER_PAGE );

      GL_SetPageHeader(AudioEqualizerPage , (uint8_t *)"Audio Equalizer");

      item = NewIcon (ID + 1, exitblue_icon, 30, 30, return_from_equalizer);
      AddPageControlObj(35,200,item, AudioEqualizerPage);

      if(PlayerCfg.b.EqualizerDisabled)
      {
#ifndef __DISABLE_EQUALIZER_CTRL
      item = NewSlidebar (ID + 2, (uint8_t *)" ", GL_LEFT_VERTICAL, GL_FALSE, AUDIO_EqualizerCtrlBand1);
      AddPageControlObj(270,70,item,AudioEqualizerPage);
      Set_SlidebarValue(AudioEqualizerPage, ID + 2 , EQUI_DB_TO_PERC(EQ_Bands[0].gainDb));

      item = NewSlidebar (ID + 3, (uint8_t *)" ", GL_LEFT_VERTICAL,  GL_FALSE,  AUDIO_EqualizerCtrlBand2);
      AddPageControlObj(225,70,item,AudioEqualizerPage);
      Set_SlidebarValue(AudioEqualizerPage, ID + 3, EQUI_DB_TO_PERC(EQ_Bands[1].gainDb));

      item = NewSlidebar (ID + 4, (uint8_t *)" ", GL_LEFT_VERTICAL,  GL_FALSE,  AUDIO_EqualizerCtrlBand3);
      AddPageControlObj(180,70,item,AudioEqualizerPage);
      Set_SlidebarValue(AudioEqualizerPage, ID + 4, EQUI_DB_TO_PERC(EQ_Bands[2].gainDb));

      item = NewSlidebar (ID + 5, (uint8_t *)" ", GL_LEFT_VERTICAL,  GL_FALSE,  AUDIO_EqualizerCtrlBand4);
      AddPageControlObj(135,70,item,AudioEqualizerPage);
      Set_SlidebarValue(AudioEqualizerPage, ID + 5, EQUI_DB_TO_PERC(EQ_Bands[3].gainDb));
#endif /* __DISABLE_EQUALIZER_CTRL  */
      }
      else
      {
      /*------------- Equalizer Slide Bar */
#ifndef __DISABLE_EQUALIZER_CTRL
      item = NewSlidebar (ID + 2, (uint8_t *)" ", GL_LEFT_VERTICAL, GL_TRUE, AUDIO_EqualizerCtrlBand1);
      AddPageControlObj(270,70,item,AudioEqualizerPage);
      Set_SlidebarValue(AudioEqualizerPage, ID + 2 , EQUI_DB_TO_PERC(EQ_Bands[0].gainDb));

      item = NewSlidebar (ID + 3, (uint8_t *)" ", GL_LEFT_VERTICAL,  GL_TRUE,  AUDIO_EqualizerCtrlBand2);
      AddPageControlObj(225,70,item,AudioEqualizerPage);
      Set_SlidebarValue(AudioEqualizerPage, ID + 3, EQUI_DB_TO_PERC(EQ_Bands[1].gainDb));

      item = NewSlidebar (ID + 4, (uint8_t *)" ", GL_LEFT_VERTICAL,  GL_TRUE,  AUDIO_EqualizerCtrlBand3);
      AddPageControlObj(180,70,item,AudioEqualizerPage);
      Set_SlidebarValue(AudioEqualizerPage, ID + 4, EQUI_DB_TO_PERC(EQ_Bands[2].gainDb));

      item = NewSlidebar (ID + 5, (uint8_t *)" ", GL_LEFT_VERTICAL,  GL_TRUE,  AUDIO_EqualizerCtrlBand4);
      AddPageControlObj(135,70,item,AudioEqualizerPage);
      Set_SlidebarValue(AudioEqualizerPage, ID + 5, EQUI_DB_TO_PERC(EQ_Bands[3].gainDb));
#endif /* __DISABLE_EQUALIZER_CTRL  */
      }
      if(PlayerCfg.b.LoudnessDisabled)
      {
#ifndef __DISABLE_LOUDNESS_CTRL
        item = NewSlidebar (ID + 6, (uint8_t *)" ", GL_LEFT_VERTICAL,  GL_FALSE,  AUDIO_LoudnessCtrl);
        AddPageControlObj(75,70,item,AudioEqualizerPage);
        MOD_GetParam(AUDIO_LDNS_MEM , (uint32_t *)&ldness_perc_value);
        Set_SlidebarValue(AudioEqualizerPage, ID + 6,  ldness_perc_value);
#endif /* __DISABLE_LOUDNESS_CTRL */
      }
      else
      {
#ifndef __DISABLE_LOUDNESS_CTRL
        item = NewSlidebar (ID + 6, (uint8_t *)" ", GL_LEFT_VERTICAL,  GL_TRUE,  AUDIO_LoudnessCtrl);
        AddPageControlObj(75,70,item,AudioEqualizerPage);
        MOD_GetParam(AUDIO_LDNS_MEM , (uint32_t *)&ldness_perc_value);
        Set_SlidebarValue(AudioEqualizerPage, ID + 6,  ldness_perc_value);
#endif /* __DISABLE_LOUDNESS_CTRL */
      }

      /*------------- Radio Button */
#ifndef __DISABLE_EQUALIZER_CTRL
      item = NewLabel(ID + 7, (uint8_t *)"           Equalizer",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(300,35,item,AudioEqualizerPage);
#endif /* __DISABLE_EQUALIZER_CTRL  */

#ifndef __DISABLE_LOUDNESS_CTRL
      item = NewLabel(ID + 7, (uint8_t *)"         Loudness" ,GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(145,35,item,AudioEqualizerPage);
#endif /* __DISABLE_LOUDNESS_CTRL */

      /*------------- Equalizer Label */
#ifndef __DISABLE_EQUALIZER_CTRL
      /* 4 Bands Equalizer */
      item= NewLabel(ID + 8, (uint8_t *)"5 dB",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,70,item,AudioEqualizerPage);

      item= NewLabel(ID + 9, (uint8_t *)"2 dB",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,95,item,AudioEqualizerPage);

      item= NewLabel(ID + 10, (uint8_t *)"0 dB",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,120,item,AudioEqualizerPage);

      item= NewLabel(ID + 11, (uint8_t *)"-2 dB",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(318,145,item,AudioEqualizerPage);

      item= NewLabel(ID + 12, (uint8_t *)"-5 dB",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(318,170,item,AudioEqualizerPage);

      item= NewLabel(ID + 13, (uint8_t *)"1KHz",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(275,195,item,AudioEqualizerPage);

      item= NewLabel(ID + 14, (uint8_t *)"3KHz",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(230,195,item,AudioEqualizerPage);

      item= NewLabel(ID + 15, (uint8_t *)"12KHz",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(185,195,item,AudioEqualizerPage);

      item= NewLabel(ID + 16, (uint8_t *)"18KHz",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(140,195,item,AudioEqualizerPage);
#endif /* __DISABLE_EQUALIZER_CTRL  */

#ifndef __DISABLE_LOUDNESS_CTRL
      /* Loudness Control */
      item= NewLabel(ID + 71, (uint8_t *)"Max",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(50,70,item,AudioEqualizerPage);

      item= NewLabel(ID + 18, (uint8_t *)" 0",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(50,120,item,AudioEqualizerPage);

      item= NewLabel(ID + 19, (uint8_t *)"Min",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(50,170,item,AudioEqualizerPage);

      item= NewLabel(ID + 20, (uint8_t *)"Gain",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(80,195,item,AudioEqualizerPage);
#endif /* __DISABLE_LOUDNESS_CTRL */
    }
    break;


  case AUDIO_SETTINGS_PAGE:
    {
      AudioSettingsPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( AudioSettingsPage, AUDIO_SETTINGS_PAGE );

      Current_Page = AudioSettingsPage;

      item = NewIcon (5,audio_icon,50,50,MOD_NullFunc);
      AddPageControlObj(185, 40,item,AudioSettingsPage);

      /* Exit button */
      ExitButton = NewButton(1, (uint8_t *)"  OK  ", return_from_settings);
      AddPageControlObj(185, 212, ExitButton, AudioSettingsPage);


      /* Audio Effects options */
      item = NewCheckbox(13, (uint8_t *)" Disable Equalizer", GL_TRUE, MOD_NullFunc);
      AddPageControlObj(230, 40+10+(22*3), item , AudioSettingsPage);

      item = NewCheckbox(14, (uint8_t *)" Disable Loudness", GL_TRUE, MOD_NullFunc);
      AddPageControlObj(230, 40+10+(22*4), item , AudioSettingsPage);


      GL_SetPageHeader( AudioSettingsPage , (uint8_t *)"Audio Player Settings");

      /* Update the current status of checkboxes */
      MOD_GetParam(AUDIO_SETTINGS_MEM , &PlayerCfg.d32);
      GL_SetChecked(AudioSettingsPage , 14 , PlayerCfg.b.LoudnessDisabled);
      GL_SetChecked(AudioSettingsPage , 13 , PlayerCfg.b.EqualizerDisabled);

    }
    break;
    
  case AUDIO_ERROR_PAGE:
    {
      AudioErrorPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( AudioErrorPage, AUDIO_ERROR_PAGE ); 

      ExitButton= NewButton(2, (uint8_t *)"  OK  ",return_from_errorpage);
      AddPageControlObj(185,212,ExitButton,AudioErrorPage);

      ErrorIcon = NewIcon (3,error_icon,50,50,MOD_NullFunc);
      AddPageControlObj(185, 40, ErrorIcon, AudioErrorPage);

      GL_SetPageHeader(AudioErrorPage , (uint8_t *)"Audio Player : Error");
      ErrorLablel  = NewLabel(4, (uint8_t *)"The selected audio file does not exist or",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,100,ErrorLablel,AudioErrorPage);

      ErrorLablel  = NewLabel(5, (uint8_t *)"is corrupted !",GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_TRUE);
      AddPageControlObj(310,113,ErrorLablel,AudioErrorPage);
    }
    break;

  default:
    Current_Page = NULL;
    break;
  }
}

/**
* @brief  Function called when Play button is pressed
* @param  None
* @retval None
*/
static void AUDIO_Play(void)
{
  uint16_t volume;

  if (AudioPlayerSettings.GuiEnabled == 1)
  {
    if (AudioPlayerSettings.CurrFileListSze == 0)
    {
      AUDIO_OpenFile();
      WaitForFileOpen = 1;

      return;
    }
  }

  /* Set the volume level */
  if (AudioPlayerSettings.MuteState)
  {
    volume = 0;
  }
  else
  {
    volume = AudioPlayerSettings.CurrVolume;
  }
  
  AUDIO_UsedStorage = PlayList->file[AudioPlayerSettings.FileIdx].line[0];
  /* Start playing the audio file */
  if (AudioPlayer_Play(PlayList->file[AudioPlayerSettings.FileIdx].line, volume) != 0)
  {
    if (AudioPlayerSettings.GuiEnabled == 1)
    {
      /* Error message to be displayed */
      Current_Page = NULL;
      AUDIO_SwitchPage(AudioPlayerPage, AUDIO_ERROR_PAGE);
    }
    
    return;
  }

  AudioPlayerSettings.CurrVolume = PlayerCfg.b.Volume;

  if (AudioPlayerSettings.GuiEnabled == 1)
  {
    ChangeOccurred = AUDIO_Q_MSG_FILEINFO;
    if (pAudioPlayerGUIQueue != NULL)
    {
      xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
    }
    ChangeOccurred = AUDIO_Q_MSG_PLAY;
    if (pAudioPlayerGUIQueue != NULL)
    {
      xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
    }
  }
}

/**
* @brief  Start Play from playlist
* @param  None
* @retval Status
*/
static int8_t AUDIO_PlayFromPlayList(void)
{
  uint16_t volume;

  /* Set the volume level */
  if (AudioPlayerSettings.MuteState)
  {
    volume = 0;
  }
  else
  {
    volume = AudioPlayerSettings.CurrVolume;
  }

  AUDIO_UsedStorage = PlayList->file[AudioPlayerSettings.FileIdx].line[0];
  /* Start playing the audio file */
  if (AudioPlayer_Play(PlayList->file[AudioPlayerSettings.FileIdx].line, volume) != 0)
  {
    if (AudioPlayerSettings.GuiEnabled == 1)
    {
      ChangeOccurred = AUDIO_Q_MSG_FILEINFO;
      if (pAudioPlayerGUIQueue != NULL)
      {
        xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
      }
    }
    return (- 1);
  }

  AudioPlayerSettings.CurrVolume = PlayerCfg.b.Volume;
  
  if (AudioPlayerSettings.GuiEnabled == 1)
  {
    ChangeOccurred = AUDIO_Q_MSG_FILEINFO;
    if (pAudioPlayerGUIQueue != NULL)
    {
      xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
    }
    ChangeOccurred = AUDIO_Q_MSG_PLAY;
    if (pAudioPlayerGUIQueue != NULL)
    {
      xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
    }
  }
  return 0;
}
/**
* @brief  Function called when Pause button is pressed
* @param  None
* @retval None
*/
static void AUDIO_Pause(void)
{
  if (AudioPlayerSettings.GuiEnabled == 1)
  {
    ChangeOccurred = AUDIO_Q_MSG_PAUSE;
    if (pAudioPlayerGUIQueue != NULL)
    {
      xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
    }
  }
  
  AudioPlayer_Pause();
}

/**
* @brief  Function called when Stop button is pressed
* @param  None
* @retval None
*/
static void AUDIO_Stop(void)
{
  uint8_t time[6];

  /* Stop audio playing */
  AudioPlayer_Stop();
  AudioPlayer_Close();

  /*************************/


  if (AudioPlayerSettings.GuiEnabled == 1)
  {
    if (AudioPlayerPage->Page_ID == AUDIO_PLAYER_PAGE )
    {

      sprintf((char *)time, "%02d:%02d", 0, 0);
      RefreshLabel(AudioPlayerPage, ID + 12, time);
      RefreshLabel(AudioPlayerPage, ID + 15, time);

      /* Update the Progress Bar */
      if(Set_SlidebarValue ( AudioPlayerPage, ID + 27, 0 )!= GL_ERROR)
      {
        RefreshPageControl(AudioPlayerPage, ID + 27);
      }
    }
  }
/**********************/

  if (AudioPlayerSettings.GuiEnabled == 1)
  {
    ChangeOccurred = AUDIO_Q_MSG_STOP;
    if (pAudioPlayerGUIQueue != NULL)
    {
      xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
    }
  }
}

/**
* @brief  Function called when Next button is pressed
* @param  None
* @retval None
*/
static void AUDIO_Next(void)
{
  /* Stop audio playing */
  AUDIO_Stop();

  do
  {
    if(AudioPlayerSettings.FileIdx < (AudioPlayerSettings.CurrFileListSze - 1))
    {
      AudioPlayerSettings.FileIdx = AudioPlayerSettings.FileIdx + 1;
    }
    else if(AudioPlayerSettings.RepeatCtrl == REPEAT_ALL)
    {
      AudioPlayerSettings.FileIdx  = 0;
    }
    else
    {
      return;
    }
    
  }while (AUDIO_PlayFromPlayList() < 0);
}

/**
* @brief  Function called when Previous button is pressed
* @param  None
* @retval None
*/
static void AUDIO_Previous(void)
{
  /* Stop audio playing */
  AUDIO_Stop();
  
  do
  {
    if(AudioPlayerSettings.FileIdx > 0)
    {
      AudioPlayerSettings.FileIdx = AudioPlayerSettings.FileIdx - 1;
    }
    else if(AudioPlayerSettings.RepeatCtrl == REPEAT_ALL)
    {
      AudioPlayerSettings.FileIdx  = AudioPlayerSettings.CurrFileListSze - 1;
    }
    else
    {
      return;
    }
    
  }while (AUDIO_PlayFromPlayList() < 0);
}

/**
* @brief  Function called when Repeat button is pressed
* @param  None
* @retval None
*/
static void AUDIO_Repeat(void)
{    
  switch (AudioPlayerSettings.RepeatCtrl)
  {
  case REPEAT_NONE:
    AudioPlayerSettings.RepeatCtrl = REPEAT_SINGLE;
    break;

  case REPEAT_SINGLE:
    AudioPlayerSettings.RepeatCtrl = REPEAT_ALL;
    break;

  case REPEAT_ALL:
    AudioPlayerSettings.RepeatCtrl = REPEAT_NONE;
    break;

  default:
    AudioPlayerSettings.RepeatCtrl = REPEAT_NONE;
    break;
  }

  /* Get the new configuration parameters */
  MOD_GetParam(AUDIO_SETTINGS_MEM , &PlayerCfg.d32);
  PlayerCfg.b.RepeatMask  = AudioPlayerSettings.RepeatCtrl;
  MOD_SetParam(AUDIO_SETTINGS_MEM , &PlayerCfg.d32);

  if (AudioPlayerSettings.GuiEnabled == 1)
  {
    /* Send message to update the playlist display */
    ChangeOccurred = AUDIO_Q_MSG_REPT;
    if (pAudioPlayerGUIQueue != NULL)
    {
      xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
    }
  }
}

/**
* @brief  Open new audio file to be played
* @param  None
* @retval None
*/
static void AUDIO_OpenFile(void)
{
  if (AudioPlayerSettings.GuiEnabled == 1)
  {
    AudioPlayerSettings.GuiEnabled = 0;
    Current_Page = NULL;
    FILMGR_DirectEx(AudioPlayerPage, MODE_SEL_FIL, AudioPlayerSettings.FileMask, AUDIO_AddFileToPlaylist);
  }
}

/**
* @brief  Function called when volume slide bar is updated
* @param  None
* @retval None
*/
static void AUDIO_VolumeCtrl(void)
{
  static uint8_t intCurrentVolume = 0;

  /* Get the new volume level */
  AudioPlayerSettings.CurrVolume = Get_SlidebarValue (AudioPlayerPage, ID + 13);

  if (intCurrentVolume != AudioPlayerSettings.CurrVolume)
  {
    /* Set the new volume level (must be expressed in "%") */
    AudioPlayer_VolumeCtrl(AudioPlayerSettings.CurrVolume);
    PlayerCfg.b.Volume = AudioPlayerSettings.CurrVolume;
    MOD_SetParam(AUDIO_SETTINGS_MEM , &PlayerCfg.d32);


    /* Update the mute state */
    if (AudioPlayerSettings.CurrVolume > 0)
    {
      AudioPlayerSettings.MuteState = 0;

      if (AudioPlayerSettings.GuiEnabled == 1)
      {
        /* Send message to update the playlist display */
        ChangeOccurred = AUDIO_Q_MSG_MUTE;
        if (pAudioPlayerGUIQueue != NULL)
        {
          xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
        }
      }
    }
  }
}

/**
* @brief  Function called when progress bar is updated
* @param  None
* @retval None
*/
static void AUDIO_ProgressCtrl(void)
{
  static uint32_t CurrPpos = 0;

  if (Current_Page != AudioPlayerPage)
  {
    return;
  }

  if (AudioPlayerSettings.GuiEnabled == 1)
  {
    if(AudioPlayer_GetState() == PLAYER_PLAYING)
    {
      /* Pause the current stream*/
      AudioPlayer_Pause();

      vTaskDelay(100);

      ChangeOccurred = AUDIO_Q_MSG_PLAY;
      if (pAudioPlayerGUIQueue != NULL)
      {
        xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
      }

      CurrPpos = Get_SlidebarValue (AudioPlayerPage, ID + 27);

      if (CurrPpos >= 99)
      {
        CurrPpos = AUDIO_FWD_RWD_STEP;
      }
      
      AudioPlayer_SetPosition(_SET_ALIGNED_POSITION(CurrPpos));

      /* Resume playing the current stream*/
      AudioPlayer_Play((uint8_t *)NULL, 0); /* Audio File path not required when in pause state */
    }

    if(AudioPlayer_GetState() == PLAYER_PAUSED)
    {
      CurrPpos = Get_SlidebarValue (AudioPlayerPage, ID + 27);

      if (CurrPpos >= 99)
      {
        CurrPpos = AUDIO_FWD_RWD_STEP;
      }

      AudioPlayer_SetPosition(_SET_ALIGNED_POSITION(CurrPpos));
    }

  }
}


/**
* @brief  Function called when Forward button is pressed
* @param  None
* @retval None
*/
static void AUDIO_Forward(void)
{
  uint32_t tLength, tElapsed, step;

  if (Current_Page != AudioPlayerPage)
  {
    return;
  }

  if ((AudioPlayerSettings.GuiEnabled == 1) && \
      (AudioPlayer_GetState() != PLAYER_IDLE) && \
      (AudioPlayer_GetState() != PLAYER_ERROR))
  {
    /* Get the total and elapsed time */
    AudioPlayer_GetTimeInfo(&tLength, &tElapsed);

    /* Pause the current stream*/
    AudioPlayer_Pause();

    vTaskDelay(100);

   ChangeOccurred = AUDIO_Q_MSG_PLAY;
   if (pAudioPlayerGUIQueue != NULL)
   {
     xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
   }

    step = ((tElapsed * 100)/ tLength) + AUDIO_FWD_RWD_STEP;

    if (step >= 100)
    {
      step = AUDIO_FWD_RWD_STEP;
    }

    AudioPlayer_SetPosition(_SET_ALIGNED_POSITION(step));

    /* Resume playing the current stream*/
    AudioPlayer_Play((uint8_t *)NULL, 0); /* Audio File path not required when in pause state */
  }
}

/**
* @brief  Function called when Rewind button is pressed
* @param  None
* @retval None
*/
static void AUDIO_Rewind(void)
{
  uint32_t tLength, tElapsed, step;

  if (Current_Page != AudioPlayerPage)
  {
    return;
  }

  if ((AudioPlayerSettings.GuiEnabled == 1) && \
      (AudioPlayer_GetState() != PLAYER_IDLE) && \
      (AudioPlayer_GetState() != PLAYER_ERROR))
  {
    /* Get the total and elapsed time */
    AudioPlayer_GetTimeInfo(&tLength, &tElapsed);

    /* Pause the current stream*/
    AudioPlayer_Pause();

    vTaskDelay(100);

   ChangeOccurred = AUDIO_Q_MSG_PLAY;
   if (pAudioPlayerGUIQueue != NULL)
   {
     xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
   }

    step = ((tElapsed * 100)/ tLength) - AUDIO_FWD_RWD_STEP;

    if (step >= 100)
    {
      step = AUDIO_FWD_RWD_STEP;
    }

    AudioPlayer_SetPosition(_SET_ALIGNED_POSITION(step));

    /* Resume playing the current stream*/
    AudioPlayer_Play((uint8_t *)NULL, 0); /* Audio File path not required when in pause state */
  }
}

/**
* @brief  Function called when Mute button is pressed
* @param  None
* @retval None
*/
static void AUDIO_MuteCtrl(void)
{
  /* Toggle the mute state */
  AudioPlayerSettings.MuteState = (AudioPlayerSettings.MuteState != 0)? 0:1;

  /* Update the volume level */
  if (AudioPlayerSettings.MuteState)
  {
    AudioPlayer_VolumeCtrl(0);
  }
  else
  {
    AudioPlayer_VolumeCtrl(AudioPlayerSettings.CurrVolume);
  }

  if (AudioPlayerSettings.GuiEnabled == 1)
  {
    /* Send message to update the playlist display */
    ChangeOccurred = AUDIO_Q_MSG_MUTE;
    if (pAudioPlayerGUIQueue != NULL)
    {
      xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
    }
  }
}


/**
* @brief  Audio module background process callback.
* @param  None
* @retval None
*/
static void AUDIO_Background(void)
{
  static uint8_t counter = 0;

  /* Task main loop */
  if(counter++ > AUDIO_BKGND_POLL)
  {
    counter = 0;

    if(AudioPlayer_GetState() == PLAYER_STOPPED)
    {
      /* Check repeat status */
      switch (AudioPlayerSettings.RepeatCtrl)
      {
      case REPEAT_SINGLE:
        AUDIO_Stop();
        AUDIO_Play();
        break;

      case REPEAT_ALL:
        AUDIO_Next();
        break;

      case REPEAT_NONE:
        if (AudioPlayerSettings.FileIdx < (AudioPlayerSettings.CurrFileListSze - 1))
        {
          AUDIO_Next();
        }
        else
        {
          if (AudioPlayerSettings.GuiEnabled == 1)
          {
            ChangeOccurred = AUDIO_Q_MSG_STOP;
            if (pAudioPlayerGUIQueue != NULL)
            {
              xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
            }
            /* Stop audio playing */
            AUDIO_Stop();
          }
        }
        break;
      }
    }
    else if(AudioPlayer_GetState() == PLAYER_PLAYING)
    {
      if( AudioPlayerPage->Page_ID == AUDIO_PLAYER_PAGE )
      {
        AUDIO_UpdateTimeInformation();
      }
    }
  }
}

/**
* @brief  Audio Manager task. Manages audio file lists and graphic updates.
* @param  pvParameters not used
* @retval None
*/
static void AudioManager_Task(void * pvParameters)
{
  uint8_t QueueMsg = 0;
  
  while(1)
  {
    if (xQueueReceive(pAudioPlayerGUIQueue, &QueueMsg, 100) == pdPASS)
    {
      if( AudioPlayerPage->Page_ID == AUDIO_PLAYER_PAGE )
      {

        if( AudioContextMenuOn )
        {
          DestroyPageControl (Current_Page, 80);
          DestroyPageControl (Current_Page, 81);
          DestroyPageControl (Current_Page, 82);
          AudioContextMenuOn = 0;

          RefreshPage(AudioPlayerPage);
        }


        if ((AudioPlayerSettings.BackgroundEnabled == 0) && (AudioPlayerSettings.GuiEnabled == 1))
        {
          /* Wait on the queue message */
          if (pAudioPlayerGUIQueue != NULL)
          {

            /* Refresh the control icons */
            AUDIO_RefreshIcons(QueueMsg);

            /* Update file list display */
            AUDIO_UpdateFileListDisplay(QueueMsg);
          }
        }
      }
    }
  }
}

/**
* @brief  Function called to update the file information window
* @param  msg: Audio mailbox queue message
* @retval None
*/
static void AUDIO_UpdateFileListDisplay(uint32_t msg)
{
  uint32_t index = 0;
  uint16_t Color = 0x1253;

  if (/*(ChangeOccurred) &&*/ (Current_Page == AudioPlayerPage) && (msg == AUDIO_Q_MSG_FILEINFO))
  {
    /* Get the current track information */
    TrackInfo = AudioPlayer_GetFileInfo();

    if (TrackInfo != NULL)
    {
      /* File Information frame */
      LCD_SetTextColor(GL_Grey);
      GL_LCD_DrawRect(35, 270, 100, 210);
      LCD_SetTextColor(GL_Black);
      GL_LCD_FillRect(36, 269, 98, 208);


      strcpy ((char *)tmpInfo1, "Title: ");
      strncat((char *)tmpInfo1, (char *)TrackInfo->Title, AUDIO_INFO_DISPLAY_MAX - 7);

      strcpy ((char *)tmpInfo2, "Artist: ");
      strncat((char *)tmpInfo2, (char *)TrackInfo->Artist, AUDIO_INFO_DISPLAY_MAX - 7);

      strcpy ((char *)tmpInfo3, "Album: ");
      strncat((char *)tmpInfo3, (char *)TrackInfo->Album, AUDIO_INFO_DISPLAY_MAX - 7);

      strcpy ((char *)tmpInfo4, "Genre: ");
      strncat((char *)tmpInfo4, (char *)TrackInfo->Genre, AUDIO_INFO_DISPLAY_MAX - 7);

      sprintf((char *)tmpInfo5, "Sample Rate: %d Hz", (int)TrackInfo->SamplingRate);

      if (strlen((char *)tmpInfo1) > AUDIO_INFO_DISPLAY_MAX)
      {
        (tmpInfo1)[AUDIO_INFO_DISPLAY_MAX - 2] = '.';
        (tmpInfo1)[AUDIO_INFO_DISPLAY_MAX - 1] = '.';
        (tmpInfo1)[AUDIO_INFO_DISPLAY_MAX] = '\0';
      }

      if (strlen((char *)tmpInfo2) > AUDIO_INFO_DISPLAY_MAX)
      {
        (tmpInfo2)[AUDIO_INFO_DISPLAY_MAX - 2] = '.';
        (tmpInfo2)[AUDIO_INFO_DISPLAY_MAX - 1] = '.';
        (tmpInfo2)[AUDIO_INFO_DISPLAY_MAX] = '\0';
      }


      if (strlen((char *)tmpInfo3) > AUDIO_INFO_DISPLAY_MAX)
      {
        (tmpInfo3)[AUDIO_INFO_DISPLAY_MAX - 2] = '.';
        (tmpInfo3)[AUDIO_INFO_DISPLAY_MAX - 1] = '.';
        (tmpInfo3)[AUDIO_INFO_DISPLAY_MAX] = '\0';
      }



      if (strlen((char *)tmpInfo4) > AUDIO_INFO_DISPLAY_MAX)
      {
        (tmpInfo4)[AUDIO_INFO_DISPLAY_MAX - 2] = '.';
        (tmpInfo4)[AUDIO_INFO_DISPLAY_MAX - 1] = '.';
        (tmpInfo4)[AUDIO_INFO_DISPLAY_MAX] = '\0';
      }
      
      if (!PlayListViewOn)
      {

        /* File Information frame */
        LCD_SetTextColor(GL_Grey);
        GL_LCD_DrawRect(35, 270, 100, 210);
        LCD_SetTextColor(GL_Black);
        GL_LCD_FillRect(36, 269, 98, 208);

        DestroyPageControl (AudioPlayerPage, 0xFE);
        DestroyPageControl (AudioPlayerPage, 0xFF);

        /* Clear labels */
        memset (tmpString, ' ', AUDIO_INFO_DISPLAY_MAX - 1);
        tmpString[AUDIO_INFO_DISPLAY_MAX - 1] = '\0';
        LCD_SetBackColor(GL_Black);
        RefreshLabel (AudioPlayerPage, ID + 10 , tmpString);
        RefreshLabel (AudioPlayerPage, ID + 11 , tmpString);
        RefreshLabel (AudioPlayerPage, ID + 43 , tmpString);
        RefreshLabel (AudioPlayerPage, ID + 44 , tmpString);
        RefreshLabel (AudioPlayerPage, ID + 45 , tmpString);

        /* Refresh File information labels */
        LCD_SetBackColor(GL_Black);
        RefreshLabel (AudioPlayerPage, ID + 10 , tmpInfo1);
        RefreshLabel (AudioPlayerPage, ID + 11 , tmpInfo2);
        RefreshLabel (AudioPlayerPage, ID + 43 , tmpInfo3);
        RefreshLabel (AudioPlayerPage, ID + 44 , tmpInfo4);
        RefreshLabel (AudioPlayerPage, ID + 45 , tmpInfo5);

        LCD_SetBackColor(GL_White);
      }
      else
      {
        /* Playlist frame */
        LCD_SetTextColor(GL_Grey);
        GL_LCD_DrawRect(35, 270, 100, 210);
        LCD_SetTextColor(GL_Black);
        GL_LCD_FillRect(36, 269, 98, 208);

        LCD_SetBackColor(GL_White);
        LCD_SetTextColor(GL_Black);

        /* Update the playlist information */
        for (index = 0; index < AudioPlayerSettings.CurrFileListSze; index++)
        {
          Color = (index == AudioPlayerSettings.FileIdx)? GL_Cyan:GL_White;

          GetFileOnly((char *)tmpString, (char *)PlayList->file[index].line);

          if (strlen((char *)tmpString) >= AUDIO_INFO_DISPLAY_MAX)
          {
            (tmpString)[AUDIO_INFO_DISPLAY_MAX - 4] = '.';
            (tmpString)[AUDIO_INFO_DISPLAY_MAX - 3] = '.';
            (tmpString)[AUDIO_INFO_DISPLAY_MAX - 2] = '.';
            (tmpString)[AUDIO_INFO_DISPLAY_MAX - 1] = '\0';
          }

          Set_LabelColor(AudioPlayerPage, FileListIDs[index], Color);
          RefreshLabel (AudioPlayerPage, FileListIDs[index], tmpString);
        }

        LCD_SetBackColor(GL_White);
        LCD_SetTextColor(GL_Black);
      }
    }
  }  
}


/**
* @brief  Function called to update the audio file time information
* @param  None
* @retval None
*/
static void AUDIO_UpdateTimeInformation(void)
{
  static uint32_t tLength = 0, tElapsed = 0;
  static uint32_t intElapsed = 0;
  uint8_t time[6];

  if (AudioPlayerSettings.GuiEnabled == 1)
  {
    if (AudioPlayerPage->Page_ID == AUDIO_PLAYER_PAGE )
    {
      /* Get the total and elapsed time */
      AudioPlayer_GetTimeInfo(&tLength, &tElapsed);

      if (intElapsed != tElapsed)
      {
        sprintf((char *)time, "%02d:%02d", (int )(tElapsed/60),(int ) tElapsed%60);
        RefreshLabel(AudioPlayerPage, ID + 12, time);

        sprintf((char *)time, "%02d:%02d", (int )(tLength/60), (int )(tLength%60));
        RefreshLabel(AudioPlayerPage, ID + 15, time);

        /* Update the Progress Bar */
        if(Set_SlidebarValue ( AudioPlayerPage, ID + 27, (uint32_t )(tElapsed*100)/tLength )!= GL_ERROR)
        {
          RefreshPageControl(AudioPlayerPage, ID + 27);
        }
      }
    }
  }
}

#ifndef __DISABLE_EQUALIZER_CTRL
/**
* @brief  Equilizer control band 1 action
* @param  None
* @retval None
*/
static void AUDIO_EqualizerCtrlBand1(void)
{
  if(PlayerCfg.b.EqualizerDisabled == 0)
  {
    App_Audio_SetEq(0, Get_SlidebarValue (AudioEqualizerPage, ID + 2));
    MOD_SetParam(AUDIO_EQU1_MEM , (uint32_t *)&EQ_Bands[0].gainDb);
  }
}

/**
* @brief  Equilizer control band 2 action
* @param  None
* @retval None
*/
static void AUDIO_EqualizerCtrlBand2(void)
{
  if(PlayerCfg.b.EqualizerDisabled == 0)
  {
    App_Audio_SetEq(1,  Get_SlidebarValue (AudioEqualizerPage, ID + 3));
    MOD_SetParam(AUDIO_EQU2_MEM , (uint32_t *)&EQ_Bands[1].gainDb);
  }
}

/**
* @brief  Equilizer control band 3 action
* @param  None
* @retval None
*/
static void AUDIO_EqualizerCtrlBand3(void)
{
  if(PlayerCfg.b.EqualizerDisabled == 0)
  {
    App_Audio_SetEq(2, Get_SlidebarValue (AudioEqualizerPage, ID + 4));
    MOD_SetParam(AUDIO_EQU3_MEM , (uint32_t *)&EQ_Bands[2].gainDb);
  }
}

/**
* @brief  Equilizer control band 4 action
* @param  None
* @retval None
*/
static void AUDIO_EqualizerCtrlBand4(void)
{
  if(PlayerCfg.b.EqualizerDisabled == 0)
  {
    App_Audio_SetEq(3, Get_SlidebarValue (AudioEqualizerPage, ID + 5));
    MOD_SetParam(AUDIO_EQU4_MEM , (uint32_t *)&EQ_Bands[3].gainDb);
  }
}
#endif /* __DISABLE_EQUALIZER_CTRL */

#ifndef __DISABLE_LOUDNESS_CTRL
/**
* @brief  Loudness control action
* @param  None
* @retval None
*/
static void AUDIO_LoudnessCtrl(void)
{
  static uint32_t ldness;
  if(PlayerCfg.b.LoudnessDisabled == 0)
  {
    ldness = Get_SlidebarValue (AudioEqualizerPage, ID + 6);
    App_Audio_SetLoudnessGain(ldness);
    MOD_SetParam(AUDIO_LDNS_MEM , (uint32_t *)&ldness);
  }
}
#endif /* __DISABLE_EQUALIZER_CTRL */

/**
* @brief  Refresh the icons state
* @param  msg: Audio mailbox queue message
* @retval None
*/
static void AUDIO_RefreshIcons(uint32_t msg)
{
  GL_PageControls_TypeDef* item;
  static uint32_t intRepeatStat = 0;
  static uint32_t intMuteStat = 0;

  /* If the current page is not Audio Player page: no need for update */
  if (Current_Page != AudioPlayerPage)
  {
    return;
  }

  /* Check which icons need to be udpated */
  switch (msg)
  {
    case AUDIO_Q_MSG_PLAY:
      DestroyPageControl (AudioPlayerPage, ID + 2);
      RefreshPageControl(AudioPlayerPage, ID + 2);

      item = NewIcon (ID + 2, pause_icon, 30, 30, AUDIO_Pause);
      AddPageControlObj(287- 80,185,item, AudioPlayerPage);
      RefreshPageControl(AudioPlayerPage, ID + 2);
      break;

    case AUDIO_Q_MSG_PAUSE:
      DestroyPageControl (AudioPlayerPage, ID + 2);
      RefreshPageControl(AudioPlayerPage, ID + 2);

      item = NewIcon (ID + 2, play_icon, 30, 30, AUDIO_Play);
      AddPageControlObj(287- 80,185,item, AudioPlayerPage);
      RefreshPageControl(AudioPlayerPage, ID + 2);
      break;

    case AUDIO_Q_MSG_STOP:
      DestroyPageControl (AudioPlayerPage, ID + 2);
      RefreshPageControl(AudioPlayerPage, ID + 2);

      item = NewIcon (ID + 2, play_icon, 30, 30, AUDIO_Play);
      AddPageControlObj(287- 80,185,item, AudioPlayerPage);
      RefreshPageControl(AudioPlayerPage, ID + 2);
      break;

    case AUDIO_Q_MSG_REPT:
      if (intRepeatStat != AudioPlayerSettings.RepeatCtrl)
      {
        intRepeatStat = AudioPlayerSettings.RepeatCtrl;

        DestroyPageControl (AudioPlayerPage, ID + 7);
        RefreshPageControl(AudioPlayerPage, ID + 7);

        switch (AudioPlayerSettings.RepeatCtrl)
        {
        case REPEAT_NONE:
          item = NewIcon (ID + 7, repeat_off_icon, 30, 30, AUDIO_Repeat);
          break;

        case REPEAT_SINGLE:
          item = NewIcon (ID + 7, repeat_single_icon, 30, 30, AUDIO_Repeat);
          break;

        case REPEAT_ALL:
          item = NewIcon (ID + 7, repeat_all_icon, 30, 30, AUDIO_Repeat);
          break;

        default:
          item = NewIcon (ID + 7, repeat_off_icon, 30, 30, AUDIO_Repeat);
          break;
        }

        AddPageControlObj(35,30+31*4,item, AudioPlayerPage);
        RefreshPageControl(AudioPlayerPage, ID + 7);
      }
      break;

    case AUDIO_Q_MSG_MUTE:
      /* Update the mute control icon --------------------*/ 
      if (intMuteStat != AudioPlayerSettings.MuteState)
      {
        intMuteStat = AudioPlayerSettings.MuteState;

        DestroyPageControl (AudioPlayerPage, ID + 5);
        RefreshPageControl(AudioPlayerPage, ID + 5);

        if (AudioPlayerSettings.MuteState)
        {
          item = NewIcon (ID + 5, mute_on_icon, 30, 30, AUDIO_MuteCtrl);
        }
        else
        {
          item = NewIcon (ID + 5, mute_off_icon, 30, 30, AUDIO_MuteCtrl);
        }

        AddPageControlObj(168- 80,185,item, AudioPlayerPage);
        RefreshPageControl(AudioPlayerPage, ID + 5);
      }
      break;

    default:
      break;
  }

  if (AudioPlayerSettings.CurrVolume != Get_SlidebarValue (AudioPlayerPage, ID + 13))
  {
    AUDIO_VolumeCtrl();
  }
}


/**
* @brief  Launch directly the audio player from external application
* @param  pParent : caller page
* @param  file_path : audio file path
* @param  close_mode : 0 if now contextual menu is needed, 1 otherwise
* @retval None
*/
void AUDIO_DirectEx(GL_Page_TypeDef* pParent, uint8_t *file_path, uint8_t close_mode)
{
  uint32_t i = 0;
  char dummy[] = "                 ";

  AUDIO_UsedStorage = file_path[0];

  AUDIO_DirectExActive = close_mode;
  AudioBackPage = pParent;
  (*pParent).ShowPage(AudioBackPage, GL_FALSE);

  if (AudioPlayerSettings.BackgroundEnabled)
  {
    AUDIO_AddFileToPlaylist();

    AUDIO_CreatePage(AUDIO_PLAYER_PAGE);

    RefreshPage(AudioPlayerPage);

    Current_Page = AudioPlayerPage;

    AudioPlayerSettings.BackgroundEnabled = 0;
    AudioPlayerSettings.GuiEnabled = 1;
  }
  else
  {

    /* Check if an other file is currently being played */
    if (AudioPlayer_GetState() != PLAYER_IDLE)
    {
      /* Stop the current playing file */
      AUDIO_Stop();
    }

    /* Get the stored parameters */
    MOD_GetParam(AUDIO_SETTINGS_MEM , &PlayerCfg.d32);

    /* Set the repeat track/playlist settings */  
    AudioPlayerSettings.RepeatCtrl = REPEAT_NONE;
    AudioPlayerSettings.BackgroundEnabled = 0;
    AudioPlayerSettings.GuiEnabled = 1;
    AudioPlayerSettings.CurrVolume = PlayerCfg.b.Volume;
    AudioPlayerSettings.MuteState = 0;
    AudioPlayerSettings.CurrFileListSze = 1;
    AudioPlayerSettings.FileMask =   MASK_WAV;
    AudioPlayerSettings.FileIdx = 0;
    AudioPlayerSettings.isEqDisabled = PlayerCfg.b.EqualizerDisabled;
    AudioPlayerSettings.isLdnDisabled = PlayerCfg.b.LoudnessDisabled;

    /* Initialize the player page */
    ChangeOccurred = 0;
    WaitForFileOpen = 0;

    PlayList = malloc(sizeof(PLAYLIST_FileTypeDef));

    /* Update the playlist information */
    strcpy ((char *)PlayList->file[0].line, (char *)file_path);

    for (i = 1; i < PLAYLIST_DEPDTH; i++)
    {
      strcpy ((char *)PlayList->file[i].line, dummy);
    }
    
    if (AudioPlayerSettings.GuiEnabled == 1)
    {
      /* Show the player page */
      AUDIO_CreatePage(AUDIO_PLAYER_PAGE);
      RefreshPage(AudioPlayerPage);

      Current_Page = AudioPlayerPage;
      
    }

    /* Initialize the audio player module */
    AudioPlayer_Init();


    /* Create the Message Queue for the GUI update task */
    if (pAudioPlayerGUIQueue == NULL)
    {
      pAudioPlayerGUIQueue = xQueueCreate(AUDIO_Q_MSG_LENGTH,   /* Queue length */
                                          1); /* Size of each item in the queue */
    }

    if (AudioManager_Task_Handle == NULL)
    {
      /* Create and start the Audio Manager task */
      xTaskCreate(AudioManager_Task, 
                  (signed char const*)"AUDIO_G",
                  AUDIO_MANAGER_STACK_SIZE,
                  NULL, 
                  AudioManager_Task_PRIO, 
                  &AudioManager_Task_Handle);
    }

    /* Start file playing */
    AUDIO_Play();
  }
}

/**
* @brief  Return to main menu
* @param  None
* @retval None
*/
static void return_to_menu (void)
{
  AUDIO_SwitchPage(AudioMainPage, PAGE_MENU);
  AudioMainPage = NULL;
  AudioPlayerSettings.GuiEnabled = 0;

}

/**
* @brief  Exit audio player page
* @param  None
* @retval None
*/
static void exit_player(void)
{
  GL_PageControls_TypeDef* button;
  AudioPlayerSettings.BackgroundEnabled = 0;
  AudioPlayerSettings.GuiEnabled = 0;

  if(AUDIO_DirectExActive == MOD_AUDIO_CTX_DISABLED)
  {
    return_from_player();
  }

  else if(AudioContextMenuOn == 0)
  {
    LCD_SetTextColor(GL_Black);
    GL_LCD_FillRect(87 - 50 , 205 - 50 , 74 + 30 , 105);
    GL_LCD_DrawRect(82 - 50 , 210  - 50, 74 + 30 , 105);
    LCD_SetTextColor(GL_White);
    GL_LCD_DrawRect(83 - 50 , 209 - 50 , 72 + 30, 103);
    LCD_SetTextColor(GL_Grey);
    GL_LCD_FillRect(84  - 50, 208 - 50 , 71 + 30 , 101);
    AudioContextMenuOn = 1;

    button = NewButton(80, (uint8_t *)"   Close    ", return_from_player);
    AddPageControlObj(206 - 50, 88 - 50, button, Current_Page);
    RefreshPageControl(Current_Page, 80); 

    if(Global_Config.b.BackgroundModeEnabled)
    {
      button = NewButton(81, (uint8_t *)" Background ", goto_background);
      AddPageControlObj(206 - 50, 120 - 50, button, Current_Page);
      RefreshPageControl(Current_Page, 81);

      button = NewButton(82, (uint8_t *)"   Cancel   ", cancel_exit_player);
      AddPageControlObj(206 - 50, 152 - 50, button, Current_Page);  
      RefreshPageControl(Current_Page, 82);  
    }
    else
    {
      button = NewButton(82, (uint8_t *)"   Cancel   ", cancel_exit_player);
      AddPageControlObj(206 - 50, 120 - 50, button, Current_Page);
      RefreshPageControl(Current_Page, 82);
    }
  }
}

/**
* @brief  Return from the direct access page
* @param  None
* @retval None
*/
static void cancel_exit_player (void)
{
 uint8_t i = 0;

  AudioPlayerSettings.BackgroundEnabled = 0;
  AudioPlayerSettings.GuiEnabled = 0;

  DestroyPageControl (Current_Page, 80);
  DestroyPageControl (Current_Page, 81);
  DestroyPageControl (Current_Page, 82);
  AudioContextMenuOn = 0;
  PlayListViewOn = 0;
  if(Current_Page == AudioPlayerPage)
  {
    for (i = 0; i < FILES_DISPLAY_NUM; i++)
    {
      Set_Label (AudioPlayerPage, FileListIDs[i], (uint8_t *)" ");
    }
  }

  RefreshPage(Current_Page);
}

/**
* @brief  Exit from player page
* @param  None
* @retval None
*/
static void return_from_player (void)
{

  AudioPlayer_Stop();
  AudioPlayer_Close();

  AUDIO_UsedStorage = 0xFF;

  if (AudioManager_Task_Handle != NULL)
  {
    vTaskDelete(AudioManager_Task_Handle);
    AudioManager_Task_Handle = NULL;
  }

  /* Delete the Message Queue used for the GUI update task */
  if (pAudioPlayerGUIQueue != NULL)
  {
    vQueueDelete(pAudioPlayerGUIQueue);
    pAudioPlayerGUIQueue = NULL;
  }

  AudioPlayer_DeInit();

  free(PlayList);
  PlayList = NULL;

  /* Set the repeat track/playlist settings */
  AudioPlayerSettings.RepeatCtrl = PlayerCfg.b.RepeatMask;
  AudioPlayerSettings.CurrVolume = PlayerCfg.b.Volume;
  AudioPlayerSettings.MuteState = 0;
  AudioPlayerSettings.CurrFileListSze = 0;
  AudioPlayerSettings.FileMask = MASK_WAV;
  AudioPlayerSettings.FileIdx = 0xFFFFFFFF;
  AudioPlayerSettings.isEqDisabled = PlayerCfg.b.EqualizerDisabled;
  AudioPlayerSettings.isLdnDisabled = PlayerCfg.b.LoudnessDisabled;

  AudioContextMenuOn = 0;
  ChangeOccurred = 0;
  WaitForFileOpen = 0;

  /* Manage exit in case of direct access */
  if (AudioBackPage != NULL)
  {
    (*AudioPlayerPage).ShowPage(AudioPlayerPage, GL_FALSE);
    DestroyPage(AudioPlayerPage);
    free(AudioPlayerPage);
    AudioPlayerPage = NULL;
    RefreshPage(AudioBackPage);

    AudioPlayerPage = NULL;
    Current_Page = NULL;
    AudioBackPage = NULL;
    AUDIO_DirectExActive = MOD_AUDIO_CTX_ENABLED;
    FILE_MGR_RefreshScroll();
    AudioPlayerSettings.BackgroundEnabled = 0;
    AudioPlayerSettings.GuiEnabled = 0;
    return;
  }

  if (AudioPlayerSettings.BackgroundEnabled == 0)
  {
    AUDIO_SwitchPage(AudioPlayerPage, AUDIO_MAIN_PAGE);
    AudioPlayerPage = NULL;
    Current_Page = NULL;
  }

  AudioPlayerSettings.BackgroundEnabled = 0;
  AudioPlayerSettings.GuiEnabled = 0;
}

/**
* @brief  Go to audio player page
* @param  None
* @retval None
*/
static void goto_player (void)
{
  uint32_t i = 0;
  char dummy[] = "                 ";

  /* Get stored parameters */
  MOD_GetParam(AUDIO_SETTINGS_MEM , &PlayerCfg.d32);

  /* Set the repeat track/playlist settings */
  AudioPlayerSettings.RepeatCtrl = PlayerCfg.b.RepeatMask;
  AudioPlayerSettings.BackgroundEnabled = 0;
  AudioPlayerSettings.GuiEnabled = 1;
  AudioPlayerSettings.CurrVolume = PlayerCfg.b.Volume;
  AudioPlayerSettings.MuteState = 0;
  AudioPlayerSettings.CurrFileListSze = 0;
  AudioPlayerSettings.FileMask = MASK_WAV;
  AudioPlayerSettings.FileIdx = 0xFFFFFFFF;
  AudioPlayerSettings.isEqDisabled = PlayerCfg.b.EqualizerDisabled;
  AudioPlayerSettings.isLdnDisabled = PlayerCfg.b.LoudnessDisabled;


  ChangeOccurred = 0;
  WaitForFileOpen = 0;
  PlayListViewOn = 0;

  /* Open the Audio Player GUI */
  AUDIO_SwitchPage(AudioMainPage, AUDIO_PLAYER_PAGE);

  AudioMainPage = NULL;
  Current_Page = AudioPlayerPage;

  /* Initialize the audio player */
  if (AudioPlayer_Init())
  {
    /* Error message to be displayed */
    Current_Page = NULL;
    AUDIO_SwitchPage(AudioPlayerPage, AUDIO_ERROR_PAGE);
  }

  /* Initialize the palylist */
  PlayList = malloc(sizeof(PLAYLIST_FileTypeDef));
  if (PlayList == NULL)
  {
    /* Error message to be displayed */
    Current_Page = NULL;
    AUDIO_SwitchPage(AudioPlayerPage, AUDIO_ERROR_PAGE);
  }

  /* Update the playlist information */
  for (i = 0; i < PLAYLIST_DEPDTH; i++)
  {
    strcpy ((char *)PlayList->file[i].line, dummy);
  }
  
  /* Create the Message Queue for the GUI update task */
  if (pAudioPlayerGUIQueue == NULL)
  {
    pAudioPlayerGUIQueue = xQueueCreate (AUDIO_Q_MSG_LENGTH,   /* Queue length */
                                         1); /* Size of each item in the queue */
  }

    if (AudioManager_Task_Handle == NULL)
    {
      /* Create and start the Audio Manager task */
      xTaskCreate(AudioManager_Task, 
                  (signed char const*)"AUDIO_G", 
                  AUDIO_MANAGER_STACK_SIZE,
                  NULL, 
                  AudioManager_Task_PRIO, 
                  &AudioManager_Task_Handle);
    }
}

/**
* @brief  Go to equalizer page
* @param  None
* @retval None
*/
static void goto_equalizer (void)
{
  /* Disable Audio player page GUI updates */
  AudioPlayerSettings.GuiEnabled = 0;
  AudioContextMenuOn = 0;
  /* Swithch page */
  AUDIO_SwitchPage(AudioPlayerPage, AUDIO_EQUALIZER_PAGE);

  /* Set the new current page id */
  Current_Page = AudioEqualizerPage;
}

/**
* @brief  Go to Player Settings page
* @param  None
* @retval None
*/
static void goto_settings (void)
{
  /* Swithch page */
  AUDIO_SwitchPage(AudioMainPage, AUDIO_SETTINGS_PAGE);

  /* Set the new current page id */
  Current_Page = AudioSettingsPage;
}

/**
* @brief  Exit Player Settings page
* @param  None
* @retval None
*/
static void return_from_settings (void)
{
  /* Get the new configuration parameters */
  MOD_GetParam(AUDIO_SETTINGS_MEM , &PlayerCfg.d32);
  PlayerCfg.b.LoudnessDisabled   = GL_IsChecked (AudioSettingsPage , 14);
  PlayerCfg.b.EqualizerDisabled  = GL_IsChecked (AudioSettingsPage , 13);

  /* Store the configuration parameters */
  MOD_SetParam(AUDIO_SETTINGS_MEM , &PlayerCfg.d32);

  /* Swithch page */
  AUDIO_SwitchPage(AudioSettingsPage, AUDIO_MAIN_PAGE);

  /* Set the new current page id */
  Current_Page = AudioMainPage;
}

/**
* @brief  Return from equalizer page
* @param  None
* @retval None
*/
static void return_from_equalizer (void)
{
  
  /* Swithch page */
  AUDIO_SwitchPage(AudioEqualizerPage, AUDIO_PLAYER_PAGE);

  /* Set the new current page id */
  Current_Page = AudioPlayerPage;

  /* re-enable Audio player page GUI updates */
  AudioPlayerSettings.GuiEnabled = 1;
}

/**
* @brief  Go to playlist page
* @param  None
* @retval None
*/
static void goto_playlist (void)
{
  if (PlayListViewOn)
  {
    return_from_playlist();
  }
  else
  {
    PlayListViewOn = 1;
  }

  ChangeOccurred = AUDIO_Q_MSG_FILEINFO;
  if (pAudioPlayerGUIQueue != NULL)
  {
    xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
  }
   vTaskDelay(300);
}

/**
* @brief  Return from playlist page
* @param  None
* @retval None
*/
static void return_from_playlist (void)
{
  if (PlayListViewOn)
  {

    PlayListViewOn = 0;

    ChangeOccurred = AUDIO_Q_MSG_FILEINFO;
    if (pAudioPlayerGUIQueue != NULL)
    {
      xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
    }
  }
}

/**
* @brief  Enter background mode: player still active, GUI disabled.
* @param  None
* @retval None
*/
static void goto_background (void)
{
  AudioContextMenuOn = 0;
    AudioPlayerSettings.BackgroundEnabled = 1;
    AudioPlayerSettings.GuiEnabled = 0;
    Current_Page = NULL;

    if (AudioBackPage != NULL)
    {
      (*AudioPlayerPage).ShowPage(AudioPlayerPage, GL_FALSE);
      DestroyPage(AudioPlayerPage);
      free(AudioPlayerPage);
      AudioPlayerPage = NULL;
      RefreshPage(AudioBackPage);
      AudioPlayerPage = NULL;
      AudioBackPage = NULL;
      FILE_MGR_RefreshScroll();
    }
    else
    {
      AUDIO_SwitchPage(AudioPlayerPage, PAGE_MENU);
      AudioPlayerPage = NULL;
    }
}

/**
* @brief  Return from error page
* @param  None
* @retval None
*/

static void return_from_errorpage (void)
{
  AUDIO_SwitchPage(AudioErrorPage, AUDIO_PLAYER_PAGE);
  AudioErrorPage = NULL;
  Current_Page = AudioPlayerPage;

}

/**
* @brief  Update Default folder
* @param  None
* @retval None
*/
static void AUDIO_AddFileToPlaylist (void)
{
  if(check_filename_ext(FILEMGR_CurrentDirectory, "wav")|| check_filename_ext(FILEMGR_CurrentDirectory, "WAV")) 
  {
    /* Initialize the filelist index */
    if (AudioPlayerSettings.CurrFileListSze == 0)
    {
      AudioPlayerSettings.FileIdx = 0;
    }

    if(AudioPlayerSettings.CurrFileListSze < PLAYLIST_DEPDTH)
    {
      /* Get the current selected file path */
      strcpy((char *)PlayList->file[AudioPlayerSettings.CurrFileListSze++].line, FILEMGR_CurrentDirectory);
    }

    if ((AudioPlayerSettings.CurrFileListSze > 1) || ((AudioPlayerSettings.CurrFileListSze == 1) && (!WaitForFileOpen)))
    {
      PlayListViewOn = 1;
    }

    if (WaitForFileOpen)
    {
      Current_Page = AudioPlayerPage;
      AudioPlayerSettings.GuiEnabled = 1;

      WaitForFileOpen = 0;
      AUDIO_Play();
    }
  }

  /* Send message to update the playlist display */
  ChangeOccurred = AUDIO_Q_MSG_FILEINFO;
  if (pAudioPlayerGUIQueue != NULL)
  {
    xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
  }

  if(AudioPlayer_GetState() != PLAYER_PLAYING)
  {
    AUDIO_UsedStorage = 0xFF;
  }
  else
  {
    AUDIO_UsedStorage = PlayList->file[AudioPlayerSettings.FileIdx].line[0];
  }

  Current_Page = AudioPlayerPage;
  AudioPlayerSettings.GuiEnabled = 1;
}

/**
* @brief  Manage media disconnected event
* @param  Disconnect_source : SD or USB disk flash
* @retval None
*/
static void AUDIO_Media_changed (uint8_t disconnect_source)
{
  
  if( AUDIO_UsedStorage == disconnect_source)
  {

    AudioPlayer_SetState(PLAYER_IDLE);

    if ((AudioPlayerPage->Page_ID == AUDIO_PLAYER_PAGE) || 
        (AudioPlayerSettings.BackgroundEnabled))
    {
      return_from_player();
    }
    
    else if (AudioEqualizerPage->Page_ID == AUDIO_EQUALIZER_PAGE)
    {
      return_from_equalizer();
      return_from_player();
    }
  }
}

/**
* @brief  Force Audio Stop
* @param  play_source : SD or USB Disk Flash
* @retval None
*/
void AUDIO_ForceStop (uint8_t play_source)
{
  if (AudioPlayerSettings.BackgroundEnabled)
  {
    if( AUDIO_UsedStorage == play_source)
    {
      return_from_player();
    }
  }
}

/**
* @brief  Draws the audio player page default background at each refresh
* @param  None
* @retval None
*/
static void DrawAudioPlayerBackgound(void)
{
  AudioPlayerSettings.GuiEnabled = 1;

  if (Current_Page == AudioPlayerPage)
  {
    if(AudioContextMenuOn == 1)
    {
      DestroyPageControl (Current_Page, 80);
      DestroyPageControl (Current_Page, 81);
      DestroyPageControl (Current_Page, 82); 
      AudioContextMenuOn = 0;
    }

    LCD_SetTextColor(GL_Grey);
    GL_LCD_DrawRect(35, 270, 100, 210);
    LCD_SetTextColor(GL_Black);
    GL_LCD_FillRect(36, 269, 98, 208);
  }
}

/**
* @brief  Update the audio player page components
* @param  None
* @retval None
*/
static void UpdateAudioPlayerPage(void)
{
  ChangeOccurred = AUDIO_Q_MSG_FILEINFO;
  if (pAudioPlayerGUIQueue != NULL)
  {
    xQueueSend(pAudioPlayerGUIQueue, &ChangeOccurred, 100);
  }

  if (AudioPlayerSettings.BackgroundEnabled)
  {
    AudioPlayerSettings.BackgroundEnabled = 0;
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
