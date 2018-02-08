/**
  ******************************************************************************
  * @file    LcdHal.c
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    31-July-2012
  * @brief   This file contains all the LCD functions whose
  *          implementation depends on the LCD Type used in your Application.
  *          You only need to change these functions implementations
  *          in order to reuse this code with other LCD
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
#include <stdlib.h>
#include "LcdHal.h"
#include "gl_fonts.h"


/** @addtogroup Embedded_GUI_Library
  * @{
  */

/** @defgroup LcdHal 
  * @brief LcdHal main functions
  * @{
  */ 

/** @defgroup LcdHal_Private_TypesDefinitions
  * @{
  */
typedef struct
{
  __IO uint16_t LCD_REG;
  __IO uint16_t LCD_RAM;
} GL_LCD_TypeDef;
/**
  * @}
  */

/** @defgroup LcdHal_Private_Defines
  * @{
  */
#define GL_LCD_BASE ((uint32_t)(0x60000000 | 0x08000000))


#define GL_LCD                ((GL_LCD_TypeDef *) GL_LCD_BASE)
#define RCC_AHBPeriph_FSMC    ((uint32_t)0x00000100)
/**
  * @}
  */

/** @defgroup LcdHal_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup LcdHal_Private_Variables
  * @{
  */

/* Private variables
   WARNING: If LcdHal functions are used by more Graphic Applications, remember that the
   following static variables will be shared among the several Graphic Objects applications
   ADD YOUR STATIC VARIABLES HERE
   ... */

/* Global variables to handle the right font */
__IO uint8_t GL_Font = GL_FONT_SMALL;
__IO uint8_t GL_FontWidth = GL_FONT_SMALL_WIDTH;
__IO uint8_t GL_FontHeight = GL_FONT_SMALL_HEIGHT;

uint16_t LCD_Height = 240;
uint16_t LCD_Width  = 320;
LCD_Direction_TypeDef LCD_Direction = _0_degree;

/* extern __IO uint16_t GL_TextColor, GL_BackColor; */
#if !defined(LCD_ILI9320)
  #define LCD_ILI9320 0x9320
#endif
#if !defined(LCD_SPFD5408)
  #define LCD_SPFD5408 0x5408
#endif
#if !defined(LCDType)
__IO uint32_t LCDType = LCD_ILI9320;
#else
  extern __IO uint32_t LCDType;
#endif

/**
  * @}
  */ 

/** @defgroup LcdHal_Private_FunctionPrototypes
  * @{
  */
static void BmpBuffer32BitRead(uint32_t* ptr32BitBuffer, uint8_t* ptrBitmap);

/**
  * @}
  */ 

/** @defgroup LcdHal_Private_Functions
  * @{
  */

/* Private functions
   ADD YOUR PRIVATE FUNCTIONS HERE */

/**
  * @brief  Sets the Text color.
  * @param  Color: specifies the Text color code RGB(5-6-5).
  * @param  GL_TextColor: Text color global variable used by GL_DrawChar
  *         and GL_DrawPicture functions.
  * @retval None
  */
void GL_SetTextColor(__IO uint16_t GL_TextColor)
{
  LCD_SetTextColor(GL_TextColor);
}

/**
  * @brief  Sets the Background color.
  * @param  Color: specifies the Background color code RGB(5-6-5).
  * @param  GL_BackColor: Background color global variable used by
  *         GL_DrawChar and GL_DrawPicture functions.
  * @retval None
  */
void GL_SetBackColor(__IO uint16_t GL_BackColor)
{
  LCD_SetBackColor(GL_BackColor);
}

/**
  * @brief  Clears the whole LCD.
  * @param  Color: specifies the Background color code RGB(5-6-5) for the Display.
  * @retval None
  */
void GL_Clear(uint16_t Color)
{
  LCD_Clear(Color);
}

/**
  * @brief  Draws a character on LCD without background.
  * @param  Xpos: the Line where to display the character shape.
  *         This parameter can be one of the following values:
  * @param  Ypos: start column address.
  * @param  c: pointer to the character data.
  * @retval None
  */
