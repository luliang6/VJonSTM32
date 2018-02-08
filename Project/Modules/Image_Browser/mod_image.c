/**
  ******************************************************************************
  * @file    mod_image.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Graphical window and events implementation of the image module
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
#include "mod_filemgr.h"
#include "mod_image.h"
#include "app_image.h"
#include "file_utils.h"
#include "str_utils.h"
#include "gl_image_res.c"
#include "gl_mgr.h"

/* Private define ------------------------------------------------------------*/
#define IMAGE_MAIN_PAGE           MOD_IMAGE_UID + 0
#define IMAGE_BROWSER_PAGE        MOD_IMAGE_UID + 1
#define IMAGE_EFFECT              MOD_IMAGE_UID + 2
#define IMAGE_SETTINGS            MOD_IMAGE_UID + 3
#define IMAGE_DIRECT              MOD_IMAGE_UID + 4
#define IMAGE_DIRECT_CAMERA       MOD_IMAGE_UID + 5
#define IMAGE_SRC_PATH            BKPSRAM_BASE  + 0

#define IMAGE_BUFFER_SIZE         512
#define IMAGE_SRC_PATH_LENGTH     19
#define IMAGE_STR_PATH_LENGTH     36

/* Private typedef -----------------------------------------------------------*/
typedef union _IMAGEBROWSER_CONFIG_TypeDef
{
  uint32_t d32;
  struct
  {
    uint32_t TimerEnabled :
      1;
    uint32_t TimerDelay :
      7;
    uint32_t ImageEffects :
      8;      
    uint32_t ImageSource :
      16;
  }
  b;
} IMAGEBROWSER_CONFIG_TypeDef ;
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/*Graphical draw process */
static void IMAGE_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex);
static void IMAGE_CreatePage(uint8_t Page);
static void IMAGE_Startup (void);
static void IMAGE_StartBrowser (void);
static void IMAGE_BuildList (char* path);
static void IMAGE_BuildCameraList (char* path);
/* Control Actions */
static void return_to_menu (void);
static void goto_browser (void);
static void goto_settings (void);
static void return_from_browser (void);
static void return_from_settings(void);
static void return_from_direct(void);
static void return_from_direct_camera (void);
static void return_from_effects (void);

static void goto_next_file (void);
static void goto_previous_file (void);
static void goto_image_effects (void);
static void Sel_folder (void);
static void Update_folder (void);
void DisplayImageLabel (GL_Page_TypeDef *pPage, uint8_t *str);
static void IMAGE_GetDefaultDirectory (void);
static void Check_DefaultPath (char *path);
static void ClearImageBackgound (void);
static void ImageDirectBackgound (void);
static void ClearCameraBackgound (void);
static void DrawImageBackgound (void);
static void HandleImageSize (void);
static void Image_background(void);
static void IMAGE_Media_changed(uint8_t disconnect_source);
  
/* Private variables ---------------------------------------------------------*/
static FIL file;
static GL_Page_TypeDef *ImageMainPage;
static GL_Page_TypeDef *ImageBrowserPage;
static GL_Page_TypeDef *ImageSettingsPage;
static GL_Page_TypeDef *ImageEffectsPage;
static GL_Page_TypeDef *ImageBackPage;
static uint8_t IMAGE_DefaultPath[129];
static uint8_t CurrentFileFullName[129];
uint32_t IMAGE_Timer = 0;
uint16_t IMAGE_counter = 0;  
uint8_t  IMAGE_UsedStorage = 0xFF; 
uint8_t  IMAGE_ClickToggle = 0;
IMAGEBROWSER_CONFIG_TypeDef image_cfg;

