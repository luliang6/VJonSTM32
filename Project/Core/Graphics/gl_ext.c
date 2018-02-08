/**
  ******************************************************************************
  * @file    gl_ext.c 
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    06-September-2012
  * @brief   graphical library extension and other graphical components
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
#include "gl_ext.h"
#include "time_utils.h"
#include "mod_core.h"
#include "images.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static uint8_t Check_BMP_file(uint8_t *buf);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Add an icon view component on a page 
  * @param  pPage : current page
  * @param  id : component ID
  * @param  index :  
  * @param  icon : icon array pointer
  * @param  caption : Icon title
  * @param  mode : display mode (MODE_NORMAL or MODE_SELECT)
  * @param  pEventHandler : Icon event handler
  * @retval icon handler
  */
GL_PageControls_TypeDef *AddIconControlObj (GL_Page_TypeDef *pPage,
                                            uint8_t id,
                                            uint8_t index,
                                            uint8_t *icon, 
                                            uint8_t *caption,
                                            uint8_t mode,
                                            void (*pEventHandler)(void))
{
  uint16_t x = 0, y = 0 , offset = 0;

  GL_PageControls_TypeDef* Icon = NULL, *Name = NULL;


  if(Check_BMP_file (icon) == 0)
  {

    if(mode == 1)
    {
      /*calculate coordinate */
      x = 300 - (index % 4)  * (BF_XSIZE + 26);
      y = 42  + (index / 4)  * (BF_XSIZE + 13);  
    }
    else
    {
      /*calculate coordinate */
      x = 300 - ((index % 3) + 1)  * (BF_XSIZE + 26);
      y = 42  + (index / 3)  * (BF_XSIZE + 13);
    }
    
    /*Place icon in the right coordinate */
    Icon = NewIcon(id, icon, BF_XSIZE, BF_YSIZE ,pEventHandler );

    AddPageControlObj(x, y, Icon, pPage );

    /*Place module name in the right coordinate */
    Name = NewLabel(id + 1 , caption, GL_HORIZONTAL, GL_FONT_SMALL, GL_Black , GL_FALSE);
    offset = (7 - strlen((char *)caption)) * 4;
    AddPageControlObj(x - offset  ,y + BF_XSIZE - 1, Name, pPage );
  }
  return Icon;
}

/**
  * @brief  Add a small icon view component on a page 
  * @param  pPage : current page
  * @param  id : component ID
  * @param  icon : icon array pointer
  * @param  caption : Icon title
  * @param  mode : display mode (MODE_NORMAL or MODE_SELECT)
  * @param  pSelectHandler: Icon select handler
  * @param  pEventHandler : Icon event handler
  * @retval icon handler
  */
GL_PageControls_TypeDef *AddSmallIconControlObj (GL_Page_TypeDef *pPage,
                                                 uint8_t id, 
                                                 uint8_t *icon, 
                                                 uint8_t *caption,
                                                 uint8_t mode,
                                                 void (*pSelectHandler)(void),
                                                 void (*pEventHandler)(void))
{
  uint8_t temp[36];
  
  GL_PageControls_TypeDef* Icon, *Name;

  if (id > 8)
  {
   return NULL; 
  }
  /*Place icon in the right coordinate */
  Icon = NewIcon(id + 3, icon, 20, 20 ,pEventHandler );

  AddPageControlObj(310, 31  + id * (23) , Icon, pPage );

  /*Place module name in the right coordinate */
  strncpy((char *)temp, (char *)caption,35); 
  
  /* Adjust Caption */

  if (strlen((char *)caption) > 35)
  {
    (temp)[35 - 2] = '.';
    (temp)[35 - 1] = '.';
    (temp)[35] = '\0';
  }

  Name = NewLabel(id + 3 +  8, temp, GL_HORIZONTAL, GL_FONT_SMALL, GL_Black, GL_FALSE );
  AddPageControlObj(310 - 25  , 38  + id * (23), Name, pPage );

  return Icon;
}

/**
  * @brief  Add a scroll component on a page 
  * @param  pPage : current page
  * @param  x : x position
  * @param  y : y position
  * @param  yl: scroll height
  * @param  pUpEventHandler : up button ebvent handler
  * @param  pDownEventHandler : down button ebvent handler
  * @param  pos : 
  * @retval None
  */
