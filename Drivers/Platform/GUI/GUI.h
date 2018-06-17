/*******************************************************
**  GUI 显示驱动
**  项目：多路PWM测量记录
**  作者：周吉
**  2018年5月21日
*******************************************************/
#ifndef __GUI_H__
#define __GUI_H__

/*******************************************************
**  INCLUDES
********************************************************/
#include "stdint.h"

/*******************************************************
**  MARCO
********************************************************/
typedef enum 
{
	Default_fonts_6x8,
	Default_fonts_8x16,
    MS_Gothic_12x12_,
    MS_Gothic_12x12,
	Terminal_12x12_,
	Terminal_12x8_,
	Segoe_UI_Histore_12x12,
	Segoe_UI_Symbol_12x12,
	Small_Fonts_12x12,
	MS_Serif_12x12,
	Courier_16x12,
	Courier_16x16,
	HoloLens_MDI2_16x8,
	SongFonts_16X16,
}   font_types_t;

/*******************************************************
**  TYPE DEFINITION
********************************************************/

/*  pixels color -------------------------------------------*/
typedef uint8_t gui_color_t;



extern const unsigned char gImage_anhuang[];


//函数接口
void GUI_Init(void);
// Display off
void GUI_Off(void);
// clear display
void GUI_clearDisplay(void);
// clear window
void GUI_clearWindow(int xpos, int ypos, int x, int y);
// set fonts
void GUI_setFonts(font_types_t font);
// show char
void GUI_ShowChar(char ch, uint16_t xpos, uint16_t ypos);
// show string
void GUI_ShowString(const char *str, uint16_t xpos, uint16_t ypos);

void GUI_DrawBMP(const uint8_t *data, int xpos, int ypos, int width, int high);

void GUI_DrawHLine(int xpos, int ypos, int len);

void GUI_DrawVLine(int xpos, int ypos, int len);

#endif
/**************************END*************************/
