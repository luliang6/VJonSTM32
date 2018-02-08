/**
  ******************************************************************************
  * @file    mem_utils.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Header for mem_utils module
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
#ifndef _MEM_UTILS_H__
#define _MEM_UTILS_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "global_includes.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
   
/* Exported types ------------------------------------------------------------*/
#define SIZE_OF_PAGE    1024  /* 1 KBytes pages */
#define MAX_PAGE_NUMBER  128  /* Handle a maximum of 128 pages */


typedef struct
{
  uint32_t mallocBase;                    /* Memory pool base address */
  uint32_t size[MAX_PAGE_NUMBER + 1];     /* Sizes Table to allow safe deallocation */
  uint8_t PageTable[MAX_PAGE_NUMBER + 1]; /* Memory page state table '1'-> Allocated '0' -> Free */
}
mem_TypeDef;
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void mem_pool_init(void);
void * p_malloc(size_t s);
void p_free(void * p);

#ifdef __cplusplus
}
#endif

#endif /* _MEM_UTILS_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
