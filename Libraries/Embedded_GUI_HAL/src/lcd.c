/**
  ******************************************************************************
  * @file    lcd.c
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    31-July-2012
  * @brief   This file includes the LCD driver for AM-240320L8TNQW00H 
  *          (LCD_ILI9320) Liquid Crystal Display Module..
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
#include "stm32f10x.h"
#include "gl_fonts.h"
#include "LcdHal.h"
#include "TscHal.h"

/** @addtogroup Embedded_GUI_Library
  * @{
  */

/** @defgroup Lcd
  * @brief Lcd main functions
  * @{
  */ 

/** @defgroup Lcd_Private_TypesDefinitions
  * @{
  */

typedef struct
{
  __IO uint16_t LCD_REG;
  __IO uint16_t LCD_RAM;
}LCD_TypeDef;

/**
  * @}
  */ 

/** @defgroup Lcd_Private_Defines
  * @{
  */
#define START_BYTE      0x70
#define SET_INDEX       0x00
#define READ_STATUS     0x01
#define LCD_WRITE_REG   0x02
#define LCD_READ_REG    0x03

/* Note: LCD /CS is CE4 - Bank 4 of NOR/SRAM Bank 1~4 */
#define LCD_BASE        ((uint32_t)(0x60000000 | 0x0C000000))

#define LCD             ((LCD_TypeDef *) LCD_BASE)

/**
  * @}
  */

/** @defgroup Lcd_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup Lcd_Private_Variables
  * @{
  */
/* Global variables to set the written text color */
#if !defined(LCDType)
__IO uint32_t LCDType = LCD_ILI9320;
#endif

__IO uint16_t GL_TextColor = 0x0000, GL_BackColor = 0xFFFF;
extern LCD_HW_Parameters_TypeDef pLcdHwParam;     /* Lcd Hardware Parameters Structure */

/* Global variables to handle the right font */
__IO uint8_t Font = FONT_BIG;
__IO uint8_t FontWidth = FONT_BIG_WIDTH, FontHeight = FONT_BIG_HEIGHT;

/**
  * @}
  */

/** @defgroup Lcd_Private_FunctionPrototypes
  * @{
  */

/**
  * @}
  */

/** @defgroup Lcd_Private_Functions
  * @{
  */

/**
  * @brief  Setups the LCD.
  * @param  None
  * @retval None
  */
void GL_LCD_Setup(void)
{

#ifdef USE_STM3210C_EVAL

  STM3210C_LCD_Init();
#elif USE_STM3210E_EVAL

  STM3210E_LCD_Init();

#elif USE_STM32100E_EVAL

  STM32100E_LCD_Init();

#elif USE_STM3210B_EVAL

  STM3210B_LCD_Init();
#elif USE_STM32100B_EVAL

  STM32100B_LCD_Init();
#endif


}

/**
  * @brief  Sets the Text color.
  * @param  Color: specifies the Text color code RGB(5-6-5).
  * @param  TextColor: Text color global variable used by LCD_DrawChar
  *         and LCD_DrawPicture functions.
  * @retval None
  */
void GL_LCD_SetTextColor(__IO uint16_t Color)
{
  GL_TextColor = Color;
}

/**
  * @brief  Sets the Background color.
  * @param  Color: specifies the Background color code RGB(5-6-5).
  * @param  BackColor: Background color global variable used by
  *         GL_LCD_DrawChar and GL_LCD_DrawPicture functions.
  * @retval None
  */
void GL_LCD_SetBackColor(__IO uint16_t Color)
{
  GL_BackColor = Color;
}

/**
  * @brief  Clears the hole LCD.
  * @param  Color: the color of the background.
  * @retval None
  */
