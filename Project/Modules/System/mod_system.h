/**
  ******************************************************************************
  * @file    mod_system.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Header for mod_system.c module
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
#ifndef __MOD_SYSTEM_H
#define __MOD_SYSTEM_H

/* Includes ------------------------------------------------------------------*/
#include "global_includes.h"
#include "mod_core.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define MOD_SYSTEM_VER           1
#define MOD_SYSTEM_UID           0x00
#define GLOBAL_SETTINGS_MEM      0
/* Exported macro ------------------------------------------------------------*/
/* Exported variables ------------------------------------------------------- */
extern MOD_InitTypeDef  mod_system ;
extern SYSTEM_CONFIG_TypeDef  Global_Config;
extern uint8_t  BL_Enabled;

/* Exported functions ------------------------------------------------------- */
void SYSTEM_JumpToBootloader (void);
void SYSTEM_RefreshSetting (void);
#endif /* __MOD_SYSTEM_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
