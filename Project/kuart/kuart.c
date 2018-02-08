/**
  ******************************************************************************
  * @file    kuart.c
  * @author  Anto at Kneron Inc.
  * @version V1.0.0
  * @date    04-February-2018
  * @brief   this program is used for output debug message to UART3, maybe
  *          someone can improve it as a more flexible version if necessary^_^
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "kuart.h"
#include "stm32f4xx_gpio.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define COM         0
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Inititialize the USART3 for output debug message
 * @param  None
 * @retval None
 */
void KUART_Init()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /* Enable GPIO clock (RX/TX in GPIO C) */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    /* Connect GPIO P10 to USART2_Tx*/
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3);

    /* Connect GPIO P11 to USART2_Rx*/
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);

    /* Configure USART Tx as alternate function  */
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Configure USART Rx as alternate function */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* USART configuration */
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART3, &USART_InitStructure);

    /* Enable USART */
    USART_Cmd(USART3, ENABLE);

    /* Clear buffer */
    USART_SendData(USART3, '\r');
    /* loop until the end of transmission */
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
    {
    }
}

/**
 * @brief  Inititialize the USART3 for debug
 * @param  str: string for output to UART
 * @retval None
 * @remark Improve this function later if necessary
 */
void KUART_Output (volatile char *str)
{
    while(*str)
    {
        USART_SendData(USART3, (uint16_t)*str++);

        /* loop until the end of transmission */
        while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
        {
        }
    }
    USART_SendData(USART3, (uint16_t)('\r'));

    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
    {
    }
    USART_SendData(USART3, (uint16_t)('\n'));
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
    {
    }
}
