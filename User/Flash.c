/***********************************************************
**	flash.c
**	Author: Ji Zhou
**	Time:	2018.5.31
***********************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "Global.h"
#include "GD25Q64CxIGx.h"
/**********************************************************
**	Type
***********************************************************/



/***********************************************************
**	Function
***********************************************************/
void Flash_Initialize(void)
{
	SPI_Flash_Init();
}

// erase sector
void Flash_eraseSector(uint32_t address)
{
	SPI_FLASH_EraseSector(address);
}

// write data
int  Flash_writeData(uint32_t address, uint8_t *data, uint32_t len)
{
	uint32_t num;
	
	SPI_FLASH_WriteData(address, data, len, &num);
	
	return num;
}

// read data
int  Flash_readData(uint32_t address, uint8_t *data, uint32_t len)
{
	uint32_t num;
	
	SPI_FLASH_ReadData(address, data, len, &num);
	return num;
}

int Flash_WRTest(void)
{
	const char *str = "this is a test string!";
	char buf[32] = {0};
	const char *ptr = str;
	int i=0;
	
	SPI_Flash_Init();
	
	Flash_eraseSector(0);
	Flash_writeData(0, (uint8_t *)str, strlen(str));
	Flash_readData(0, (uint8_t *)buf, strlen(str));
	
	while(*ptr != '\0')
	{
		if(*ptr++ != buf[i++])
		{
			return -1;
		}
	}
	return 0;
}