void GL_LCD_Clear(uint16_t Color)
{
  uint32_t index = 0;

  GL_LCD_SetCursor(0x00, 0x013F);

  GL_LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */

  for (index = 0; index < 76800; index++)
  {
    if ( pLcdHwParam.LCD_Connection_Mode == GL_SPI )
      GL_LCD_WriteRAM(Color);
    else
      LCD->LCD_RAM = Color;
  }
  if ( pLcdHwParam.LCD_Connection_Mode == GL_SPI )
    GL_LCD_CtrlLinesWrite(pLcdHwParam.LCD_Ctrl_Port_NCS, pLcdHwParam.LCD_Ctrl_Pin_NCS, GL_HIGH);
}

/**
  * @brief  Sets the cursor position.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @retval None
  */
void GL_LCD_SetCursor(uint8_t Xpos, uint16_t Ypos)
{
  GL_LCD_WriteReg(R32, Xpos);
  GL_LCD_WriteReg(R33, Ypos);
}

/**
  * @brief  Sets a display window
  * @param  Xpos: specifies the X bottom left position.
  * @param  Ypos: specifies the Y bottom left position.
  * @param  Height: display window height.
  * @param  Width: display window width.
  * @retval None
  */
void GL_LCD_SetDisplayWindow(uint8_t Xpos, uint16_t Ypos, uint8_t Height, uint16_t Width)
{
  /* Horizontal GRAM Start Address */
  if (Xpos >= Height)
  {
    GL_LCD_WriteReg(R80, (Xpos - Height + 1));
  }
  else
  {
    GL_LCD_WriteReg(R80, 0);
  }
  /* Horizontal GRAM End Address */
  GL_LCD_WriteReg(R81, Xpos);
  /* Vertical GRAM Start Address */
  if (Ypos >= Width)
  {
    GL_LCD_WriteReg(R82, (Ypos - Width + 1));
  }
  else
  {
    GL_LCD_WriteReg(R82, 0);
  }
  /* Vertical GRAM End Address */
  GL_LCD_WriteReg(R83, Ypos);
  GL_LCD_SetCursor(Xpos, Ypos);
}

/**
  * @brief  Displays a line.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Length: line length.
  * @param  Direction: line direction.
  *         This parameter can be one of the following values:
  *     @arg  Vertical
  *     @arg  Horizontal
  * @retval None
  */
void GL_LCD_DrawLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction)
{
  uint32_t counter = 0;

  GL_LCD_SetCursor(Xpos, Ypos);

  if (Direction == Horizontal)
  {
    GL_LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */

    for (counter = 0; counter < Length; counter++)
    {
      GL_LCD_WriteRAM(GL_TextColor);
    }
    if ( pLcdHwParam.LCD_Connection_Mode == GL_SPI )
      GL_LCD_CtrlLinesWrite(pLcdHwParam.LCD_Ctrl_Port_NCS, pLcdHwParam.LCD_Ctrl_Pin_NCS, GL_HIGH);
  }
  else
  {
    for (counter = 0; counter < Length; counter++)
    {
      GL_LCD_WriteRAMWord(GL_TextColor);
      Xpos++;
      GL_LCD_SetCursor(Xpos, Ypos);
    }
  }
}

/**
  * @brief  Displays a rectangle.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Height: display rectangle height.
  * @param  Width: display rectangle width.
  * @retval None
  */
void GL_LCD_DrawRect(uint8_t Xpos, uint16_t Ypos, uint8_t Height, uint16_t Width)
{
  GL_LCD_DrawLine(Xpos, Ypos, Width, Horizontal);
  GL_LCD_DrawLine((Xpos + Height), Ypos, Width, Horizontal);

  GL_LCD_DrawLine(Xpos, Ypos, Height, Vertical);
  GL_LCD_DrawLine(Xpos, (Ypos - Width + 1), Height, Vertical);
}

/**
  * @brief  Displays a circle.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Radius: the radius size of the circle
  * @retval None
  */
