/**
  ******************************************************************************
  * @file    image_bmp.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   This file contains the methods to process images in BMP format
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
#include "image_bmp.h"
#include "mem_utils.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define BI_RGB  0L
#define BI_RLE8  1L
#define BI_RLE4  2L
#define BI_BITFIELDS 3L
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


IMAGEHDR         image_params;
uint16_t         BMP_factx, BMP_facty;
uint16_t         BMP_pos_x, BMP_pos_y;
extern uint16_t  IMAGE_display_area_width;
extern uint16_t  IMAGE_display_area_height; 
/* Private function prototypes -----------------------------------------------*/
static void Display_Line (PIMAGEHDR pimage , 
                          uint8_t *pbuff,  
                          uint8_t fact_x, 
                          uint8_t effect);

static void ComputeImagePitch(int32_t bpp,
                              int32_t width,
                              int32_t *pitch,
                              int32_t *bytesperpixel);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Decode, resize and display a BMP image
  * @param  file: file pointer
  * @param  effect: image effect
  * @retval Status
  */
int32_t  BMP_Decode(FIL *file, uint8_t effect)
{
  int32_t  h, compression;
  int32_t  headsize , numOfReadBytes;
  BMPFILEHEAD         bmpf;
  BMPINFOHEAD         bmpi;
  BMPCOREHEAD         bmpc;
  uint8_t             headbuffer[INFOHEADSIZE];
  uint8_t             *BMP_ImageBuffer;  



  /* read BMP file header*/

  if (f_read(file, &headbuffer, FILEHEADSIZE, (void *)&numOfReadBytes) != FR_OK) /*Error*/
  {
    {return -1;
    }
  }

  bmpf.bfType[0] = headbuffer[0];
  bmpf.bfType[1] = headbuffer[1];

  /* Is it really a bmp file ? */
  if ((bmpf.bfType[0] | (uint16_t)(bmpf.bfType[1] << 8)) != 0x4D42) /* 'BM' */
  {
    return -1;
  } /* not bmp image*/

  /*bmpf.bfSize = (dwread(&headbuffer[2]));*/
  bmpf.bfOffBits = *(uint16_t *) & headbuffer[10];

  /* Read remaining header size */
  if (f_read(file, &headsize, sizeof(uint32_t), (void *)&numOfReadBytes) != FR_OK)
  {
    return -1;
  } /* not bmp image*/

  /* might be windows or os/2 header */
  if (headsize == COREHEADSIZE)
  {

    /* read os/2 header */
    if (f_read(file, &headbuffer, COREHEADSIZE - sizeof(uint32_t), (void *)&numOfReadBytes) != FR_OK)
    {
      return -1;
    } /* not bmp image*/

    /* Get data */
    bmpc.bcWidth    = (uint16_t)(headbuffer[0] + (uint16_t)(headbuffer[1]<<8));
    bmpc.bcHeight   = (uint16_t)(headbuffer[2] + ((uint16_t)headbuffer[3]<<8));
    bmpc.bcPlanes   = (uint16_t)(headbuffer[4] + ((uint16_t)headbuffer[5]<<8));
    bmpc.bcBitCount = (uint16_t)(headbuffer[6] + ((uint16_t)headbuffer[7]<<8));

    image_params.width   = (int32_t)bmpc.bcWidth;
    image_params.height  = (int32_t)bmpc.bcHeight;
    image_params.bpp     = bmpc.bcBitCount;

    if (image_params.bpp <= 8)
      image_params.palsize = 1 << image_params.bpp;
    else
      image_params.palsize = 0;

    compression = BI_RGB;
  }
  else
  {
    /* read windows header */
    if (f_read(file, &headbuffer, INFOHEADSIZE - sizeof(uint32_t), (void *)&numOfReadBytes) != FR_OK)
    {
      return -1;
    } /* not bmp image*/

 
    bmpi.BiWidth =  headbuffer[0]|
                   (headbuffer[1] << 8) |
                   (headbuffer[2] << 16) |
                   (headbuffer[3] << 24);

    
    bmpi.BiHeight = headbuffer[4]|
                   (headbuffer[5] << 8) |
                   (headbuffer[6] << 16) |
                   (headbuffer[7] << 24);
                     
    
    bmpi.BiPlanes = headbuffer[8]|
                   (headbuffer[9] << 8);
    
    bmpi.BiBitCount =  headbuffer[10]|
                      (headbuffer[11] << 8);
    
    bmpi.BiCompression = headbuffer[12]|
                   (headbuffer[13] << 8) |
                   (headbuffer[14] << 16) |
                   (headbuffer[15] << 24);
    
    bmpi.BiSizeImage =headbuffer[16]|
                   (headbuffer[17] << 8) |
                   (headbuffer[18] << 16) |
                   (headbuffer[19] << 24);
    
    bmpi.BiXpelsPerMeter = headbuffer[20]|
                   (headbuffer[21] << 8) |
                   (headbuffer[22] << 16) |
                   (headbuffer[23] << 24);
    
    bmpi.BiYpelsPerMeter =  headbuffer[24]|
                   (headbuffer[25] << 8) |
                   (headbuffer[26] << 16) |
                   (headbuffer[27] << 24);
    
    bmpi.BiClrUsed = headbuffer[28]|
                   (headbuffer[29] << 8) |
                   (headbuffer[30] << 16) |
                   (headbuffer[31] << 24);
    
    bmpi.BiClrImportant =  headbuffer[32]|
                   (headbuffer[33] << 8) |
                   (headbuffer[34] << 16) |
                   (headbuffer[35] << 24);
    

    image_params.width = (int32_t)bmpi.BiWidth;
    image_params.height = (int32_t)bmpi.BiHeight;
    image_params.bpp = bmpi.BiBitCount;
    image_params.palsize = (int32_t)bmpi.BiClrUsed;

    if (image_params.palsize > 256)
    {
      image_params.palsize = 0;
    }
    else if (image_params.palsize == 0 && image_params.bpp <= 8)
    {
      image_params.palsize = 1 << image_params.bpp;
    }

    compression = bmpi.BiCompression;
  }
  image_params.compression = IMAGE_BGR; /* right side up, BGR order*/
  image_params.planes = 1;

  /* only 16, 24 and 32 bpp bitmaps*/
  if (image_params.bpp < 8)
  {
    return -2; /* image loading error*/
  }

  /* compute byte line size and bytes per pixel*/
  ComputeImagePitch(image_params.bpp,
                    image_params.width,
                    &image_params.pitch,
                    &image_params.bytesperpixel);

  if (image_params.pitch > READ_SIZE)
  {
    return -2;
  }

  /* determine 16bpp 5/5/5 or 5/6/5 format*/
  if (image_params.bpp == 16)
  {
    uint32_t format = 0x7c00;  /* default is 5/5/5*/

    if (compression == BI_BITFIELDS)
    {
      uint8_t buf[4];
      if (f_read(file, &buf, sizeof(uint32_t), (void *)&numOfReadBytes) != FR_OK)
      {
        return -1;
      }

      format =      buf[0]|
                   (buf[1] << 8) |
                   (buf[2] << 16) |
                   (buf[3] << 24);
    }
    if (format == 0x7c00)
      image_params.compression |= IMAGE_555;
    /* else it's 5/6/5 format, no flag required*/
  }

  /* Ignore compressed format */
  if ((compression != BI_BITFIELDS) && (compression != BI_RGB))
  {
    return -2;
  }
  
  if((image_params.width < 160) || (image_params.height < 120))
  {
    return -1;
  }
  
  /* decode image data*/
  f_lseek (file, bmpf.bfOffBits);


  h = image_params.height;

  BMP_factx = BMP_facty = 1;
  
  while (BMP_factx < 256)
  {
    if((image_params.width /BMP_factx ) <= IMAGE_display_area_width)
      break;
    BMP_factx *= 2; 
  }
  
  while (BMP_facty < 256)
  {
    if((image_params.height/BMP_facty) <= IMAGE_display_area_height)
      break;
    BMP_facty *= 2; 
  } 
  
  BMP_pos_x = (319 + (bmpi.BiWidth / BMP_factx)) / 2;
  BMP_pos_y = (239 + (bmpi.BiHeight / BMP_facty)) / 2;
  
  if((BMP_factx != 1) || (BMP_facty != 1))
  {
    BMP_pos_y += 5;
  }

  /* Prepare LCD For Write*/
  LCD_SetDisplayWindow(BMP_pos_y, BMP_pos_x, bmpi.BiHeight / BMP_facty, bmpi.BiWidth / BMP_factx);
  LCD_WriteReg(R3, 0x1008);
  LCD_SetCursor(BMP_pos_y, BMP_pos_x);
  
  /* Prepare to write GRAM */
  LCD_WriteRAM_Prepare();

  BMP_ImageBuffer = (uint8_t *) malloc (image_params.pitch );
    
  /* For every row ... */
  while (--h >= 0)
  {
    if ((h % BMP_facty) == 0)
    {
      if (f_read(file, BMP_ImageBuffer, image_params.pitch, (void *)&numOfReadBytes) != FR_OK)
      {
        free(BMP_ImageBuffer);
        return -1;
      }
      else
      {
        Display_Line (&image_params , BMP_ImageBuffer, BMP_factx, effect);
      }
    }
    else
    {
      f_lseek(file, file->fptr + image_params.pitch);
    }
  }

  free(BMP_ImageBuffer);

  LCD_WindowModeDisable();
  return 0; /* bmp image ok*/
}