void GL_LCD_DrawCharTransparent(uint16_t Xpos, uint16_t Ypos, const uint16_t *c) /* 16bit char */
{
  uint32_t line_index = 0, pixel_index = 0;
  uint8_t Xaddress = 0;
  uint16_t Yaddress = 0;

  Xaddress = Xpos;
  Yaddress = Ypos;

  for (line_index = 0; line_index < GL_FontHeight; line_index++)
  {
    for (pixel_index = 0; pixel_index < GL_FontWidth; pixel_index++)
    {
      /* SmallFonts have bytes in reverse order */
      if (( GL_Font == GL_FONT_BIG   && (((const uint16_t*)c)[line_index] & (1 << pixel_index)) == 0x00) ||
          ( GL_Font == GL_FONT_SMALL && (((const uint16_t*)c)[line_index] & (0x80 >> pixel_index)) == 0x00))
      {
        Yaddress--;
      }
      else
      {
        LCD_PutPixel(Xaddress, Yaddress--, GL_TextColor, SinglePixel);
      }
    }
    Xaddress++;
    Yaddress = Ypos;
  }
}

/**
  * @brief  Displays a maximum of 20 char on the LCD.
  * @param  Line: the Line where to display the character shape.
  * @param  *ptr: pointer to the string to display on LCD.
  * @param  Transparent_Flag: if TRUE the character is printed without changing the background
  * @retval None
  */
void GL_DisplayAdjStringLine(uint16_t Line, uint16_t Column, uint8_t *ptr, GL_bool Transparent_Flag)
{
  uint32_t index = 0;
  uint32_t iMaxChar = (Column / (GL_FontWidth - 1));

  /* Send the string character by character on lCD */
  while ((*ptr != 0) & (index < iMaxChar))
  { /* Display one character on LCD */
    GL_LCD_DisplayChar(Line, Column, *ptr, Transparent_Flag);
    /* Decrement the column position by GL_FontWidth */
    if ( *ptr == 'A' || *ptr == 'G' || *ptr == 'M' || *ptr == 'O' || *ptr == 'Q' || *ptr == 'X' || *ptr == 'm')
      Column -= (GL_FontWidth);
    else
      Column -= (GL_FontWidth - 1);
    /* Point on the next character */
    ptr++;
    /* Increment the character counter */
    index++;
  }
}

/**
  * @brief  Displays one character (16dots width, 24dots height).
  * @param  Line: the Line where to display the character shape.
  *         This parameter can be one of the following values:
  *     @arg  - Linex: where x can be 0..9
  * @param  Column: start column address.
  * @param  Ascii: character ascii code, must be between 0x20 and 0x7E.
  * @param  GL_bool Trasparent_Flag, if TRUE it does not print the GL_BackColor
  * @retval None
  */
void GL_LCD_DisplayChar(uint16_t Line, uint16_t Column, uint8_t Ascii, GL_bool Transparent_Flag)
{
  Ascii -= 32;
  switch (GL_Font)
  {
    case GL_FONT_BIG:
      if (Transparent_Flag == GL_TRUE)
        GL_LCD_DrawCharTransparent(Line, Column, &GL_Font16x24.table[Ascii * GL_FontHeight] );
      else
        GL_LCD_DrawChar(Line, Column, (&GL_Font16x24.table[Ascii * GL_FontHeight]));
      break;

    case GL_FONT_SMALL:
      if (Transparent_Flag == GL_TRUE)
        GL_LCD_DrawCharTransparent(Line, Column, &GL_Font8x12_bold.table[Ascii * GL_FontHeight] );
      else
        GL_LCD_DrawChar(Line, Column, &GL_Font8x12_bold.table[Ascii * GL_FontHeight]);
      break;

    default:
      break;
  }
}

/**
  * @brief  Sets a display window
  * @param  Xpos: specifies the X bottom left position.
  * @param  Ypos: specifies the Y bottom left position.
  * @param  Height: display window height.
  * @param  Width: display window width.
  * @retval None
  */
