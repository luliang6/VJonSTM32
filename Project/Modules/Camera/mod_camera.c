/**
  ******************************************************************************
  * @file    mod_camera.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Graphical window and events implementation of the camera module
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
#include "mod_camera.h"
#include "camera_api.h"
#include "app_camera.h"
#include "gl_mgr.h"
#include "gl_camera_res.c"

#include "ObjDetectorAPI.h"
#include "faceDetection.h"

/* Private typedef -----------------------------------------------------------*/
#define _X(s) (119 + s)
#define _Y(s) (159 + s)

/* Private define ------------------------------------------------------------*/
#define CAMERA_MAIN_PAGE           MOD_CAMERA_UID + 0
#define CAMERA_CAPTURE_PAGE        MOD_CAMERA_UID + 1
#define CAMERA_SETTINGS            MOD_CAMERA_UID + 2
#define CAMERA_SNAPSHOT            MOD_CAMERA_UID + 3
#define CAMERA_DST_PATH            BKPSRAM_BASE + 0x100
#define CAMERA_DST_PATH_SIZE       19
#define Camera_Task_PRIO          ( tskIDLE_PRIORITY + 4 )
#define Camera_Task_TASK          ( 768 )

#define CAPTURE_IDLE              0
#define CAPTURE_ONGOING           1
/* Private typedef -----------------------------------------------------------*/

typedef union _CAMERA_CONFIG_TypeDef
{
    uint32_t d32;
    struct
    {
    uint32_t ImageStorage :
        16;
    uint32_t ImageFormat:
        4;
    uint32_t ImageEffect :
        4;
    uint32_t CameraBrighness :
        8;
    }
    b;
} CAMERA_CONFIG_TypeDef ;


typedef struct _CAMERA_TIMER_TypeDef
{
    uint8_t Enabled;
    uint8_t Delay;
} CAMERA_TIMER_TypeDef;

uint8_t  CAMERA_InitDone = 0;
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/*Graphical draw process */
static void CAMERA_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex);
static void CAMERA_CreatePage(uint8_t Page);
static void CAMERA_GetDefaultDirectory (void);
static void CAMERA_Startup (void);
static void CAMERA_Background (void);
static void CAMERA_Media_changed (uint8_t disconnect_source);
static void InitCamera (void);
static void Camera_Task(void * pvParameters);
/* Control Actions */
static void Show_Frame(void);
static void Capture_Brightness_Inc(void);
static void Capture_Brightness_Dec(void);
static void goto_settings (void);
static void return_from_settings (void);
static void return_to_menu (void);
static void goto_capture (void);
static void return_from_capture(void);
static void goto_start_capture(void);
static void goto_view(void);
static void Sel_folder (void);
static void Update_folder (void);
static void Check_DefaultPath (char *path);
static void DrawCameraBackgound (void);


/* Private variables ---------------------------------------------------------*/
static GL_Page_TypeDef *CameraMainPage;
static GL_Page_TypeDef *CameraCapturePage;
static GL_Page_TypeDef *CameraSettingsPage;

CAMERA_CONFIG_TypeDef   camera_cfg;

xTaskHandle             Camera_Task_Handle = NULL;
xSemaphoreHandle        Camera_xSemaphore = NULL;
int32_t                 Camera_Brightness = 0;
uint8_t                 CAMERA_DefaultPath[129];
uint8_t                 Camera_UsedStorage = 0xFF;
uint8_t                 capture_status = CAPTURE_IDLE;
CAMERA_TIMER_TypeDef    capture_timer;

MOD_InitTypeDef  mod_camera =
{
    MOD_CAMERA_UID,
    MOD_CAMERA_VER,
    (uint8_t *)"Camera",
    (uint8_t *)camera_icon,
    CAMERA_Startup,
    CAMERA_Background,
    CAMERA_Media_changed,
};

extern ImageBuffer_TypeDef ImageBuffer;
extern void RGB16toRGB24(uint8_t *pDestBuffer, uint8_t *pSrcBuffer, uint8_t is_bgr);

/* Private functions ---------------------------------------------------------*/

/**
* @brief  Handle the Module startup action and display the main menu
* @param  None
* @retval None
*/
static void CAMERA_Startup (void)
{
    vTaskSuspend(Core_Time_Task_Handle);
    CAMERA_SwitchPage(GL_HomePage, CAMERA_MAIN_PAGE);
    Camera_Config(BMP_QVGA) ;
    Camera_UsedStorage = 0xFF;
    capture_timer.Enabled = 0;
}