void GL_LCD_DrawCircle(uint8_t Xpos, uint16_t Ypos, uint16_t Radius)
{
  s32  D;/* Decision Variable */
  uint32_t  CurX;/* Current X Value */
  uint32_t  CurY;/* Current Y Value */

  D = 3 - (Radius << 1);
  CurX = 0;
  CurY = Radius;

  while (CurX <= CurY)
  {
    GL_LCD_SetCursor(Xpos + CurX, Ypos + CurY);
    GL_LCD_WriteRAMWord(GL_TextColor);

    GL_LCD_SetCursor(Xpos + CurX, Ypos - CurY);
    GL_LCD_WriteRAMWord(GL_TextColor);

    GL_LCD_SetCursor(Xpos - CurX, Ypos + CurY);
    GL_LCD_WriteRAMWord(GL_TextColor);

    GL_LCD_SetCursor(Xpos - CurX, Ypos - CurY);
    GL_LCD_WriteRAMWord(GL_TextColor);

    GL_LCD_SetCursor(Xpos + CurY, Ypos + CurX);
    GL_LCD_WriteRAMWord(GL_TextColor);

    GL_LCD_SetCursor(Xpos + CurY, Ypos - CurX);
    GL_LCD_WriteRAMWord(GL_TextColor);

    GL_LCD_SetCursor(Xpos - CurY, Ypos + CurX);
    GL_LCD_WriteRAMWord(GL_TextColor);

    GL_LCD_SetCursor(Xpos - CurY, Ypos - CurX);
    GL_LCD_WriteRAMWord(GL_TextColor);

    if (D < 0)
    {
      D += (CurX << 2) + 6;
    }
    else
    {
      D += ((CurX - CurY) << 2) + 10;
      CurY--;
    }
    CurX++;
  }
}

/**
  * @brief  Reset LCD control line(/CS) and Send Start-Byte
  * @param  Start_Byte: the Start-Byte to be sents
  * @retval None
  */
void GL_LCD_nCS_StartByte(uint8_t Start_Byte)
{
  if ( pLcdHwParam.LCD_Connection_Mode == GL_SPI )
  {
    GL_LCD_CtrlLinesWrite(pLcdHwParam.LCD_Ctrl_Port_NCS, pLcdHwParam.LCD_Ctrl_Pin_NCS, GL_LOW);
  }

  SPI_I2S_SendData(pLcdHwParam.LCD_Bus_Port, Start_Byte);
  while (SPI_I2S_GetFlagStatus(pLcdHwParam.LCD_Bus_Port, SPI_I2S_FLAG_BSY) != RESET)
  {}
}

/**
  * @brief  Writes index to select the LCD register.
  * @param  LCD_Reg: address of the selected register.
  * @retval None
  */
void GL_LCD_WriteRegIndex(uint8_t LCD_Reg)
{
  /* Reset LCD control line(/CS) and Send Start-Byte */
  GL_LCD_nCS_StartByte(START_BYTE | SET_INDEX);

  /* Write 16-bit Reg Index (High Byte is 0) */
  SPI_I2S_SendData(pLcdHwParam.LCD_Bus_Port, 0x00);
  while (SPI_I2S_GetFlagStatus(pLcdHwParam.LCD_Bus_Port, SPI_I2S_FLAG_BSY) != RESET)
  {}
  SPI_I2S_SendData(pLcdHwParam.LCD_Bus_Port, LCD_Reg);
  while (SPI_I2S_GetFlagStatus(pLcdHwParam.LCD_Bus_Port, SPI_I2S_FLAG_BSY) != RESET)
  {}
  if ( pLcdHwParam.LCD_Connection_Mode == GL_SPI )
  {
    GL_LCD_CtrlLinesWrite(pLcdHwParam.LCD_Ctrl_Port_NCS, pLcdHwParam.LCD_Ctrl_Pin_NCS, GL_HIGH);
  }
}

/**
  * @brief  Reads the selected LCD Register.
  * @param  LCD_Reg: address of the selected register.
  * @retval uint16_t - LCD Register Value.
  */