/**
  * @brief  compute image line size and bytes per pixel from bits per pixel and width
  * @param  bpp: bit per pixel
  * @param  width : image width
  * @param  pitch : image line width
  * @param  bytesperpixel : bytes per pixzl
  * @retval None
  */
static void ComputeImagePitch(int32_t bpp, int32_t width, int32_t *pitch, int32_t *bytesperpixel)
{
  int32_t linesize;
  int32_t bytespp = 1;

  if (bpp == 1)
    linesize = PIX2BYTES(width);
  else if (bpp <= 4)
    linesize = PIX2BYTES(width << 2);
  else if (bpp <= 8)
    linesize = width;
  else if (bpp <= 16)
  {
    linesize = width * 2;
    bytespp = 2;
  }
  else if (bpp <= 24)
  {
    linesize = width * 3;
    bytespp = 3;
  }
  else
  {
    linesize = width * 4;
    bytespp = 4;
  }

  /* rows are DWORD right aligned*/
  *pitch = (linesize + 3) & ~3;
  *bytesperpixel = bytespp;
}

/**
  * @brief  Display a line on the LCD screen
  * @param  pimage: image params structure
  * @param  pbuff: pointer to the bmp data for a single line  
  * @param  fact_x : scaling x factor
  * @param  effect : image effect to be applied
  * @retval None
*/
static void Display_Line ( PIMAGEHDR pimage , uint8_t *pbuff,  uint8_t fact_x, uint8_t effect)
{
  uint16_t i = 0;
  uint16_t pixel = 0;
  uint8_t  r_pix = 0, g_pix = 0, b_pix = 0;
  
  for (i = 0 ; i < image_params.pitch; i += (image_params.bytesperpixel * fact_x))
  {
    if (image_params.bpp == 24)
    {
      b_pix = pbuff[i];
      g_pix = pbuff[i+1] ;
      r_pix = pbuff[i+2];
    }
    
    else if(image_params.bpp == 16)
    {
      pixel = (pbuff[i+1] << 8 | pbuff[i]);
      
      r_pix = BMP_PIXEL16_TO_R(pixel);
      g_pix = BMP_PIXEL16_TO_G(pixel);
      b_pix = BMP_PIXEL16_TO_B(pixel);
    }
    switch((effect > 0) ? (effect - 1) : effect)
    {
    case BMP_NORMAL:
      pixel  = BMP_RGB_TO_PIXEL(r_pix,g_pix,b_pix);
      break;
      
    case BMP_BLUISH:
      pixel  = BMP_RGB_TO_PIXEL(b_pix,0,0);
      break;

    case BMP_GREENISH:
      pixel  = BMP_RGB_TO_PIXEL(0,g_pix,0);
      break;
      
    case BMP_REDISH:
      pixel  = BMP_RGB_TO_PIXEL(0,0,r_pix);
      break;
      
    case BMP_BLACK_WHITE:
      {
        if((b_pix + g_pix + r_pix)/3 > BMP_BLACK_LEVEL)
        {
          pixel  = BMP_RGB_TO_PIXEL(0xFF,0xFF,0xFF);
        }
        else
        {
          pixel = BMP_RGB_TO_PIXEL(0,0,0);
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
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
