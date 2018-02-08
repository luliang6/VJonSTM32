/**
  ******************************************************************************
  * @file    app_image.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   This file contains the image processing mechanism
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
#include "str_utils.h"
#include "app_image.h"
#include "global_includes.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

uint16_t  IMAGE_display_area_width;
uint16_t  IMAGE_display_area_height; 
uint8_t   IMAGE_IsDefaultSize; 

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Decode, resize and display an image
  * @param  file: file pointer
  * @param  file_name: file name pointer
  * @param  effect: image effect
  * @retval Status
  */

int32_t Image_Decode(FIL *file, char *file_name, uint8_t effect)
{
  int32_t ret = 0;

  if (check_filename_ext(file_name, "bmp") || check_filename_ext(file_name, "BMP"))
  {
    ret = BMP_Decode(file, effect);
  }
  else if (check_filename_ext(file_name, "jpg") || check_filename_ext(file_name, "JPG"))
  {
    ret =  JPEG_Decode (file, effect); 
  }

  return ret;
}

/**
  * @brief  Restore default display size
  * @param  None
  * @retval None
  */
void Image_SetDefaultSize(void)
{
  IMAGE_display_area_width  = 240;
  IMAGE_display_area_height = 160;
  IMAGE_IsDefaultSize = 1;
}


/**
  * @brief  Display the image in the whole screen
  * @param  None
  * @retval None
  */
void Image_Enlarge(void)
{
  IMAGE_display_area_width  = 320;
  IMAGE_display_area_height = 240;
  IMAGE_IsDefaultSize = 0;
}

/**
  * @brief  Display the image in the whole screen
  * @param  None
  * @retval None
  */
uint8_t  Image_IsSmall(void)
{
   return IMAGE_IsDefaultSize;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