void GL_AddScroll (GL_Page_TypeDef *pPage,
                   uint16_t x,
                   uint16_t y, 
                   uint16_t yl,
                   void (*pUpEventHandler)(void),
                   void (*pDwnEventHandler)(void),
                   uint8_t pos)
{
  GL_PageControls_TypeDef* Icon  ;

  Icon = NewIcon(0xFE, ArrowUpUntouched, 22, 22 ,pUpEventHandler );

  AddPageControlObj(319- x, y, Icon, pPage );
  GL_RefreshControlNow(Icon);

  Icon = NewIcon(0xFF, ArrowDownUntouched, 22, 22 ,pDwnEventHandler );

  AddPageControlObj(319- x, y + yl, Icon, pPage );
  GL_RefreshControlNow(Icon);

  LCD_SetTextColor(0x9CF5);
  GL_LCD_DrawRect( y + 22, 319 - x, yl-22, 22);

  LCD_SetTextColor(0xCF1F);   
  GL_LCD_FillRect( y + 23, 318 - x, yl-24, 20);

  LCD_SetTextColor(0x9CF5);
  GL_LCD_DrawRect( y + 23, 318 - x, yl-24, 20);

  LCD_SetTextColor(GL_Grey);
  GL_LCD_DrawRect( y + 24, 317 - x, yl-26, 18);

  GL_UpdateScrollPosition (pPage, x, y, yl, pos)  ;
}



/**
  * @brief  Update a scroll marker on a scroll bar 
  * @param  pPage : current page
  * @param  x : x position
  * @param  y : y position
  * @param  yl: scroll height
  * @param  position :   
  * @retval None
  */
void GL_UpdateScrollPosition (GL_Page_TypeDef *pPage,
                   uint16_t x,
                   uint16_t y, 
                   uint16_t yl,
                   uint8_t position)
{
  static uint8_t prev_pos = 0;
  uint16_t rel_pos = 0;
  uint16_t prev_rel_pos = 0;

  prev_rel_pos = ((100 - prev_pos) *  (yl - 30) )/ 100;
  rel_pos = ((100 - position) *  (yl - 30) )/ 100;

  if(prev_pos != position)
  {
    LCD_SetTextColor(0xCF1F);
    GL_LCD_FillRect( y - 5 + yl - prev_rel_pos, 316 - x, 3, 16); 
    prev_pos = position; 
  }
  LCD_SetTextColor(0x9CF5);
  GL_LCD_FillRect( y - 5 + yl - rel_pos, 316 - x, 3, 16);   
}
/**
  * @brief  Add a header on a page 
  * @param  pPage : current page
  * @param  caption : page title
  * @retval None
  */
void GL_SetPageHeader (GL_Page_TypeDef* pPage, 
                       uint8_t *caption)
{
  GL_PageControls_TypeDef* pCaptionObj;
  uint8_t tmpCaption[MAX_CAPTION_SIZE];

  GL_CenterCaption(tmpCaption, caption, MAX_CAPTION_SIZE);
  pCaptionObj = NewButton (0, tmpCaption, MOD_NullFunc);
  AddPageControlObj(315, 0, pCaptionObj, pPage);
}

/**
  * @brief  Add a menu item in the module menu in a page 
  * @param  pPage : current page
  * @param  caption : sub-menu title
  * @param  pEventHandler : sub-menu event handler
  * @retval None
  */
void GL_SetMenuItem (GL_Page_TypeDef* pPage, 
                     uint8_t *caption,
                     uint8_t index,
                     void (*pEventHandler)(void))
{
  GL_PageControls_TypeDef* pCaptionObj;
  uint8_t tmpCaption[25];

  GL_CenterCaption(tmpCaption, caption, 25);
  pCaptionObj = NewButton(1 + index, tmpCaption, pEventHandler);
  AddPageControlObj(250, 100 + index * 30, pCaptionObj, pPage);
}

/**
  * @brief  Refresh a component immediately 
  * @param  item : control component
  * @retval None
  */
void GL_RefreshControlNow (GL_PageControls_TypeDef* item)
{
  item->SetObjVisible(item ,item->objCoordinates);
}

/**
  * @brief  Set or reset a checkbox state
  * @param  pPage : current page
  * @param  id :   
  * @param  state : check box state
  * @retval None
  */
