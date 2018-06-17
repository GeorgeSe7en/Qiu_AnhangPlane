/***************************************************************************//**
 * @addtogroup stm32
 * @{
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>

#include "stm32f1xx_hal.h"

#include "GUI.h"
#include "drv_oled.h"

extern void delay_ms(uint32_t ms);
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#ifdef STM32_OLED_DEBUG
#define oled_debug(format,args...)      stm32_printf(format, ##args)
#else
#define oled_debug(format,args...)
#endif

#define STM32_OLED_WIDTH    128u
#define STM32_OLED_HIGHT   64u

#define CMD_WR  0 
#define DATA_WR 1
#if 1
/* Private function prototypes -----------------------------------------------*/
static void oled_displayOn(void);
static void oled_displayOff(void);
static void oled_displayUpdate(void);
static void oled_displayClear(gui_color_t *c);
static void oled_setPixel(gui_color_t *c, int x, int y);
static void oled_getPixel(gui_color_t *c, int x, int y);
static void oled_drawHLine(gui_color_t *c, int x1, int x2, int y);
static void oled_drawVLine(gui_color_t *c, int x , int y1, int y2);
static void oled_drawRawHLine(const uint8_t *pixels, int x1, int x2, int y);
static void oled_drawBMP(const uint8_t *pData, int x , int y, int width, int high);
/* Private variables ---------------------------------------------------------*/
static const struct graphic_driver_ops oled_ops =
{
    oled_displayOn,
    oled_displayOff,
    oled_displayUpdate,
    oled_displayClear,

    oled_setPixel,
    oled_getPixel,
    oled_drawHLine,
    oled_drawVLine,
    oled_drawRawHLine,
	oled_drawBMP,
};

static const struct graphic_driver_info oled_info =
{
    COLOR_PIXEL_MONO,               // pixel format
    1,                              // pixel bits
    STM32_OLED_WIDTH,
    STM32_OLED_HIGHT,
};

/* Private functions ---------------------------------------------------------*/
//向SSD1106写入一个字节。
//dat:要写入的数据/命令
//cmd:数据/命令标志 0,表示命令;1,表示数据;
static inline void SSD1306_WR_Byte(uint8_t dat, uint8_t cmd)
{	
	uint8_t i;		

	if(cmd) 
    { OLED_DC_HIGH(); }
	else 
    { OLED_DC_LOW(); }		  
	OLED_CS_LOW();
	for(i=0;i<8;i++)
	{			  
		OLED_CLK_LOW();
		if(dat&0x80) 
        { OLED_DATA_HIGH();}
		else 
        { OLED_DATA_LOW();}
		OLED_CLK_HIGH();
		dat<<=1;   
	}				 		  
	OLED_CS_HIGH();
	OLED_DC_HIGH();  
} 

static void oled_writeCmd(const uint8_t data)
{
    SSD1306_WR_Byte(data, CMD_WR);
}

static void oled_writeLongCmd(const uint8_t *data, uint8_t size)
{
    uint8_t i;

    for (i = 0; i < size; i++)
    {
        SSD1306_WR_Byte(*data++, CMD_WR);
    }
}

static void oled_writeData(const uint8_t *data, uint8_t size)
{
    uint8_t i;

    for (i = 0; i < size; i++)
    {
        SSD1306_WR_Byte(*data++, DATA_WR);
    }
}