MOD_InitTypeDef  mod_image =
{
  MOD_IMAGE_UID,
  MOD_IMAGE_VER,
  (uint8_t *)"Image",
  (uint8_t *)image_icon,
  IMAGE_Startup,
  Image_background,  
  IMAGE_Media_changed,
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Handle the Module startup action and display the main menu
  * @param  None
  * @retval None
  */
static void IMAGE_Startup (void)
{
  vTaskSuspend(Core_Time_Task_Handle);
  Image_SetDefaultSize();
  IMAGE_SwitchPage(GL_HomePage, IMAGE_MAIN_PAGE);
  IMAGE_UsedStorage = 0xFF;
}

/**
  * @brief  Retreive the default storage path from the settings
  * @param  None
  * @retval None
  */
static void IMAGE_GetDefaultDirectory (void)
{
  Check_DefaultPath((char *)IMAGE_DefaultPath);
}

/**
  * @brief  Launch directly the image viewer from command line
  * @param  pParent : caller page
  * @param  file_path : image file path
  * @retval None
  */
void IMAGE_DirectEx(GL_Page_TypeDef* pParent, uint8_t *file_path)
{

  strcpy((char *)CurrentFileFullName, (char *)file_path);

  IMAGE_UsedStorage = file_path[0];

  (*pParent).ShowPage(pParent, GL_FALSE);
  ImageBackPage = pParent;
  IMAGE_CreatePage(IMAGE_DIRECT);
  RefreshPage(ImageBrowserPage);
}

/**
  * @brief  Display the camera viewer page
  * @param  pParent: caller page
  * @param  dir_path: image directory path
  * @retval None
  */
void IMAGE_CameraView(GL_Page_TypeDef* pParent, uint8_t *dir_path)
{
  GL_Coordinate_TypeDef    ClickRect;

  ClickRect.MaxX = 280;
  ClickRect.MinX = 40;
  ClickRect.MaxY = 195;
  ClickRect.MinY = 45;

  IMAGE_BuildCameraList((char *)dir_path);
  strncpy((char *)IMAGE_DefaultPath, (char *)dir_path, 128);

   MOD_RegisterClickHandler (HandleImageSize, ClickRect);
  (*pParent).ShowPage(pParent, GL_FALSE);
  ImageBackPage = pParent;
  IMAGE_CreatePage(IMAGE_DIRECT_CAMERA);

  RefreshPage(ImageBrowserPage);

  IMAGE_UsedStorage = dir_path[0];


}

/**
  * @brief  Close parent page and display a child sub-page
  * @param  pParent : parent page
  * @param  PageIndex : sub-page index
  * @retval None
  */
static void IMAGE_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex)
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

  IMAGE_CreatePage(PageIndex);

  switch (PageIndex)
  {
  case IMAGE_MAIN_PAGE:
    NextPage = ImageMainPage;
    break;

  case IMAGE_DIRECT_CAMERA:
  case IMAGE_BROWSER_PAGE:
    NextPage = ImageBrowserPage;
    break;

  case IMAGE_SETTINGS:
    NextPage = ImageSettingsPage;
    break;

  case IMAGE_EFFECT:
    NextPage = ImageEffectsPage;
    break;


  case IMAGE_DIRECT:
    NextPage = ImageBrowserPage;
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
static void IMAGE_CreatePage(uint8_t Page)
{
  uint8_t path[128];
  GL_Coordinate_TypeDef    ClickRect;


  GL_PageControls_TypeDef* icon;
  GL_PageControls_TypeDef* item;
  GL_PageControls_TypeDef* ImageLabel;
  GL_PageControls_TypeDef* BackButton;
  GL_PageControls_TypeDef* ErrorMessage;
  GL_ComboBoxGrp_TypeDef* pTmp;
  GL_RadioButtonGrp_TypeDef *rGroup;
  GL_SetBackColor( GL_White );
  GL_SetTextColor( GL_Blue );

  switch (Page)
  {
  case IMAGE_MAIN_PAGE:
    {

      ImageMainPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( ImageMainPage, IMAGE_MAIN_PAGE );
      icon = NewIcon (4, image_icon, 50, 50, MOD_NullFunc);
      AddPageControlObj(185, 40, icon, ImageMainPage);

      GL_SetMenuItem(ImageMainPage, (uint8_t *)"Image Browser", 0, goto_browser );
      GL_SetMenuItem(ImageMainPage, (uint8_t *)"Image effects", 1, goto_image_effects );
      GL_SetMenuItem(ImageMainPage, (uint8_t *)"Settings", 2, goto_settings );
      GL_SetMenuItem(ImageMainPage, (uint8_t *)"Return", 3, return_to_menu );
      GL_SetPageHeader(ImageMainPage , (uint8_t *)"Image Menu");
    }
    break;

  case IMAGE_BROWSER_PAGE:
    {

      ClickRect.MaxX = 280;
      ClickRect.MinX = 40;
      ClickRect.MaxY = 195;
      ClickRect.MinY = 45;
      
      MOD_RegisterClickHandler (HandleImageSize, ClickRect);
      IMAGE_GetDefaultDirectory();

      IMAGE_BuildList((char *)IMAGE_DefaultPath);

      ImageBrowserPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( ImageBrowserPage, IMAGE_BROWSER_PAGE );

      BackButton = NewButton(1, (uint8_t *)" Return ", return_from_browser);
      AddPageControlObj(195, 212, BackButton, ImageBrowserPage);

      if (FILEMGR_FileList.ptr != 0)
      {
        GL_SetPageHeader(ImageBrowserPage , (uint8_t *)"Image Browser");
        
        ImageLabel = NewLabel(4, (uint8_t *)"", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
        AddPageControlObj(280, 30, ImageLabel, ImageBrowserPage);

        MOD_GetParam(IMAGE_SETTINGS_MEM , &image_cfg.d32);

        if (image_cfg.b.TimerEnabled)
        {
          switch (image_cfg.b.TimerDelay)
          {
          case 1:
            {
              IMAGE_Timer = 2;
              break;
            }
          case 2:
            {
              IMAGE_Timer = 5;
              break;
            }
          case 3:
            {
              IMAGE_Timer = 10;
              break;
            }
          }
        }
        else
        {
          icon = NewIcon (2, forward_icon, 30, 30, goto_next_file);
          AddPageControlObj(37, 105, icon, ImageBrowserPage);

          icon = NewIcon (3, back_icon, 30, 30, goto_previous_file);
          AddPageControlObj(313, 105, icon, ImageBrowserPage);
        }
        FILEMGR_FileList.idx = 0;
        ImageBrowserPage->CustomPostDraw = DrawImageBackgound;
      }
      else
      {
        icon = NewIcon (3, error_icon, 50, 50, MOD_NullFunc);
        AddPageControlObj(185, 40, icon, ImageBrowserPage);

        GL_SetPageHeader(ImageBrowserPage , (uint8_t *)"Image Browser : Error");
        ErrorMessage  = NewLabel(4, (uint8_t *)"Cannot find BMP Files, Please verify", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
        AddPageControlObj(310, 100, ErrorMessage, ImageBrowserPage);

        ErrorMessage  = NewLabel(5, (uint8_t *)"if the media is connected and try again.", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
        AddPageControlObj(310, 113, ErrorMessage, ImageBrowserPage);
        MOD_UnRegisterClickHandler();
        break;
      }
    }
    break;

  case IMAGE_SETTINGS:
    {
      ImageSettingsPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( ImageSettingsPage, IMAGE_SETTINGS );

      GL_SetPageHeader(ImageSettingsPage , (uint8_t *)"Browser Settings");

      BackButton = NewButton(1, (uint8_t *)" Return ", return_from_settings);
      AddPageControlObj(195, 212, BackButton, ImageSettingsPage);

      item = NewCheckbox(3 , (uint8_t *)"Automatic advance", GL_TRUE, MOD_NullFunc);
      AddPageControlObj(260, 50, item, ImageSettingsPage);

      item = NewComboBoxGrp(2);

      AddComboOption (item->objPTR, (uint8_t *)"2  seconds", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"5  seconds", MOD_NullFunc);
      AddComboOption (item->objPTR, (uint8_t *)"10 seconds", MOD_NullFunc);

      AddPageControlObj( 260, 80, item, ImageSettingsPage);


      MOD_GetParam(IMAGE_SETTINGS_MEM , &image_cfg.d32);

      if (image_cfg.b.TimerEnabled)
      {
        GL_SetChecked(ImageSettingsPage , 3 , image_cfg.b.TimerEnabled);
        IMAGE_Timer = image_cfg.b.TimerDelay;
      }
      else
      {
        IMAGE_Timer = 0;
      }


      pTmp = (GL_ComboBoxGrp_TypeDef*)(item->objPTR);

      if ((IMAGE_Timer > 0) && (IMAGE_Timer < 4))
      {
        pTmp->ComboOptions[0]->IsActive = GL_FALSE;
        pTmp->ComboOptions[IMAGE_Timer -1]->IsActive = GL_TRUE;
      }

      item  = NewLabel(4, (uint8_t *)"Image source:", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(260, 125, item, ImageSettingsPage);

      item  = NewButton(6, (uint8_t *)"...", Sel_folder);
      AddPageControlObj(120 , 140, item, ImageSettingsPage);

      Check_DefaultPath((char *)path);
      
      if(strlen((char *)path) > IMAGE_SRC_PATH_LENGTH)
      {
        path[IMAGE_SRC_PATH_LENGTH - 2] = '.'; 
        path[IMAGE_SRC_PATH_LENGTH - 1] = '.'; 
        path[IMAGE_SRC_PATH_LENGTH]     =  0;        
      }
 
      item  = NewLabel(7, path, GL_HORIZONTAL, GL_FONT_SMALL, GL_Blue, GL_FALSE);
      AddPageControlObj(260, 145, item, ImageSettingsPage);

      break;
    }
  case IMAGE_DIRECT:
    {
      ImageBrowserPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( ImageBrowserPage, IMAGE_DIRECT );
      BackButton = NewButton(1, (uint8_t *)" Return ", return_from_direct);
      AddPageControlObj(195, 212, BackButton, ImageBrowserPage);
      GL_SetPageHeader(ImageBrowserPage , (uint8_t *)"Image Browser");
      
      ImageLabel = NewLabel(4, (uint8_t *)"", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(280, 30, ImageLabel, ImageBrowserPage);
      ImageBrowserPage->CustomPostDraw = ImageDirectBackgound;

   }
    break;
    
  case IMAGE_DIRECT_CAMERA:
    {
      ImageBrowserPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( ImageBrowserPage, IMAGE_DIRECT_CAMERA );
      BackButton = NewButton(1, (uint8_t *)" Return ", return_from_direct_camera);
      AddPageControlObj(195, 212, BackButton, ImageBrowserPage);
      GL_SetPageHeader(ImageBrowserPage , (uint8_t *)"Camera Viewer");

      ImageLabel = NewLabel(2, (uint8_t *)"", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(280, 30, ImageLabel, ImageBrowserPage);

      icon = NewIcon (3, forward_icon, 30, 30, goto_next_file);
      AddPageControlObj(37, 105, icon, ImageBrowserPage);

      icon = NewIcon (5, back_icon, 30, 30, goto_previous_file);
      AddPageControlObj(313, 105, icon, ImageBrowserPage);

      ImageLabel = NewLabel(4, (uint8_t *)"", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(280, 30, ImageLabel, ImageBrowserPage);

      ImageBrowserPage->CustomPostDraw = ClearCameraBackgound;
    }
    break;

  case IMAGE_EFFECT: 
    {
      MOD_GetParam(IMAGE_SETTINGS_MEM , &image_cfg.d32);

      ImageEffectsPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( ImageEffectsPage, IMAGE_EFFECT );

      GL_SetPageHeader(ImageEffectsPage , (uint8_t *)"Image Effcets");

      BackButton = NewButton(1, (uint8_t *)" Return ", return_from_effects);
      AddPageControlObj(195, 212, BackButton, ImageEffectsPage);

      rGroup = NewRadioButtonGrp(3);
      item = RADIO_BUTTON_ADD(rGroup, (uint8_t *)"Normal",MOD_NullFunc);
      AddPageControlObj( 260, 55, item, ImageEffectsPage);
      item = RADIO_BUTTON_ADD(rGroup, (uint8_t *)"Bluish",MOD_NullFunc);
      AddPageControlObj( 260, 55 + 25, item, ImageEffectsPage);
      item = RADIO_BUTTON_ADD(rGroup, (uint8_t *)"Greenish",MOD_NullFunc);
      AddPageControlObj( 260, 55 + 50, item, ImageEffectsPage);
      item = RADIO_BUTTON_ADD(rGroup, (uint8_t *)"Redish",MOD_NullFunc);
      AddPageControlObj( 260, 55 + 75, item, ImageEffectsPage);
      item = RADIO_BUTTON_ADD(rGroup, (uint8_t *)"Black & White",MOD_NullFunc);
      AddPageControlObj( 260, 55 + 100, item, ImageEffectsPage);

      if ((image_cfg.b.ImageEffects > 0) && (image_cfg.b.ImageEffects < 6))
      {
        ((GL_RadioOption_TypeDef*)(rGroup->RadioOptions[0]->objPTR))->IsChecked = GL_FALSE;
        ((GL_RadioOption_TypeDef*)(rGroup->RadioOptions[image_cfg.b.ImageEffects -1]->objPTR))->IsChecked = GL_TRUE;
      }

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
  IMAGE_SwitchPage(ImageMainPage, PAGE_MENU);
  LCD_WindowModeDisable();
  ImageMainPage = NULL;
}

/**
  * @brief  Dsisplay the image browser page
  * @param  None
  * @retval None
  */
static void goto_browser (void)
{
  IMAGE_SwitchPage(ImageMainPage, IMAGE_BROWSER_PAGE);
  ImageMainPage = NULL;
}

/**
  * @brief  Display the settings page
  * @param  None
  * @retval None
  */
static void goto_settings(void)
{
  IMAGE_SwitchPage(ImageMainPage, IMAGE_SETTINGS);
  ImageMainPage = NULL;
}

/**
  * @brief  return to the module main menu from the browser page
  * @param  None
  * @retval None
  */
static void return_from_browser (void)
{
  MOD_UnRegisterClickHandler();  
  IMAGE_SwitchPage(ImageBrowserPage, IMAGE_MAIN_PAGE);
  ImageBrowserPage = NULL;
  IMAGE_UsedStorage = 0xFF;
}

/**
  * @brief  return to the module main page from the settings page
  * @param  None
  * @retval None
  */
static void return_from_settings (void)
{
  image_cfg.b.TimerEnabled = GL_IsChecked (ImageSettingsPage , 3);
  image_cfg.b.TimerDelay   = GetComboOptionActive(ImageSettingsPage, 2);

  MOD_SetParam(IMAGE_SETTINGS_MEM , &image_cfg.d32);

  IMAGE_SwitchPage(ImageSettingsPage, IMAGE_MAIN_PAGE);
  ImageSettingsPage = NULL;
}

/**
  * @brief  return from the direct image viewer to the caller page
  * @param  None
  * @retval None
  */
static void return_from_direct (void)
{
  LCD_WindowModeDisable();
  MOD_UnRegisterClickHandler();
  (*ImageBrowserPage).ShowPage(ImageBrowserPage, GL_FALSE);
  DestroyPage(ImageBrowserPage);
  free(ImageBrowserPage);
  ImageBrowserPage = NULL;
  RefreshPage(ImageBackPage);
  FILE_MGR_RefreshScroll();
  IMAGE_UsedStorage = 0xFF;
}

/**
  * @brief  retuirn to the module main page from the direct camera viewer
  * @param  None
  * @retval None
  */
static void return_from_direct_camera (void)
{
  LCD_WindowModeDisable();
  MOD_UnRegisterClickHandler();
  (*ImageBrowserPage).ShowPage(ImageBrowserPage, GL_FALSE);
  DestroyPage(ImageBrowserPage);
  free(ImageBrowserPage);
  ImageBrowserPage = NULL;
  RefreshPage(ImageBackPage);
  IMAGE_UsedStorage = 0xFF;

}

/**
  * @brief  Display the next image in the list
  * @param  None
  * @retval None
  */
static void goto_next_file (void)
{
  FRESULT res = FR_OK;
  char temp[129];
  FILEMGR_FileList.idx ++;


  if (FILEMGR_FileList.idx == FILEMGR_FileList.ptr)
  {
    FILEMGR_FileList.idx = 0;
  }
  
  strncpy(temp, (char *)IMAGE_DefaultPath, 128);
  strcat(temp, "/");
  strcat(temp, (char *)FILEMGR_FileList.file[FILEMGR_FileList.idx ].line);

  res = f_open(&file, temp, FA_OPEN_EXISTING | FA_READ);
  if ( res == FR_OK)
  {
    if(Image_IsSmall())
    {
      DisplayImageLabel ( ImageBrowserPage, FILEMGR_FileList.file[FILEMGR_FileList.idx ].line);
      ClearImageBackgound();
    }
    else
    {
      GL_Clear(Black);
    }
    strcpy((char *)CurrentFileFullName, temp);
    IMAGE_counter = 0;
    IMAGE_UsedStorage = temp[0]; 
    if( Image_Decode (&file,(char *)FILEMGR_FileList.file[FILEMGR_FileList.idx].line , image_cfg.b.ImageEffects) < 0)
    {
      GL_SetTextColor(GL_White);
      GL_DisplayAdjStringLine(115 , 250, (uint8_t *)"Image Format NOT supported", GL_TRUE);
    }

    f_close(&file);

  }
   else
  {
    GL_SetTextColor(GL_White);
    GL_DisplayAdjStringLine(115 , 250, (uint8_t *)"Cannot open this file.", GL_TRUE);
  }
}

/**
  * @brief  Display the previous image in the list
  * @param  None
  * @retval None
  */
static void goto_previous_file (void)
{
  char temp[129];
  FRESULT res = FR_OK;

  FILEMGR_FileList.idx --;


  if (FILEMGR_FileList.idx < 0 )
  {
    FILEMGR_FileList.idx = FILEMGR_FileList.ptr - 1;
  }

  strncpy(temp, (char *)IMAGE_DefaultPath, 128);
  strcat(temp, "/");
  strcat(temp, (char *)FILEMGR_FileList.file[FILEMGR_FileList.idx ].line);

  res = f_open(&file, temp, FA_OPEN_EXISTING | FA_READ);
  if ( res == FR_OK)
  {
    if(Image_IsSmall())
    {
      DisplayImageLabel ( ImageBrowserPage, FILEMGR_FileList.file[FILEMGR_FileList.idx ].line);
      ClearImageBackgound();
    }
    else
    {
      GL_Clear(Black);
    }
    strcpy((char *)CurrentFileFullName, temp);
    IMAGE_counter = 0;
    IMAGE_UsedStorage = temp[0];
    if(Image_Decode (&file,(char *)FILEMGR_FileList.file[FILEMGR_FileList.idx].line , image_cfg.b.ImageEffects) < 0)
    {
      GL_SetTextColor(GL_White);
      GL_DisplayAdjStringLine(115 , 250, (uint8_t *)"Image Format NOT supported", GL_TRUE);
    }

    f_close(&file);

  }
  else
  {
    GL_SetTextColor(GL_White);
    GL_DisplayAdjStringLine(115 , 250, (uint8_t *)"Cannot open this file.", GL_TRUE);
  }
}

/**
  * @brief  Dsiplay the first image in the list when creating the image browser  page
  * @param  None
  * @retval None
  */
static void IMAGE_StartBrowser (void)
{
  char temp[128];
  FRESULT res = FR_OK;

  if (FILEMGR_FileList.ptr > 0)
  {
    strncpy(temp, (char *)IMAGE_DefaultPath, 128);
    strcat(temp, "/");
    strcat(temp, (char *)FILEMGR_FileList.file[FILEMGR_FileList.idx].line);
    DisplayImageLabel ( ImageBrowserPage, FILEMGR_FileList.file[FILEMGR_FileList.idx].line);
    
    res = f_open(&file, temp, FA_OPEN_EXISTING | FA_READ);
    if (res == FR_OK)
    {
      strcpy((char *)CurrentFileFullName, temp);
      IMAGE_counter = 0;
      IMAGE_UsedStorage = temp[0];
      if (Image_Decode (&file,(char *)FILEMGR_FileList.file[FILEMGR_FileList.idx].line , image_cfg.b.ImageEffects) < 0)
      {
        GL_SetTextColor(GL_White);
        GL_DisplayAdjStringLine(115 , 250, (uint8_t *)"Image Format NOT supported", GL_TRUE);
      }
      f_close(&file);
      }
    else
    {
      GL_SetTextColor(GL_White);
      GL_DisplayAdjStringLine(115 , 250, (uint8_t *)"Cannot open this file.", GL_TRUE);
    }
  }
}

/**
  * @brief  Built the list of images available in the given path
  * @param  path : path where the image are looked in
  * @retval None
  */
static void IMAGE_BuildList (char* path)
{
  FRESULT res;
  FILINFO fno;
  DIR dir;
  char *fn;
#if _USE_LFN
  static char lfn[_MAX_LFN];
  fno.lfname = lfn;
  fno.lfsize = sizeof(lfn);
#endif
  FILEMGR_FileList.ptr = 0;
  FILEMGR_FileList.idx = 0;

  res = f_opendir(&dir, path);
  if (res == FR_OK)
  {
    for (;;)
    {
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0)
      {
        break;
      }
      if (fno.fname[0] == '.')
      {
        continue;
      }
#if _USE_LFN
      fn = *fno.lfname ? fno.lfname : fno.fname;
#else
      fn = fno.fname;
#endif
      if (fno.fattrib & AM_DIR)
      {
        continue;
      }
      else
      {

        if ((check_filename_ext(fn, "bmp")) || (check_filename_ext(fn, "BMP")) || (check_filename_ext(fn, "jpg"))|| (check_filename_ext(fn, "JPG")))
        {
          if (FILEMGR_FileList.ptr < FILELIST_DEPDTH)
          {
            strncpy((char *)FILEMGR_FileList.file[FILEMGR_FileList.ptr++].line, fn, 128);
          }
        }
      }
    }
  }
}

/**
  * @brief   Built the list of images taken by the camera available in the given path
  * @param   path : path where the image are looked in
  * @retval  None
  */
static void IMAGE_BuildCameraList (char* path)
{
  FRESULT res;
  FILINFO fno;
  DIR dir;
  char *fn;
#if _USE_LFN
  static char lfn[_MAX_LFN ];
  fno.lfname = lfn;
  fno.lfsize = sizeof(lfn);
#endif

  FILEMGR_FileList.ptr = 0;
  FILEMGR_FileList.idx = 0;
  
  res = f_opendir(&dir, path);
  if (res == FR_OK)
  {
    for (;;)
    {
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0)
      {
        break;
      }
      if (fno.fname[0] == '.')
      {
        continue;
      }
#if _USE_LFN
      fn = *fno.lfname ? fno.lfname : fno.fname;
#else
      fn = fno.fname;
#endif
      if (fno.fattrib & AM_DIR)
      {
        continue;
      }
      else
      {
        if ((check_filename_ext(fn, "bmp")) || (check_filename_ext(fn, "BMP"))|| (check_filename_ext(fn, "jpg")) || (check_filename_ext(fn, "JPG")))
        {
          if (strstr(fn, "Camera"))
          {
            if (FILEMGR_FileList.ptr < FILELIST_DEPDTH)
            {
              strncpy((char *)FILEMGR_FileList.file[FILEMGR_FileList.ptr++].line, fn, 128);
            }
          }
        }
      }
    }
  }
}

/**
  * @brief  Display the image file name
  * @param  pPage : pointer to active page
  * @param  str : image file name
  * @retval None
  */
void DisplayImageLabel (GL_Page_TypeDef* pPage, uint8_t *str)
{
  uint8_t temp[IMAGE_STR_PATH_LENGTH];
  
  memset (temp, ' ', IMAGE_STR_PATH_LENGTH -1);
  temp[IMAGE_STR_PATH_LENGTH -1 ] = 0;
  
  RefreshLabel (pPage , 4 , temp);
  
  GetFileOnly((char *)temp, (char *)str);
  
  if(strlen((char *)str) > IMAGE_STR_PATH_LENGTH)
  {
    temp[IMAGE_STR_PATH_LENGTH - 3] = '.'; 
    temp[IMAGE_STR_PATH_LENGTH - 2] = '.'; 
    temp[IMAGE_STR_PATH_LENGTH - 1] = '\0';        
  }
  RefreshLabel (pPage , 4 , temp);
}

/**
  * @brief  return to module main page after a medium disconnection event
  * @param  disconnect source : USB disk flash or SD
  * @retval None
  */
static void IMAGE_Media_changed (uint8_t disconnect_source)
{
  if(ImageBrowserPage != NULL)
  {
    if(IMAGE_UsedStorage == disconnect_source)
    {    
      if (ImageBrowserPage->Page_ID == IMAGE_BROWSER_PAGE)
      {
        LCD_WindowModeDisable();
        MOD_UnRegisterClickHandler();
        IMAGE_SwitchPage(ImageBrowserPage, IMAGE_MAIN_PAGE);
        ImageBrowserPage = NULL;
        FILEMGR_FileList.ptr = 0;
        FILEMGR_FileList.idx = 0;
      }
      else if (ImageBrowserPage->Page_ID == IMAGE_DIRECT_CAMERA)
      {
        return_from_direct_camera();
      }
      else if(ImageBrowserPage->Page_ID == IMAGE_DIRECT)
      {
        return_from_direct();
      }
    }
  }
}

/**
  * @brief  Image browser background process callback
  * @param  None
  * @retval None
  */
static void Image_background(void)
{
  if(ImageBrowserPage != NULL)
  {
    if (ImageBrowserPage->Page_ID == IMAGE_BROWSER_PAGE)
    {
      if (FILEMGR_FileList.ptr != 0)
      {      
        if((IMAGE_counter++ > (IMAGE_Timer * 100)) && (image_cfg.b.TimerEnabled))
        {
          IMAGE_counter = 0;
          goto_next_file();
        }
      }
    }
  }
}
/**
  * @brief  Select Default folder
  * @param  None
  * @retval None
  */
static void Sel_folder (void)
{
  FILMGR_DirectEx(ImageSettingsPage, MODE_SEL_DIR, MASK_NONE, Update_folder);
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
    
    strncpy((char *)CurrentFileFullName, FILEMGR_CurrentDirectory, 128);
    strncpy((char *)IMAGE_SRC_PATH, FILEMGR_CurrentDirectory, 128);    
    
    if(strlen((char *)CurrentFileFullName) > IMAGE_SRC_PATH_LENGTH)
    {
     CurrentFileFullName[IMAGE_SRC_PATH_LENGTH - 2] = '.'; 
     CurrentFileFullName[IMAGE_SRC_PATH_LENGTH - 1] = '.'; 
     CurrentFileFullName[IMAGE_SRC_PATH_LENGTH]     =  0;        
    }
    RefreshLabel(ImageSettingsPage, 7, (uint8_t *)CurrentFileFullName);

  }
}

/**
  * @brief  check Default folder
  * @param  path: pointer to Image storage path
  * @retval None
  */
static void Check_DefaultPath (char *path)
{
  if ((*((char *)IMAGE_SRC_PATH) == '0') || (*((char *)IMAGE_SRC_PATH) == '1'))
  {
    strncpy((char *)path, (char *)IMAGE_SRC_PATH, 128);
  }
  else
  {
    strcpy((char *)path, "0:");
  }
}

/**
  * @brief  Draw the image browser backgound
  * @param  None
  * @retval None
  */

static void ClearImageBackgound (void)
{
  LCD_SetTextColor(GL_Black);
  GL_LCD_FillRect(45, 280, 160, 240);
  
}

/**
  * @brief  Draw the image browser backgound from direct action
  * @param  None
  * @retval None
  */

static void ImageDirectBackgound (void)
{
  FRESULT res = FR_OK;
  GL_Coordinate_TypeDef    ClickRect;

  ClickRect.MaxX = 280;
  ClickRect.MinX = 40;
  ClickRect.MaxY = 195;
  ClickRect.MinY = 45;

  MOD_RegisterClickHandler (HandleImageSize , ClickRect);
  DisplayImageLabel (ImageBrowserPage, CurrentFileFullName);
  ClearImageBackgound();


  res = f_open(&file, (char *)CurrentFileFullName, FA_OPEN_EXISTING | FA_READ);

  if (res == FR_OK)
  {
    Image_SetDefaultSize();
    IMAGE_UsedStorage = CurrentFileFullName[0];
    if(Image_Decode (&file, (char *)CurrentFileFullName, image_cfg.b.ImageEffects) < 0)
    {
      GL_SetTextColor(GL_White);
      GL_DisplayAdjStringLine(115 , 250, (uint8_t *)"Image Format NOT supported", GL_TRUE);
      MOD_UnRegisterClickHandler();
    }
    f_close(&file);
    
  }
  else
  {
    GL_SetTextColor(GL_White);
    GL_DisplayAdjStringLine(115 , 250, (uint8_t *)"Cannot open this file", GL_TRUE);
    MOD_UnRegisterClickHandler();  
  }
}
/**
  * @brief  Draw the Camera viewer backgound
  * @param  None
  * @retval None
  */

static void ClearCameraBackgound (void)
{
  FRESULT res = FR_OK;
  char temp[129];

  MOD_GetParam(IMAGE_SETTINGS_MEM , &image_cfg.d32);

  LCD_SetTextColor(GL_Black);
  GL_LCD_FillRect(45, 280, 160, 240);

  if (FILEMGR_FileList.ptr > 0)
  {
    strncpy(temp, (char *)IMAGE_DefaultPath, 128);
    strcat(temp, "/");
    strcat(temp, (char *)FILEMGR_FileList.file[FILEMGR_FileList.ptr-1].line);
    strcpy((char *)CurrentFileFullName, temp);
    FILEMGR_FileList.idx = 0;
    DisplayImageLabel ( ImageBrowserPage, FILEMGR_FileList.file[FILEMGR_FileList.ptr-1].line);

    res = f_open(&file, temp, FA_OPEN_EXISTING | FA_READ);
    if ( res == FR_OK)
    {
      Image_SetDefaultSize();
      IMAGE_UsedStorage = temp[0];
      if(Image_Decode (&file, (char *)FILEMGR_FileList.file[FILEMGR_FileList.ptr-1].line, image_cfg.b.ImageEffects) < 0)
      {
        GL_SetTextColor(GL_White);
        GL_DisplayAdjStringLine(115 , 250, (uint8_t *)"Image Format NOT supported", GL_TRUE);
        MOD_UnRegisterClickHandler();
      }
      f_close(&file);
    }
    else 
    {
      GL_SetTextColor(GL_White);
      GL_DisplayAdjStringLine(115 , 250, (uint8_t *)"Cannot open this file.", GL_TRUE);
      MOD_UnRegisterClickHandler();
    }
  }
  else
  {
    DisplayImageLabel (ImageBrowserPage, (uint8_t *)"Cannot find camera images !");
    DestroyPageControl(ImageBrowserPage,3);
    DestroyPageControl(ImageBrowserPage,5);
    MOD_UnRegisterClickHandler();  

  }
}

/**
  * @brief  Draw the image browser backgound
  * @param  None
  * @retval None
  */

static void DrawImageBackgound (void)
{
  Image_SetDefaultSize();
  ClearImageBackgound();
  IMAGE_counter = 0;
  IMAGE_StartBrowser();  
}
/**
  * @brief  Display immage effect page
  * @param  None
  * @retval None
  */
static void goto_image_effects (void)
{
  IMAGE_SwitchPage(ImageMainPage, IMAGE_EFFECT);
  ImageMainPage = NULL;
}

/**
  * @brief  return to the module main page from the effects page
  * @param  None
  * @retval None
  */
static void return_from_effects (void)
{
  image_cfg.b.ImageEffects = GetRadioOptionChecked (ImageEffectsPage , 3);

  MOD_SetParam(IMAGE_SETTINGS_MEM , &image_cfg.d32);

  IMAGE_SwitchPage(ImageEffectsPage, IMAGE_MAIN_PAGE);
  ImageEffectsPage = NULL;
}

/**
  * @brief  Click event callback handler
  * @param  None
  * @retval None
  */
static void HandleImageSize (void)
{
  IMAGE_counter = 0;
  if(IMAGE_ClickToggle == 0)
  {
    GL_Clear(Black);
    Image_Enlarge();
  }
  else
  {
    Image_SetDefaultSize();
    ImageBrowserPage->Page_Active = GL_TRUE; 
    RefreshPage(ImageBrowserPage);

  }

  f_close(&file);

  if (f_open(&file, (char *)CurrentFileFullName, FA_OPEN_EXISTING | FA_READ) == FR_OK)
  {
    IMAGE_UsedStorage = CurrentFileFullName[0]; 
    if(Image_Decode (&file,(char *)CurrentFileFullName , image_cfg.b.ImageEffects)< 0)
    {
      GL_SetTextColor(GL_White);
      GL_DisplayAdjStringLine(115 , 250, (uint8_t *)"Image Format NOT supported", GL_TRUE);
    }
    f_close(&file);

  }

  if(IMAGE_ClickToggle == 0)
  {
    ImageBrowserPage->Page_Active = GL_FALSE; 
  }
  
  IMAGE_ClickToggle = (IMAGE_ClickToggle == 0) ? 1 : 0; 

}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
