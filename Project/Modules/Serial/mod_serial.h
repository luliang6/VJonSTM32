/**
  ******************************************************************************
  * @file    mod_serial.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Header for mod_serial.c module
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
#ifndef __MOD_SERIAL_H
#define __MOD_SERIAL_H

/* Includes ------------------------------------------------------------------*/
#include "global_includes.h"

/* Exported constants --------------------------------------------------------*/
#define MOD_SERIAL_VER             1
#define MOD_SERIAL_UID             0xC0
#define SERIAL_SETTINGS_MEM        15

/* Exported macro ------------------------------------------------------------*/
/* Exported variables ------------------------------------------------------- */
extern MOD_InitTypeDef  mod_serial ;

/* Exported types ------------------------------------------------------------*/
typedef union _SERIAL_SETTINGS_TypeDef 
{
  uint32_t d32;
  struct
  {
    uint32_t BaudRate : 3;
    uint32_t DataLength : 2;
    uint32_t StopBits : 3;
    uint32_t Parity : 2;
    uint32_t FlowControl : 3;
    uint32_t Reserved : 19;
  }b;
}
SERIAL_SETTINGS_TypeDef;

/* Exported constants --------------------------------------------------------*/
#define MAXBUFFERSIZE  75 /* 25x3 : three lines */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void charMaj_function(void);
void charA_function(void);
void charB_function(void);
void charC_function(void);
void charD_function(void);
void charE_function(void);
void charF_function(void);
void charG_function(void);
void charH_function(void);
void charI_function(void);
void charJ_function(void);
void charK_function(void);
void charL_function(void);
void charM_function(void);
void charN_function(void);
void charO_function(void);
void charP_function(void);
void charQ_function(void);
void charR_function(void);
void charS_function(void);
void charT_function(void);
void charU_function(void);
void charV_function(void);
void charW_function(void);
void charX_function(void);
void charY_function(void);
void charZ_function(void);
void charErase_function(void);
void charInterComma_function(void);
void charPntSemicolon_function(void);
void charSpace_function(void);
void char123_function(void);
void send_function(void);
void char0_function(void);
void char1_function(void);
void char2_function(void);
void char3_function(void);
void char4_function(void);
void char5_function(void);
void char6_function(void);
void char7_function(void);
void char8_function(void);
void char9_function(void);
void charSlash_function(void);
void charAsterisk_function(void);
void charLeftParen_function(void);
void charRightParen_function(void);
void charDollar_function(void);
void charAmpersand_function(void);
void charAt_function(void);
void charHash_function(void);
void charLeftSquare_function(void);
void charRightSquare_function(void);
void charPlus_function(void);
void charMinus_function(void);
void charPoint_function(void);
void charComma_function(void);
void charColon_function(void);
void charSemiColon_function(void);
void charInter_function(void);
void charExcl_function(void);
void charPercent_function(void);
void charEqual_function(void);
void charABC_function(void);
void DisplayMsg(uint32_t Index);
void DisplayErrorMsg(void);

#endif /* __MOD_SERIAL_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
