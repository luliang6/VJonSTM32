/**
  ******************************************************************************
  * @file    mod_core.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   This file contains the functions prototypes for the module manager.
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
#ifndef __MODULE_CORE_H
#define __MODULE_CORE_H

#ifdef __cplusplus
extern "C"
{
#endif

  /* Includes ------------------------------------------------------------------*/
#include "graphicObject.h"
#include "gl_res.h"
#include "usbh_core.h"
#include "usbh_usr.h"
#include "usbh_msc_core.h"
#include "global_includes.h"


  /* Exported constants --------------------------------------------------------*/
#define MAX_MODULES_NUM     9
#define MAX_GROUP_NUM       3
#define RTCClockSourceLSE

  /* Exported types ------------------------------------------------------------*/

  typedef void (*ClickEventHandler)(void);

  typedef struct _MOD_InitTypeDef
  {
    uint16_t id;
    uint16_t rev;
    uint8_t  *name;
    uint8_t  *icon;
    void     (*startup)(void);
    void     (*background)(void);
    void     (*cleanup)(uint8_t );
  }
  MOD_InitTypeDef;


  typedef struct _Group_InitTypeDef
  {
    MOD_InitTypeDef  *module[MAX_MODULES_NUM];
    uint8_t  counter;
  }
  GROUP_InitTypeDef;
  /* Exported macro ------------------------------------------------------------*/
  /* Exported functions ------------------------------------------------------- */
  uint8_t MOD_AddModule(MOD_InitTypeDef *module, uint8_t group);
  void MOD_SetParam(uint16_t ID , uint32_t *cfg);
  void MOD_GetParam(uint16_t ID , uint32_t *cfg);
  void MOD_NullFunc(void);
  void MOD_PreInit(void);
  void MOD_LibInit(void);
  void MOD_fClickHandler(void);
  void MOD_UnRegisterClickHandler(void);  
  void MOD_RegisterClickHandler(void (*pEventHandler)(void), GL_Coordinate_TypeDef gRect);
  void MOD_HandleModulesBackground (void)  ;
  void MOD_HandleModulesClanup (uint8_t disconnect_source);
  extern uint8_t MOD_counter;
  extern GROUP_InitTypeDef      MOD_table[];

#ifdef __cplusplus
}
#endif

#endif /* __MODULE_CORE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
