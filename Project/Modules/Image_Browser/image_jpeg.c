/**
  ******************************************************************************
  * @file    image_jpeg.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   This file contains the methods to process images in JPEG format
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
#include "image_jpeg.h"
#include "mem_utils.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define OUTPUT_BUFFER       (JPEG_MAX_DISPLAY_WIDTH * 3) 
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
uint16_t  pos_x = 0, pos_y = 0;
OutParams_Typedef  out_params;
static uint8_t image_effect=0;
extern uint16_t  IMAGE_display_area_width;
extern uint16_t  IMAGE_display_area_height; 
/* Private function prototypes -----------------------------------------------*/
static void Display_Line (uint8_t* Row, uint32_t row_stride);
static void Get_Scale    (uint32_t width, uint32_t height, uint32_t *num, uint32_t *denom);
static void Set_Scale    (uint32_t width, uint32_t height);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Decode, resize and display a BMP image
  * @param  file: file pointer
  * @param  effect: image effect  
  * @retval Status
  */
int32_t  JPEG_Decode(FIL *file, uint8_t effect)
{
  int32_t ret = 0;
  
  /* Init decompress params */
  out_params.out_buff = p_malloc(OUTPUT_BUFFER);
  out_params.out_process = Display_Line;
  out_params.get_scale   = Get_Scale;
  out_params.set_scale   = Set_Scale;
  image_effect           = effect;  

  /* Start decompress  */
  if((jpeg_decompress(file, &out_params) < 0 ) ||
     (out_params.out_buff == NULL))
  {
    ret = -1;
  }
  p_free(out_params.out_buff);
  LCD_WindowModeDisable();
  return ret; /* JPEG image ok*/
}


/**
  * @brief  Display a line on the LCD screen
  * @param  Row: image params structure
  * @param  row_stride : scaling x factor
  * @retval None
  */
static void Display_Line (uint8_t* Row, uint32_t row_stride)
{
  uint32_t i = 0;
  uint16_t    pixel;
  RGB_typedef *RGB_matrix =  (RGB_typedef *)Row;

  for(i = 0; i < row_stride ; i++)
  {

    switch((image_effect > 0) ? (image_effect - 1) : image_effect)
    {
    case JPEG_NORMAL:
      pixel  = JPEG_RGB_TO_PIXEL(RGB_matrix[i].B,RGB_matrix[i].G,RGB_matrix[i].R);
      break;

    case JPEG_BLUISH:
      pixel  = JPEG_RGB_TO_PIXEL(RGB_matrix[i].B,0,0);
      break;

    case JPEG_GREENISH:
      pixel  = JPEG_RGB_TO_PIXEL(0,RGB_matrix[i].G,0);
      break;
      
    case JPEG_REDISH:
      pixel  = JPEG_RGB_TO_PIXEL(0,0,RGB_matrix[i].R);
      break;
      
    case JPEG_BLACK_WHITE:
      {
        if((RGB_matrix[i].B + RGB_matrix[i].G + RGB_matrix[i].R)/3 > JPEG_BLACK_LEVEL)
        {
          pixel  = JPEG_RGB_TO_PIXEL(0xFF,0xFF,0xFF);
        }
        else
        {
          pixel = JPEG_RGB_TO_PIXEL(0,0,0);
        }
      }
      break;
     
    default:
      pixel  = 0xFFFF;
      break;
    }


    LCD_WriteRAM(pixel);
  }
}


/**
  * @brief  Display a line on the LCD screen
  * @param  width: image width
  * @param  height: image height
  * @param  num:  scaling numerator
  * @param  denom: scaling denominator
  * @retval None
  */

static void Get_Scale (uint32_t width, uint32_t height, uint32_t *num, uint32_t *denom)
{
  uint32_t factx = 8 , facty = 8;

  /*Libjpeg supported only scaling ratios of 1/1, 1/2, 1/4, and 1/8 */

  while (factx > 1)
  {
    if((width * factx / 8) <= IMAGE_display_area_width)
       break;
    factx /= 2;
  }

  while (facty > 1)
  {
    if(( height * facty / 8) <= IMAGE_display_area_height)
       break;
    facty /= 2;
  }

  *num = (factx < facty) ? factx : facty;
  *denom = 8;
}


/**
  * @brief  Display a line on the LCD screen
  * @param  width: image width
  * @param  height: image height
  * @retval None
  */

static void Set_Scale (uint32_t width, uint32_t height)
{
  pos_x = (320 - (uint16_t )(width)) / 2;
  pos_y = (240 - (uint16_t )(height)) / 2 ;

  if(height < 240)
  {
    pos_y += 5;
  }

  /* Prepare LCD For Write*/
  LCD_SetDisplayWindow(height - 1 + pos_y, width  + pos_x - 1, height, width );
  LCD_WriteReg(R3, 0x1018);
  LCD_SetCursor(pos_y, width  + pos_x - 1);

  /* Prepare to write GRAM */
  LCD_WriteRAM_Prepare();

}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
