/**
  ******************************************************************************
  * @file    app_rtc.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   This file contains the RTC configuration process
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
#include "app_rtc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "gl_mgr.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define LSE_MAX_TRIALS_NB         6

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
RTC_InitTypeDef   RTC_InitStructure;
uint32_t RTC_Timeout = 0x10000;
int8_t RTC_Error = 0;
uint8_t RTC_HandlerFlag;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
* @brief  Configures the RTC peripheral.
* @param  None
* @retval None
*/
int8_t RTC_Configuration(void)
{
  uint8_t trialno;
  RTC_Error = 0;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);

#ifdef RTC_CLOCK_SOURCE_LSI
  /* Enable the LSI OSC */
  RCC_LSICmd(ENABLE);
  /* Wait till LSI is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
  {
    if (RTC_Timeout-- == 0)
    {
      RTC_Error = -1;
      return -1;
    }
  }

  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
#endif

#ifdef RTC_CLOCK_SOURCE_LSE

  for ( trialno = 0 ; trialno <= LSE_MAX_TRIALS_NB ; trialno++)
  {
    /* Enable the LSE OSC */
    RCC_LSEConfig(RCC_LSE_ON);
    RTC_Timeout = 0x10000;
    
    if ((trialno == 1) && (trialno == 4))
    {
      GL_State_Message((uint8_t *)"RTC and backup Starting.. ");
    }

    else if ((trialno == 2)&& (trialno == 5))
    {
      GL_State_Message((uint8_t *)"RTC and backup Starting...");
    }
    
    else if ((trialno == 3)&& (trialno == 6))
    {
      GL_State_Message((uint8_t *)"RTC and backup Starting.  ");
    }
    vTaskDelay(500);
    /* Wait till LSE is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
    {
      if (RTC_Timeout-- == 0)
      {
        RTC_Error = -1;
        RCC_LSEConfig(RCC_LSE_OFF);
        break;
      }
    }
    
    if (RTC_Error == 0)
    {
      break;
    }
  }

  if (RTC_Error == -1)
  {
    return -1;
  }
  
  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
#endif
  
#ifdef RTC_CLOCK_SOURCE_HSE
  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div31);
#endif
  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

#ifdef RTC_CLOCK_SOURCE_HSE
  /*Calender Configuration*/
  RTC_InitStructure.RTC_AsynchPrediv =  0x189D;
  RTC_InitStructure.RTC_SynchPrediv = 0x7F;
#endif

#ifdef RTC_CLOCK_SOURCE_LSI
  /*Calender Configuration*/
  RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
  RTC_InitStructure.RTC_SynchPrediv =  0xCB;
#endif

#ifdef RTC_CLOCK_SOURCE_LSE
  /*Calender Configuration*/
  RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
  RTC_InitStructure.RTC_SynchPrediv =  0xFF;
#endif

  /* Wait for RTC APB registers synchronisation */
  if ( RTC_WaitForSynchro() == ERROR)
  {
    RTC_Error = -1;
    return -1;
  }
  return 0;
}

/**
* @brief  This function handles RTC Alarm A interrupt request.
* @param  None
* @retval None
*/
void RTC_Alarm_IRQHandler(void)
{
  /* Clear the EXTIL line 17 */
  EXTI_ClearITPendingBit(EXTI_Line17);

  /* Check on the AlarmA falg and on the number of interrupts per Second (60*8) */
  if (RTC_GetITStatus(RTC_IT_ALRA) != RESET)
  {
    /* Clear RTC AlarmA Flags */
    RTC_ClearITPendingBit(RTC_IT_ALRA);
    RTC_HandlerFlag = ENABLE;
  }
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
