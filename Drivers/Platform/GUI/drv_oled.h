/*******************************************************
**  SSD1306 驱动
**  项目：多路PWM测量记录
**  作者：周吉
**  2018年5月21日
*******************************************************/
#ifndef __DRV_OLED_H__
#define __DRV_OLED_H__

/******************************************************
**  INCLUDES
******************************************************/
#include <stdint.h>
#include "GUI.h"

/**********************************************************
**	MARCO
**********************************************************/
//  OLED RST Pin
#define OLED_RST_PORT       GPIOC
#define OLED_RST_PIN        GPIO_PIN_4

#define OLED_RST_HIGH()     HAL_GPIO_WritePin(OLED_RST_PORT, OLED_RST_PIN, GPIO_PIN_SET)
#define OLED_RST_LOW()      HAL_GPIO_WritePin(OLED_RST_PORT, OLED_RST_PIN, GPIO_PIN_RESET)

//  OLED CS Pin
#define OLED_CS_PORT        GPIOB
#define OLED_CS_PIN         GPIO_PIN_0

#define OLED_CS_HIGH()      HAL_GPIO_WritePin(OLED_CS_PORT, OLED_CS_PIN, GPIO_PIN_SET)
#define OLED_CS_LOW()       HAL_GPIO_WritePin(OLED_CS_PORT, OLED_CS_PIN, GPIO_PIN_RESET)

//  OLED DC Pin
#define OLED_DC_PORT        GPIOC
#define OLED_DC_PIN         GPIO_PIN_5

#define OLED_DC_HIGH()      HAL_GPIO_WritePin(OLED_DC_PORT, OLED_DC_PIN, GPIO_PIN_SET)
#define OLED_DC_LOW()       HAL_GPIO_WritePin(OLED_DC_PORT, OLED_DC_PIN, GPIO_PIN_RESET)

//  OLED CLK Pin
#define OLED_CLK_PORT       GPIOA
#define OLED_CLK_PIN        GPIO_PIN_6

#define OLED_CLK_HIGH()     HAL_GPIO_WritePin(OLED_CLK_PORT, OLED_CLK_PIN, GPIO_PIN_SET)
#define OLED_CLK_LOW()      HAL_GPIO_WritePin(OLED_CLK_PORT, OLED_CLK_PIN, GPIO_PIN_RESET)

//  OLED DATA Pin
#define OLED_DATA_PORT      GPIOA
#define OLED_DATA_PIN       GPIO_PIN_7

#define OLED_DATA_HIGH()    HAL_GPIO_WritePin(OLED_DATA_PORT, OLED_DATA_PIN, GPIO_PIN_SET)
#define OLED_DATA_LOW()     HAL_GPIO_WritePin(OLED_DATA_PORT, OLED_DATA_PIN, GPIO_PIN_RESET)

/******************************************************
**  TYPE Definition
******************************************************/

/*  oled driver --------------------------------------------*/
struct graphic_driver_ops
{
    void (*displayOn)(void);
    void (*displayOff)(void);
    void (*displayUpdate)(void);
    void (*displayclear)(gui_color_t *c);
    void (*setPixel)(gui_color_t *c, int x, int y);
    void (*getPixel)(gui_color_t *c, int x, int y);
    void (*drawHLine)(gui_color_t *c, int x1, int x2, int y);
    void (*drawVLine)(gui_color_t *c, int x , int y1, int y2);
    void (*drawRawHLine)(const uint8_t *pixels, int x1, int x2, int y);
	void (*oled_drawBMP)(const uint8_t *pData, int x , int y, int width, int high);
};

// pixel format
#define COLOR_PIXEL_MONO    1

struct graphic_driver_info
{
    uint8_t pixel_format;               // pixel format
    uint8_t bits_per_pixel;             // pixel bits
    int width;
    int height;
};

struct OLED_Handle
{
    const struct graphic_driver_info * info;
    const struct graphic_driver_ops * ops;
};

const struct OLED_Handle *stm32_hw_oled_init(void);

#endif
/******************************END*********************/
