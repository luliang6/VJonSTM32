/**
  ******************************************************************************
  * @file    libjpeg_utils.h 
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   This file contains the headers of libjpeg_utils.c file.
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
#ifndef ___LIBJPEG_UTILS__H
#define ___LIBJPEG_UTILS__H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include "jpeglib.h"

/* Exported types ------------------------------------------------------------*/
typedef  void (*OUT_FUNCT)(uint8_t*,  uint32_t );   
typedef  void (*GET_SCALE)(uint32_t, uint32_t, uint32_t*, uint32_t*);   
typedef  void (*SET_SCALE)(uint32_t, uint32_t);   

typedef struct
{
  uint8_t  *out_buff;
  GET_SCALE get_scale;
  SET_SCALE set_scale;
  OUT_FUNCT out_process;
  
}OutParams_Typedef; 



typedef struct
{
  uint8_t  *in_buff;
  uint32_t in_length; 
  uint32_t in_width; 
  uint32_t in_height; 
  uint32_t in_bpp; 
  J_COLOR_SPACE in_colorspace;
  uint32_t in_imagequality;
  
}InParams_Typedef; 
                       

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int32_t jpeg_decompress(FIL *file, OutParams_Typedef  *out_params);
int32_t jpeg_compress(FIL *file, InParams_Typedef  *in_params);

#ifdef __cplusplus
}
#endif

#endif /* ___LIBJPEG_UTILS__H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
