/**
  ******************************************************************************
  * @file    ipcam_api.c 
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   This file contains the routinue needed to configure OV9655/OV2640 
  *          Camera modules.
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
#include "global_includes.h"
#include "dcmi_ov9655.h"
#include "dcmi_ov2640.h"
#include "mod_ethernet_config.h"
#include "camera_api.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static __IO uint32_t TimingDelay;
extern IPCAM_ImageBuffer_TypeDef IPCAM_ImageBuffer;
extern uint8_t  DMA_Interrupt_arbiter;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Configures OV9655 or OV2640 Camera module mounted on STM324xG-EVAL board.
  * @param  None
  * @retval None
  */
void eth_Camera_Config(void)
{
  DMA_Interrupt_arbiter = 2;
	
  if(IPCAM_ImageBuffer.Camera == OV9655_CAMERA)
  {
    switch (IPCAM_ImageBuffer.ImageFormat)
    {
      case BMP_QQVGA:
      {
        /* Configure the OV9655 camera and set the QQVGA mode */
        OV9655_Init(BMP_QQVGA);
        OV9655_QQVGAConfig();
        break;
      }
      case BMP_QVGA:
      {
        /* Configure the OV9655 camera and set set the QVGA mode */
        OV9655_Init(BMP_QVGA);
        OV9655_QVGAConfig();
        break;
      }
      default:
      {
        /* Configure the OV9655 camera and set the QQVGA mode */
        OV9655_Init(BMP_QQVGA);
        OV9655_QQVGAConfig();
        break;
      }
    }
  }
  else if(IPCAM_ImageBuffer.Camera == OV2640_CAMERA)
  {
    switch (IPCAM_ImageBuffer.ImageFormat)
    {
      case BMP_QQVGA:
      {
        /* Configure the OV2640 camera and set the QQVGA mode */
        OV2640_Init(BMP_QQVGA);
        OV2640_QQVGAConfig();
        break;
      }
      case BMP_QVGA:
      {
        /* Configure the OV2640 camera and set the QQVGA mode */
        OV2640_Init(BMP_QVGA);
        OV2640_QVGAConfig();
        break;
      }
      default:
      {
        /* Configure the OV2640 camera and set the QQVGA mode */
        OV2640_Init(BMP_QQVGA);
        OV2640_QQVGAConfig();
        break; 
      }
    }
  }
}

  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
