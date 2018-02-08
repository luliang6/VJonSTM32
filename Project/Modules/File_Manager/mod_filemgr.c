/**
  ******************************************************************************
  * @file    mod_filemgr.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Graphical window and events implementation of the file manager module
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
#include "mod_image.h"
#include "mod_audio.h"
#include "mod_filemgr.h"
#include "file_utils.h"
#include "str_utils.h"
#include "gl_filemgr_res.c"
#include "gl_mgr.h"

/* Private define ------------------------------------------------------------*/
#define WHOLE_DISK_EXPLORER                    MOD_FILEMGR_UID + 0
#define MSD_FILE_EXPLORER                      MOD_FILEMGR_UID + 1
#define USB_FILE_EXPLORER                      MOD_FILEMGR_UID + 2
#define DRIVE_NOT_READY                        MOD_FILEMGR_UID + 3
#define DIRECT_ACCESS_EXPLORER                 MOD_FILEMGR_UID + 4

#define  USB_STORAGE_DISK                      "0:/"
#define  MSD_STORAGE_DISK                      "1:/"

#define FILELIST_MAX_DISPLAY                   8

/* Private typedef -----------------------------------------------------------*/
typedef  void (*pFunction)(void);
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/*Graphical draw process */
static void FILEMGR_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex);
static void FILEMGR_CreatePage(uint8_t Page);
static void FILEMGR_Startup (void);
static void FILE_MGR_Media_changed (uint8_t disconnect_source);
/* Control Actions */
static void goto_usb_drive (void);
static void return_from_usb_drive (void);

static void goto_msd_drive (void);
static void return_from_msd_drive (void);
static void back_to_drives (void);

static void return_to_menu (void);
static void return_to_whole_drive (void);
static void Add_CtxMenu (uint8_t mode);
/* File list Handling */
static uint8_t Explore_Disk (char* path);
static void Display_Files (uint8_t offset);
static void Sort_Files (void);
static void Display_Previous (void);
static void Display_Next (void);
static void Remove_OldList (GL_Page_TypeDef *pPage);
static void Refresh_FileList (GL_Page_TypeDef *pPage);
static void Explore_Directory (void);
static void return_from_direct (void);
static void Return_Selected_Item (void);
static void Delete_Selected_Item (void);
static void Open_Selected_Item (void);
static void Cancel_ctx_menu (void);
static void Explore_SubFolder (void);
static void Remove_CtxMenu (void);
static void Refresh_CustomDraw (void);

/* Private variables ---------------------------------------------------------*/
uint8_t Set_Scroll = 0;
uint8_t Display_list_ptr = 0;

char FILEMGR_CurrentDirectoryBk[FILEMGR_DIRECTORYSIZE];
uint8_t MSD_Drive_Ready = 0;
extern FATFS            MSD_fatfs;
extern uint8_t          USB_Host_Application_Ready;
static GL_Page_TypeDef *Backup_Page;
static GL_Page_TypeDef *Instance_Page;
static GL_Page_TypeDef *Current_Page;
static GL_Page_TypeDef *MainDiskPage;
static GL_Page_TypeDef *MSDSelectedDrive;
static GL_Page_TypeDef *USBSelectedDrive;
static GL_Page_TypeDef *NoDrive;

pFunction              Select_Hanlder;

uint8_t FileMgr_Mode;
uint8_t FileMgr_Mask;
uint16_t selected_item ;
uint16_t FileMgr_Nesting = 0;
uint8_t Context_Menu = 0;
MOD_InitTypeDef  mod_filemgr =
{
  MOD_FILEMGR_UID,
  MOD_FILEMGR_VER,
  (uint8_t *)"Files",
  (uint8_t *)filemgr_icon,
  FILEMGR_Startup,
  NULL,  
  FILE_MGR_Media_changed,
};
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Handle the Module startup action and display the main menu
  * @param  None
  * @retval None
  */
static void FILEMGR_Startup (void)
{
  FileMgr_Nesting = 0;
  vTaskSuspend(Core_Time_Task_Handle);
  FILEMGR_SwitchPage(GL_HomePage, WHOLE_DISK_EXPLORER);
  FileMgr_Nesting++;
  Set_Scroll = 0;
  FILEMGR_FileList.full = 0;
  MSDSelectedDrive = NULL;
  USBSelectedDrive = NULL;
  FileMgr_Mode = MODE_NONE;
  FileMgr_Mask = MASK_NONE;

}

/**
  * @brief  Close parent page and display a child sub-page
  * @param  pParent : parent page
  * @param  PageIndex : sub-page index
  * @retval None
  */
