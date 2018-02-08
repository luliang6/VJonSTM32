/**
  @page STM32F2-F4_Demonstration_Builder_Binary   STM32F2xx and STM32F4xx Demonstration Builder's binary files.

  @verbatim
  ******************** (C) COPYRIGHT 2012 STMicroelectronics *******************
  * @file    readme.txt 
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   Description of the STM32F2xx and STM32F4xx Demonstration Builder's
  *          binary files.
  ******************************************************************************
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
   @endverbatim


@par Description

This directory contains binary files (*.hex and *.dfu) of the "STM32F2xx and STM32F4xx
Demonstration Builder" application. 

You can use the STM32F2xx/STM32F4xx’s embedded Bootloader or any in-system programming
tool to reprogram this application using these binary files, as described below:

 + Using "in-system programming tool"
    - Connect the STM322xG-EVAL or STM324xG-EVAL board to a PC with a USB cable 
      through USB connector CN21
    - Use "*.hex“ file with STM32 ST-LINK Utility to program the application firmware.        
      Note: you can download STM32 ST-LINK Utility from the link below
           http://www.st.com/internet/com/SOFTWARE_RESOURCES/TOOL/DEVICE_PROGRAMMER/stm32_st-link_utility.zip
  
 + Using "Bootloader (USB FS Device in DFU mode)"
    - Configure the STM322xG-EVAL or STM324xG-EVAL board to boot from "System Memory"
       boot pins BOOT0:1 / BOOT1:0)
    - Connect a 'USB type A to Micro-B' cable between PC and USB connector CN8,
      the board will be detected as USB device in DFU mode.
    - Use "*.dfu" file with "DFUse\DFUse Demonstration" tool 
      (available for download from www.st.com) to reprogram the demonstration firmware 
 
 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