void GL_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width)
{
  LCD_SetDisplayWindow(Xpos, Ypos, Height, Width);
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
void GL_DrawLine(uint16_t Ypos, uint16_t Xpos, uint16_t Length, uint8_t Direction)
{
  LCD_DrawLine(Ypos, Xpos, Length, Direction);
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
  GL_DrawLine(Xpos, Ypos, Width, Horizontal);
  GL_DrawLine((Xpos + Height), Ypos, Width, Horizontal);

  GL_DrawLine(Xpos, Ypos, Height, Vertical);
  GL_DrawLine(Xpos, (Ypos - Width + 1), Height, Vertical);
}

/**
  * @brief  Displays a Filled rectangle.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Height: display rectangle height.
  * @param  Width: display rectangle width.
  * @retval None
  */
void GL_LCD_FillRect(u16 Xpos, u16 Ypos, u16 Height, u16 Width )
{
  u32 i;
  for (i = 0 ; i < Height ; i++)
  {
    LCD_DrawLine(Xpos++, Ypos, Width , Horizontal);
  }
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
    LCD_SetCursor(Xpos + CurX, Ypos + CurY);
    LCD_WriteRAM(GL_TextColor);

    LCD_SetCursor(Xpos + CurX, Ypos - CurY);
    LCD_WriteRAM(GL_TextColor);

    LCD_SetCursor(Xpos - CurX, Ypos + CurY);
    LCD_WriteRAM(GL_TextColor);

    LCD_SetCursor(Xpos - CurX, Ypos - CurY);
    LCD_WriteRAM(GL_TextColor);

    LCD_SetCursor(Xpos + CurY, Ypos + CurX);
    LCD_WriteRAM(GL_TextColor);

    LCD_SetCursor(Xpos + CurY, Ypos - CurX);
    LCD_WriteRAM(GL_TextColor);

    LCD_SetCursor(Xpos - CurY, Ypos + CurX);
    LCD_WriteRAM(GL_TextColor);

    LCD_SetCursor(Xpos - CurY, Ypos - CurX);
    LCD_WriteRAM(GL_TextColor);

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
  * @brief  Displays a bitmap picture
  * @param  BmpAddress: Bmp picture address
  * @retval None
  */
void GL_DrawBMP(uint8_t* ptrBitmap)
{
  uint32_t uDataAddr = 0, uBmpSize = 0;
  uint16_t uBmpData;

  BmpBuffer32BitRead(&uBmpSize, ptrBitmap + 2);
  BmpBuffer32BitRead(&uDataAddr, ptrBitmap + 10);

  /* Set GRAM write direction and BGR = 1 */
  /* I/D=00 (Horizontal : decrement, Vertical : decrement) */
  /* AM=1 (address is updated in vertical writing direction) */
  LCD_WriteReg(R3, 0x1008);
  LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */

  /* Read bitmap data and write them to LCD */
  for (; uDataAddr < uBmpSize; uDataAddr += 2)
  {
    uBmpData = (uint16_t)(*(ptrBitmap + uDataAddr)) + (uint16_t)((*(ptrBitmap + uDataAddr + 1)) << 8);
    LCD_WriteRAM( uBmpData );
  }

  /* Set GRAM write direction and BGR = 1 */
  /* I/D = 01 (Horizontal : increment, Vertical : decrement) */
  /* AM = 1 (address is updated in vertical writing direction) */
  LCD_WriteReg(R3, 0x1018);
}

/**
  * @brief  Sets the Font (Big or Small).
  * @param  uFont: specifies the Font (GL_FONT_BIG or GL_FONT_SMALL).
  * @retval None
  */
void GL_SetFont(uint8_t uFont)
{
  GL_Font = uFont;

  switch (uFont)
  {
    case GL_FONT_BIG:
      GL_FontWidth  = GL_FONT_BIG_WIDTH;
      GL_FontHeight = GL_FONT_BIG_HEIGHT;
      break;

    case GL_FONT_SMALL:
      GL_FontWidth  = GL_FONT_SMALL_WIDTH;
      GL_FontHeight = GL_FONT_SMALL_HEIGHT;
      break;

    default:
      break;
  } /* End switch */
}

/**
  * @brief  Switches the backlight either ON or OFF
  * @param  state. This parameter can be one of the following values:
  *     @arg   GL_ON
  *     @arg   GL_OFF
  * @retval None
  */
void GL_BackLightSwitch(uint8_t u8_State)
{
  if (u8_State == GL_OFF)
  { /* Turning OFF the LCD Backlight */
    /* GPIO_ResetBits(LCD_GPIO_DATA_PORT, GPIO_Pin_4); */
    LCD_DisplayOff();
  }
  else if (u8_State == GL_ON)
  { /* Turning ON the LCD Backlight */
    /* GPIO_SetBits(LCD_GPIO_DATA_PORT, GPIO_Pin_4); */
    LCD_DisplayOn();
  }
}

/**
  * @brief  Initializes the LCD.
  * @param  None
  * @retval None
  */
void GL_LCD_Init(void)
{
#if defined (STM32F2XX)
  STM3220F_LCD_Init();  
#elif defined (STM32F4XX)  
  STM324xG_LCD_Init();
#endif  
}

/**
  * @brief  Disables LCD Window mode.
  * @param  None
  * @retval None
  */
void GL_LCD_WindowModeDisable(void)
{
  LCD_SetDisplayWindow(239, 0x13F, 240, 320);
  LCD_WriteReg(R3, 0x1018);
}

/**
  * @brief  Draw one pixel at position given by Xpos, Ypos of color Color.
  *     Three different modes are specified by PixelSpec in order to save time:
  *     1.FirstPixel, MiddlePixel and LasPixel are used for fastened block-writing
  *       to GRAM (for instance: drawing rectangle, horizontal line etc.)
  *     2.SinglePixel is used for drawing stand-alone pixel. (for instance:
  *       drawing circle, character etc.).
  * @param  Xpos: specifies X position
  * @param  Ypos: specifies Y position
  * @param  Color: RGB color of point
  * @param  PixelSpec: specifies Mode of putting pixel.
  *         This parameter can be one of the following values:
  *     @arg  - FirstPixel:  Starting pixel of block-writing sequence.
  *     @arg  - MiddlePixel: Middle-located pixel of block-writing sequence.
  *     @arg  - LasPixel:    Ending pixel of block-writing sequence
  *     @arg  - SinglePixel: Separated pixel.
  * @retval None
  */
void LCD_PutPixel(uint16_t Xpos, uint16_t Ypos, uint16_t Color, uint8_t PixelSpec)
{
  /*Start part of put pixel for first pixel of block and for single one.
  It consists of set the cursor's position and GRAM prepare sequence.*/
  if ((PixelSpec & (FirstPixel | SinglePixel)) != GL_RESET)
  {
    LCD_SetCursor(Xpos, Ypos);
    LCD_WriteRAM_Prepare();
    
  }

  /*Write pixel's color to GRAM. Obligatory for all modes of PutPixel call.*/
  LCD_WriteRAM(Color);
}

/**
  * @brief  Get color of pixel located at appropriate position
  * @param  Xpos: specifies X position
  * @param  Ypos: specifies Y position
  * @retval uint16_t - RGB color of required pixel.
  */
uint16_t LCD_GetPixel(uint16_t Xpos, uint16_t Ypos)
{
  uint16_t tmpColor = 0, tmpRed = 0, tmpBlue = 0;

  /*Read the color of given pixel*/
  LCD_SetCursor(Xpos , Ypos);
  
  tmpColor = GL_LCD_ReadRAM();
  tmpColor = GL_LCD_ReadRAM();
  tmpColor = GL_LCD_ReadRAM();    

  /*Swap the R and B color channels*/
  tmpRed = (tmpColor & Blue) << 11;
  tmpBlue = (tmpColor & Red) >> 11;
  tmpColor &= ~(Red | Blue);
  tmpColor |= (tmpRed | tmpBlue);

  return tmpColor;
}

/**
  * @brief  Draws a character on LCD.
  * @param  Xpos: the Line where to display the character shape.
  *         This parameter can be one of the following values:
  *     @arg  - Linex: where x can be 0..9
  * @param  Ypos: start column address.
  * @param  c: pointer to the character data.
  * @retval None
  */
void GL_LCD_DrawChar(uint8_t Xpos, uint16_t Ypos, const uint16_t *c) /* 16bit char */
{
  uint32_t line_index = 0, pixel_index = 0;
  uint8_t Xaddress = 0;
  uint16_t Yaddress = 0;
  __IO uint16_t tmp_color = 0;

  Xaddress = Xpos;
  Yaddress = Ypos;

  for (line_index = 0; line_index < GL_FontHeight; line_index++)
  {
    /* SmallFonts have bytes in reverse order */
    if (( GL_Font == GL_FONT_BIG   && (((const uint16_t*)c)[line_index] & (1 << 0)) == 0x00) ||
        ( GL_Font == GL_FONT_SMALL && (((const uint16_t*)c)[line_index] & (0x80 >> 0)) == 0x00))
    {
      tmp_color = GL_BackColor;
    }
    else
    {
      tmp_color = GL_TextColor;
    }

    LCD_PutPixel(Xaddress, Yaddress--, tmp_color, FirstPixel);

    for (pixel_index = 1; pixel_index < GL_FontWidth - 1; pixel_index++)
    {
      /* SmallFonts have bytes in reverse order */
      if (( GL_Font == GL_FONT_BIG   && (((const uint16_t*)c)[line_index] & (1 << pixel_index)) == 0x00) ||
          ( GL_Font == GL_FONT_SMALL && (((const uint16_t*)c)[line_index] & (0x80 >> pixel_index)) == 0x00))
      {
        tmp_color = GL_BackColor;
      }
      else
      {
        tmp_color = GL_TextColor;
      }

      LCD_PutPixel(Xaddress, Yaddress--, tmp_color, MiddlePixel);
    }
    pixel_index++;
    /* SmallFonts have bytes in reverse order */
    if (( GL_Font == GL_FONT_BIG   && (((const uint16_t*)c)[line_index] & (1 << pixel_index)) == 0x00) ||
        ( GL_Font == GL_FONT_SMALL && (((const uint16_t*)c)[line_index] & (0x80 >> pixel_index)) == 0x00))
    {
      tmp_color = GL_BackColor;
    }
    else
    {
      tmp_color = GL_TextColor;
    }

    LCD_PutPixel(Xaddress, Yaddress--, tmp_color, LastPixel);

    Xaddress++;
    Yaddress = Ypos;
  }
}

/**
  * @brief  Copy 4 bytes from bitmap array to 32Bit buffer
  * @param  ptrBitmap - Bitmap pointer
  * @param  ptr32BitBuffer - 32Bit buffer to fill
  * @retval None
  */
void BmpBuffer32BitRead(uint32_t* ptr32BitBuffer, uint8_t* ptrBitmap)
{
  *ptr32BitBuffer = 0;
  *ptr32BitBuffer = (*ptrBitmap);
  *ptr32BitBuffer += (*(ptrBitmap + 1)) << 8;
  *ptr32BitBuffer += (*(ptrBitmap + 2)) << 16;
  *ptr32BitBuffer += (*(ptrBitmap + 3)) << 24;
}

/**
  * @brief  Reads the LCD RAM.
  * @param  None
  * @retval uint16_t - LCD RAM Value.
  */
uint16_t GL_LCD_ReadRAM(void)
{
  return LCD_ReadRAM();
}

/**
  * @brief  LCD_Change_Direction
  * @param  RCC_APBPeriph: specifies the APB peripheral to gates its clock.
  * @param  Direction: The Drawing Direction
  *         This parameter can be one of the following values:
  *     @arg  _0_degree
  *     @arg  _90_degree
  *     @arg  _180_degree
  *     @arg  _270_degree
  * @retval None
  */
void LCD_Change_Direction(LCD_Direction_TypeDef Direction)
{
  LCD_Direction = Direction;

  if (LCD_Direction == _0_degree)
  {
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=01 (Horizontal : increment, Vertical : decrement) */
    /* AM=1 (address is updated in vertical writing direction) */
    LCD_WriteReg(R3, 0x1018);
  }
  else if (LCD_Direction == _90_degree)
  {
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=11 (Horizontal : increment, Vertical : increment) */
    /* AM=0 (address is updated in orizontal writing direction) */
    LCD_WriteReg(R3, 0x1030);
  }
  else if (LCD_Direction == _180_degree)
  {
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=10 (Horizontal : decrement, Vertical : increment) */
    /* AM=1 (address is updated in vertical writing direction) */
    LCD_WriteReg(R3, 0x1028);
  }
  else if (LCD_Direction == _270_degree)
  {
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=00 (Horizontal : decrement, Vertical : decrement) */
    /* AM=0 (address is updated in orizontal writing direction) */
    LCD_WriteReg(R3, 0x1000);
  }
}

/**
  * @brief  LCD_WriteChar
  * @param  Xpos: The X axis position
  * @param  Ypos: The Y axis position
  * @param  *c:   The Character pointer
  * @retval None
  */
void LCD_WriteChar(uint16_t Xpos, uint16_t Ypos, const uint16_t *c)
{
  uint32_t index = 0, counter = 0;
  uint16_t Xaddress = 0;
  uint16_t Yaddress = 0;

  if ((LCD_Direction == _0_degree) || (LCD_Direction == _180_degree))
  {
    Xaddress = Xpos;
    LCD_SetCursor(Xaddress, Ypos);
  }
  else if ((LCD_Direction == _90_degree) || (LCD_Direction == _270_degree))
  {
    Yaddress = Ypos;
    LCD_SetCursor(Xpos, Yaddress);
  }

  for (index = 0; index < GL_FontHeight; index++)
  {
    if ((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
    {
      LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
    }
    for (counter = 0; counter < GL_FontWidth; counter++)
    {
      /* SmallFonts have bytes in reverse order */
      if (( GL_Font == GL_FONT_BIG   && (((const uint16_t*)c)[index] & (1 << counter)) == 0x00) ||
          ( GL_Font == GL_FONT_SMALL && (c[index] & (0x80 >> counter)) == 0x00))
      {
        LCD_WriteRAM(GL_BackColor);
      }
      else
      {
        LCD_WriteRAM(GL_TextColor);
      }
    }

    if (LCD_Direction == _0_degree)
    {
      Xaddress++;
      LCD_SetCursor(Xaddress, Ypos);
    }
    else if (LCD_Direction == _90_degree)
    {
      Yaddress++;
      LCD_SetCursor(Xpos, Yaddress);
    }
    else if (LCD_Direction == _180_degree)
    {
      Xaddress--;
      LCD_SetCursor(Xaddress, Ypos);
    }
    else if (LCD_Direction == _270_degree)
    {
      Yaddress--;
      LCD_SetCursor(Xpos, Yaddress);
    }
  }
}

/**
  * @brief  LCD_PrintChar
  * @param  Line:   The X axis position
  * @param  Column: The Y axis position
  * @param  Ascii:  The Character pointer
  * @retval None
  */
void LCD_PrintChar(uint16_t Line, uint16_t Column, uint8_t Ascii)
{
  Ascii -= 32;
  switch (GL_Font)
  {
    case GL_FONT_BIG:
      LCD_WriteChar( Line, Column, &GL_Font16x24.table[Ascii * GL_FontHeight] );
      break;

    case GL_FONT_SMALL:
      LCD_WriteChar( Line, Column, &GL_Font8x12_bold.table[Ascii * GL_FontHeight] );
      break;

    default:
      break;
  }
}

/**
  * @brief  LCD_PrintStringLine
  * @param  Line:   The X axis position
  * @param  Column: The Y axis position
  * @param  *ptr:  The String of characters to be printed
  * @retval None
  */
void LCD_PrintStringLine(uint16_t Line, uint16_t Column, uint8_t *ptr)
{
  uint32_t counter = 0;
  uint16_t refcolumn = 0;
  uint8_t max_char_line = 0;
  static uint16_t line;

  line = Line;

  if (LCD_Direction == _0_degree)
  {
    max_char_line = (LCD_Width / GL_FontWidth);
    refcolumn = Column;
  }
  else if (LCD_Direction == _90_degree)
  {
    max_char_line = (LCD_Height / GL_FontWidth);
    refcolumn = Column;
  }
  else if (LCD_Direction == _180_degree)
  {
    max_char_line = (LCD_Width / GL_FontWidth);
    refcolumn = Column;
    line = LCD_Height - Line;
  }
  else if (LCD_Direction == _270_degree)
  {
    max_char_line = (LCD_Height / GL_FontWidth);
    refcolumn = Column;
    line = LCD_Width - Line;
  }

  /* Send the string character by character on LCD */
  while ((*ptr != 0) & (counter < max_char_line))
  {
    if (LCD_Direction == _0_degree)
    {
      /* Display one character on LCD */
      LCD_PrintChar(line, refcolumn, *ptr);
      /* Decrement the column position by 16 */
      refcolumn -= GL_FontWidth;
    }
    else if (LCD_Direction == _90_degree)
    {
      /* Display one character on LCD */
      LCD_PrintChar(refcolumn, line, *ptr);
      /* Increment the column position by 16 */
      refcolumn += GL_FontWidth;
    }
    else if (LCD_Direction == _180_degree)
    {
      /* Display one character on LCD */
      LCD_PrintChar(line, refcolumn, *ptr);
      /* Decrement the column position by 16 */
      refcolumn += GL_FontWidth;
    }
    else if (LCD_Direction == _270_degree)
    {
      /* Display one character on LCD */
      LCD_PrintChar(refcolumn, line, *ptr);
      /* Increment the column position by 16 */
      refcolumn -= GL_FontWidth;
    }

    /* Point on the next character */
    ptr++;
    /* Increment the character counter */
    counter++;
  }
}

/**
  * @brief  LCD_DrawMonoBMP
  * @param  *Pict:   The pointer to the image
  * @param  Xpos_Init: The X axis position
  * @param  Ypos_Init: The Y axis position
  * @param  Height:    The Height of the image
  * @param  Width:     The Width of the image
  * @retval None
  */
void LCD_DrawMonoBMP(const uint8_t *Pict, uint16_t Xpos_Init, uint16_t Ypos_Init, uint16_t Height, uint16_t Width)
{
  int32_t index = 0, counter = 0;


  if (LCD_Direction == _0_degree)
  {
    LCD_SetDisplayWindow(Xpos_Init, Ypos_Init, Height, Width);
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=00 (Horizontal : decrement, Vertical : decrement) */
    /* AM=1 (address is updated in vertical writing direction) */
    LCD_WriteReg(R3, 0x1008);
  }
  else if (LCD_Direction == _90_degree)
  {
    LCD_SetDisplayWindow(Xpos_Init + Width - 1, Ypos_Init, Width, Height);
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=01 (Horizontal : increment, Vertical : decrement) */
    /* AM=0 (address is updated in horizontal writing direction) */
    LCD_WriteReg(R3, 0x1010);
  }
  else if (LCD_Direction == _180_degree)
  {
    LCD_SetDisplayWindow(Xpos_Init + Height - 1, Ypos_Init + Width - 1, Height, Width);
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=11 (Horizontal : increment, Vertical : increment) */
    /* AM=1 (address is updated in vertical writing direction) */
    LCD_WriteReg(R3, 0x1038);
  }
  else if (LCD_Direction == _270_degree)
  {
    LCD_SetDisplayWindow(Xpos_Init, Ypos_Init + Height - 1, Width, Height);
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=10 (Horizontal : decrement, Vertical : increment) */
    /* AM=0 (address is updated in horizontal writing direction) */
    LCD_WriteReg(R3, 0x1020);
  }

  LCD_SetCursor(Xpos_Init, Ypos_Init);

  if ((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    /* Prepare to write GRAM */
    LCD_WriteRAM_Prepare(); 
  }

  for (index = 0; index < (Height*Width) / 8; index++)
  {
    for (counter = 7; counter >= 0; counter--)
    {
      if ((Pict[index] & (1 << counter)) == 0x00)
      {
        LCD_WriteRAM(GL_BackColor);
      }
      else
      {
        LCD_WriteRAM(GL_TextColor);
      }
    }
  }
  LCD_Change_Direction(LCD_Direction);
  GL_SetDisplayWindow(LCD_Height - 1, LCD_Width - 1, LCD_Height, LCD_Width);
}

/**
  * @brief  Fill area with color.
  * @param  maxX: Maximum X coordinate
  * @param  minX: Minimum X coordinate
  * @param  maxY: Maximum Y coordinate
  * @param  minY: Minimum Y coordinate
  * @param  ptrBitmap: pointer to the image
  * @retval None
  */
void LCD_FillArea(uint16_t Xpos_Init, uint16_t Ypos_Init, uint16_t Height, uint16_t Width, uint16_t color)
{
  uint32_t area = 0;
  uint32_t index = 0;

  area =  Width *  Height;
  if (LCD_Direction == _0_degree)
  {
    GL_SetDisplayWindow(Xpos_Init, Ypos_Init, Height, Width);
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=00 (Horizontal : decrement, Vertical : decrement) */
    /* AM=1 (address is updated in vertical writing direction) */
    LCD_WriteReg(R3, 0x1008);
  }
  else if (LCD_Direction == _90_degree)
  {
    GL_SetDisplayWindow(Xpos_Init + Width - 1, Ypos_Init, Width, Height);
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=01 (Horizontal : increment, Vertical : decrement) */
    /* AM=0 (address is updated in horizontal writing direction) */
    LCD_WriteReg(R3, 0x1010);
  }
  else if (LCD_Direction == _180_degree)
  {
    GL_SetDisplayWindow(Xpos_Init + Height - 1, Ypos_Init + Width - 1, Height, Width);
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=11 (Horizontal : increment, Vertical : increment) */
    /* AM=1 (address is updated in vertical writing direction) */
    LCD_WriteReg(R3, 0x1038);
  }
  else if (LCD_Direction == _270_degree)
  {
    GL_SetDisplayWindow(Xpos_Init, Ypos_Init + Height - 1, Width, Height);
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=10 (Horizontal : decrement, Vertical : increment) */
    /* AM=0 (address is updated in horizontal writing direction) */
    LCD_WriteReg(R3, 0x1020);
  }

  LCD_SetCursor(Xpos_Init, Ypos_Init);

  if ((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
  }

  for (index = 0;index < area; index++)
  {
    LCD_WriteRAM(color);
  }

  LCD_Change_Direction(LCD_Direction);
  GL_SetDisplayWindow(LCD_Height - 1, LCD_Width - 1, LCD_Height, LCD_Width);
}

/**
  * @brief  LCD_DrawColorBMP
  * @param  *ptrBitmap:   The pointer to the image
  * @param  Xpos_Init: The X axis position
  * @param  Ypos_Init: The Y axis position
  * @param  Height:    The Height of the image
  * @param  Width:     The Width of the image
  * @retval None
  */
void LCD_DrawColorBMP(uint8_t* ptrBitmap, uint16_t Xpos_Init, uint16_t Ypos_Init, uint16_t Height, uint16_t Width)
{
  uint32_t uDataAddr = 0, uBmpSize = 0;
  uint16_t uBmpData = 0;

  BmpBuffer32BitRead(&uBmpSize, ptrBitmap + 2);  
  BmpBuffer32BitRead(&uDataAddr, ptrBitmap + 10);  

  if (LCD_Direction == _0_degree)
  {
    GL_SetDisplayWindow(Xpos_Init, Ypos_Init, Height, Width);
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=00 (Horizontal : decrement, Vertical : decrement) */
    /* AM=1 (address is updated in vertical writing direction) */
    LCD_WriteReg(R3, 0x1008);
  }
  else if (LCD_Direction == _90_degree)
  {
    GL_SetDisplayWindow(Xpos_Init + Width - 1, Ypos_Init, Width, Height);
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=01 (Horizontal : increment, Vertical : decrement) */
    /* AM=0 (address is updated in horizontal writing direction) */
    LCD_WriteReg(R3, 0x1010);
  }
  else if (LCD_Direction == _180_degree)
  {
    GL_SetDisplayWindow(Xpos_Init + Height - 1, Ypos_Init + Width - 1, Height, Width);
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=11 (Horizontal : increment, Vertical : increment) */
    /* AM=1 (address is updated in vertical writing direction) */
    LCD_WriteReg(R3, 0x1038);
  }
  else if (LCD_Direction == _270_degree)
  {
    GL_SetDisplayWindow(Xpos_Init, Ypos_Init + Height - 1, Width, Height);
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=10 (Horizontal : decrement, Vertical : increment) */
    /* AM=0 (address is updated in horizontal writing direction) */
    LCD_WriteReg(R3, 0x1020);
  }

  LCD_SetCursor(Xpos_Init, Ypos_Init);

  if ((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    /* Prepare to write GRAM */
    LCD_WriteRAM_Prepare(); 
  }

  /* Read bitmap data and write them to LCD */
  for (; uDataAddr < uBmpSize; uDataAddr += 2)
  {
    uBmpData = (uint16_t)(*(ptrBitmap + uDataAddr)) + (uint16_t)((*(ptrBitmap + uDataAddr + 1)) << 8);
    LCD_WriteRAM( uBmpData );
  }
  LCD_Change_Direction(LCD_Direction);
  GL_SetDisplayWindow(LCD_Height - 1, LCD_Width - 1, LCD_Height, LCD_Width);
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
