/************************************************************
**  开关机电源管理
**  项目：多路PWM测量记录
**  作者：周吉
**  2018年5月22日
************************************************************/
#include <stdint.h>
#include <stdlib.h>

#include "Global.h"
/************************************************************
**  MARCO DEFINITION
************************************************************/

/************************************************************
**  EXTERN REFERENCE
************************************************************/
extern IWDG_HandleTypeDef hiwdg;

/************************************************************
**  VARIABLE DEFINITION
************************************************************/

/************************************************************
**  FUNCTION
************************************************************/
void POWER_On(void)
{
	uint32_t tcn;
	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
	/*Configure GPIO pins : PB3 */
	GPIO_InitStruct.Pin = GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	/*Configure GPIO pins : PD2 */
	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	  
	// start timer
    tcn = Time_GetCount();
    // pull power hold pin low
    POWER_HOLD_LOW();
    while(Time_GetCount() - tcn < 3000)      //  delay 3000ms
	{
		if(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2) == GPIO_PIN_SET)
		{
			tcn = Time_GetCount();
		}
	}
    //  power on 
    POWER_HOLD_HIGH();

    return;
}

void POWER_Off(void)
{
    //  callback a user function before power off device
    POWER_OffCallback();

    // power off
    POWER_HOLD_LOW();
    for(;;)
    {
		#if WATCHDOG_ENABLE
		HAL_IWDG_Refresh(&hiwdg);
		#endif
        //  do nothing
    }
}

/**
  * @brief  Power Off callback
  * @param  None
  * @retval None
  */
__weak void POWER_OffCallback(void)
{
	/* NOTE : This function Should not be modified, when the callback is needed,
            the POWER_OffCallback could be implemented in the user file
   */
}