void GL_SetChecked (GL_Page_TypeDef* pPage , uint16_t id , uint8_t state)
{
  uint32_t index = 0;
  GL_Checkbox_TypeDef* pTmp;
  
  /* search for the required object */
  while ( index++ < pPage->ControlCount )
  {
    if (pPage->PageControls[index]->objType == GL_CHECKBOX)
    {

      pTmp = (GL_Checkbox_TypeDef*)(pPage->PageControls[index]->objPTR);

      if ( pTmp->ID == id )
      {
        pTmp->IsChecked = (GL_bool)state;
        return;
      }
    }
  }
}

/**
  * @brief  check wether a checkbox is set
  * @param  pPage : current page
  * @param  id : checkbox Id 
  * @retval state
  */
GL_bool GL_IsChecked (GL_Page_TypeDef* pPage , uint16_t id)
{
  uint32_t index = 0;
  GL_Checkbox_TypeDef* pTmp;

  /* search for the required object */
  while ( index++ < pPage->ControlCount )
  {
    if (pPage->PageControls[index]->objType == GL_CHECKBOX)
    {

      pTmp = (GL_Checkbox_TypeDef*)(pPage->PageControls[index]->objPTR);

      if ( pTmp->ID == id )
      {
        return pTmp->IsChecked;
      }
    }
  }
  return GL_FALSE;
}

/**
  * @brief  Refresh a label immediately
  * @param  pPage : current page
  * @param  id : checkbox Id 
  * @param  str : pointer to the new label string
  * @retval state
  */
GL_bool RefreshLabel (GL_Page_TypeDef* pPage , uint16_t id , uint8_t *str)
{
  Set_Label (pPage ,id , str);
  RefreshPageControl(pPage,id);

  return GL_TRUE;
}


/**
  * @brief  Refresh a label immediately
  * @param  pPage : current page
  * @param  id : checkbox Id 
  * @param  color : initial color code
  * @retval state
  */
GL_bool ChangeLabelColor (GL_Page_TypeDef* pPage , uint16_t id , uint16_t color)
{
  uint32_t index = 0;
  GL_Label_TypeDef* pTmp;
  
  /* search for the required object */
  while ( index++ < pPage->ControlCount )
  {
    if (pPage->PageControls[index]->objType == GL_LABEL)
    {

      pTmp = (GL_Label_TypeDef*)(pPage->PageControls[index]->objPTR);

      if ( pTmp->ID == id )
      {
        pTmp->Colour = color;
        RefreshPageControl(pPage,id);
        return GL_TRUE;
      }
    }
  }
  return GL_FALSE;
}
/**
  * @brief  check the bmp icon format
  * @param  buf : icon array
  * @retval state
  */
static uint8_t Check_BMP_file(uint8_t *buf)
{
  uint16_t  Type;
  uint32_t  Width;
  uint32_t  Height;
  uint16_t  Bpp;
  uint32_t  Compression ; 

  Type        = *(__packed uint16_t *)(buf + 0 );
  Width       = *(__packed uint32_t *)(buf + 18 );
  Height      = *(__packed uint32_t *)(buf + 22 );
  Bpp         = *(__packed uint16_t *)(buf + 28 );
  Compression = *(__packed uint32_t *)(buf + 30 );


  if(( Type != BF_TYPE)||
     ( Width != BF_XSIZE)||
     ( Height != BF_YSIZE)||
     ( Bpp != 16)||
     ( Compression != BI_BITFIELDS))
  {
    return 1;
  }

  return 0;
}

/**
  * @brief  Center a string in a buffer
  * @param  dBuf : destination buffer
  * @param  sBuf : source buffer 
  * @param  MaxLength : max string length
  * @retval None
  */
void GL_CenterCaption (uint8_t * dBuf, 
                       uint8_t * sBuf, 
                       uint32_t MaxLength)
{
  uint32_t tmp_length = 0 , idx = 0;

  tmp_length = min(p_strlen(sBuf), MaxLength - 1);

  for ( ; idx < MaxLength ; idx ++)
  {
    dBuf[idx] = ' ';
  }
  dBuf[idx-1] = 0;
  for ( idx = 0; idx < tmp_length ; idx ++)
  { 
    dBuf[idx + (MaxLength - tmp_length)/2 + 1] = sBuf[idx];
  }
}

/**
  * @brief  Invert color from RGB to BGR
  * @param  color : initial color code
  * @retval Inverted color code
  */
uint16_t GL_AdjustColor (uint16_t color)
{
  uint16_t r,g,b ,ret;

  r= (color)&0x3F;
  g= (color >> 5)&0x7F;
  b= (color >> 11)&0x3F;

  ret = (r << 11)| ( g << 5) | (b);

  return ret;
}

