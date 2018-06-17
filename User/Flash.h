/**
 *	@! Flash header file
 *	@! Author: Ji Zhou
 * 	@! Time:	2018.5.31
**/
 
#ifndef __FLASH_H__
#define __FLASH_H__
 
/*! Includes --------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>

#include "Global.h"
#include "GD25Q64CxIGx.h"
 
/*! Macro -----------------------------------------------------*/
#define Flash_getStartAddress() 		(GD25Q_FLASH_START_ADDR)
#define Flash_getEndAddress() 			(GD25Q_FLASH_TOTAL_SIZE)
#define Flash_getSectorSize() 			(GD25Q_SECTOR_SIZE)
#define Flash_getPageSize()				(GD25Q_PAGE_SIZE)
 
/*! Static Inline Function ------------------------------------*/
 
/*! Function -------------------------------------------------*/
void Flash_Initialize(void); 
void Flash_eraseSector(uint32_t address);
int Flash_writeData(uint32_t address, uint8_t *data, uint32_t len);
int Flash_readData(uint32_t address, uint8_t *data, uint32_t len);

#endif
/*****************************END*******************************/
