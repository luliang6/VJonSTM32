/**
  ******************************************************************************
  * @file    eth_bsp.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Header for eth_bsp.c file.
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
#ifndef __ETH_BSP_H
#define __ETH_BSP_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "global_includes.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define DP83848_PHY_ADDRESS       0x01 /* Relative to stm322xg-EVAL Board */

/* Specific defines for EXTI line, used to manage Ethernet link status */
#define ETH_LINK_EXTI_LINE             EXTI_Line14
#define ETH_LINK_EXTI_PORT_SOURCE      EXTI_PortSourceGPIOB
#define ETH_LINK_EXTI_PIN_SOURCE       EXTI_PinSource14
#define ETH_LINK_EXTI_IRQn             EXTI15_10_IRQn 
/* PB14 */
#define ETH_LINK_PIN                   GPIO_Pin_14
#define ETH_LINK_GPIO_PORT             GPIOB
#define ETH_LINK_GPIO_CLK              RCC_AHB1Periph_GPIOB
/* PHY registers */
#define PHY_MICR                  0x11 /* MII Interrupt Control Register */
#define PHY_MICR_INT_EN           ((uint16_t)0x0002) /* PHY Enable interrupts */
#define PHY_MICR_INT_OE           ((uint16_t)0x0001) /* PHY Enable output interrupt events */
#define PHY_MISR                  0x12 /* MII Interrupt Status and Misc. Control Register */
#define PHY_MISR_LINK_INT_EN      ((uint16_t)0x0020) /* Enable Interrupt on change of link status */
#define PHY_MISR_LINK_MASK        ((uint16_t)0x2000) /* Change of Link Status mask */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void  ETH_BSP_Config(void);
uint32_t EthLink_PHYITConfig(uint16_t PHYAddress);
void EthLink_EXTIConfig(void);
void EthLink_ITHandler(uint16_t PHYAddress);
void EthLink_EXTIITCmd(FunctionalState NewState);

#ifdef __cplusplus
}
#endif

#endif /* __ETH_BSP_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
