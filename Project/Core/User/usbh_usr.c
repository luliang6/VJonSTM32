/**
  ******************************************************************************
  * @file    usbh_usr.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   This file includes the usb host library user callbacks
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
#include "mod_console.h"
#include "mod_filemgr.h"
#include "mod_audio.h"
#include "usbh_usr.h"
#include "ff.h"
#include "usbh_msc_core.h"
#include "usbh_msc_scsi.h"
#include "usbh_msc_bot.h"
#include "ff.h"

#ifdef FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */

USBH_Usr_cb_TypeDef USBH_USR_cb =
  {
    USBH_USR_Init,
    USBH_USR_DeInit,
    USBH_USR_DeviceAttached,
    USBH_USR_ResetDevice,
    USBH_USR_DeviceDisconnected,
    USBH_USR_OverCurrentDetected,
    USBH_USR_DeviceSpeedDetected,
    USBH_USR_Device_DescAvailable,
    USBH_USR_DeviceAddressAssigned,
    USBH_USR_Configuration_DescAvailable,
    USBH_USR_Manufacturer_String,
    USBH_USR_Product_String,
    USBH_USR_SerialNum_String,
    USBH_USR_EnumerationDone,
    USBH_USR_UserInput,
    USBH_USR_MSC_Application,
    USBH_USR_DeviceNotSupported,
    USBH_USR_UnrecoveredError
  };

FATFS USBH_fatfs;
uint8_t USB_Host_Application_Ready;

/*--------------- LCD Messages ---------------*/
const uint8_t MSG_HOST_INIT[]        = "[USB] Host Library Initialized";
const uint8_t MSG_DEV_ATTACHED[]     = "[USB] Device Attached ";
const uint8_t MSG_DEV_DISCONNECTED[] = "[USB] Device Disconnected";
const uint8_t MSG_DEV_ENUMERATED[]   = "[USB] Enumeration completed ";
const uint8_t MSG_DEV_HIGHSPEED[]    = "[USB] High speed device detected";
const uint8_t MSG_DEV_FULLSPEED[]    = "[USB] Full speed device detected";
const uint8_t MSG_DEV_LOWSPEED[]     = "[USB] Low speed device detected";
const uint8_t MSG_DEV_ERROR[]        = "[USB] Device fault ";

const uint8_t MSG_MSC_CLASS[]      = "[USB] Mass storage device connected";
const uint8_t MSG_HID_CLASS[]      = "[USB] HID device connected";
const uint8_t MSG_DISK_SIZE[]      = "[USB] Size of the disk in MBytes: ";
const uint8_t MSG_LUN[]            = "[USB] LUN Available in the device:";
const uint8_t MSG_ROOT_CONT[]      = "[USB] Exploring disk flash ...";
const uint8_t MSG_WR_PROTECT[]      = "[USB] The disk is write protected";
const uint8_t MSG_UNREC_ERROR[]     = "[USB] UNRECOVERED ERROR STATE";


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Add the host lib initialization message to the console
  * @param  None
  * @retval None
  */
void USBH_USR_Init(void)
{
  CONSOLE_LOG((uint8_t *)"[USB] Host Initialized." );
  USB_Host_Application_Ready = 0;
}

/**
  * @brief  Add the device attachement message to the console
  * @param  None
  * @retval None
  */
void USBH_USR_DeviceAttached(void)
{
  CONSOLE_LOG((void *)MSG_DEV_ATTACHED);
}


/**
  * @brief  Add the unrecovered error message to the console
  * @param  None
  * @retval None
  */
void USBH_USR_UnrecoveredError (void)
{

  /* Set default screen color*/
  CONSOLE_LOG((void *)MSG_UNREC_ERROR);
}


/**
  * @brief Add the device disconnection message to the console and free 
  *        USB associated resources
  * @param  None
  * @retval None
  */
void USBH_USR_DeviceDisconnected (void)
{
  CONSOLE_LOG((void *)MSG_DEV_DISCONNECTED);
  
  MOD_HandleModulesClanup(USB_MEDIA_STORAGE);
  USB_Host_Application_Ready = 0;
  f_mount(0, NULL);
}
/**
  * @brief  callback of the device reset event
  * @param  None
  * @retval None
  */
void USBH_USR_ResetDevice(void)
{
  /* callback for USB-Reset */
}


/**
  * @brief  Add the device speed message to the console
  * @param  Device speed
  * @retval None
  */
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed)
{
  if (DeviceSpeed == HPRT0_PRTSPD_HIGH_SPEED)
  {
    CONSOLE_LOG((void *)MSG_DEV_HIGHSPEED);
  }
  else if (DeviceSpeed == HPRT0_PRTSPD_FULL_SPEED)
  {
    CONSOLE_LOG((void *)MSG_DEV_FULLSPEED);
  }
  else if (DeviceSpeed == HPRT0_PRTSPD_LOW_SPEED)
  {
    CONSOLE_LOG((void *)MSG_DEV_LOWSPEED);
  }
  else
  {
    CONSOLE_LOG((void *)MSG_DEV_ERROR);
  }
}