static int ssd1306_init(void)
{
    uint8_t data[2];

	OLED_RST_HIGH();
	delay_ms(100);
	OLED_RST_LOW();
	delay_ms(200);
	OLED_RST_HIGH(); 
	delay_ms(1);
    // Turn off panel
    oled_writeCmd(0xAE);

    // Set display clock
    data[0] = 0xD5;
    data[1] = 0x80;         // default
    oled_writeLongCmd(data, 2);
    // Set charge pump
    data[0] = 0x8D;
    data[1] = 0x14;         // enable
    oled_writeLongCmd(data, 2);
    // Set pre-charge period
    data[0] = 0xD9;
    data[1] = 0xF1;
    oled_writeLongCmd(data, 2);
    // Set Vcomh deselect level
    data[0] = 0xDB;
    data[1] = 0x30;         // 0x83 x Vcc
    oled_writeLongCmd(data, 2);
    // Set contrast
    data[0] = 0x81;
    data[1] = 0xEF;
    oled_writeLongCmd(data, 2);

    // Set memory addressing mode
    data[0] = 0x20;
    data[1] = 0x02;         // Page Address mode
    oled_writeLongCmd(data, 2);
    // Set segment remap
    oled_writeCmd(0xA1);    // colume 127 -> SEG0
    // Set normal display
    oled_writeCmd(0xA6);
    // Set multiplex ratio
    data[0] = 0xA8;
    data[1] = 0x3f;         // N = 64, default
    oled_writeLongCmd(data, 2);
    // Set COM output scan direction
    oled_writeCmd(0xC8);    // from COM[N-1] to COM0
    // Set COM pin
    data[0] = 0xDA;
    data[1] = 0x12;         // alternative, disable left/right remap, default
    oled_writeLongCmd(data, 2);
    // Set display offset
    data[0] = 0xD3;
    data[1] = 0x00;         // default
    oled_writeLongCmd(data, 2);
    // Set low column address
    oled_writeCmd(0x00);    // default
    // Set high column address
    oled_writeCmd(0x10);    // default
    // Set display start line
    oled_writeCmd(0x40);    // default

    // Turn on display
    oled_writeCmd(0xA4);
    // Turn on panel
    oled_writeCmd(0xAF);

	data[0] = 0x00;
    oled_displayClear((gui_color_t *)data);

    return 0;
}

/***************************************************************************
**  Base Graphic Operates
***************************************************************************/
// private variable
static uint8_t frame_buffer[(STM32_OLED_HIGHT+7)/8][STM32_OLED_WIDTH] = {0};

struct ui_updateArea_t
{
    uint8_t flag;
    uint8_t minRow;
    uint8_t maxRow;
};

//  8 pages
struct ui_updateArea_t oled_update[8] =
{
	{0, STM32_OLED_WIDTH, 0},
	{0, STM32_OLED_WIDTH, 0},
	{0, STM32_OLED_WIDTH, 0},
	{0, STM32_OLED_WIDTH, 0},
	{0, STM32_OLED_WIDTH, 0},
	{0, STM32_OLED_WIDTH, 0},
	{0, STM32_OLED_WIDTH, 0},
	{0, STM32_OLED_WIDTH, 0},
};

// set GUI update area
static void oled_setUpdateFlag(uint16_t col, uint16_t minRow, uint16_t maxRow)
{
    uint8_t page = col>>3;
    oled_update[page].flag = 1;
    if(oled_update[page].minRow > minRow) oled_update[page].minRow = minRow;
    if(oled_update[page].maxRow < maxRow) oled_update[page].maxRow = maxRow;
}
/***************************************************************************//**
 * @brief
 *   Get the color of a pixel
 *
 * @details
 *
 * @note
 *
 * @param[out] c
 *  Pointer to color
 *
 * @param[in] x
 *  Horizontal position
 *
 * @param[in] y
 *  Vertical position
 ******************************************************************************/
static void oled_getPixel(gui_color_t *c, int x, int y)
{
    if ((x >= STM32_OLED_WIDTH) || (y >= STM32_OLED_HIGHT))
    {
        return;
    }

    // Set column address 
    // Set page address

    if (frame_buffer[y>>3][x] & (1 << (y & 0x7)))
    {
        *(uint8_t *)c = 0x01;
    }
    else
    {
        *(uint8_t *)c = 0x00;
    }
    return;
}

/***************************************************************************//**
 * @brief
 *   Draw a pixel with specified color
 *
 * @details
 *
 * @note
 *
 * @param[in] c
 *  Pointer to color
 *
 * @param[in] x
 *  Horizontal position
 *
 * @param[in] y
 *  Vertical position
 ******************************************************************************/
static void oled_setPixel(gui_color_t *c, int x, int y)
{
    if ((x >= STM32_OLED_WIDTH) || (y >= STM32_OLED_HIGHT))
    {
        return;
    }

    // Set column address 
    // Set page address

    if (*(uint8_t *)c)
    {
        frame_buffer[y>>3][x] |= 1 << (y & 0x7);
    }
    else
    {
        frame_buffer[y>>3][x] &= ~(1 << (y & 0x7));
    }

    // set update flag
    oled_setUpdateFlag(y, x, x);
    return;
}

/***************************************************************************//**
 * @brief
 *   Draw a horizontal line with raw color
 *
 * @details
 *
 * @note
 *
 * @param[in] pixels
 *  Pointer to raw color
 *
 * @param[in] x1
 *  Horizontal start position
 *
 * @param[in] x2
 *  Horizontal end position
 *
 * @param[in] y
 *  Vertical position
 ******************************************************************************/