uint16_t GL_LCD_ReadReg(uint8_t LCD_Reg)
{
  if ( pLcdHwParam.LCD_Connection_Mode == GL_SPI )
  {
    uint16_t tmp = 0;
    uint8_t i = 0;

    /* LCD_SPI_PORT prescaler: 4 */
    pLcdHwParam.LCD_Bus_Port->CR1 &= 0xFFC7;
    pLcdHwParam.LCD_Bus_Port->CR1 |= 0x0008;

    /* Write 16-bit Index (then Read Reg) */
    GL_LCD_WriteRegIndex(LCD_Reg);

    /* Read 16-bit Reg */
    /* Reset LCD control line(/CS) and Send Start-Byte */
    GL_LCD_nCS_StartByte(START_BYTE | LCD_READ_REG);

    for (i = 0; i < 5; i++)
    {
      SPI_I2S_SendData(pLcdHwParam.LCD_Bus_Port, 0xFF);
      while (SPI_I2S_GetFlagStatus(pLcdHwParam.LCD_Bus_Port, SPI_I2S_FLAG_BSY) != RESET)
      {}
      /* One byte of invalid dummy data read after the start byte */
      while (SPI_I2S_GetFlagStatus(pLcdHwParam.LCD_Bus_Port, SPI_I2S_FLAG_RXNE) == RESET)
      {}
      SPI_I2S_ReceiveData(pLcdHwParam.LCD_Bus_Port);
    }

    SPI_I2S_SendData(pLcdHwParam.LCD_Bus_Port, 0xFF);
    /* Read upper byte */
    while (SPI_I2S_GetFlagStatus(pLcdHwParam.LCD_Bus_Port, SPI_I2S_FLAG_BSY) != RESET)
    {}
    /* Read lower byte */
    while (SPI_I2S_GetFlagStatus(pLcdHwParam.LCD_Bus_Port, SPI_I2S_FLAG_RXNE) == RESET)
    {}
    tmp = SPI_I2S_ReceiveData(pLcdHwParam.LCD_Bus_Port);


    SPI_I2S_SendData(pLcdHwParam.LCD_Bus_Port, 0xFF);
    while (SPI_I2S_GetFlagStatus(pLcdHwParam.LCD_Bus_Port, SPI_I2S_FLAG_BSY) != RESET)
    {}
    /* Read lower byte */
    while (SPI_I2S_GetFlagStatus(pLcdHwParam.LCD_Bus_Port, SPI_I2S_FLAG_RXNE) == RESET)
    {}
    tmp = ((tmp & 0xFF) << 8) | SPI_I2S_ReceiveData(pLcdHwParam.LCD_Bus_Port);

    GL_LCD_CtrlLinesWrite(pLcdHwParam.LCD_Ctrl_Port_NCS, pLcdHwParam.LCD_Ctrl_Pin_NCS, GL_HIGH);

    /* LCD_SPI_PORT prescaler: 2 */
    pLcdHwParam.LCD_Bus_Port->CR1 &= 0xFFC7;

    return tmp;
  }
  else
  {
    /* Write 16-bit Index (then Read Reg) */
    LCD->LCD_REG = LCD_Reg;
    /* Read 16-bit Reg */
    return (LCD->LCD_RAM);
  }
}

/**
  * @brief  Reads the LCD RAM.
  * @param  None
  * @retval uint16_t - LCD RAM Value.
  */
uint16_t GL_LCD_ReadRAM(void)
{
  __IO uint16_t tmp;
  /* Write 16-bit Index (then Read Reg) */
  LCD->LCD_REG = R34; /* Select GRAM Reg */
  /* Read 16-bit Reg */
  tmp = LCD->LCD_RAM;
  return LCD->LCD_RAM;
}

/**
  * @brief  Prepare to write to the LCD RAM.
  * @param  None
  * @retval None
  */
void GL_LCD_WriteRAM_Prepare(void)
{
  if (pLcdHwParam.LCD_Connection_Mode == GL_SPI)
  {
    GL_LCD_WriteRegIndex(R34); /* Select GRAM Reg */

    /* Reset LCD control line(/CS) and Send Start-Byte */
    GL_LCD_nCS_StartByte(START_BYTE | LCD_WRITE_REG);
  }
  else
  {
    LCD->LCD_REG = R34;
  }
}

