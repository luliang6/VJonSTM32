/**
  ******************************************************************************
  * @file    camera_api.c 
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   This file contains the routine needed to configure OV9655/OV2640 
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
#include "camera_api.h"
#include "dcmi_ov9655.h"
#include "dcmi_ov2640.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

extern xSemaphoreHandle        Camera_xSemaphore;
extern uint8_t  DMA_Interrupt_arbiter;
void DMA2_Stream1_IRQHandler(void);


Camera_TypeDef Camera;                 /* Camera devices */

static OV9655_IDTypeDef OV9655_Camera_ID;
static OV2640_IDTypeDef OV2640_Camera_ID;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Configures OV9655 or OV2640 Camera module mounted on STM324xG-EVAL board.
  * @param  ImageBuffer: Pointer to the camera configuration structure
  * @retval None
  */
void Camera_Config(ImageFormat_TypeDef ImageFormat)
{

  DMA_Interrupt_arbiter = 1;
  /* Initializes the DCMI interface (I2C and GPIO) used to configure the camera */
//   OV2640_HW_Init();
	OV9655_HW_Init();

  /* Read the OV9655/OV2640 Manufacturer identifier */
  OV9655_ReadID(&OV9655_Camera_ID);
//   OV2640_ReadID(&OV2640_Camera_ID);

  if(OV9655_Camera_ID.PID  == 0x96)
  {
    Camera = OV9655_CAMERA;
  }
  else if(OV2640_Camera_ID.PIDH  == 0x26)
  {
    Camera = OV2640_CAMERA;
    
  }


  if(Camera == OV9655_CAMERA)
  {
    switch (ImageFormat)
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
  else if(Camera == OV2640_CAMERA)
  {
    switch (ImageFormat)
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

/**
  * @brief  Configures the brightness of the camera module.
  * @param  brightness_value : brighness value
  * @retval None
  */
void  Camera_SetBrighnessValue(int8_t brightness_value)
{
  if (OV9655_Camera_ID.PID  == 0x96)
  {
    OV9655_BrightnessConfig(brightness_value);
  }
  else if (OV2640_Camera_ID.PIDH  == 0x26)
  {
    OV2640_BrightnessConfig(0x20 + brightness_value / 4);
  }
}

/*******************************************************************************
* Function Name  : Camera_IRQHandler
* Description    : This function handles DMA2_Stream1 interrupt.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Camera_IRQHandler(void)
{
  portBASE_TYPE CameraTaskWoken = pdFALSE;

  /* Transfer complete interrupt */
  if (DMA_GetFlagStatus(DMA2_Stream1, DMA_FLAG_TCIF1) != RESET)
  {
    /* Give the semaphore to wakeup camera blocking task */
    if( Camera_xSemaphore != NULL)
    {
      xSemaphoreGiveFromISR(Camera_xSemaphore, &CameraTaskWoken);

      /* Switch tasks if necessary */
      if (CameraTaskWoken != pdFALSE)
      {
        portEND_SWITCHING_ISR(CameraTaskWoken);
      }
    }
    /* Clear the Interrupt flag */
    DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_TCIF1);
  }

  /* Half Transfer complete interrupt */
  else if (DMA_GetFlagStatus(DMA2_Stream1, DMA_FLAG_HTIF1) != RESET)
  {
    /* Clear the Interrupt flag */
    DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_HTIF1);
  }

  /* FIFO Error interrupt */
  else if (DMA_GetFlagStatus(DMA2_Stream1, DMA_FLAG_FEIF1) != RESET)
  {

    /* Clear the Interrupt flag */
    DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_FEIF1);
  }

  /* Transfer error interrupt */
  else if (DMA_GetFlagStatus(DMA2_Stream1, DMA_FLAG_TEIF1) != RESET)
  {
    DCMI_ClearFlag(DCMI_FLAG_OVFRI);

    /* Clear the Interrupt flag */
    DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_TEIF1);
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
