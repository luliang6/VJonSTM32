/**
  ******************************************************************************
  * @file    libjpeg_utils.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   This file illustrates how to use the libjpeg code as a subroutine 
  *          library to compress or decompress image files.
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
#include "libjpeg_utils.h"
#include "cdjpeg.h"
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  JPEG decompression routine.
  * @param  file: JPEG file 
  * @param  out_params : output parameter structure.
  * @retval Status
  */
int32_t jpeg_decompress(FIL *file, OutParams_Typedef  *out_params)
{
  struct jpeg_decompress_struct cinfo; /* This struct contains the JPEG decompression parameters */
  struct jpeg_error_mgr jerr; /* This struct represents a JPEG error handler */
  JSAMPROW buffer[2] = {0}; /* Output row buffer */

  buffer[0] = out_params->out_buff;
	
  /* Step 1: allocate and initialize JPEG decompression object */
  cinfo.err = jpeg_std_error(&jerr);

  /* Initialize the JPEG decompression object */  
  jpeg_create_decompress(&cinfo);
  
  /* Step 2: specify data source */
  jpeg_stdio_src (&cinfo, file);
  
  /* Step 3: read image parameters with jpeg_read_header() */
  jpeg_read_header(&cinfo, TRUE);
  
  
  /* Step 4: get parameters for scaling */
  out_params->get_scale (cinfo.image_width, cinfo.image_height, (uint32_t *)&cinfo.scale_num, (uint32_t *)&cinfo.scale_denom) ;
  
  if((cinfo.image_width > 1024) || (cinfo.image_height > 768) || 
     (cinfo.image_width < 160 ) || (cinfo.image_height < 120) ||
       (cinfo.progressive_mode == 1))
  {    
    jpeg_destroy_decompress(&cinfo);    
    return -1;
  }
  
  /* Step 5: set parameters for decompression */
#ifdef USE_STM324xG_EVAL
  cinfo.dct_method  = JDCT_FLOAT;    
#elif defined (USE_STM322xG_EVAL)
  cinfo.dct_method  = JDCT_IFAST;
#endif  
  
  /* Step 6: start decompressor */
  jpeg_start_decompress(&cinfo);
  
  /* Step 7: Init output process */
  out_params->set_scale (cinfo.output_width, cinfo.output_height);
  
  /* Step 8: process decompressing */ 
  while (cinfo.output_scanline < cinfo.output_height)
  {
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
    if (out_params->out_process != NULL)
    {
      out_params->out_process(buffer[0], cinfo.output_width);
    }
  }
  
  /* Step 7: Finish decompression */
  jpeg_finish_decompress(&cinfo);
  
  /* Step 8: Release JPEG decompression object */
  jpeg_destroy_decompress(&cinfo);
  
  return 0;
}

/**
  * @brief  JPEG compression routine.
  * @param  file: JPEG file 
  * @param  in_params : input parameter structure.
  * @retval Status
  */
int32_t jpeg_compress(FIL *file, InParams_Typedef  *in_params)
{
  static struct jpeg_compress_struct cinfo; /* This struct contains the JPEG compression parameters */
  struct jpeg_error_mgr jerr; /* This struct represents a JPEG error handler */
  JSAMPROW row_pointer; /* Pointer to a single row */
  uint32_t row_stride = 0; /* Physical row width in image buffer */

  /* Step 1: allocate and initialize JPEG compression object */
  /* Set up the error handler */
  cinfo.err = jpeg_std_error(&jerr);

  /* Initialize the JPEG compression object */  
  jpeg_create_compress(&cinfo);

  /* Step 2: specify data destination */
  jpeg_stdio_dest(&cinfo, file);

  /* Step 3: set parameters for compression */
  cinfo.image_width = in_params->in_width;
  cinfo.image_height = in_params->in_height;
  cinfo.input_components = in_params->in_bpp;
  cinfo.in_color_space = in_params->in_colorspace;
 
  /* Set default compression parameters */
  jpeg_set_defaults(&cinfo);

#ifdef USE_STM324xG_EVAL
  cinfo.dct_method  = JDCT_FLOAT;    
#elif defined (USE_STM322xG_EVAL)
  cinfo.dct_method  = JDCT_IFAST;
#endif  
  jpeg_set_quality(&cinfo, in_params->in_imagequality, TRUE);

  /* Step 4: start compressor */
  jpeg_start_compress(&cinfo, TRUE);

  row_stride = cinfo.image_width * cinfo.input_components;
  while (cinfo.next_scanline < cinfo.image_height)
  {
    row_pointer = (JSAMPROW)&in_params->in_buff[cinfo.next_scanline * row_stride];
    jpeg_write_scanlines(&cinfo, &row_pointer, 1);
  }

  /* Step 5: finish compression */
  jpeg_finish_compress(&cinfo);

  /* Step 6: release JPEG compression object */
  jpeg_destroy_compress(&cinfo);

  return 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
