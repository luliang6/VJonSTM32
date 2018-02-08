/**
  ******************************************************************************
  * @file    mod_ethernet_config.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Module Ethernet configuration file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MOD_ETHERNET_CONFIG_H
#define __MOD_ETHERNET_CONFIG_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "global_includes.h"
#include "camera_api.h"
#include "app_ethernet.h"

/* lwip includes */
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"
#include "lwip/tcpip.h"

/* Scheduler includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdio.h>

/* Exported types ------------------------------------------------------------*/
typedef union _ETHERNET_SETTINGS_TypeDef 
{
  uint32_t d32;
  struct
  {
    uint32_t DHCPEnable : 1;
    uint32_t SelectedImageFormat : 3;
    uint32_t Reserved : 28;
  }b;
}
ETHERNET_SETTINGS_TypeDef;

typedef struct _ETHERNET_SettingsTypeDef
{
  __IO uint32_t BackgroundEnabled;
  __IO uint32_t DistantControlEnabled;
  __IO uint32_t WebserverEnabled;  
  __IO uint32_t InitDone;
  __IO uint32_t DisableBackgroundLater;
  __IO uint32_t DisableDControlLater;
}ETHERNET_SettingsTypeDef;

/* Exported constants --------------------------------------------------------*/
/*--------------- Tasks Priority -------------*/
#define DHCP_THREAD_PRIO                     (configMAX_PRIORITIES - 6)
#define DHCP_THREAD_STACK_SIZE               (configMINIMAL_STACK_SIZE)
#define HTTP_THREAD_PRIO                     (configMAX_PRIORITIES - 5)
#define HTTP_THREAD_STACK_SIZE               (1024)
#define ETH_GUI_THREAD_PRIO                  (configMAX_PRIORITIES - 4)
#define ETH_GUI_THREAD_STACK_SIZE            (configMINIMAL_STACK_SIZE)
#define ETH_TIME_WAITING_FOR_INPUT           ((portTickType)100)

/* MAC ADDRESS*/
#define MAC_ADDR0       2
#define MAC_ADDR1       0
#define MAC_ADDR2       0
#define MAC_ADDR3       0
#define MAC_ADDR4       0
#define MAC_ADDR5       0
 
/*Static IP ADDRESS*/
#define IP_ADDR0        192
#define IP_ADDR1        168
#define IP_ADDR2        1
#define IP_ADDR3        108
   
/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0        192
#define GW_ADDR1        168
#define GW_ADDR2        1
#define GW_ADDR3        1

/* MII and RMII mode selection, for STM32-EVAL Board RevB *********************/
//#define RMII_MODE  // User have to provide the 50 MHz clock by soldering a 50 MHz
                     // oscillator (ref SM7745HEV-50.0M or equivalent) on the U3
                     // footprint located under CN3 and also removing jumper on JP5. 
                     // This oscillator is not provided with the board. 
                     // For more details, please refer to STM3220G-EVAL evaluation
                     // board User manual (UM1057) or STM3240G-EVAL evaluation
                     // board User manual (UM1461).
                                     
#define MII_MODE

/* Uncomment the define below to clock the PHY from external 25MHz crystal (only for MII mode) */
#ifdef 	MII_MODE
 #define PHY_CLOCK_MCO
#endif

/* STM322xG-EVAL jumpers setting
    +==========================================================================================+
    +  Jumper |       MII mode configuration            |      RMII mode configuration         +
    +==========================================================================================+
    +  JP5    | 2-3 provide 25MHz clock by MCO(PA8)     |             Not fitted               +
    +         | 1-2 provide 25MHz clock by ext. Crystal |                                      +
    + -----------------------------------------------------------------------------------------+
    +  JP6    |                 2-3                     |                 1-2                  +
    + -----------------------------------------------------------------------------------------+
    +  JP8    |                 Open                    |                Close                 +
    +==========================================================================================+
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Time_Update(void);
void Delay(uint32_t nCount);
extern const uint8_t ethernet_webserver_icon[];
extern const uint8_t ethernet_settings_icon[];
extern const uint8_t ethernet_remotecontrol_icon[];
extern const uint8_t ethernet_conn_icon[];
extern const uint8_t ethernet_disconn_icon[];
extern const uint8_t ethernet_dhcp_icon[];

#ifdef __cplusplus
}
#endif

#endif /* __MOD_ETHERNET_CONFIG_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