/**
  * @brief  Writes 1 word to the LCD RAM.
  * @param  RGB_Code: the pixel color in RGB mode (5-6-5).
  * @retval None
  */
void GL_LCD_WriteRAMWord(uint16_t RGB_Code)
{
  GL_LCD_WriteRAM_Prepare();

  GL_LCD_WriteRAM(RGB_Code);

  if ( pLcdHwParam.LCD_Connection_Mode == GL_SPI )
  {
    GL_LCD_CtrlLinesWrite(pLcdHwParam.LCD_Ctrl_Port_NCS, pLcdHwParam.LCD_Ctrl_Pin_NCS, GL_HIGH);
  }
}

/**
  * @brief  Writes to the selected LCD register.
  * @param  LCD_Reg: address of the selected register.
  * @param  LCD_RegValue: value to write to the selected register.
  * @retval None
  */
void GL_LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue)
{
  if (pLcdHwParam.LCD_Connection_Mode == GL_SPI)
  {
    /* Write 16-bit Index (then Write Reg) */
    GL_LCD_WriteRegIndex(LCD_Reg);

    /* Write 16-bit Reg */
    /* Reset LCD control line(/CS) and Send Start-Byte */
    GL_LCD_nCS_StartByte(START_BYTE | LCD_WRITE_REG);

    SPI_I2S_SendData(pLcdHwParam.LCD_Bus_Port, LCD_RegValue >> 8);
    while (SPI_I2S_GetFlagStatus(pLcdHwParam.LCD_Bus_Port, SPI_I2S_FLAG_BSY) != RESET)
    {}
    SPI_I2S_SendData(pLcdHwParam.LCD_Bus_Port, (LCD_RegValue & 0xFF));
    while (SPI_I2S_GetFlagStatus(pLcdHwParam.LCD_Bus_Port, SPI_I2S_FLAG_BSY) != RESET)
    {}

    GL_LCD_CtrlLinesWrite(pLcdHwParam.LCD_Ctrl_Port_NCS, pLcdHwParam.LCD_Ctrl_Pin_NCS, GL_HIGH);
  }
  else
  {
    /* Write 16-bit Index, then Write Reg */
    LCD->LCD_REG = LCD_Reg;
    /* Write 16-bit Reg */
    LCD->LCD_RAM = LCD_RegValue;
  }
}

/**
  * @brief  Writes to the LCD RAM.
  * @param  RGB_Code: the pixel color in RGB mode (5-6-5).
  * @retval None
  */
void GL_LCD_WriteRAM(uint16_t RGB_Code)
{
  if (pLcdHwParam.LCD_Connection_Mode == GL_SPI)
  {
    SPI_I2S_SendData(pLcdHwParam.LCD_Bus_Port, RGB_Code >> 8);
    while (SPI_I2S_GetFlagStatus(pLcdHwParam.LCD_Bus_Port, SPI_I2S_FLAG_BSY) != RESET)
    {}
    SPI_I2S_SendData(pLcdHwParam.LCD_Bus_Port, RGB_Code & 0xFF);
    while (SPI_I2S_GetFlagStatus(pLcdHwParam.LCD_Bus_Port, SPI_I2S_FLAG_BSY) != RESET)
    {}
  }
  else
  {
    /* Write 16-bit GRAM Reg */
    LCD->LCD_RAM = RGB_Code;
  }
}

/**
  * @brief  Power on the LCD.
  * @param  None
  * @retval None
  */
