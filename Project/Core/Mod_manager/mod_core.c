/**
  ******************************************************************************
  * @file    mod_core.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief    module manager implementation
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
#include "mod_console.h"
#include "mod_filemgr.h"
#include "mem_utils.h"
#include "mod_audio.h"
#include "mod_core.h"
#include "app_rtc.h"
#include "mod_console.h"
#include "app_console.h"
#include "app_console.h"
#include "bsp.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GROUP_InitTypeDef              MOD_table[MAX_GROUP_NUM];
USB_OTG_CORE_HANDLE            USB_OTG_Core;
USBH_HOST                      USB_Host;
FATFS                          MSD_fatfs;
extern __IO uint8_t            touch_done;
extern __IO uint32_t           u32_TSXCoordinate;
extern __IO uint32_t           u32_TSYCoordinate;
ClickEventHandler              MOD_ClickHandler = NULL;
GL_Coordinate_TypeDef          MOD_ClickZone;
/* Private function prototypes -----------------------------------------------*/
static int8_t  SDStorage_StateChanged (void);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialize the target hardware and the console
  * @param  None
  * @retval None
  */
void MOD_PreInit(void)
{

  memset(MOD_table, 0, sizeof(GROUP_InitTypeDef));

  /* Init Console to save events log*/
  CONSOLE_Init();

  CONSOLE_LOG((uint8_t *)"[SYSTEM] Module Manager running....");

  /* Init Board specific hardware */
  BSP_Init();
  
  /*Init heap memory (located in SRAM)*/ 
  mem_pool_init();
}

/**
  * @brief  Initialize the middleware libraries and stacks
  * @param  None
  * @retval None
  */
void MOD_LibInit(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  MOD_GetParam(GLOBAL_SETTINGS_MEM , &Global_Config.d32);

  /* Force settings change to apply them */
  Global_Config.b.Configuration_Changed = 1;

  /* Starting USB Init. Process */
  GL_State_Message((uint8_t *)"USB Host Starting.  ");

  /*Init USB Host */
  USBH_Init(&USB_OTG_Core,
            USB_OTG_FS_CORE_ID,
            &USB_Host,
            &USBH_MSC_cb,
            &USBH_USR_cb);

  GL_State_Message((uint8_t *)"USB Host Started.");

  GL_State_Message((uint8_t *)"RTC and backup Starting.  ");
  /* Init RTC and Backup */
  if ( RTC_Configuration() == 0)
  {
    GL_State_Message((uint8_t *)"RTC and backup Started.");
    CONSOLE_LOG((uint8_t *)"[SYSTEM] RTC and backup Started.");
  }
  else
  {
    GL_State_Message((uint8_t *)"ERR : RTC could not be started.");
    CONSOLE_LOG((uint8_t *)"[ERR] RTC start-up FAILED .");
  }

  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  SD_Init();

  if ( f_mount( 1, &MSD_fatfs ) != FR_OK )
  {
    /* fatfs initialisation fails*/
    CONSOLE_LOG((uint8_t *)"[FS] Cannot initialize FS on drive 1.");
  }
  else
  {
    CONSOLE_LOG((uint8_t *)"[FS] FS on drive 1 initialized.");
  }
}

/**
  * @brief  Add a module in the module's list
  * @param  module: module structure pointer
  * @param  group: group that the module belong to  
  * @retval Status
  */
uint8_t  MOD_AddModule(MOD_InitTypeDef *module, uint8_t group)
{
  uint8_t idx = 0;
  char temp[40];

  /* check for id unicity */
  for ( ; idx < MOD_table[group].counter ; idx ++)
  {
    if (MOD_table[group].module[idx]->id == module->id)
    {
      sprintf(temp, "ERROR : %s module cannot be loaded", MOD_table[group].module[idx]->name);
      CONSOLE_LOG((uint8_t *)temp);
      return 1;
    }
  }
  MOD_table[group].module[MOD_table[group].counter] = module;
  MOD_table[group].counter ++;
  sprintf(temp, "[SYSTEM] %s module v%d.%d.%d loaded.",
          MOD_table[group].module[idx]->name,
          HIBYTE(MOD_table[group].module[idx]->rev) , LOBYTE(MOD_table[group].module[idx]->rev) >> 4, MOD_table[group].module[idx]->rev & 0xF );

  CONSOLE_LOG((uint8_t *)temp);
  return 0;
}

/**
  * @brief  Retreive parameters from the backup memory
  * @param  mem_base: parameters memory address in the backup sram
  * @param  cfg: configuration parameters structure 
  * @retval None
  */
void MOD_GetParam(uint16_t mem_base , uint32_t *cfg)
{
  if ( RTC_Error == 0)
  {
    *cfg = RTC_ReadBackupRegister(mem_base);
  }
  else
  {
    *cfg = 0;
  }
}

/**
  * @brief  Save parameters in the backup memory
  * @param  mem_base: parameters memory address in the backup sram
  * @param  cfg: configuration parameters structure 
  * @retval None
  */