/**
  * @brief  Add the USB device vendor and MFC Ids to the console
  * @param  device descriptor
  * @retval None
  */
void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{
  USBH_DevDesc_TypeDef *hs = DeviceDesc;
  char temp[40];

  sprintf (temp, "[USB] VID : %04Xh" , (int)((*hs).idVendor));
  CONSOLE_LOG((uint8_t *)temp);
  sprintf (temp, "[USB] PID : %04Xh" , (int)((*hs).idProduct));
  CONSOLE_LOG((uint8_t *)temp);
}

/**
  * @brief  Device addressed event callbacak
  * @param  None
  * @retval None
  */
void USBH_USR_DeviceAddressAssigned(void)
{
}

/**
  * @brief  Add the device class description to the console
  * @param  Configuration descriptor
  * @retval None
  */
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
    USBH_InterfaceDesc_TypeDef *itfDesc,
    USBH_EpDesc_TypeDef *epDesc)
{
  USBH_InterfaceDesc_TypeDef *id;

  id = itfDesc;

  if ((*id).bInterfaceClass  == 0x08)
  {
    CONSOLE_LOG((void *)MSG_MSC_CLASS);
  }
  else if ((*id).bInterfaceClass  == 0x03)
  {
    CONSOLE_LOG((void *)MSG_HID_CLASS);
  }
}

/**
  * @brief  Add the MFC String to the console
  * @param  Manufacturer String
  * @retval None
  */
void USBH_USR_Manufacturer_String(void *ManufacturerString)
{
  char temp[40];
  sprintf (temp, "[USB] Manufacturer : %s", (char *)ManufacturerString);
  CONSOLE_LOG((uint8_t *)temp);
}

/**
  * @brief  Add the Product String to the console
  * @param  Product String
  * @retval None
  */
void USBH_USR_Product_String(void *ProductString)
{

  char temp[40];
  sprintf (temp, "[USB] Product : %s", (char *)ProductString);
  CONSOLE_LOG((uint8_t *)temp);
}

/**
  * @brief  Add the Srial Number String to the console
  * @param  SerialNum_String
  * @retval None
  */
void USBH_USR_SerialNum_String(void *SerialNumString)
{

  char temp[40];
  sprintf (temp, "[USB] S\\N : %s", (char *)SerialNumString);
  CONSOLE_LOG((uint8_t *)temp);
}



/**
  * @brief  Enumeration complete event callback
  * @param  None
  * @retval None
  */
void USBH_USR_EnumerationDone(void)
{

  /* Enumeration complete */
  CONSOLE_LOG((void *)MSG_DEV_ENUMERATED);
}

/**
  * @brief  Device is not supported callback
  * @param  None
  * @retval None
  */
void USBH_USR_DeviceNotSupported(void)
{
  CONSOLE_LOG((uint8_t *)"[USB] Device not supported.");
}

/**
  * @brief  User Action for application state entry callback
  * @param  None
  * @retval USBH_USR_Status : User response for key button
  */
USBH_USR_Status USBH_USR_UserInput(void)
{
  //return USBH_USR_NO_RESP;
  return USBH_USR_RESP_OK;
}

/**
  * @brief  Over Current Detected on VBUS
  * @param  None
  * @retval Staus
  */
void USBH_USR_OverCurrentDetected (void)
{
  CONSOLE_LOG((uint8_t *)"[USB] Overcurrent detected.");
}


/**
  * @brief  Mass storage application main handler
  * @param  None
  * @retval Staus
  */
int USBH_USR_MSC_Application(void)
{

  char temp[40];

  if(USB_Host_Application_Ready == 0)
  {
    /* Initializes the File System*/
    if ( f_mount( 0, &USBH_fatfs ) != FR_OK )
    {
      /* efs initialisation fails*/
      CONSOLE_LOG((uint8_t *)"[FS] Cannot initialize FS on drive 0.");
      USB_Host_Application_Ready = 1;
      return -1;
    }
    CONSOLE_LOG((uint8_t *)"[FS] FS on drive 0 initialized.");
    sprintf (temp, "[FS] USB Disk capacity:%d MB", (int)((USBH_MSC_Param.MSCapacity * \
      USBH_MSC_Param.MSPageLength) / 1024 / 1024));
    CONSOLE_LOG((uint8_t *)temp);

    USB_Host_Application_Ready = 1;
  }
  return(0);
}
/**
  * @brief  De-init User state and associated variables
  * @param  None
  * @retval None
  */
void USBH_USR_DeInit(void)
{
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
