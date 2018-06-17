/***********************************************************
**	Uart.c
**	Author: Ji Zhou
**	Time:	2018.5.31
***********************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "stm32f1xx_hal.h"
#include "Global.h"

/*! Macro ------------------------------------------------*/
#define GPS_RECIVE_BUF_NUMS		16u

/**********************************************************
**	Extern Variable
***********************************************************/
extern UART_HandleTypeDef huart4;		// gps
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;		// debug uart

/**********************************************************
**	private variable
***********************************************************/
//gps
static uint8_t gps_recive[GPS_RECIVE_BUF_NUMS][GPS_RECIVE_BUF_LEN];
static uint8_t gps_recive_flag = 0, gps_recive_cnt = 0;

//cmd uart
static uint8_t cmd_recive[CMD_BUF_LEN];
static uint8_t cmd_recive_flag = 0;

//send log txt
static uint8_t log_send_flag = 0;
/*********************************************************
**	Function
**********************************************************/
// uart error callback
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart3)
	{	
		cmd_recive_flag |= 0x80;
	}
	
	if(huart == &huart4)
	{	
		HAL_UART_Receive_IT(huart, gps_recive[gps_recive_cnt], GPS_RECIVE_BUF_LEN);
	}
}

// user uart IT call back
// TX
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart3)
	{
		//send log
		if(log_send_flag)
		{
		}
	}
}

// RX
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart4)
	{	
		//gps
		if(++gps_recive_cnt == GPS_RECIVE_BUF_NUMS) gps_recive_cnt = 0;
		gps_recive_flag |= 0x01;
		HAL_UART_Receive_IT(huart, gps_recive[gps_recive_cnt], GPS_RECIVE_BUF_LEN);
	}
	else if(huart == &huart3)
	{
		// cmd
		cmd_recive_flag |= 0x01;
		//HAL_UART_Receive_IT(huart, cmd_recive[cmd_recive_cnt], CMD_BUF_LEN);
	}
}

// recive data init
void uart_rx_init(void)
{
	HAL_UART_Receive_IT(&huart3, cmd_recive, CMD_BUF_LEN);
	HAL_UART_Receive_IT(&huart4, gps_recive[gps_recive_cnt], GPS_RECIVE_BUF_LEN);
}

// recive data processing
void uart_rx_processing(void)
{
	if(gps_recive_flag & 0x01)
	{
		if(huart1.gState == HAL_UART_STATE_READY)
		{
			HAL_UART_Transmit_IT(&huart1, gps_recive[gps_recive_cnt ? gps_recive_cnt - 1 : GPS_RECIVE_BUF_NUMS - 1], GPS_RECIVE_BUF_LEN);
			gps_recive_flag &= ~0x01;
		}
	}
}

int get_LineFromGPSMsg(char *buf)
{
	static uint8_t *ptr = gps_recive[0];
	uint8_t *tptr = ptr, flag = 0;
	uint8_t *const endPtr = huart4.pRxBuffPtr;

	while(tptr != endPtr)
	{
		if(flag)
		{
			if(*tptr != '\r' && *tptr != '\n')
			{
				*buf++ = *tptr;
				if(tptr == &gps_recive[GPS_RECIVE_BUF_NUMS -1][GPS_RECIVE_BUF_LEN - 1])
				{
					tptr = gps_recive[0];
				}
				else
				{
					tptr++;
				}
			}
			else
			{
				*buf = '\0';
				ptr = tptr;
				return 1;
			}
		}
		else
		{
			if(*tptr != '\r' && *tptr != '\n')
			{
				*buf++ = *tptr;
				flag = 1;
			}
			if(tptr == &gps_recive[GPS_RECIVE_BUF_NUMS - 1][GPS_RECIVE_BUF_LEN - 1])
			{
				tptr = gps_recive[0];
			}
			else
			{
				tptr++;
			}
		}
	}
	return 0;
}

// cmd RX
void CMD_receive(void)
{
	HAL_UART_Receive_IT(&huart3, cmd_recive, CMD_BUF_LEN);
}

void CMD_SendRXError(void)
{
	while((HAL_UART_GetState(&huart3) & HAL_UART_STATE_BUSY_TX) == HAL_UART_STATE_BUSY_TX);
	HAL_UART_Transmit_IT(&huart3, (uint8_t *)"\r\nRx Error!", sizeof("\r\nRx Error!") - 1);
}

uint8_t CMD_getFlag(void)
{
	return cmd_recive_flag;
}
void CMD_clearFlag(uint8_t cflag)
{
	cmd_recive_flag &= ~cflag;
}

bool CMD_getDirect(Direct_t *cmd)
{
	uint8_t sum;
	int i;
	
	if(cmd_recive[0] == 0xFE && cmd_recive[1] == 0xFC)
	{
		for(sum=0,i=0;i<CMD_BUF_LEN;i++)
		{
			sum += cmd_recive[i];
		}
		if(sum == 0)
		{
			cmd->cmd = cmd_recive[2];
			cmd->year = (cmd_recive[3]>>4)*10 + (cmd_recive[3]&0x0F);
			cmd->month = (cmd_recive[4]>>4)*10 + (cmd_recive[4]&0x0F);
			cmd->day = (cmd_recive[5]>>4)*10 + (cmd_recive[5]&0x0F);
			return true;
		}
	}
	return false;
}