void MOD_SetParam(uint16_t mem_base , uint32_t *cfg)
{
  if ( RTC_Error == 0)
  {
    RTC_WriteBackupRegister(mem_base, *cfg);
  }
}

/**
  * @brief  Error callback function
  * @param  None
  * @retval None
  */
void vApplicationMallocFailedHook( void )
{
  char  temp[128];
  GL_Clear(Black);
  GL_SetTextColor(GL_White);
  GL_SetFont(GL_FONT_SMALL);
  sprintf(temp, "Actual heap size : %d Bytes" , (int)xPortGetFreeHeapSize() );
  GL_DisplayAdjStringLine(214, 310,  (uint8_t *)"[Memory Error]", GL_TRUE);
  GL_DisplayAdjStringLine(227, 310,  (uint8_t *)temp, GL_TRUE);
  while (1)
  {}
}

/**
  * @brief  "Do nothing" function
  * @param  None
  * @retval None
  */
void MOD_NullFunc(void)
{}

/**
  * @brief  Activate the Click event in a predefined area and associate an event
  * @param  pEventHandler : Click event
  * @param  gRect :area coordinates
  * @retval None
*/
void MOD_RegisterClickHandler(void (*pEventHandler)(void), GL_Coordinate_TypeDef gRect)
{
  MOD_ClickHandler = pEventHandler;
  MOD_ClickZone.MaxX = gRect.MaxX ;
  MOD_ClickZone.MinX = gRect.MinX ;
  MOD_ClickZone.MaxY = gRect.MaxY;
  MOD_ClickZone.MinY = gRect.MinY;
}

/**
  * @brief  De-activate the Click event in a predefined area
  * @param  None
  * @retval None
*/
void MOD_UnRegisterClickHandler(void)
{
  MOD_ClickHandler = NULL;
}

/**
  * @brief  Process the Click event handler
  * @param  None
  * @retval None
*/
void MOD_fClickHandler(void)
{
  if((touch_done)&& (u32_TSXCoordinate >  MOD_ClickZone.MinX) && 
                    (u32_TSXCoordinate <  MOD_ClickZone.MaxX) && 
                    (u32_TSYCoordinate >  MOD_ClickZone.MinY) && 
                    (u32_TSYCoordinate <  MOD_ClickZone.MaxY ))
  {
    /*Prevent click event when a messages is displayed */
    if((MOD_ClickHandler != NULL) &&
       (PagesList[0]->Page_ID != 0xFFFE)&&
       (PagesList[1]->Page_ID != 0xFFFE)&&
       (PagesList[2]->Page_ID != 0xFFFE))
    {
      MOD_ClickHandler();
    }
  }
}

/**
  * @brief  Handle Modules Background processes in the main task
  * @param  None
  * @retval None
*/
void MOD_HandleModulesBackground (void)
{
  uint32_t idx = 0, group = 0;  
  
   
  for (group = 0; group < MAX_GROUP_NUM; group ++)
  {
    for (idx = 0 ; idx < MOD_table[group].counter ; idx ++)
    {
      if (MOD_table[group].module[idx]->background != NULL)
      {
        MOD_table[group].module[idx]->background();
      }
    }
  }

  if((USB_Host_Application_Ready == 0) || (HCD_IsDeviceConnected(&USB_OTG_Core) == 0))
  {
    USBH_Process(&USB_OTG_Core, &USB_Host);   
  }
  
  if(SDStorage_StateChanged() < 0)
  {
      MOD_HandleModulesClanup(MSD_MEDIA_STORAGE);
  }
}

/**
  * @brief  Handle Modules Cleanup processes in the USB disconnect task
  * @param  disconnect_source : source of the disconnect event (SD or USB)
  * @retval None
*/
void MOD_HandleModulesClanup (uint8_t disconnect_source)
{
  uint32_t idx = 0, group = 0;

  
  MESSAGES_CloseEx ();
          
  for (group = 0; group < MAX_GROUP_NUM; group ++)
  {
    for (idx = 0 ; idx < MOD_table[group].counter ; idx ++)
    {
      if (MOD_table[group].module[idx]->cleanup != NULL)
      {
        MOD_table[group].module[idx]->cleanup(disconnect_source);
      }
    }
  }
}



/**
  * @brief  check whether the sd medium is ready/connected
  * @param  None
  * @retval Status
  */

static int8_t  SDStorage_StateChanged (void)
{
  SDTransferState status = SD_TRANSFER_OK;
  static SDTransferState prev_status = SD_TRANSFER_OK;

  if((status = SD_GetStatus()) != SD_TRANSFER_OK)
  {
    SD_Init();
  }

  if (prev_status != status)
  {
    prev_status = status;

    if(status != SD_TRANSFER_OK)
    {
      return -1;
    }
    else
    {
      return 0;
    }
  }

  return 0;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
