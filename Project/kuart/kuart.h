/**
  ******************************************************************************
  * @file    kuart.h
  * @author  Anto at Kneron Inc.
  * @version V1.0.0
  * @date    04-February-2018
  * @brief   this program is used for output debug message to UART3
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __KUART_H
#define __KUART_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void KUART_Init (void);
void KUART_Output (volatile char *str);

#ifdef __cplusplus
}
#endif

#endif /* __KUART_H */
