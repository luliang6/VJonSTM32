/**
  ******************************************************************************
  * @file    mod_filemgr.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Header for mod_filemgr.c module
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
#ifndef __MOD_FILEMGR_H
#define __MOD_FILEMGR_H

/* Includes ------------------------------------------------------------------*/
#include "global_includes.h"
#include "mod_core.h"
#include "file_utils.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define MOD_FILEMGR_VER            1
#define MOD_FILEMGR_UID            0x90


#define MODE_NONE                  0
#define MODE_SEL_DIR               1
#define MODE_SEL_FIL               2
#define MODE_REM_FIL               3

#define MASK_NONE                  0   
#define MASK_WAV                   1
#define MASK_MP3                   2
#define MASK_WMA                   4
#define MASK_BMP                   8
#define MASK_JPG                   0x10


#define NO_MEDIA_STORAGE           0xFF
#define USB_MEDIA_STORAGE          '0'
#define MSD_MEDIA_STORAGE          '1'

/* Exported macro ------------------------------------------------------------*/
/* Exported variables ------------------------------------------------------- */
extern MOD_InitTypeDef  mod_filemgr ;
/* Exported functions ------------------------------------------------------- */
extern void FILMGR_DirectEx(GL_Page_TypeDef* pParent, 
                            uint8_t sel_mode, 
                            uint16_t filter, 
                            void (*pEventHandler)(void));

extern void FILE_MGR_RefreshScroll (void);


#endif /* __MOD_FILEMGR_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
