/**
  ******************************************************************************
  * @file    image_bmp.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Header for image_bmp.c module
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
#ifndef __IMAGE_BMP_H
#define __IMAGE_BMP_H

/* Includes ------------------------------------------------------------------*/
#include "global_includes.h"
#include  "LcdHal.h"
#include  "ff.h"

/* Exported types ------------------------------------------------------------*/
typedef struct
{
  /* BITMAPFILEHEADER*/
  uint8_t bfType[2];
  uint32_t bfSize;
  uint16_t bfReserved1;
  uint16_t bfReserved2;
  uint32_t bfOffBits;
}
BMPFILEHEAD;

/* windows style*/
typedef struct
{
  /* BITMAPINFOHEADER*/
  uint32_t BiSize;
  uint32_t BiWidth;
  uint32_t BiHeight;
  uint16_t BiPlanes;
  uint16_t BiBitCount;
  uint32_t BiCompression;
  uint32_t BiSizeImage;
  uint32_t BiXpelsPerMeter;
  uint32_t BiYpelsPerMeter;
  uint32_t BiClrUsed;
  uint32_t BiClrImportant;
}
BMPINFOHEAD;

/* os/2 style*/
typedef struct
{
  /* BITMAPCOREHEADER*/
  uint32_t bcSize;
  uint16_t bcWidth;
  uint16_t bcHeight;
  uint16_t bcPlanes;
  uint16_t bcBitCount;
}
BMPCOREHEAD;

/* Buffered input functions to replace stdio functions*/
typedef struct
{  /* structure for reading images from buffer   */
  uint8_t *start;  /* The pointer to the beginning of the buffer */
  uint32_t offset; /* The current offset within the buffer       */
  uint32_t size;   /* The total size of the buffer               */
}
buffer_t;

/* In-core color palette structure*/
typedef struct
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t _padding;
}
PALENTRY;

typedef struct
{
  int32_t  width;         /* image width in pixels*/
  int32_t  height;        /* image height in pixels*/
  int32_t  planes;        /* # image planes*/
  int32_t  bpp;           /* bits per pixel (1, 4 or 8)*/
  int32_t  pitch;         /* bytes per line*/
  int32_t  bytesperpixel; /* bytes per pixel*/
  int32_t  compression;   /* compression algorithm*/
  int32_t  palsize;       /* palette size*/
  int32_t         transcolor; /* transparent color or -1 if none*/

}
IMAGEHDR, *PIMAGEHDR;

/* Exported constants --------------------------------------------------------*/

#define IMAGE_UPSIDEDOWN       0x01 /* compression flag: upside down image*/
#define IMAGE_BGR              0x00 /* compression flag: BGR byte order*/
#define IMAGE_RGB              0x02 /* compression flag: RGB not BGR bytes*/
#define IMAGE_ALPHA_CHANNEL    0x04 /* compression flag: 32-bit w/alpha */
#define IMAGE_555              0x08 /* compression flag: 5/5/5 format*/
#define READ_SIZE              4092

#define FILEHEADSIZE           14
#define INFOHEADSIZE           40
#define COREHEADSIZE           12


#define BMP_NORMAL             0
#define BMP_BLUISH             1
#define BMP_GREENISH           2
#define BMP_REDISH             3
#define BMP_BLACK_WHITE        4

#define BMP_BLACK_LEVEL        150

#define BMP_MAX_DISPLAY_WIDTH     320
#define BMP_MAX_DISPLAY_HEIGHT    240
/* Exported macro ------------------------------------------------------------*/
#define BMP_RGB_TO_PIXEL(r,g,b)  ((uint16_t) (((r & 0x00F8) >> 3)|\
                                              ((g & 0x00FC) << 3)|\
                                              ((b & 0x00F8) << 8))) 


#define BMP_PIXEL16_TO_R(pixel)       ((pixel & 0x1F) << 3)
#define BMP_PIXEL16_TO_G(pixel)       (((pixel >> 5) & 0x3F) << 2)
#define BMP_PIXEL16_TO_B(pixel)       (((pixel >> 11) & 0x1F) << 3)

#define PIX2BYTES(n)      (((n)+7)/8)

/* Exported functions ------------------------------------------------------- */
int32_t  BMP_Decode(FIL *file, uint8_t effect);


#endif /* __IMAGE_BMP_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
