/************************************************************
**  开关机电源管理
**  项目：多路PWM测量记录
**  作者：周吉
**  2018年5月22日
************************************************************/

#ifndef __PIN_H__
#define __PIN_H__
/************************************************************
**  INCLUDE FILES
************************************************************/

/***********************************************************
**  MARCO DEFINITION
***********************************************************/
//  POWER PIN
#define POWER_HOLD_PORT     GPIOB
#define POWER_HOLD_PIN      GPIO_PIN_3

#define POWER_HOLD_HIGH()   HAL_GPIO_WritePin(POWER_HOLD_PORT, POWER_HOLD_PIN, GPIO_PIN_SET)
#define POWER_HOLD_LOW()    HAL_GPIO_WritePin(POWER_HOLD_PORT, POWER_HOLD_PIN, GPIO_PIN_RESET)
/**********************************************************/
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

#endif
/************************END********************/
