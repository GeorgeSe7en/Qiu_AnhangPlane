/*******************************************************
**  GUI 显示驱动
**  项目：多路PWM测量记录
**  作者：周吉
**  2018年5月21日
*******************************************************/
#include "stdlib.h"

#include "drv_oled.h"
#include "font.h"
#include "GUI.h"

#ifndef __FONTS_INCLUDE_H__
#define __FONTS_INCLUDE_H__

#include "oledfont.h"
#include "Courier_16x12.h"
#include "Courier_16x16.h"
#include "HoloLens_MDI2_16x16.h"
#include "MS_Gothic_12x12.h"
#include "MS_Gothic_12x12_.h"
#include "MS_Serif_12x12.h"
#include "Segoe_UI_Histore_12x12.h"
#include "Segoe_UI_Symbol_12x12.h"
#include "Small_Fonts_12x12.h"
#include "SongFonts_12x12.h"
#include "SongFonts_16X16.h"
#include "Terminal_12x8_.h"
#include "Terminal_12x12_.h"

// chinese
#include "SongFonts_CH_16x16.h"

#endif
/***********************************************************
**  MARCO
***********************************************************/

/***********************************************************
**  Variable
***********************************************************/

// fonts info
typedef struct fonts_t
{
	const uint8_t width;
	const uint8_t high;
	const uint8_t *fonts;
}	fonts_t;

fonts_t fontsMap[] =
{
	//	font_widthxhigh_fonst
	{6, 8,  (uint8_t*)F6x8},
	{8, 16, (uint8_t*)F8X16},
	{6, 12, (uint8_t *)MS_Gothic_12x12__fonts},
	{6, 12, (uint8_t *)MS_Gothic_12x12_fonts},
	{6, 12, (uint8_t *)Terminal_12x12__fonts},
	{6, 8,  (uint8_t *)Terminal_12x8__fonts},
	{6, 12, (uint8_t *)Segoe_UI_Histore_12x12_fonts},
	{6, 12, (uint8_t *)Segoe_UI_Symbol_12x12_fonts},
	{6, 12, (uint8_t *)Small_Fonts_12x12_fonts},
	{6, 12, (uint8_t *)MS_Serif_12x12_fonts},
	{8, 12, (uint8_t *)Courier_16x12_fonts},
	{8, 16, (uint8_t *)Courier_16x16_fonts},
	{8, 8,  (uint8_t *)HoloLens_MDI2_16x8_fonts},
	{8, 16, (uint8_t *)SongFonts_16x16_fonts},
};

//
const static struct OLED_Handle *gui_handle=NULL;
static font_types_t fonts = Courier_16x12;
/************************************************************
**  FUNCTION
************************************************************/
// Initialize oled 
void GUI_Init(void)
{
    gui_handle = stm32_hw_oled_init();
}

// Display off
void GUI_Off(void)
{
    gui_handle->ops->displayOff();
}

// set fonts
void GUI_setFonts(font_types_t font)
{
    fonts = font;
}

// show char
void GUI_ShowChar(char ch, uint16_t xpos, uint16_t ypos)
{
    const uint8_t *pData;
    int chWidth, chHigh;
	
	if(ch < ' ')
	{
		return;
	}
	
	uint8_t index = ch - ' ';
	chWidth = fontsMap[fonts].width;
	chHigh = fontsMap[fonts].high;
	
	pData = fontsMap[fonts].fonts + (chHigh+7)/8*chWidth*index;
	gui_handle->ops->oled_drawBMP(pData, xpos, ypos, chWidth, chHigh);
	
	gui_handle->ops->displayUpdate();
	
	return;
}


// show string
void GUI_ShowChinese(const uint16_t wchar, uint16_t xpos, uint16_t ypos)
{
	const uint8_t *pdata;
	int i;
	
	for(i=0;SongFonts_CH_16x16_Index_Map[i] != CH_INDEX_MAP_END; i++)
	{
		if(wchar == SongFonts_CH_16x16_Index_Map[i])
		{
			pdata = (const uint8_t *)SontFonst_CH_16x16;
			pdata += i * 32;
			
			gui_handle->ops->oled_drawBMP(pdata, xpos, ypos, 16, 16);
			gui_handle->ops->displayUpdate();
			
			break;
		}
	}
	return ;
}

void GUI_ShowString(const char *str, uint16_t xpos, uint16_t ypos)
{
	uint16_t wchar;
	
	// find char
	while(*str != '\0')
	{
		wchar = (uint8_t )*str++;
		if(wchar > 0x80)		//chinese
		{
			if(*str == '\0')
			{
				break;
			}
			wchar <<= 8;
			wchar |= *str++;
			GUI_ShowChinese(wchar, xpos, ypos);
			xpos += 16;
		}
		else
		{
			GUI_ShowChar(wchar, xpos, ypos);
			xpos += fontsMap[fonts].width;
		}
	}
	return ;
}

void GUI_DrawBMP(const uint8_t *data, int xpos, int ypos, int width, int high)
{
	gui_handle->ops->oled_drawBMP(data, xpos, ypos, width, high);
	gui_handle->ops->displayUpdate();
}

void GUI_DrawHLine(int xpos, int ypos, int len)
{
	gui_color_t c = 1;
	gui_handle->ops->drawHLine(&c, xpos, xpos + len - 1, ypos);
	gui_handle->ops->displayUpdate();
}

void GUI_DrawVLine(int xpos, int ypos, int len)
{
	gui_color_t c=1;
	gui_handle->ops->drawVLine(&c, xpos, ypos, ypos + len -1);
	gui_handle->ops->displayUpdate();
}

void GUI_clearWindow(int xpos, int ypos, int x, int y)
{
	gui_color_t c=0;
	int i;
	
	for(i=0;i<y;i++)
	{
		gui_handle->ops->drawHLine(&c, xpos, xpos + x, ypos+i);
	}
	gui_handle->ops->displayUpdate();
}

// clear display
void GUI_clearDisplay(void)
{
	gui_color_t c = 0;
    gui_handle->ops->displayclear(&c);
	gui_handle->ops->displayUpdate();
	return;
}
