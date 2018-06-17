#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


/******************************************************
**	Includes
******************************************************/
#include "stm32f1xx_hal.h"
#include "Global.h"
/******************************************************
**	Extern Definition
******************************************************/
extern UART_HandleTypeDef huart3;

/******************************************************
**	Variable
******************************************************/
static char print_buffer[PRINT_BUF_LEN];

/******************************************************
**	Function
******************************************************/
int uart_printf(const char *fmt, ...)
{
	va_list arg_ptr;
	int num;
	
	// arg start
	va_start(arg_ptr, fmt);
	num = vsnprintf(print_buffer, PRINT_BUF_LEN, fmt, arg_ptr);
	while(uart_printStatus()) __NOP();
	while(HAL_OK != HAL_UART_Transmit_IT(&huart3, (uint8_t *)print_buffer, num));
	// arg end
	va_end(arg_ptr);
	
	return num;
}

int uart_print(const char *str)
{
	uint32_t num;

	num = strlen(str);
	while(uart_printStatus()) __NOP();
	while(HAL_OK != HAL_UART_Transmit_IT(&huart3, (uint8_t *)str, num));
	return num;
}

int uart_send(const uint8_t *dat, uint32_t len)
{
	while(uart_printStatus()) __NOP();
	while(HAL_OK != HAL_UART_Transmit_IT(&huart3, (uint8_t *)dat, len));
	return len;
}

int uart_printStatus(void)
{
	if((HAL_UART_GetState(&huart3) & HAL_UART_STATE_BUSY_TX) == HAL_UART_STATE_BUSY_TX)
	{
		return 1;
	}
	return 0;
}