static void oled_drawRawHLine(const uint8_t *pixels, int x1, int x2, int y)
{
    uint8_t *color, ycolor;
    uint32_t i;

    if ((x1 >= STM32_OLED_WIDTH) || (y >= STM32_OLED_HIGHT))
    {
        return;
    }
    if (x2 >= STM32_OLED_WIDTH)
    {
        x2 = STM32_OLED_WIDTH - 1;
    }

    // Set column address
    // Set page address
    color = &frame_buffer[y>>3][x1];
    
    ycolor = 1 << (y & 0x07);
    for (i = 0; i < x2 - x1; i++)
    {
        if (*pixels++)
        {
            color[i] |= ycolor;
        }
        else
        {
            color[i] &= ~ycolor;
        }
    }
    
    // set update flag
    oled_setUpdateFlag(y, x1, x2);

    oled_debug("rawH (%d-%d, %d) %x\n", x1, x2, y, *pixels);
}

/***************************************************************************//**
 * @brief
 *   Draw a horizontal line with specified color
 *
 * @details
 *
 * @note
 *
 * @param[in] c
 *  Pointer to color
 *
 * @param[in] x1
 *  Horizontal start position
 *
 * @param[in] x2
 *  Horizontal end position
 *
 * @param[in] y
 *  Vertical position
 ******************************************************************************/
static void oled_drawHLine(gui_color_t *c, int x1, int x2, int y)
{
    uint8_t *color, ycolor;
    uint32_t i;

    if ((x1 >= STM32_OLED_WIDTH) || (y >= STM32_OLED_HIGHT))
    {
        return;
    }
    if (x2 >= STM32_OLED_WIDTH)
    {
        x2 = STM32_OLED_WIDTH - 1;
    }

    // Set column address
    // Set page address
    color = &frame_buffer[y>>3][x1];

    ycolor = 1 << (y & 0x07);
    if (*(uint8_t *)c)
    {
        for (i = 0; i < x2 - x1; i++)
        {
            color[i] |= ycolor;
        }
    }
    else
    {
        for (i = 0; i < x2 - x1; i++)
        {
            color[i] &= ~ycolor;
        }
    }
    
    // set update flag
    oled_setUpdateFlag(y, x1, x2);
}

/***************************************************************************//**
 * @brief
 *   Draw a vertical line with specified color
 *
 * @details
 *
 * @note
 *
 * @param[in] c
 *  Pointer to color
 *
 * @param[in] x
 *  Horizontal position
 *
 * @param[in] y1
 *  Vertical start position
 *
 * @param[in] y2
 *  Vertical end position
 ******************************************************************************/
static void oled_drawVLine(gui_color_t *c, int x , int y1, int y2)
{
    uint8_t *color;
    uint8_t tcolor[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}, index;
    uint32_t i;

    if ((x >= STM32_OLED_WIDTH) || (y1 >= STM32_OLED_HIGHT))
    {
        return;
    }
    if (y2 >= STM32_OLED_HIGHT)
    {
        y2 = STM32_OLED_HIGHT - 1;
    }

        // Set column address
    // Set page address
    color = &frame_buffer[y1>>3][x];

    if (*(uint8_t *)c)
    {
        oled_setUpdateFlag(y1, x, x);
        for (i = y1; i <= y2; i++)
        {
            index = i & 0x07;
            if(0 == index)
            {
                color = &frame_buffer[i>>3][x];
                oled_setUpdateFlag(i, x, x);
            }
            color[0] |= tcolor[index];
        }
    }
    else
    {
        oled_setUpdateFlag(y1, x, x);
        for (i = y1; i <= y2; i++)
        {
            index = i & 0x07;
            if(0 == index)
            {
                color = &frame_buffer[i>>3][x];
                oled_setUpdateFlag(i, x, x);
            }
            color[0] &= ~tcolor[index];
        }
    }

    oled_debug(" VLine (%d, %d-%d) %x\n", x, y1, y2, *(uint8_t *)c);
}


/***************************************************************************//**
 * @brief
 *   Draw a vertical line with specified color
 *
 * @details
 *
 * @note
 *
 * @param[in] c
 *  Pointer to color
 *
 * @param[in] x
 *  Horizontal position
 *
 * @param[in] y1
 *  Vertical start position
 *
 * @param[in] y2
 *  Vertical end position
 ******************************************************************************/