void GL_LCD_PowerOn(void)
{
  /* Power On sequence ---------------------------------------------------------*/
  GL_LCD_WriteReg(R16, 0x0000); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
  GL_LCD_WriteReg(R17, 0x0000); /* DC1[2:0], DC0[2:0], VC[2:0] */
  GL_LCD_WriteReg(R18, 0x0000); /* VREG1OUT voltage */
  GL_LCD_WriteReg(R19, 0x0000); /* VDV[4:0] for VCOM amplitude */
  GL_Delay(20);                 /* Dis-charge capacitor power voltage (200ms) */
  GL_LCD_WriteReg(R16, 0x17B0); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
  GL_LCD_WriteReg(R17, 0x0137); /* DC1[2:0], DC0[2:0], VC[2:0] */
  GL_Delay(5);                  /* Delay 50 ms */
  GL_LCD_WriteReg(R18, 0x0139); /* VREG1OUT voltage */
  GL_Delay(5);                  /* delay 50 ms */
  GL_LCD_WriteReg(R19, 0x1d00); /* VDV[4:0] for VCOM amplitude */
  GL_LCD_WriteReg(R41, 0x0013); /* VCM[4:0] for VCOMH */
  GL_Delay(5);                  /* delay 50 ms */
  GL_LCD_WriteReg(R7, 0x0173);  /* 262K color and display ON */
}

/**
  * @brief  Enables the Display.
  * @param  None
  * @retval None
  */
void GL_LCD_DisplayOn(void)
{
  /* Display On */
  GL_LCD_WriteReg(R7, 0x0173); /* 262K color and display ON */
}

/**
  * @brief  Disables the Display.
  * @param  None
  * @retval None
  */
void GL_LCD_DisplayOff(void)
{
  /* Display Off */
  GL_LCD_WriteReg(R7, 0x0);
}

/**
  * @brief  Configures LCD control lines in Output Push-Pull mode.
  * @param  None
  * @retval None
  */
void LCD_CtrlLinesConfigSPI(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable LCD_GPIO_DATA_PORT clock */
  GL_RCC_APBPeriphClockCmd(pLcdHwParam.LCD_Rcc_BusPeriph_GPIO_Ncs, GL_ENABLE, 2);

  /* Configure NCS (PB.02) in Output Push-Pull mode */
  GPIO_InitStructure.GPIO_Pin = pLcdHwParam.LCD_Ctrl_Pin_NCS;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GL_GPIO_Init(pLcdHwParam.LCD_Ctrl_Port_NCS, &GPIO_InitStructure);
}

/**
  * @brief  Configures LCD Control lines (FSMC Pins) in alternate function mode.
  * @param  None
  * @retval None
  */
void LCD_CtrlLinesConfigFSMC(void)
{

  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable FSMC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
  GL_RCC_AHBPeriphClockCmd( pLcdHwParam.LCD_Rcc_Bus_Periph, GL_ENABLE );
  GL_RCC_APBPeriphClockCmd( RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF |
                            RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO, GL_ENABLE, 2);

  /* Set PD.00(D2), PD.01(D3), PD.04(NOE), PD.05(NWE), PD.08(D13), PD.09(D14),
     PD.10(D15), PD.14(D0), PD.15(D1) as alternate function push pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
                                GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 |
                                GPIO_Pin_15;

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

  GL_GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* Set PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9), PE.13(D10),
     PE.14(D11), PE.15(D12) as alternate function push pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
                                GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
                                GPIO_Pin_15;

  GL_GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* Set PF.00(A0 (RS)) as alternate function push pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;

  GL_GPIO_Init(GPIOF, &GPIO_InitStructure);

  /* Set PG.12(NE4 (LCD/CS)) as alternate function push pull - CE3(LCD /CS) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;

  GL_GPIO_Init(GPIOG, &GPIO_InitStructure);
}

/**
  * @brief  Switches the backlight either ON or OFF
  *         Fuction added by Salvo Bonina - Systems LAB
  * @param  state: ON or OFF
  * @retval None
  */
void GL_LCD_BackLightSwitch(uint8_t u8_State)
{
  if (u8_State == OFF)
  { /* Turning OFF the LCD Backlight */
    LCD_DisplayOff();
  }
  else if (u8_State == ON)
  { /* Turning ON the LCD Backlight */
    LCD_DisplayOn();
  }
}

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
