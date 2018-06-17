/*******************************************************
**  项目：多路PWM测量记录
**  作者：周吉
**  2018年5月21日
*******************************************************/

/******************************************************
**	INCLUDES
******************************************************/
#include <stdint.h>

#include "stm32f1xx_hal.h"

void delay_ms(uint32_t ms)
{
	uint32_t t = HAL_GetTick();
	
	while(HAL_GetTick() - t < ms);
}

void delay_us(uint32_t us)
{
    /* This function is not that accurate */
    uint32_t i = SystemCoreClock / 1000000 * us / 3;

    for(; i > 0; i--);
}