/**
* @brief  Close parent page and display a child sub-page
* @param  pParent : parent page
* @param  PageIndex : sub-page index
* @retval None
*/
static void CAMERA_SwitchPage(GL_Page_TypeDef* pParent, uint32_t PageIndex)
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

    CAMERA_CreatePage(PageIndex);

    switch (PageIndex)
    {
        case CAMERA_MAIN_PAGE:
            NextPage = CameraMainPage;
            break;

        case CAMERA_CAPTURE_PAGE:
            NextPage = CameraCapturePage;
            break;

        case CAMERA_SETTINGS:
            NextPage = CameraSettingsPage;
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
static void CAMERA_CreatePage(uint8_t Page)
{

    GL_PageControls_TypeDef* icon, *item;
    GL_PageControls_TypeDef* BackButton;
    GL_PageControls_TypeDef* CaptureButton;

    GL_PageControls_TypeDef* BrightnessInc;
    GL_PageControls_TypeDef* BrightnessDec;
    GL_PageControls_TypeDef* BrightnessLabel;

    GL_PageControls_TypeDef* ZoomInc;
    GL_PageControls_TypeDef* ZoomDec;
    GL_PageControls_TypeDef* ZoomLabel;


    GL_ComboBoxGrp_TypeDef* pTmp;

    uint8_t path[129];

    GL_SetBackColor( GL_White );
    GL_SetTextColor( GL_Blue );

    switch (Page)
    {
        case CAMERA_MAIN_PAGE:
        {
            CameraMainPage = malloc(sizeof(GL_Page_TypeDef));
            Create_PageObj( CameraMainPage, CAMERA_MAIN_PAGE );
            icon = NewIcon (5, camera_icon, 50, 50, MOD_NullFunc);
            AddPageControlObj(185, 40, icon, CameraMainPage);

            GL_SetMenuItem(CameraMainPage, (uint8_t *)"Capture", 0, goto_capture );
            GL_SetMenuItem(CameraMainPage, (uint8_t *)"Settings", 1, goto_settings );
            GL_SetMenuItem(CameraMainPage, (uint8_t *)"Viewer", 2, goto_view );
            GL_SetMenuItem(CameraMainPage, (uint8_t *)"Return", 3, return_to_menu );
            GL_SetPageHeader(CameraMainPage, (uint8_t *)"Camera Menu");
        }
        break;

        case CAMERA_CAPTURE_PAGE:
        {
            CAMERA_GetDefaultDirectory();
            MOD_GetParam(CAMERA_SETTINGS_MEM, &camera_cfg.d32);

            CameraCapturePage = malloc(sizeof(GL_Page_TypeDef));
            Create_PageObj( CameraCapturePage, CAMERA_CAPTURE_PAGE );


//       CaptureButton = NewButton(1, (uint8_t *)" Capture", MOD_NullFunc);
            CaptureButton = NewButton(1, (uint8_t *)" Capture", goto_start_capture);
            AddPageControlObj(150, 212, CaptureButton, CameraCapturePage);


            BackButton = NewButton(2, (uint8_t *)" Return ", return_from_capture);
            AddPageControlObj(230, 212, BackButton, CameraCapturePage);

            GL_SetPageHeader(CameraCapturePage, (uint8_t *)"Camera Capture");

            CameraCapturePage->CustomPreDraw = DrawCameraBackgound;
            CameraCapturePage->CustomPostDraw = InitCamera;

            BrightnessInc = NewButton(3, (uint8_t *)" + ", Capture_Brightness_Inc);
            AddPageControlObj(35, 50, BrightnessInc, CameraCapturePage);

            BrightnessDec = NewButton(4, (uint8_t *)" - ", Capture_Brightness_Dec);
            AddPageControlObj(35, 170, BrightnessDec, CameraCapturePage);

            if ( camera_cfg.b.ImageEffect <= 1)
            {
                BrightnessLabel  = NewLabel(5, (uint8_t *)"Brightness", GL_RIGHT_VERTICAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
            }
            else
            {
                BrightnessLabel  = NewLabel(5, (uint8_t *)"Brightness", GL_RIGHT_VERTICAL, GL_FONT_SMALL, GL_Grey, GL_FALSE);
            }

            AddPageControlObj(15, 82, BrightnessLabel, CameraCapturePage);


            ZoomInc = NewButton(6, (uint8_t *)" + ", MOD_NullFunc);
            AddPageControlObj(315, 50, ZoomInc, CameraCapturePage);

            ZoomDec = NewButton(7, (uint8_t *)" - ", MOD_NullFunc);
            AddPageControlObj(315, 170, ZoomDec, CameraCapturePage);

            ZoomLabel  = NewLabel(8, (uint8_t *)"   Zoom", GL_RIGHT_VERTICAL, GL_FONT_SMALL, GL_Grey, GL_FALSE);
            AddPageControlObj(295, 82, ZoomLabel, CameraCapturePage);

            BrightnessLabel  = NewLabel(9, (uint8_t *)"", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
            AddPageControlObj(280, 30, BrightnessLabel, CameraCapturePage);

            break;
        }

        case CAMERA_SETTINGS:
        {
            MOD_GetParam(CAMERA_SETTINGS_MEM, &camera_cfg.d32);

            CameraSettingsPage = malloc(sizeof(GL_Page_TypeDef));
            Create_PageObj( CameraSettingsPage, CAMERA_CAPTURE_PAGE );

            GL_SetPageHeader(CameraSettingsPage, (uint8_t *)"Camera Settings");

            BackButton = NewButton(1, (uint8_t *)" Return ", return_from_settings);
            AddPageControlObj(195, 212, BackButton, CameraSettingsPage);

            item  = NewLabel(2, (uint8_t *)"Storage :", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
            AddPageControlObj(260, 140, item, CameraSettingsPage);


            item  = NewButton(9, (uint8_t *)"...", Sel_folder);
            AddPageControlObj(120, 150, item, CameraSettingsPage);

            Check_DefaultPath((char *)path);

            if(strlen((char *)path) > CAMERA_DST_PATH_SIZE)
            {
                path[CAMERA_DST_PATH_SIZE - 2] = '.';
                path[CAMERA_DST_PATH_SIZE - 1] = '.';
                path[CAMERA_DST_PATH_SIZE]     =  0;
            }

            item  = NewLabel(3, path, GL_HORIZONTAL, GL_FONT_SMALL, GL_Blue, GL_FALSE);
            AddPageControlObj(260, 155, item, CameraSettingsPage);

            item  = NewLabel(4, (uint8_t *)"Image Format:", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
            AddPageControlObj(260, 50, item, CameraSettingsPage);

            item = NewComboBoxGrp(5);
            AddComboOption (item->objPTR, (uint8_t *)"BMP 320x240", MOD_NullFunc);
            AddComboOption (item->objPTR, (uint8_t *)"JPG 320x240", MOD_NullFunc);
            AddPageControlObj( 260, 65, item, CameraSettingsPage);

            pTmp = (GL_ComboBoxGrp_TypeDef*)(item->objPTR);

            if ((camera_cfg.b.ImageFormat > 0) && (camera_cfg.b.ImageFormat < 3))
            {
                pTmp->ComboOptions[0]->IsActive = GL_FALSE;
                pTmp->ComboOptions[camera_cfg.b.ImageFormat -1]->IsActive = GL_TRUE;
            }

            item  = NewLabel(6, (uint8_t *)"Image Effect:", GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE);
            AddPageControlObj(260, 95, item, CameraSettingsPage);

            item = NewComboBoxGrp(7);
            AddComboOption (item->objPTR, (uint8_t *)"Normal", MOD_NullFunc);

            if(Camera == OV2640_CAMERA)
            {
                AddComboOption (item->objPTR, (uint8_t *)"Antique", MOD_NullFunc);
                AddComboOption (item->objPTR, (uint8_t *)"Bluish", MOD_NullFunc);
                AddComboOption (item->objPTR, (uint8_t *)"Greenish", MOD_NullFunc);
                AddComboOption (item->objPTR, (uint8_t *)"Redish", MOD_NullFunc);
                AddComboOption (item->objPTR, (uint8_t *)"GrayScale", MOD_NullFunc);
                AddComboOption (item->objPTR, (uint8_t *)"Negative", MOD_NullFunc);
                AddComboOption (item->objPTR, (uint8_t *)"Neg. GrayScale", MOD_NullFunc);
            }

            AddPageControlObj( 260, 110, item, CameraSettingsPage);
            if(Camera == OV2640_CAMERA)
            {
                pTmp = (GL_ComboBoxGrp_TypeDef*)(item->objPTR);

                if ((camera_cfg.b.ImageEffect > 0) && (camera_cfg.b.ImageEffect < 9))
                {
                    pTmp->ComboOptions[0]->IsActive = GL_FALSE;
                    pTmp->ComboOptions[camera_cfg.b.ImageEffect -1]->IsActive = GL_TRUE;
                }
            }

        }
        break;

        default:
            break;
    }
}

/**
* @brief  Retrieve default storage path from settings
* @param  None
* @retval None
*/
static void CAMERA_GetDefaultDirectory (void)
{
    Check_DefaultPath((char *)CAMERA_DefaultPath);
}

/**
* @brief  return to global main menu action
* @param  None
* @retval None
*/
static void return_to_menu (void)
{
    CAMERA_SwitchPage(CameraMainPage, PAGE_MENU);
    CameraMainPage = NULL;
}

/**
* @brief  Display the capture page
* @param  None
* @retval None
*/
static void goto_capture (void)
{
    CAMERA_InitDone = 0;
    CAMERA_SwitchPage(CameraMainPage, CAMERA_CAPTURE_PAGE);
    CameraMainPage = NULL;
}

/**
* @brief  return the module main menu from the capture page
* @param  None
* @retval None
*/
static void return_from_capture (void)
{
    if(capture_status == CAPTURE_IDLE)
    {
        DMA_Cmd(DMA2_Stream1, DISABLE);
        DCMI_Cmd(DISABLE);

        if (Camera_xSemaphore != NULL)
        {
            vQueueDelete( Camera_xSemaphore );
            Camera_xSemaphore = NULL;
        }

        if (Camera_Task_Handle != NULL)
        {
            vTaskDelete(Camera_Task_Handle);
            Camera_Task_Handle = NULL;
        }

        CAMERA_SwitchPage(CameraCapturePage, CAMERA_MAIN_PAGE);
        CameraCapturePage = NULL;


        MOD_GetParam(CAMERA_SETTINGS_MEM, &camera_cfg.d32);
        camera_cfg.b.CameraBrighness = Camera_Brightness;
        MOD_SetParam(CAMERA_SETTINGS_MEM, &camera_cfg.d32);
        CAMERA_InitDone = 0;
    }
}

/**
* @brief  Set the capture sate to start
* @param  None
* @retval None
*/

static void goto_start_capture(void)
{
    uint8_t  pbuf[100];

    if(capture_status == CAPTURE_IDLE)
    {
        capture_status = CAPTURE_ONGOING;
        MOD_GetParam(CAMERA_SETTINGS_MEM, &camera_cfg.d32);

        DCMI_CaptureCmd(DISABLE);

        Camera_UsedStorage = CAMERA_DefaultPath[0];

        if ( Save_Image_To_File(CAMERA_DefaultPath, pbuf, \
                                camera_cfg.b.ImageFormat > 0 ? camera_cfg.b.ImageFormat - 1 : 0 ) != 0)
        {
            RefreshLabel(CameraCapturePage, 9, pbuf);
        }
        else
        {
            RefreshLabel(CameraCapturePage, 9, (uint8_t *)"Error : capture failed");
        }
        Camera_UsedStorage = 0xFF;
        capture_timer.Delay = 50;
        capture_timer.Enabled = 1;

    }
}


/**
* @brief  Handle Cemera background task
* @param  None
* @retval None
*/
static void CAMERA_Background (void)
{

    if(capture_timer.Enabled == 1)
    {
        if(--capture_timer.Delay == 0)
        {
            RefreshLabel(CameraCapturePage, 9, (uint8_t *)"                                    ");
            DCMI_CaptureCmd(ENABLE);
            capture_status = CAPTURE_IDLE;
            capture_timer.Enabled = 0;
        }
    }
}


/**
* @brief  Cemera main task
* @param  pvParameters : task parameter
* @retval None
*/
static void Camera_Task(void * pvParameters)
{
    static uint8_t error_flag = 0;

    while (1)
    {
        if (xSemaphoreTake(Camera_xSemaphore, 300) == pdTRUE)
        {
            if (CameraCapturePage != NULL)
            {
                if (CameraCapturePage->Page_ID == CAMERA_CAPTURE_PAGE)
                {
                    if(error_flag == 0)
                    {
                        portENTER_CRITICAL();
                        Show_Frame();
                        portEXIT_CRITICAL();
                    }
                    else
                    {
                        /*Discard wrong data */
                        error_flag = 0;
                    }

                    DMA_Cmd(DMA2_Stream1, ENABLE);
                    DCMI_CaptureCmd(ENABLE);
                }
            }
        }
        else if(DCMI_GetFlagStatus(DCMI_FLAG_OVFRI) == SET)
        {
            DCMI_ClearFlag(DCMI_FLAG_OVFRI);
            DCMI_CaptureCmd(ENABLE);
            DMA_Cmd(DMA2_Stream1, ENABLE);
            error_flag = 1;
        }
    }
}


/**
* @brief  Display the camera flow in the screen.
* @param  None
* @retval None
*/
static void Show_Frame(void)
{
    uint32_t i, j  = 0;
    uint8_t    *pBuff = (uint8_t *)(FSMC_SRAM_ADDRESS);// 0x68000000

    LCD_SetDisplayWindow(_X(60), _Y(80), 120, 160);
    LCD_WriteReg(R3, 0x1038);
    LCD_SetCursor(60, 80);
    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */

    for (i = 0 ; i < IMAGE_COLUMN_SIZE ; i += 2)
    {
        for (j = 0 ; j < IMAGE_LINE_SIZE  * 2 ; j += 4)
        {
            LCD_WriteRAM(pBuff[2 * i * IMAGE_LINE_SIZE + j + 1] << 8 | pBuff[2 * i * IMAGE_LINE_SIZE  + j]);
        }
    }

    /*

    // 镜像翻转RawData
    for ( i = 0 ; i < IMAGE_COLUMN_SIZE; i++)
    {
        for ( j= 0 ; j < 2 * IMAGE_LINE_SIZE; j++)
        {
            ImageBuffer.SrcData[j + (2 * IMAGE_LINE_SIZE * i)] = ImageBuffer.RawData[j + (2 * IMAGE_LINE_SIZE * ( IMAGE_COLUMN_SIZE- 1 - i))];
        }
    }

    // Convert RGB16 image to RGB24
    // SrcData的地址是0x68038400
    // DestData的地址是0x680a8c00
    RGB16toRGB24(ImageBuffer.DestData, ImageBuffer.SrcData, 1);

    // Gray = (R*30 + G*59 + B*11 + 50) / 100
    uint8_t    *pDestGray = (uint8_t *) ImageBuffer.DestData;
    uint8_t    *pSouceGray = pDestGray;

    for (int m = 0; m < 320 * 240; m++)
    {
        uint8_t bgr_b = (*pSouceGray);
        pSouceGray++;
        uint8_t bgr_g = (*pSouceGray++);
        pSouceGray++;
        uint8_t bgr_r = (*pSouceGray);
        pSouceGray++;

        *pDestGray = ((30 * bgr_r + 59 * bgr_g + 11 * bgr_b) * 1.0f / 100);
        pDestGray++;

    }
    */

    LCD_WindowModeDisable();
}

/**
* @brief  Increment the brightness
* @param  None
* @retval None
*/
static void Capture_Brightness_Inc(void)
{
    MOD_GetParam(CAMERA_SETTINGS_MEM, &camera_cfg.d32);

    if ( camera_cfg.b.ImageEffect <= 1)
    {
        if ( Camera_Brightness < 0x6F)
        {
            Camera_Brightness += 10;
        }
        Camera_SetBrighnessValue(Camera_Brightness);
    }
}

/**
* @brief  Decrement the brightness
* @param  None
* @retval None
*/
static void Capture_Brightness_Dec(void)
{
    MOD_GetParam(CAMERA_SETTINGS_MEM, &camera_cfg.d32);

    if ( camera_cfg.b.ImageEffect <= 1)
    {
        if ( Camera_Brightness > 10)
        {
            Camera_Brightness -= 10;
        }
        Camera_SetBrighnessValue(Camera_Brightness);
    }
}

/**
* @brief  Display the settings page
* @param  None
* @retval None
*/
static void goto_settings (void)
{
    CAMERA_SwitchPage(CameraMainPage, CAMERA_SETTINGS);
    CameraMainPage = NULL;
}

/**
* @brief  Display the viewer page
* @param  None
* @retval None
*/
static void goto_view(void)
{
    CAMERA_GetDefaultDirectory();
    IMAGE_CameraView(CameraMainPage, CAMERA_DefaultPath);
}

/**
* @brief  return to the module main menu from the settings page
* @param  None
* @retval None
*/
static void return_from_settings (void)
{

    MOD_GetParam(CAMERA_SETTINGS_MEM, &camera_cfg.d32);
    camera_cfg.b.CameraBrighness = Camera_Brightness;
    camera_cfg.b.ImageFormat     = GetComboOptionActive(CameraSettingsPage, 5);
    camera_cfg.b.ImageEffect     = GetComboOptionActive(CameraSettingsPage, 7);

    MOD_SetParam(CAMERA_SETTINGS_MEM, &camera_cfg.d32);

    CAMERA_SwitchPage(CameraSettingsPage, CAMERA_MAIN_PAGE);
    CameraSettingsPage = NULL;
}

/**
* @brief  Select Default folder
* @param  None
* @retval None
*/
static void Sel_folder (void)
{
    FILMGR_DirectEx(CameraSettingsPage, MODE_SEL_DIR, MASK_NONE, Update_folder);
}

/**
* @brief  Update Default folder callback
* @param  None
* @retval None
*/
static void Update_folder (void)
{
    if(( FILEMGR_CurrentDirectory[0] == USB_MEDIA_STORAGE) ||
       ( FILEMGR_CurrentDirectory[0] == MSD_MEDIA_STORAGE))
    {

        strncpy(((char *)CAMERA_DST_PATH), (char *)FILEMGR_CurrentDirectory, 128);
        strncpy((char *)CAMERA_DefaultPath, FILEMGR_CurrentDirectory, 128);

        if(strlen((char *)CAMERA_DefaultPath) > CAMERA_DST_PATH_SIZE)
        {
            CAMERA_DefaultPath[CAMERA_DST_PATH_SIZE - 2] = '.';
            CAMERA_DefaultPath[CAMERA_DST_PATH_SIZE - 1] = '.';
            CAMERA_DefaultPath[CAMERA_DST_PATH_SIZE]     =  0;
        }

        RefreshLabel(CameraSettingsPage, 3, (uint8_t *)CAMERA_DefaultPath);
    }
}
/**
* @brief  Set Default folder (saved in backup sram)
* @param  path : pointer to the Default folder
* @retval None
*/
static void Check_DefaultPath (char *path)
{
    if ((*((char *)(CAMERA_DST_PATH )) == '0') || (*((char *)(CAMERA_DST_PATH)) == '1'))
    {
        strncpy((char *)path, (char *)(CAMERA_DST_PATH), 128);
    }
    else
    {
        strcpy((char *)path, "0:");
    }
}

/**
* @brief  Draw the camera page backgound
* @param  None
* @retval None
*/
static void DrawCameraBackgound (void)
{
    LCD_SetTextColor(GL_Black);
    GL_LCD_FillRect(45, 280, 160, 240);
}


/**
* @brief  Initialize the camera module process
* @param  None
* @retval None
*/
static void InitCamera (void)
{
    if(CAMERA_InitDone == 0)
    {
        CAMERA_InitDone = 1;
        /* Create binary semaphore used for informing image task of image capture */
        if (Camera_xSemaphore == NULL)
        {
            vSemaphoreCreateBinary(Camera_xSemaphore);
            xSemaphoreTake(Camera_xSemaphore, 0);
        }

        if(Camera_Task_Handle == NULL)
        {
            xTaskCreate(Camera_Task,
                        (signed char const*)"CAMERA_P",
                        Camera_Task_TASK,
                        NULL,
                        Camera_Task_PRIO,
                        &Camera_Task_Handle);
        }

        MOD_GetParam(CAMERA_SETTINGS_MEM, &camera_cfg.d32);
        Camera_Brightness = camera_cfg.b.CameraBrighness;
        Camera_SetBrighnessValue((uint8_t)Camera_Brightness);

        /* Enable DMA transfer */
        DMA_Cmd(DMA2_Stream1, ENABLE);

        /* Enable DCMI interface */
        DCMI_Cmd(ENABLE);

        /* Start Image capture */
        DCMI_CaptureCmd(ENABLE);

        /* Apply effects */
        Apply_camera_native_effects (camera_cfg.b.ImageEffect);
    }
    else
    {
        xSemaphoreGive( Camera_xSemaphore );
    }
}

/**
* @brief  return to module main page after a medium disconnection event
* @param  Disconnect_source : SD or USB disk flash
* @retval None
*/
static void CAMERA_Media_changed (uint8_t disconnect_source)
{
    if(CameraCapturePage != NULL)
    {
        if (CameraCapturePage->Page_ID == CAMERA_CAPTURE_PAGE)
        {
            if(Camera_UsedStorage ==  disconnect_source)
            {
                CAMERA_SwitchPage(CameraCapturePage, PAGE_MENU);
                CameraCapturePage = NULL;
            }
        }
    }
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