/**
  * @brief  Add a calendar componenet in a page
  * @param  pPage : current page
  * @param  id : calendar component id
  * @param  day : current day
  * @param  month : current month
  * @param  year : current year
  * @retval None
  */
void GL_AddCalendar (GL_Page_TypeDef* pPage, 
                     uint16_t id, 
                     uint8_t day,
                     uint8_t month,
                     uint16_t year)
{
  uint16_t idx , cnt = 1 , x , y;
  uint8_t offset, max ;

  uint8_t strDate [MAX_CAPTION_SIZE];
  uint8_t tempDate [MAX_CAPTION_SIZE];
  uint8_t temp[3];

  uint8_t strMonth[][12] = 
  {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",
  };

  sprintf ((char *)tempDate , "%s, %d",strMonth[month-1], year); 
  GL_CenterCaption (strDate, tempDate, MAX_CAPTION_SIZE);

  GL_PageControls_TypeDef* Mon;
  GL_PageControls_TypeDef* Tue;
  GL_PageControls_TypeDef* Wed;
  GL_PageControls_TypeDef* Thu;
  GL_PageControls_TypeDef* Fri;
  GL_PageControls_TypeDef* Sat;
  GL_PageControls_TypeDef* Sun;

  GL_PageControls_TypeDef* Day;  
  GL_PageControls_TypeDef* Month;  

  Mon= NewButton(id + 0, (uint8_t *)"Mon",MOD_NullFunc);
  Tue= NewButton(id + 1, (uint8_t *)"Tue",MOD_NullFunc);
  Wed= NewButton(id + 2, (uint8_t *)"Wed",MOD_NullFunc);
  Thu= NewButton(id + 3, (uint8_t *)"Thu",MOD_NullFunc);
  Fri= NewButton(id + 4, (uint8_t *)"Fri",MOD_NullFunc);
  Sat= NewButton(id + 5, (uint8_t *)"Sat",MOD_NullFunc);
  Sun= NewButton(id + 6, (uint8_t *)"Sun",MOD_NullFunc);

  AddPageControlObj(274, 59,Mon,pPage);
  AddPageControlObj(243, 59,Tue,pPage);
  AddPageControlObj(212, 59,Wed,pPage);
  AddPageControlObj(181, 59,Thu,pPage);
  AddPageControlObj(150, 59,Fri,pPage);
  AddPageControlObj(118, 59,Sat,pPage);
  AddPageControlObj( 87, 59,Sun,pPage);

  Month= NewLabel(id+ 7,strDate,GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_FALSE);
  AddPageControlObj(319, 42, Month, pPage);

  GetDateOffset (year, month, &offset , &max);

  /* draw component */
  for ( idx = offset  ; cnt <= max ; idx ++)
  {
    x = 274 -  (idx % 7) * 31;
    y = 90  +  (idx / 7) * 18;

    if(cnt < 10)
    {
      sprintf((char *)temp, "  %d",cnt);
    }
    else
    {
      sprintf((char *)temp, " %d",cnt);
    }
    if(cnt == day)
    {
      Day= NewLabel(id + 8 + cnt ,temp,GL_HORIZONTAL,GL_FONT_SMALL,GL_Red, GL_FALSE);
    }
    else
    {
      Day= NewLabel(id + 8 + cnt ,temp,GL_HORIZONTAL,GL_FONT_SMALL,GL_Black, GL_FALSE);
    }
    cnt++;
    AddPageControlObj(x, y, Day, pPage);
  }
}

/**
  * @brief  update a calendar componenet in a page
  * @param  pPage : current page
  * @param  id : calendar componenet id
  * @param  day : current day
  * @param  month : current month
  * @param  year : current year
  * @retval None
  */
void GL_UpdateCalendar (GL_Page_TypeDef* pPage, 
                        uint16_t id, 
                        uint8_t day,
                        uint8_t month,
                        uint16_t year)
{
  
  for (uint8_t idx = id ; idx < 50 + id ; idx ++)
  {
    DestroyPageControl (pPage, idx);
  }

  GL_AddCalendar (pPage, 
                  id, 
                  day,
                  month,
                  year);

  LCD_SetTextColor(GL_White);
  GL_LCD_FillRect(87,274,110,220 );
  
  for (uint8_t idx = id ; idx < 50 + id ; idx ++)
  {
    RefreshPageControl (pPage, idx);
  }

}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