static void oled_drawBMP(const uint8_t *pData, int x , int y, int width, int high)
{
	#if 0
	const uint8_t *data;
    uint8_t *color, ptr;
	uint16_t tColor;

	uint8_t lMsk[8] = {0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF};
	uint8_t hMsk[8] = {0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80};
    uint32_t i, j, wIndex, hIndex;
	#else
	const uint8_t *data;
    uint8_t *color;
	uint8_t tcolor[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
	uint32_t i, j;
	#endif
    if ((x >= STM32_OLED_WIDTH) || (y >= STM32_OLED_HIGHT))
    {
        return;
    }
    // Set column address
    // Set page address
	#if 0
    color = &frame_buffer[y>>3][x];
	for(hIndex = y; hIndex<(y+high) && hIndex < STM32_OLED_HIGHT; hIndex += 8)
	{
		color = &frame_buffer[hIndex>>3][x];
		data = &pData[((hIndex - y)>>3)*high];
		if((hIndex&0x07) == 0)
		{
			if(high + y - hIndex >= 8)
			{
				for(i=0;i < width && i + x < STM32_OLED_WIDTH;i++)
				{
					*color++ = *data++;
				}
			}
			else
			{
				for(i=0;i < width && i + x < STM32_OLED_WIDTH;i++)
				{
					*color = (*color & ~lMsk[hIndex&0x07]) | (*data & lMsk[hIndex&0x07]);
					color++;
					data++;
				}
			}
			// set update flag
			oled_setUpdateFlag(hIndex, x, i+x-1);
		}
		else if(high + y - hIndex  >= 8)
		{
			for(i=0;i < width && i + x < STM32_OLED_WIDTH; i++)
			{
				*color = (*color & ~hMsk[hIndex&0x07]) | (*data << (hIndex&0x07));
				color++;
				data++;
			}
			// set update flag
			oled_setUpdateFlag(hIndex, x, i+x-1);
			
			color = &frame_buffer[(hIndex>>3) + 1][x];
			data = &pData[((hIndex - y)>>3)*high];
			for(i=0;i < width && i + x < STM32_OLED_WIDTH;i++)
			{
				*color = (*color & ~lMsk[8-(hIndex&0x07)]) | (*data >> (8-(hIndex&0x07)));
				color++;
				data++;
			}
			// set update flag
			oled_setUpdateFlag(hIndex+8, x, i+x-1);
		}
		else if(high + y - hIndex + (hIndex&0x07) >= 8)
		{
			for(i=0;i < width && i + x < STM32_OLED_WIDTH; i++)
			{
				*color = (*color & ~hMsk[hIndex&0x07]) | (*data << (hIndex&0x07));
				color++;
				data++;
			}
			// set update flag
			oled_setUpdateFlag(hIndex, x, i+x-1);
			
			color = &frame_buffer[(hIndex>>3) + 1][x];
			data = &pData[((hIndex - y)>>3)*high];
			for(i=0;i < width && i + x < STM32_OLED_WIDTH;i++)
			{
				*color = (*color & ~lMsk[high + y - hIndex + (hIndex&0x07) - 8]) | ((*data >> (8-(hIndex&0x07)))&(lMsk[high + y - hIndex + (hIndex&0x07) - 8]));
				color++;
				data++;
			}
			// set update flag
			oled_setUpdateFlag(hIndex+1, x, i+x-1);
		}
		else
		{
			for(i=0;i < width  && i + x < STM32_OLED_WIDTH; i++)
			{
				tColor = (uint16_t)*color << (8-(hIndex&7) - (high + y - hIndex));
				tColor &= 0xFF00;
				tColor |= *data << (8-(high + y - hIndex));
				tColor <<= 8-(high + y - hIndex);
				tColor |= *color << (8-(hIndex&7));
				*color = tColor >> (8-(hIndex&7));
				color++;
				data++;
			}
			// set update flag
			oled_setUpdateFlag(hIndex, x, i+x-1);
		}
	}
	#else
	// second mode
	for(i=y;i-y<high && i<STM32_OLED_HIGHT;i++)
	{
		color = &frame_buffer[i>>3][x];
		data = &pData[((i-y)>>3)*width];
		for(j=x;j-x<width && j<STM32_OLED_WIDTH;j++)
		{
			if(*data & tcolor[(i-y)&7])
			{
				*color |= tcolor[i&7];
			}
			else
			{
				*color &= ~tcolor[i&7];
			}
			color++;
			data++;
		}
		// set update flag
		oled_setUpdateFlag(i, x, j-1);
	}
	#endif
}

/***************************************************************************//**
 * @brief
 *   Draw a vertical line with specified color
 *
 * @details
 *
 * @note
 *
 * @param[in] c
 *  Pointer to color
 *
 * @param[in] x
 *  Horizontal position
 *
 * @param[in] y1
 *  Vertical start position
 *
 * @param[in] y2
 *  Vertical end position
 ******************************************************************************/
static void oled_displayClear(gui_color_t *c)
{
    uint8_t tcolor[] = {0xFF, 0x00};
    uint32_t i, j;

    if (*(uint8_t *)c)
    {
        for(i=0;i<STM32_OLED_HIGHT>>3;i++)
        {
            oled_writeCmd(0x00 + 0);        // Lower Column Start Address
            oled_writeCmd(0x10 + 0);        // Higher Column Start Address
            oled_writeCmd(0xB0 + i);        // page address
            for(j=0;j<STM32_OLED_WIDTH;j++)
            {
                frame_buffer[i][j] = tcolor[0];
                oled_writeData(&tcolor[0], 1);
            }
			// clear update flag
			oled_update[i].flag = 0;
			oled_update[i].minRow = STM32_OLED_WIDTH;
			oled_update[i].maxRow = 0;
        }
    }
    else
    {
        for(i=0;i<STM32_OLED_HIGHT>>3;i++)
        {
            oled_writeCmd(0x00 + 0);        // Lower Column Start Address
            oled_writeCmd(0x10 + 0);        // Higher Column Start Address
            oled_writeCmd(0xB0 + i);        // page address
            for(j=0;j<STM32_OLED_WIDTH;j++)
            {
                frame_buffer[i][j] = tcolor[1];
                oled_writeData(&tcolor[1], 1);
				
            }
			// clear update flag
			oled_update[i].flag = 0;
			oled_update[i].minRow = STM32_OLED_WIDTH;
			oled_update[i].maxRow = 0;
        }
    }

    return;
}

/***************************************************************************//**
 * @brief
 *   Open OLED device
 *
 * @details
 *
 * @note
 *
 * @param[in] dev
 *   Pointer to device descriptor
 *
 * @param[in] oflag
 *   Device open flag
 *
 * @return
 *   Error code
 ******************************************************************************/
static void oled_displayOn(void)
{
    //SET DCDC命令 //DCDC ON //DISPLAY ON
    uint8_t cmdbuf[] = {0x8D, 0x14, 0xAF};
    
    oled_writeLongCmd(cmdbuf, 3);
}

/***************************************************************************//**
 * @brief
 *   Close OLED device
 *
 * @details
 *
 * @note
 *
 * @param[in] dev
 *   Pointer to device descriptor
 *
 * @return
 *   Error code
 ******************************************************************************/
static void oled_displayOff(void)
{
    //SET DCDC命令 //DCDC OFF //DISPLAY OFF
    uint8_t cmdbuf[] = {0x8D, 0x10, 0xAE};
    
    oled_writeLongCmd(cmdbuf, 3);
}

static void oled_displayUpdate(void)
{
    uint8_t page;

    for(page=0;page<8;page++)
    {
        if(oled_update[page].flag)      // 非0，当前页有更新
        {
            // set page start address
            oled_writeCmd(0xB0 + page);

            // set lower column start address
            oled_writeCmd(oled_update[page].minRow&0x0F);

            // set higher column start address
            oled_writeCmd((oled_update[page].minRow>>4)|0x10);

            // update display data
            oled_writeData(&frame_buffer[page][oled_update[page].minRow], 
                            oled_update[page].maxRow - oled_update[page].minRow + 1);
			
			// clear update flag
			oled_update[page].flag = 0;
			oled_update[page].minRow = STM32_OLED_WIDTH;
			oled_update[page].maxRow = 0;
        }
    }
}

/***************************************************************************//**
 * @brief
 *   Initialize OLED device
 *
 * @details
 *
 * @note
 *
 ******************************************************************************/
const struct OLED_Handle *stm32_hw_oled_init(void)
{
    static struct OLED_Handle handle;

    /* Init ssd1306 */
    ssd1306_init();
    oled_debug("OLED: H/W init OK!\n");

    handle.info = &oled_info;
    handle.ops = &oled_ops;

    return &handle;
}

#endif
 /* defined(stm32_USING_OLED) */
/***************************************************************************//**
 * @}
 ******************************************************************************/