static void FILEMGR_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex)
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

  FILEMGR_CreatePage(PageIndex);

  switch (PageIndex)
  {
  case WHOLE_DISK_EXPLORER:
    NextPage = MainDiskPage;
    break;

  case USB_FILE_EXPLORER:
    NextPage = USBSelectedDrive;
    break;
    
  case MSD_FILE_EXPLORER:
    NextPage = MSDSelectedDrive;
    break;

  case DRIVE_NOT_READY:
    NextPage = NoDrive;
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
static void FILEMGR_CreatePage(uint8_t Page)
{

  GL_PageControls_TypeDef *ExitButton;
  GL_PageControls_TypeDef* NoDriveLabel;
  GL_PageControls_TypeDef* ErrorIcon;

  GL_SetBackColor( GL_White );
  GL_SetTextColor( GL_Blue );

  switch (Page)
  {
  case DIRECT_ACCESS_EXPLORER:
    {
      MainDiskPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( MainDiskPage, DIRECT_ACCESS_EXPLORER );

      ExitButton = NewButton(2, (uint8_t *)" Return ", return_from_direct);
      AddPageControlObj(195, 212, ExitButton, MainDiskPage);

      GL_SetPageHeader(MainDiskPage , (uint8_t *)"Drive Browser");

      /* To Do : get info from USB Disk flash */

      AddIconControlObj (MainDiskPage, 3, 0,
                         (uint8_t *)drive_icon,
                         (uint8_t *)"microSD",
                         1,
                         goto_msd_drive) ;

      AddIconControlObj (MainDiskPage, 5, 1,
                         (uint8_t *)drive_icon,
                         (uint8_t *)"USB Disk",
                         1,
                         goto_usb_drive) ;

    }
    break;

  case WHOLE_DISK_EXPLORER:
    {
      MainDiskPage = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( MainDiskPage, WHOLE_DISK_EXPLORER );

      ExitButton = NewButton(2, (uint8_t *)" Return ", return_to_menu);
      AddPageControlObj(195, 212, ExitButton, MainDiskPage);

      GL_SetPageHeader(MainDiskPage , (uint8_t *)"Drive Browser");

      /* To Do : get info from USB Disk flash */

      AddIconControlObj (MainDiskPage, 3, 0,
                         (uint8_t *)drive_icon,
                         (uint8_t *)"microSD",
                         1,
                         goto_msd_drive) ;

      AddIconControlObj (MainDiskPage, 5, 1,
                         (uint8_t *)drive_icon,
                         (uint8_t *)"USB Disk",
                         1,
                         goto_usb_drive) ;
    }
    break;


  case MSD_FILE_EXPLORER:
    {
      Display_list_ptr = 0;
      strcpy(FILEMGR_CurrentDirectory, "1:");

      MSDSelectedDrive = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( MSDSelectedDrive, MSD_FILE_EXPLORER );

      ExitButton = NewButton(2, (uint8_t *)" Return ", back_to_drives);
      AddPageControlObj(195, 212, ExitButton, MSDSelectedDrive);

      GL_SetPageHeader(MSDSelectedDrive , (uint8_t *)"File Browser [MSD:\\]");

      Explore_Disk(MSD_STORAGE_DISK);

      if (FILEMGR_FileList.ptr > FILELIST_MAX_DISPLAY)
      {
        GL_AddScroll (MSDSelectedDrive, 293, 28, 170, Display_Previous, Display_Next, 0);
        Set_Scroll = 1;
      }

      Display_Files(0);
      MSDSelectedDrive->CustomPreDraw = Refresh_CustomDraw;
    }
    break;

  case USB_FILE_EXPLORER:
    {
      Display_list_ptr = 0;
      strcpy(FILEMGR_CurrentDirectory, "0:");

      USBSelectedDrive = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( USBSelectedDrive, USB_FILE_EXPLORER );

      ExitButton = NewButton(2, (uint8_t *)" Return ", back_to_drives);
      AddPageControlObj(195, 212, ExitButton, USBSelectedDrive);

      GL_SetPageHeader(USBSelectedDrive , (uint8_t *)"File Browser [USB:\\]");

      Explore_Disk(USB_STORAGE_DISK);

      if (FILEMGR_FileList.ptr > FILELIST_MAX_DISPLAY)
      {
        GL_AddScroll (USBSelectedDrive, 293, 28, 170, Display_Previous, Display_Next, 0);
        Set_Scroll = 1;
      }

      Display_Files(0);
      USBSelectedDrive->CustomPreDraw = Refresh_CustomDraw;
    }
    break;

  case DRIVE_NOT_READY:
    {

      NoDrive = malloc(sizeof(GL_Page_TypeDef));
      Create_PageObj( NoDrive, DRIVE_NOT_READY );

      ExitButton = NewButton(2, (uint8_t *)"  OK  ", return_to_whole_drive);
      AddPageControlObj(185, 212, ExitButton, NoDrive);

      ErrorIcon = NewIcon (3, error_icon, 50, 50, MOD_NullFunc);
      AddPageControlObj(185, 40, ErrorIcon, NoDrive);

      GL_SetPageHeader(NoDrive , (uint8_t *)"File manager : Error");
      NoDriveLabel  = NewLabel(4, (uint8_t *)"This storage unit is not mounted. Verify", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(310, 100, NoDriveLabel, NoDrive);

      NoDriveLabel  = NewLabel(5, (uint8_t *)"if the media is connected and try again.", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
      AddPageControlObj(310, 113, NoDriveLabel, NoDrive);

      strcpy(FILEMGR_CurrentDirectory, "");
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
  FileMgr_Nesting = 0;
  if (FileMgr_Mode == MODE_NONE)
  {
    FILEMGR_SwitchPage(MainDiskPage, PAGE_MENU);
    MainDiskPage = NULL;
    Current_Page  = NULL;
  }
  else
  {
    return_from_direct();
  }
}

/**
  * @brief  Display the drives page
  * @param  None
  * @retval None
  */
static void goto_usb_drive (void)
{
  if (USB_Host_Application_Ready != 0)
  {
    FILEMGR_SwitchPage(MainDiskPage, USB_FILE_EXPLORER);
  }
  else
  {
    FILEMGR_SwitchPage(MainDiskPage, DRIVE_NOT_READY);
  }
}

/**
  * @brief  return to the module main menu from the drives page
  * @param  None
  * @retval None
  */
static void return_from_usb_drive (void)
{
  FILEMGR_SwitchPage(USBSelectedDrive, WHOLE_DISK_EXPLORER);
  USBSelectedDrive = NULL;
}

/**
  * @brief  Display the content of the microSD drive
  * @param  None
  * @retval None
  */
static void goto_msd_drive (void)
{
  SD_Init();

  if (SD_GetStatus() == 0)
  {
    f_mount( 1, &MSD_fatfs );
    FILEMGR_SwitchPage(MainDiskPage, MSD_FILE_EXPLORER);
  }
  else
  {
    FILEMGR_SwitchPage(MainDiskPage, DRIVE_NOT_READY);
  }

}

/**
  * @brief  Return from the microSD content page
  * @param  None
  * @retval None
  */
static void return_from_msd_drive (void)
{
  FILEMGR_SwitchPage(MSDSelectedDrive, WHOLE_DISK_EXPLORER);
  MSDSelectedDrive = NULL;
}

/**
  * @brief  return to the drives page from error page
  * @param  None
  * @retval None
  */
static void return_to_whole_drive (void)
{
  FILEMGR_SwitchPage(NoDrive, WHOLE_DISK_EXPLORER);
  NoDrive = NULL;
}

/**
  * @brief  return to the drives page from MSD or USB content page
  * @param  None
  * @retval None
  */
static void back_to_drives (void)
{

  Remove_CtxMenu();

  FILEMGR_SwitchPage(Current_Page, WHOLE_DISK_EXPLORER); ///current page == null

  MSDSelectedDrive = NULL;
  USBSelectedDrive = NULL;
}

/**
  * @brief  Copy disk content in the explorer list
  * @param  path: pointer to root path
  * @retval Status
  */
static uint8_t Explore_Disk (char* path)
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
  Display_list_ptr = 0;
  strncpy((char *)FILEMGR_FileList.file[FILEMGR_FileList.ptr].line, ".." , FILELIST_LINESIZE);
  FILEMGR_FileList.file[FILEMGR_FileList.ptr].type = 1;
  FILEMGR_FileList.ptr++;

  res = f_opendir(&dir, path);

  if (res == FR_OK)
  {

    while (1)
    {
      res = f_readdir(&dir, &fno);

      if (res != FR_OK || fno.fname[0] == 0)
      {

        Sort_Files();
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

      if (FILEMGR_FileList.ptr < FILELIST_DEPDTH)
      {
        if ((fno.fattrib & AM_DIR) == AM_DIR)
        {
          strncpy((char *)FILEMGR_FileList.file[FILEMGR_FileList.ptr].line, (char *)fn, FILEMGR_FILESIZE);
          FILEMGR_FileList.file[FILEMGR_FileList.ptr].type = 1;
          FILEMGR_FileList.ptr++;
        }
        else
        {
          if (((FileMgr_Mask & MASK_WAV) && (check_filename_ext((char *)fn, "wav" ))) ||
                  ((FileMgr_Mask & MASK_BMP) && (check_filename_ext((char *)fn, "bmp" ))) ||
                    ((FileMgr_Mask & MASK_JPG) && (check_filename_ext((char *)fn, "jpg" ))) ||
                      (FileMgr_Mask == MASK_NONE))
          {

            strncpy((char *)FILEMGR_FileList.file[FILEMGR_FileList.ptr].line, (char *)fn, FILEMGR_FILESIZE);
            FILEMGR_FileList.file[FILEMGR_FileList.ptr].type = 0;
            FILEMGR_FileList.ptr++;
          }
          else
          {
            continue;
          }
        }
      }
      else
      {
        FILEMGR_FileList.full = 1;
      }
      
    }
  }
  return res;
}

/**
  * @brief  Displays disk content
  * @param  offset : position in the file list
  * @retval None
  */
static void Display_Files (uint8_t offset)
{
  uint8_t idx = 0;

  for ( ; offset < FILEMGR_FileList.ptr ; offset ++)
  {
    if (FILEMGR_FileList.file[offset].type == 1)
    {
      
      if (FileMgr_Mode == MODE_NONE)
      {
        AddSmallIconControlObj (Current_Page,
                                idx ,
                                (uint8_t *)folder_small_icon,
                                (uint8_t *)FILEMGR_FileList.file[offset].line,
                                MODE_NONE,
                                Return_Selected_Item,
                                Explore_Directory) ;
      }

      else if (FileMgr_Mode == MODE_SEL_DIR)
      {
        AddSmallIconControlObj (Current_Page,
                                idx ,
                                (uint8_t *)folder_small_icon,
                                (uint8_t *)FILEMGR_FileList.file[offset].line,
                                MODE_SEL_DIR,
                                Return_Selected_Item,
                                Explore_Directory) ;
      }
      else if (FileMgr_Mode == MODE_SEL_FIL)
      {
        AddSmallIconControlObj (Current_Page,
                                idx ,
                                (uint8_t *)folder_small_icon,
                                (uint8_t *)FILEMGR_FileList.file[offset].line,
                                MODE_NONE,
                                Return_Selected_Item,
                                Explore_Directory) ;
      }

    }
    else
    {
      if (FileMgr_Mode == MODE_SEL_FIL)
      {
        AddSmallIconControlObj (Current_Page,
                                idx,
                                (uint8_t *)file_small_icon,
                                (uint8_t *)FILEMGR_FileList.file[offset].line,
                                MODE_SEL_FIL,
                                Return_Selected_Item,
                                Explore_Directory) ;
      }

      else if (FileMgr_Mode == MODE_NONE)
      {
        AddSmallIconControlObj (Current_Page,
                                idx,
                                (uint8_t *)file_small_icon,
                                (uint8_t *)FILEMGR_FileList.file[offset].line,
                                MODE_REM_FIL,
                                Delete_Selected_Item,
                                Explore_Directory) ;
      }

    }

    if (idx == (FILELIST_MAX_DISPLAY - 1))
    {
      return;
    }
    idx++;

  }
  
}

/**
  * @brief  return to drives page after a medium disconnection event
  * @param  None
  * @retval None
  */
static void FILE_MGR_Media_changed (uint8_t disconnect_source)
{
  if  (Current_Page != NULL)
  {
    if ((Current_Page->Page_ID == MSD_FILE_EXPLORER) || 
        (Current_Page->Page_ID == USB_FILE_EXPLORER))
    {
      if(FILEMGR_CurrentDirectory[0] == disconnect_source )
      {
        if (FileMgr_Mode == MODE_NONE)
          FILEMGR_SwitchPage(Current_Page, WHOLE_DISK_EXPLORER);
        else
          return_from_direct();


        MSDSelectedDrive = NULL;
        USBSelectedDrive = NULL;
      }
    }
  }
}

/**
  * @brief  Displays the next view in the explorer
  * @param  None
  * @retval None
  */
static void Display_Next (void)
{

  Remove_CtxMenu();

  if (Display_list_ptr < FILEMGR_FileList.ptr - FILELIST_MAX_DISPLAY)
  {
    Remove_OldList(Current_Page);
    Display_list_ptr ++;
    Display_Files(Display_list_ptr);
    Refresh_FileList(Current_Page);
    FILE_MGR_RefreshScroll();
  }
  
}


/**
  * @brief  Displays the previous view in the explorer
  * @param  None
  * @retval None
  */
static void Display_Previous (void)
{

  Remove_CtxMenu();

  if (Display_list_ptr > 0)
  {
    Remove_OldList(Current_Page);
    Display_list_ptr --;
    Display_Files(Display_list_ptr);

    Refresh_FileList(Current_Page);

    FILE_MGR_RefreshScroll();
  }
  
}

/**
  * @brief  Clear old view in the explorer
  * @param  pPage : active page
  * @retval None
  */
static void Remove_OldList (GL_Page_TypeDef *pPage)
{

  DestroyPageControl (pPage, 3 );
  DestroyPageControl (pPage, 4 );
  DestroyPageControl (pPage, 5 );
  DestroyPageControl (pPage, 6 );
  DestroyPageControl (pPage, 7 );
  DestroyPageControl (pPage, 8 );
  DestroyPageControl (pPage, 9 );
  DestroyPageControl (pPage, 10 );

  DestroyPageControl (pPage, 3 + 8 );
  DestroyPageControl (pPage, 4 + 8 );
  DestroyPageControl (pPage, 5 + 8 );
  DestroyPageControl (pPage, 6 + 8 );
  DestroyPageControl (pPage, 7 + 8 );
  DestroyPageControl (pPage, 8 + 8 );
  DestroyPageControl (pPage, 9 + 8 );
  DestroyPageControl (pPage, 10 + 8 );

  DestroyPageControl (pPage, 3 + 16 );
  DestroyPageControl (pPage, 4 + 16 );
  DestroyPageControl (pPage, 5 + 16 );
  DestroyPageControl (pPage, 6 + 16 );
  DestroyPageControl (pPage, 7 + 16 );
  DestroyPageControl (pPage, 8 + 16 );
  DestroyPageControl (pPage, 9 + 16 );
  DestroyPageControl (pPage, 10 + 16 );

  if (Set_Scroll == 1)
  {
    DestroyPageControl (pPage, 0xFE );
    DestroyPageControl (pPage, 0xFF );
  }
}

/**
  * @brief  Clear old view in the explorer
  * @param  pPage : active page
  * @retval None
  */
static void Refresh_FileList (GL_Page_TypeDef *pPage)
{

  LCD_SetTextColor(GL_White);
  GL_LCD_FillRect(28 , 315 , 185 , 288);

  RefreshPageControl (pPage, 3 );
  RefreshPageControl (pPage, 4 );
  RefreshPageControl (pPage, 5 );
  RefreshPageControl (pPage, 6 );
  RefreshPageControl (pPage, 7 );
  RefreshPageControl (pPage, 8 );
  RefreshPageControl (pPage, 9 );
  RefreshPageControl (pPage, 10 );

  RefreshPageControl (pPage, 3 + 8 );
  RefreshPageControl (pPage, 4 + 8 );
  RefreshPageControl (pPage, 5 + 8 );
  RefreshPageControl (pPage, 6 + 8 );
  RefreshPageControl (pPage, 7 + 8 );
  RefreshPageControl (pPage, 8 + 8 );
  RefreshPageControl (pPage, 9 + 8 );
  RefreshPageControl (pPage, 10 + 8 );

  RefreshPageControl (pPage, 3 + 16 );
  RefreshPageControl (pPage, 4 + 16 );
  RefreshPageControl (pPage, 5 + 16 );
  RefreshPageControl (pPage, 6 + 16 );
  RefreshPageControl (pPage, 7 + 16 );
  RefreshPageControl (pPage, 8 + 16 );
  RefreshPageControl (pPage, 9 + 16 );
  RefreshPageControl (pPage, 10 + 16 );
}

/**
  * @brief  Sort files by name in the file list
  * @param  None
  * @retval None
  */
static void Sort_Files (void)
{
  uint8_t i = 0 , j = 0;
  uint8_t temp [FILELIST_LINESIZE];
  uint8_t temp2;

  for ( ; i < FILEMGR_FileList.ptr ; i ++)
  {
    if (FILEMGR_FileList.file[i].type == 0)
    {

      for (j = i ; j < FILEMGR_FileList.ptr ; j ++)
      {
        if (FILEMGR_FileList.file[j].type == 1)
        {
          strncpy((char *)temp , (char *)FILEMGR_FileList.file[i].line, FILELIST_LINESIZE );
          strncpy((char *)(char *)FILEMGR_FileList.file[i].line , (char *)FILEMGR_FileList.file[j].line, FILELIST_LINESIZE );
          strncpy((char *)FILEMGR_FileList.file[j].line, (char *)temp, FILELIST_LINESIZE);

          temp2 = FILEMGR_FileList.file[i].type;
          FILEMGR_FileList.file[i].type = FILEMGR_FileList.file[j].type;
          FILEMGR_FileList.file[j].type = temp2;
          ;
        }
      }
    }
  }
}

/**
  * @brief  Activate explorer view following icon action
  * @param  None
  * @retval None
  */
static void Explore_Directory (void)
{

  if(Context_Menu == 0)
  {
    selected_item = GL_GetLastEvent() + Display_list_ptr - 3;
  }

  /* Handle up icon */
  if (selected_item == 0)
  {
    if (strcmp (FILEMGR_CurrentDirectory, "0:") == 0)
    {
      return_from_usb_drive();
    }
    else if (strcmp (FILEMGR_CurrentDirectory, "1:") == 0)
    {
      return_from_msd_drive();
    }
    else
    {

      GetParentDir(FILEMGR_CurrentDirectory);

      Explore_Disk(FILEMGR_CurrentDirectory);

      Remove_OldList(Current_Page);

      Display_Files(0);
      Refresh_FileList(Current_Page);

      if (FILEMGR_FileList.ptr > FILELIST_MAX_DISPLAY)
      {
        GL_AddScroll (Current_Page, 293, 28, 170, Display_Previous, Display_Next, 0);
        Set_Scroll = 1;
      }
      else if (Set_Scroll == 1 )
      {
        Set_Scroll = 0;
        DestroyPageControl (Current_Page, 0xFE);
        DestroyPageControl (Current_Page, 0xFF);
        LCD_SetTextColor(GL_White);
        GL_LCD_FillRect(28,  40 , 200, 40);

      }
    }

  }
  else
  {
    if ((FILEMGR_FileList.file[selected_item].type == 1) && 
        ((FileMgr_Mode == MODE_NONE ) || 
         (FileMgr_Mode == MODE_SEL_FIL )))
    {
      Explore_SubFolder();
    }
    else //File
    {
      Add_CtxMenu(FileMgr_Mode);
    }

  }

}


/**
  * @brief  Refresh the scroll state
  * @param  None
  * @retval None
  */
void FILE_MGR_RefreshScroll (void)
{
  if(Current_Page->Page_Active == GL_TRUE)
  {
    if ((Current_Page->Page_ID & MOD_FILEMGR_UID) == MOD_FILEMGR_UID)
    {
      if (Set_Scroll == 1)
      {
        DestroyPageControl (Current_Page, 0xFE);
        DestroyPageControl (Current_Page, 0xFF);
        GL_AddScroll (Current_Page, 293, 28, 170, Display_Previous, Display_Next, 0);
        GL_UpdateScrollPosition (NULL, 293, 28, 170, (Display_list_ptr * 100)/(FILEMGR_FileList.ptr - FILELIST_MAX_DISPLAY));
      }
    }
  }
}

/**
  * @brief  Refresh the scroll state
  * @param  None
  * @retval None
  */
void FILE_MGR_KillScroll (void)
{
  DestroyPageControl (Current_Page, 0xFE);
  DestroyPageControl (Current_Page, 0xFF);
}

/**
  * @brief  Launch directly the file browser from command line
  * @param  pParent : caller page
  * @param  sel_mode : 
  * @param  filter :
  * @param  pEventHandler :     
  * @retval None
  */
void FILMGR_DirectEx(GL_Page_TypeDef* pParent, uint8_t sel_mode, uint16_t filter, void (*pEventHandler)(void))
{
  if(FileMgr_Nesting > 0)
  {
    Instance_Page = Current_Page; 
    FileMgr_Nesting++;
    strcpy(FILEMGR_CurrentDirectoryBk, FILEMGR_CurrentDirectory);
  }
  
  strcpy(FILEMGR_CurrentDirectory, "");
  
  (*pParent).ShowPage(pParent, GL_FALSE);
  Backup_Page = pParent;
  FileMgr_Mode = sel_mode;
  FileMgr_Mask = filter;
  Set_Scroll = 0;
  FILEMGR_FileList.full = 0;
  MSDSelectedDrive = NULL;
  USBSelectedDrive = NULL;
  Select_Hanlder = pEventHandler;
  FILEMGR_CreatePage(DIRECT_ACCESS_EXPLORER);
  Current_Page = MainDiskPage;
  RefreshPage(MainDiskPage);
}

/**
  * @brief  Return from the direct access page
  * @param  None
  * @retval None
  */
static void return_from_direct (void)
{
  (*MainDiskPage).ShowPage(MainDiskPage, GL_FALSE);
  DestroyPage(MainDiskPage);
  free(MainDiskPage);

  FileMgr_Mode = MODE_NONE;

  Select_Hanlder();
  MainDiskPage = NULL;

  if(FileMgr_Nesting > 0)
  {
    Current_Page = Instance_Page;
    FileMgr_Nesting--;
    strcpy(FILEMGR_CurrentDirectory, FILEMGR_CurrentDirectoryBk);
  }
  else
  {
    strcpy(FILEMGR_CurrentDirectory, "");
  }

  RefreshPage(Backup_Page);
}


/**
  * @brief  Return from the direct access page
  * @param  None
  * @retval None
  */
static void Return_Selected_Item (void)
{
  Context_Menu = 0;
  if (selected_item != 0)
  {
    strcat (FILEMGR_CurrentDirectory, "/");
    strcat (FILEMGR_CurrentDirectory, (char *)FILEMGR_FileList.file[selected_item].line);
  }
  return_from_direct();
}


/**
  * @brief  Return from the direct access page
  * @param  None
  * @retval None
  */
static void Open_Selected_Item (void)
{
  Remove_CtxMenu();

  if (check_filename_ext((char *)FILEMGR_FileList.file[selected_item].line, "bmp") || 
      check_filename_ext((char *)FILEMGR_FileList.file[selected_item].line, "BMP") ||
      check_filename_ext((char *)FILEMGR_FileList.file[selected_item].line, "jpg") ||
      check_filename_ext((char *)FILEMGR_FileList.file[selected_item].line, "JPG"))
  {
    strcat (FILEMGR_CurrentDirectory, "/");
    strcat (FILEMGR_CurrentDirectory, (char *)FILEMGR_FileList.file[selected_item].line);
    IMAGE_DirectEx(Current_Page, (uint8_t *)FILEMGR_CurrentDirectory);
    GetParentDir(FILEMGR_CurrentDirectory);
  }

  else if (check_filename_ext((char *)FILEMGR_FileList.file[selected_item].line, "wav") || 
           check_filename_ext((char *)FILEMGR_FileList.file[selected_item].line, "WAV"))
  {

    strcat (FILEMGR_CurrentDirectory, "/");
    strcat (FILEMGR_CurrentDirectory, (char *)FILEMGR_FileList.file[selected_item].line);
    AUDIO_DirectEx(Current_Page, (uint8_t *)FILEMGR_CurrentDirectory, MOD_AUDIO_CTX_ENABLED);
    GetParentDir(FILEMGR_CurrentDirectory);
  }
  else
  {
    RefreshPage(Current_Page);
    FILE_MGR_RefreshScroll();
  }

}

/**
  * @brief  Return from the direct access page
  * @param  None
  * @retval None
  */
static void Cancel_ctx_menu (void)
{
  Remove_CtxMenu();
  RefreshPage(Current_Page);
  FILE_MGR_RefreshScroll();
}



/**
  * @brief  Return from the direct access page
  * @param  None
  * @retval None
  */
static void Delete_Selected_Item (void)
{
  uint8_t temp[128];
  uint16_t item ;
  item = selected_item;

  strcpy((char *)temp, (char *)FILEMGR_CurrentDirectory);

  if (item != 0)
  {
    strcat ((char *)temp, "/");
    strcat ((char *)temp, (char *)FILEMGR_FileList.file[item].line);
  }

  f_unlink((char *)temp);

  DestroyPageControl (Current_Page, 80);
  DestroyPageControl (Current_Page, 81);
  DestroyPageControl (Current_Page, 82);
  Context_Menu = 0;

  GetParentDir((char *)temp);
  Explore_Disk((char *)temp);

  if (FILEMGR_FileList.ptr <= FILELIST_MAX_DISPLAY)
  {
    Set_Scroll = 0;
    FILE_MGR_KillScroll();
  }

  if (Display_list_ptr > 0)
  {
    Display_list_ptr --;
  }

  Remove_OldList(Current_Page);
  Display_Files(Display_list_ptr);
  Refresh_FileList(Current_Page);
  FILE_MGR_RefreshScroll();
}


/**
  * @brief  Return from the direct access page
  * @param  None
  * @retval None
  */
static void Add_CtxMenu (uint8_t mode)
{
  GL_PageControls_TypeDef* button;

  if(Context_Menu == 0)
  {
    ChangeLabelColor (Current_Page , 3 + 8 + selected_item - Display_list_ptr ,GL_Blue);
    LCD_SetTextColor(GL_Black);
    GL_LCD_FillRect(87 , 205 , 74 + 30 , 96);
    GL_LCD_DrawRect(82 , 210 , 74 + 30 , 96);
    LCD_SetTextColor(GL_White);
    GL_LCD_DrawRect(83 , 209 , 72 + 30, 94);
    LCD_SetTextColor(GL_Grey);
    GL_LCD_FillRect(84 , 208 , 71+ 30 , 92);
    Context_Menu = 1;

    if (mode == MODE_NONE)
    {

  if ((check_filename_ext((char *)FILEMGR_FileList.file[selected_item].line, "bmp")) || 
      (check_filename_ext((char *)FILEMGR_FileList.file[selected_item].line, "BMP")) ||
      (check_filename_ext((char *)FILEMGR_FileList.file[selected_item].line, "jpg")) || 
      (check_filename_ext((char *)FILEMGR_FileList.file[selected_item].line, "JPG")))
  {
     button = NewButton(80,  (uint8_t *)"  Display  ", Open_Selected_Item);

      AddPageControlObj(206, 88, button, Current_Page);
      RefreshPageControl(Current_Page, 80); 

      button = NewButton(81, (uint8_t *)"  Delete   ", Delete_Selected_Item);
      AddPageControlObj(206, 120, button, Current_Page);
      RefreshPageControl(Current_Page, 81);

      button = NewButton(82, (uint8_t *)"  Cancel   ", Cancel_ctx_menu);
      AddPageControlObj(206, 152, button, Current_Page);
      RefreshPageControl(Current_Page, 82);
  }

  else if ((check_filename_ext((char *)FILEMGR_FileList.file[selected_item].line, "wav")) || (check_filename_ext((char *)FILEMGR_FileList.file[selected_item].line, "WAV")))
  {

      button = NewButton(80, (uint8_t *)"   Play    ", Open_Selected_Item);

      AddPageControlObj(206, 88, button, Current_Page);
      RefreshPageControl(Current_Page, 80); 

      button = NewButton(81, (uint8_t *)"  Delete   ", Delete_Selected_Item);
      AddPageControlObj(206, 120, button, Current_Page);
      RefreshPageControl(Current_Page, 81);

      button = NewButton(82, (uint8_t *)"  Cancel   ", Cancel_ctx_menu);
      AddPageControlObj(206, 152, button, Current_Page);
      RefreshPageControl(Current_Page, 82);
  }
  else
  {
      button = NewButton(81, (uint8_t *)"  Delete   ", Delete_Selected_Item);
      AddPageControlObj(206, 88, button, Current_Page);
      RefreshPageControl(Current_Page, 81);

      button = NewButton(82, (uint8_t *)"  Cancel   ", Cancel_ctx_menu);
      AddPageControlObj(206, 120, button, Current_Page);
      RefreshPageControl(Current_Page, 82);   
  }

    }
    else if (mode ==  MODE_SEL_FIL )
    {
      button = NewButton(80, (uint8_t *)"  Select   ", Return_Selected_Item);
      AddPageControlObj(206, 88, button, Current_Page);
      RefreshPageControl(Current_Page, 80);

      button = NewButton(81, (uint8_t *)"  Cancel   ", Cancel_ctx_menu);
      AddPageControlObj(206, 120, button, Current_Page);
      RefreshPageControl(Current_Page, 81);

    }
    else if (mode ==  MODE_SEL_DIR)
    {
      button = NewButton(80, (uint8_t *)"  Select   ", Return_Selected_Item);
      AddPageControlObj(206, 88, button, Current_Page);
      RefreshPageControl(Current_Page, 80); 

      button = NewButton(81, (uint8_t *)"  Browse   ", Explore_SubFolder);
      AddPageControlObj(206, 120, button, Current_Page);
      RefreshPageControl(Current_Page, 81);

      button = NewButton(82, (uint8_t *)"  Cancel   ", Cancel_ctx_menu);
      AddPageControlObj(206, 152, button, Current_Page);
      RefreshPageControl(Current_Page, 82);

    }

  }
}

/**
  * @brief  Return from the direct access page
  * @param  None
  * @retval None
  */
static void Remove_CtxMenu (void)

{
  if(Context_Menu == 1)
  {
    ChangeLabelColor (Current_Page , 3 + 8 + selected_item - Display_list_ptr ,GL_Black);
    DestroyPageControl (Current_Page, 80);
    DestroyPageControl (Current_Page, 81);
    DestroyPageControl (Current_Page, 82); 
    Context_Menu = 0;
  }
}

/**
  * @brief  Refresh the contextual menu when refreshing the parent page
  * @param  None
  * @retval None
  */
static void Refresh_CustomDraw (void)
{
  Remove_CtxMenu();
  FILE_MGR_RefreshScroll();
}
/**
  * @brief  Return from the direct access page
  * @param  None
  * @retval None
  */
static void Explore_SubFolder (void)
{
  /* Handle directories */
  strcat (FILEMGR_CurrentDirectory, "/");
  strcat (FILEMGR_CurrentDirectory, (char *)FILEMGR_FileList.file[selected_item].line);

  Explore_Disk(FILEMGR_CurrentDirectory);

  Remove_OldList(Current_Page);

  Display_Files(0);
  Refresh_FileList(Current_Page);


  if (FILEMGR_FileList.ptr > FILELIST_MAX_DISPLAY)
  {
    GL_AddScroll (Current_Page, 293, 28, 170, Display_Previous, Display_Next, 0);
    Set_Scroll = 1;
  }
  else if (Set_Scroll == 1 )
  {
    Set_Scroll = 0;
    DestroyPageControl (Current_Page, 0xFE);
    DestroyPageControl (Current_Page, 0xFF);
    LCD_SetTextColor(GL_White);
    GL_LCD_FillRect(28,  40 , 200, 40);
  }

  Remove_CtxMenu();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
