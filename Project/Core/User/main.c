/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   This file provides main program functions
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
#include "gl_mgr.h"

#include "mod_audio.h"
#include "mod_recorder.h"
#include "mod_camera.h"
#include "mod_calendar.h"
#include "mod_credits.h"
#include "mod_ethernet.h"
#include "mod_image.h"
#include "mod_system.h"
#include "mod_serial.h"
#include "mod_usbd.h"
#include "mod_console.h"
#include "mod_filemgr.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define Background_Task_PRIO    ( tskIDLE_PRIORITY  + 10 )
#define Background_Task_STACK   ( 768 )

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
xTaskHandle                   Task_Handle;

/* Private function prototypes -----------------------------------------------*/
static void Background_Task(void * pvParameters);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* Create background task */
    xTaskCreate(Background_Task,
                (signed char const*)"BK_GND",
                Background_Task_STACK,
                NULL,
                Background_Task_PRIO,
                &Task_Handle);

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();
}


/**
  * @brief  Background task
  * @param  pvParameters not used
  * @retval None
  */
static void Background_Task(void * pvParameters)
{
    static uint32_t ticks = 0;

    /* Init Modules managicvObjectDetection 3.1>er */
    MOD_PreInit();

    /*Add modules here */
    //MOD_AddModule(&mod_system, UTILITIES_GROUP);
    //MOD_AddModule(&mod_audio, MULTIMEDIA_GROUP);
    //MOD_AddModule(&mod_image, MULTIMEDIA_GROUP);
    //MOD_AddModule(&mod_ethernet, CONNECTIVITY_GROUP);
    MOD_AddModule(&mod_camera, MULTIMEDIA_GROUP);
    //MOD_AddModule(&mod_usbd, CONNECTIVITY_GROUP);
    //MOD_AddModule(&mod_recorder, MULTIMEDIA_GROUP);
    //MOD_AddModule(&mod_serial, CONNECTIVITY_GROUP);
    //MOD_AddModule(&mod_calendar, UTILITIES_GROUP);
    //MOD_AddModule(&mod_filemgr, UTILITIES_GROUP);
    // MOD_AddModule(&mod_console, UTILITIES_GROUP);
    // MOD_AddModule(&mod_credits, UTILITIES_GROUP);

    CONSOLE_LOG((uint8_t *)"[SYSTEM] System modules added.");

    /* Show startup Page */
    GL_Startup();

    /* Init Libraries and stack here */
    MOD_LibInit();

    /* Show the main menu and start*/
    GL_Init();

	/* Initial USART3 for debug message */
	//KUART_Init();
	
	/* Output to USART3 for test */
	//KUART_Output("Hello! \n");
	
    while (1)
    {
        if ( ticks++ > 10 )
        {
            ticks = 0;

            /* toggle LED1..4 each 100ms */
//            STM_EVAL_LEDToggle(LED1);
//       STM_EVAL_LEDToggle(LED2);
//       STM_EVAL_LEDToggle(LED3);
//       STM_EVAL_LEDToggle(LED4);

            /* Wait for HMI events */
            GL_Handle_Inputs();

            /*Update GUI processes */
            GL_Background_Handler();

            /* power saving mode */
            GL_LowPower();

            /* Apply new settings if they were changed */
            GL_HandleSystemSettingsChange();

            /* Display processes running in background */
            GL_UpdateBackgroundProcessStatus();
        }

        /* Handle modules background tasks */
        MOD_HandleModulesBackground();

        /* This task is handled periodically, each 10 mS */
        vTaskDelay(10);
    }
}


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {}
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
