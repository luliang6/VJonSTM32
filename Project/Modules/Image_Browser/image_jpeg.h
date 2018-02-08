/**
  ******************************************************************************
  * @file    image_jpeg.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Header for image_jpeg.c module
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
#ifndef __IMAGE_JPEG_H
#define __IMAGE_JPEG_H

/* Includes ------------------------------------------------------------------*/
#include  "global_includes.h"
#include "libjpeg_utils.h"
#include  "LcdHal.h"
#include  "ff.h"

/* Exported types ------------------------------------------------------------*/
/* This structure defines the decompression processing parameters */
typedef struct 
{
  uint32_t ImageHeight;  /* Output image height */
  uint32_t ImageWidth;   /* Output image width */
  uint32_t NumberComponent; /* Usually 1= grey scaled, 3= color YCbCr or YIQ, 4 =color CMYK*/
  uint16_t BitperSample;  /* Number of bits per sample */
  uint16_t Length;    /* Output image length */
}JpegImage_typedef;

/* This structure defines the RGB color base */
typedef struct RGB
{
  uint8_t R;
  uint8_t G;
  uint8_t B;
}RGB_typedef;
/* Exported constants --------------------------------------------------------*/
#define JPEG_NORMAL         0
#define JPEG_BLUISH         1
#define JPEG_GREENISH       2
#define JPEG_REDISH         3
#define JPEG_BLACK_WHITE    4

#define JPEG_BLACK_LEVEL    128

#define JPEG_MAX_DISPLAY_WIDTH     320
#define JPEG_MAX_DISPLAY_HEIGHT    240
/* Exported macro ------------------------------------------------------------*/
#define JPEG_RGB_TO_PIXEL(b,g,r)  ((uint16_t) (((b & 0x00F8) >> 3)|\
                                               ((g & 0x00FC) << 3)|\
                                               ((r & 0x00F8) << 8))) 

/* Exported functions ------------------------------------------------------- */
int32_t  JPEG_Decode(FIL *file, uint8_t effect);


#endif /* __IMAGE_JPEG_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
