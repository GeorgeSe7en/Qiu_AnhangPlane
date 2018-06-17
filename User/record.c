/*
 *  Flash.c
 */
/*! Includes ---------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "record.h"
/*! Macro -----------------------------------------------*/
//
#define HEAD_TAG_LEN        4
#define TIME_MSG_LEN        24
#define PWM_MSG_LEN         74

//
#define FIRST_MEG_LEN       (HEAD_TAG_LEN + TIME_MSG_LEN + PWM_MSG_LEN)
#define OTHER_MSG_LEN       (HEAD_TAG_LEN + PWM_MSG_LEN)
/*! Types ---------------------------------------------------*/
enum PAGE_TYPE
{
    FILE_START = 0xFE,
    FILE_COUNT = 0xFD,
    FILE_END   = 0xFC,
};

/*
 *  |---------------------------------------------------------|
 *  |Tag   |  Index |  length high | length low |  4 ----- n  |
 *  |---------------------------------------------------------|
 *  |1byte |  1byte |  1byte       | 1byte      |     data    |
 *  |---------------------------------------------------------|
 */

struct File_StartEndAddress
{
    uint32_t firstFileAddress;
    uint32_t lastFileAddress;
};

#define FILE_ADDR_LEN   (sizeof(struct File_StartEndAddress))
/*! Private Data -----------------------------------------*/
// const data
static const char *LTimeFormat = "%c:%d-%02d:%02d:%02d\r\n";
static const char *RTimeFormat = "%c:%d/%02d/%02d-%02d:%02d:%02d\r\n";
static const char *PwmFormat = "PWM1:%d, PWM2:%d, PWM3:%d, PWM4:%d, PWM5:%d, PWM6:%d\r\n";

// file address record
static struct File_StartEndAddress file_Address = {RECORD_DATA_BEGIN_ADDRESS, RECORD_DATA_BEGIN_ADDRESS};
static uint32_t file_AddressSaveAddress = RECORD_ADDR_BEGIN_ADDRESS;
static uint32_t currentFileAddress = RECORD_DATA_BEGIN_ADDRESS;
static uint8_t lastFile_operate;
// file interface
static FILE_Message_t writeFile, readFile;

/*! Private Function --------------------------------------*/
static int FILE_updateFirstAddress(uint32_t addr);
static int FILE_saveAddress(struct File_StartEndAddress *addr);

/*! Function --------------------------------------------*/
/*
 *  Initialize file system
 */
void FILE_Init(void)
{
    struct File_StartEndAddress flashOperateAddress;
    int32_t address, startAddress = RECORD_ADDR_BEGIN_ADDRESS / FILE_ADDR_LEN , endAddress = RECORD_ADDR_END_ADDRESS / FILE_ADDR_LEN - 1;
    uint8_t buf[FILE_ADDR_LEN * 8];
	
    printlog("\r\nFile manage initialize!");
	Flash_Initialize();
    
	Flash_readData(startAddress, buf, FILE_ADDR_LEN*8);
	
    while(startAddress <= endAddress)
    {
        address = (startAddress + endAddress) / 2;
		
        Flash_readData(address * FILE_ADDR_LEN, (uint8_t *)&flashOperateAddress, FILE_ADDR_LEN);
		
        if(flashOperateAddress.firstFileAddress >= RECORD_DATA_BEGIN_ADDRESS &&
           flashOperateAddress.firstFileAddress < RECORD_DATA_END_ADDRESS &&
           flashOperateAddress.lastFileAddress >= RECORD_DATA_BEGIN_ADDRESS &&
           flashOperateAddress.lastFileAddress < RECORD_DATA_END_ADDRESS )
        {

            file_Address.firstFileAddress = flashOperateAddress.firstFileAddress;
            file_Address.lastFileAddress = flashOperateAddress.lastFileAddress;
            address += 1;
			
            Flash_readData(address * FILE_ADDR_LEN, (uint8_t *)&flashOperateAddress, FILE_ADDR_LEN);
			
            if(flashOperateAddress.firstFileAddress >= RECORD_DATA_BEGIN_ADDRESS &&
            flashOperateAddress.firstFileAddress < RECORD_DATA_END_ADDRESS &&
            flashOperateAddress.lastFileAddress >= RECORD_DATA_BEGIN_ADDRESS &&
            flashOperateAddress.lastFileAddress < RECORD_DATA_END_ADDRESS )
            {
				file_Address.firstFileAddress = flashOperateAddress.firstFileAddress;
				file_Address.lastFileAddress = flashOperateAddress.lastFileAddress;
                startAddress = address;
            }
            else
            {
                file_AddressSaveAddress = address * FILE_ADDR_LEN;
                break;
            }
        }
        else
        {
            endAddress = address-1;
        }
    }
    currentFileAddress = file_Address.lastFileAddress;
    // write file
    writeFile.busy = 0;

    // read file
    readFile.busy = 0;

    //uart_printf("\r\n Flash initialize!");
    //uart_printf("\r\n ST:0x%x, SP:0x%x", file_Address.firstFileAddress, file_Address.lastFileAddress);
}

/*
 *  Clear the record
 */
int FILE_clearRecord(void)
{
    if(writeFile.busy || readFile.busy) return -1;      // busy

    file_Address.firstFileAddress = RECORD_DATA_BEGIN_ADDRESS;
    file_Address.lastFileAddress = RECORD_DATA_BEGIN_ADDRESS;
    file_AddressSaveAddress = RECORD_ADDR_BEGIN_ADDRESS;
    currentFileAddress = RECORD_DATA_BEGIN_ADDRESS;

    Flash_eraseSector(RECORD_ADDR_BEGIN_ADDRESS);
    Flash_eraseSector(RECORD_DATA_BEGIN_ADDRESS);
    return 0;
}

/*
 *  Find File
 */
int FILE_findLast(uint32_t *addr)
{
    uint8_t buf[4];
    uint32_t tAddress, opAddress;

    //uart_printf("\r\n find Last!");

    //uart_printf("\r\n ZJ002: 0x%x, 0x%x", file_Address.firstFileAddress, file_Address.lastFileAddress);

    lastFile_operate = 0;
    tAddress = currentFileAddress = file_Address.lastFileAddress;
    //uart_printf("\r\n find Last! ST:0x%x", tAddress);
    if(writeFile.busy && writeFile.startAddress == tAddress)
    {
        if(writeFile.pageIndex == 0)
        {
            *addr = (uint32_t )&writeFile.dataBuffer[0];
            lastFile_operate = 2;
            //uart_printf("\r\n find Last! writeFile 2");
            return 0;
        }
        else
        {
            *addr = currentFileAddress = tAddress;
            lastFile_operate = 1;
            //uart_printf("\r\n find Last! writeFile 1");
            return 0;
        }
    }

    do
    {
		if(tAddress == RECORD_DATA_BEGIN_ADDRESS)
        {
            tAddress = RECORD_DATA_END_ADDRESS - PAGE_SIZE;
        }
        else
        {
            tAddress -= PAGE_SIZE;
        }
        //uart_printf("\r\n find Last! ReadFlash: 0x%x", tAddress);
        Flash_readData(tAddress, buf, 4);
        //uart_printf("\r\n BUF: %02x, %02x %02x %02x", buf[0], buf[1], buf[2], buf[3]);
        if(buf[0] == FILE_START)
        {
            *addr = currentFileAddress = tAddress;
            return 0;
        }
		else if(buf[0] != FILE_COUNT && buf[0] != FILE_END)
		{
			return -1;
		}
    }while(opAddress != file_Address.firstFileAddress);
	Flash_readData(tAddress, buf, 4);
    if(buf[0] == FILE_START)
    {
        *addr = currentFileAddress = tAddress;
        if(writeFile.startAddress)
        return 0;
    }
    return -1;
}

int FILE_findFirst(uint32_t *addr)
{
    uint8_t buf[4];
    uint32_t tAddress;

    lastFile_operate = 0;
    tAddress = currentFileAddress = file_Address.firstFileAddress;

    if(writeFile.busy && writeFile.startAddress == tAddress)
    {
        if(writeFile.pageIndex == 0)
        {
            *addr = (uint32_t )&writeFile.dataBuffer[0];
            lastFile_operate = 2;
            return 0;
        }
        else
        {
            *addr = currentFileAddress = tAddress;
            lastFile_operate = 1;
            return 0;
        }
    }

    Flash_readData(tAddress, buf, 4);
    if(buf[0] == FILE_START)
    {
        *addr = currentFileAddress = tAddress;
        return 0;
    }

    return -1;
}

int FILE_findFront(uint32_t *addr)
{
    uint8_t buf[4];
    uint32_t tAddress, opAddress;

	lastFile_operate = 0;
    tAddress = currentFileAddress;

    do
    {
		if(tAddress == RECORD_DATA_BEGIN_ADDRESS)
        {
            tAddress = RECORD_DATA_END_ADDRESS - PAGE_SIZE;
        }
        else
        {
            tAddress -= PAGE_SIZE;
        }
        Flash_readData(tAddress, buf, 4);
        //uart_printf("\r\n ZJ003: 0x%x, %x %x %x %x", tAddress, buf[0], buf[1], buf[2], buf[3]);
        if(buf[0] == FILE_START)
        {
            *addr = currentFileAddress = tAddress;
            return 0;
        }
		else if(buf[0] != FILE_COUNT && buf[0] != FILE_END)
		{
			return -1;
		}
    }while(opAddress != file_Address.firstFileAddress);

	Flash_readData(tAddress, buf, 4);
    if(buf[0] == FILE_START)
    {
        *addr = currentFileAddress = tAddress;
        return 0;
    }
		
    return -1;
}

int FILE_findBack(uint32_t *addr)
{
    uint8_t buf[4];
    uint32_t tAddress;

    lastFile_operate = 0;
    tAddress = currentFileAddress;

    if(writeFile.busy && writeFile.startAddress == tAddress)
    {
        if(writeFile.pageIndex == 0)
        {
            *addr = (uint32_t )&writeFile.dataBuffer[0];
            lastFile_operate = 2;
            return 0;
        }
        else
        {
            *addr = currentFileAddress = tAddress;
            lastFile_operate = 1;
            return 0;
        }
    }
    
    do
    {
		tAddress += PAGE_SIZE;
        if(tAddress == RECORD_DATA_END_ADDRESS)
        {
            tAddress = RECORD_DATA_BEGIN_ADDRESS;
        }
		
        Flash_readData(tAddress, buf, 4);
        if(buf[0] == FILE_START)
        {
            *addr = currentFileAddress = tAddress;
            return 0;
        }
        else if(buf[0] != FILE_COUNT && buf[0] != FILE_END)
		{
			return -1;
		}
    }while(tAddress != file_Address.lastFileAddress);

    if(writeFile.busy && writeFile.startAddress == tAddress)
    {
        if(writeFile.pageIndex == 0)
        {
            *addr = (uint32_t )&writeFile.dataBuffer[0];
            lastFile_operate = 2;
            return 0;
        }
        else
        {
            *addr = currentFileAddress = tAddress;
            lastFile_operate = 1;
            return 0;
        }
    }

    Flash_readData(tAddress, buf, 4);
    if(buf[0] == FILE_START)
    {
        *addr = currentFileAddress = tAddress;
        return 0;
    }
		
	
    return -1;
}

/*
 *  close file
 */
int FILE_close(FILE_Message_t *file)
{
    if(file != NULL)
    {
        file->busy = 0;
        file = NULL;
    }
    return 0;
}

/*
 *  read file
 */
int FILE_readInit(uint32_t address, FILE_Message_t **pfile)
{
    uint8_t buf[OTHER_MSG_LEN];
    uint16_t length;
    uint8_t y, mon, d, h, min, s;
    char tag;
    uint16_t day;
    uint32_t pwm1, pwm2, pwm3, pwm4, pwm5, pwm6;
	uint8_t indexNum;
    uint32_t chAddress;
    FILE_Message_t *file;

    if(readFile.busy) return -1;
    file = &readFile;
    //uart_printf("\r\n read initialize!");

    if(lastFile_operate == 2)
    {
        file->startAddress = writeFile.startAddress;
        file->indexNum = writeFile.indexNum;
        file->dataLength = writeFile.bufferPtr;
        file->bufferPtr = FIRST_MEG_LEN;
        file->pwm.pwm1 = writeFile.pwm.pwm1;
        file->pwm.pwm2 = writeFile.pwm.pwm2;
        file->pwm.pwm3 = writeFile.pwm.pwm3;
        file->pwm.pwm4 = writeFile.pwm.pwm4;
        file->pwm.pwm5 = writeFile.pwm.pwm5;
        file->pwm.pwm6 = writeFile.pwm.pwm6;
        memcpy(file->dataBuffer, writeFile.dataBuffer, file->dataLength);

        file->dataBuffer[0] = FILE_START;
        file->dataBuffer[1] = file->indexNum;
        file->dataBuffer[2] = file->dataLength>>8;
        file->dataBuffer[3] = (file->dataLength & 0x0FF);
        sprintf((char *)&file->dataBuffer[HEAD_TAG_LEN + TIME_MSG_LEN], PwmFormat,
                    file->pwm.pwm1, file->pwm.pwm2, file->pwm.pwm3,
                    file->pwm.pwm4, file->pwm.pwm5, file->pwm.pwm6);
    }
    else
    {
        // read flash
        Flash_readData(address, buf, 4);
        if(buf[0] != FILE_START || buf[1] != 0x00)
        {
            // not a file head
            return -1;
        }
        length = buf[2]; length <<= 8; length |= buf[3];
        if(length < FIRST_MEG_LEN || length > PAGE_SIZE)
        {
            // file length false
            return -1;
        }

        // file page index
        file->indexNum = 0;
        // page data length
        file->dataLength = length;

        Flash_readData(address, file->dataBuffer, file->dataLength);
    }
    // find time
    if(file->dataBuffer[HEAD_TAG_LEN] == 'L')
    {
        if(5 != sscanf((char *)&file->dataBuffer[HEAD_TAG_LEN], LTimeFormat, &tag, &day, &h, &min, &s)) return -1;
        // time
        file->time.tag = tag;
        file->time.month = day>>8;
        file->time.day = (day&0x0FF);
        file->time.hour = h;
        file->time.minute = min;
        file->time.second = s;
    }
    else if(file->dataBuffer[HEAD_TAG_LEN] == 'R')
    {
        if(7 != sscanf((char *)&file->dataBuffer[HEAD_TAG_LEN], RTimeFormat, &tag, &y, &mon, &d, &h, &min, &s)) return -1;
        file->time.tag = tag;
        file->time.year = y;
        file->time.month = mon;
        file->time.day = d;
        file->time.hour = h;
        file->time.minute = min;
        file->time.second = s;
    }
    else
    {   // no time message                
        return -1;
    }

    if(6 != sscanf((char *)&file->dataBuffer[28], PwmFormat, &pwm1, &pwm2, &pwm3, &pwm4, &pwm5, &pwm6)) return -1;

    file->pwm.pwm1 = pwm1;
    file->pwm.pwm2 = pwm2;
    file->pwm.pwm3 = pwm3;
    file->pwm.pwm4 = pwm4;
    file->pwm.pwm5 = pwm5; 
    file->pwm.pwm6 = pwm6;

    if(lastFile_operate)
    {
        if(lastFile_operate == 1)
        {
            file->pwm.pwm1 = writeFile.pwm.pwm1;
            file->pwm.pwm2 = writeFile.pwm.pwm2;
            file->pwm.pwm3 = writeFile.pwm.pwm3;
            file->pwm.pwm4 = writeFile.pwm.pwm4;
            file->pwm.pwm5 = writeFile.pwm.pwm5;
            file->pwm.pwm6 = writeFile.pwm.pwm6;
            sprintf((char *)&file->dataBuffer[HEAD_TAG_LEN + TIME_MSG_LEN], PwmFormat,
                    file->pwm.pwm1, file->pwm.pwm2, file->pwm.pwm3,
                    file->pwm.pwm4, file->pwm.pwm5, file->pwm.pwm6);
        }
    }
    else
    {
        // find the total pwm counter times
        indexNum = file->indexNum;
        chAddress = address;
        do
        {
            indexNum += 1;
            chAddress += PAGE_SIZE;
            if(chAddress >= RECORD_DATA_END_ADDRESS)
            {
                chAddress = chAddress - RECORD_DATA_END_ADDRESS + RECORD_DATA_BEGIN_ADDRESS;
            }
            Flash_readData(chAddress, buf, OTHER_MSG_LEN);
            ////uart_printf("\r\nread 0x%x val: %02x %02x %02x %02x", chAddress, buf[0], buf[1], buf[2], buf[3]);
            // check
            if(buf[0] == FILE_START) break;     // other file start
            if((buf[0] != FILE_COUNT && buf[0] != FILE_END) || buf[1] != indexNum) break; // discontinuity

            if(6 != sscanf((char *)&buf[4], PwmFormat, &pwm1, &pwm2, &pwm3, &pwm4, &pwm5, &pwm6)) break;

            file->pwm.pwm1 = pwm1;
            file->pwm.pwm2 = pwm2;
            file->pwm.pwm3 = pwm3;
            file->pwm.pwm4 = pwm4;
            file->pwm.pwm5 = pwm5;
            file->pwm.pwm6 = pwm6;
            ////uart_printf("Copy PWM!\r\n");
            strcpy((char *)&file->dataBuffer[28], (char *)&buf[4]);
        }while(1);
    }

    file->bufferPtr = FIRST_MEG_LEN;      // buffer process data index
    file->startAddress = address;   // file start address
    file->pageIndex = 1;        // page read index
	file->indexNum = 1;
	//uart_printf("\r\n read init:0x%x", file->startAddress);
    file->busy = 1;
    *pfile = file;
    return 0;
}

int FILE_readPage(FILE_Message_t *file)
{
    uint8_t buf[4], checkWrite = 0;
    uint16_t length;
    uint32_t address;
    uint32_t pwm1, pwm2, pwm3, pwm4, pwm5, pwm6;
    
    if(file != &readFile) return -1;
    if(lastFile_operate == 2) return -1;

    address = file->startAddress + file->pageIndex * PAGE_SIZE;
    if(address >= RECORD_DATA_END_ADDRESS)
    {
        address = address - RECORD_DATA_END_ADDRESS + RECORD_DATA_BEGIN_ADDRESS;
    }
    Flash_readData(address, buf, 4);

    // check
    if(buf[0] == FILE_START) checkWrite = 1;     // other file start
    if((buf[0] != FILE_COUNT && buf[0] != FILE_END) || buf[1] != file->indexNum) checkWrite = 1; // discontinuity
    length = buf[2]; length <<= 8; length |= buf[3];
    if(length < OTHER_MSG_LEN || length > PAGE_SIZE)
    {
        // length too big or small
        checkWrite = 1;
    }

    if(checkWrite)
    {
        if(lastFile_operate == 1 && file->indexNum == writeFile.indexNum && file->pageIndex == writeFile.pageIndex)
        {
            file->dataLength = writeFile.bufferPtr;
            file->pwm.pwm1 = writeFile.pwm.pwm1;
            file->pwm.pwm2 = writeFile.pwm.pwm2;
            file->pwm.pwm3 = writeFile.pwm.pwm3;
            file->pwm.pwm4 = writeFile.pwm.pwm4;
            file->pwm.pwm5 = writeFile.pwm.pwm5;
            file->pwm.pwm6 = writeFile.pwm.pwm6;
            memcpy(file->dataBuffer, writeFile.dataBuffer, file->dataLength);

            file->dataBuffer[0] = FILE_COUNT;
            file->dataBuffer[1] = file->indexNum;
            file->dataBuffer[2] = file->dataLength>>8;
            file->dataBuffer[3] = (file->dataLength & 0x0FF);
            sprintf((char *)&file->dataBuffer[HEAD_TAG_LEN], PwmFormat,
                    file->pwm.pwm1, file->pwm.pwm2, file->pwm.pwm3,
                    file->pwm.pwm4, file->pwm.pwm5, file->pwm.pwm6);
            lastFile_operate = 2;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        file->dataLength = length;
        Flash_readData(address, file->dataBuffer, file->dataLength);
    }
    if(6 != sscanf((char *)&file->dataBuffer[4], PwmFormat, &pwm1, &pwm2, &pwm3, &pwm4, &pwm5, &pwm6)) return -1;
    
    file->pwm.pwm1 = pwm1;
    file->pwm.pwm2 = pwm2;
    file->pwm.pwm3 = pwm3;
    file->pwm.pwm4 = pwm4;
    file->pwm.pwm5 = pwm5; 
    file->pwm.pwm6 = pwm6;

    file->bufferPtr = OTHER_MSG_LEN;      // buffer process data index
    file->pageIndex++;        // page read index
    file->indexNum++;

    return 0;
}

/*
 *  write file operate
 */
int FILE_writeInit(FILE_Message_t **file)
{
    if(writeFile.busy) return -1;

    writeFile.startAddress = file_Address.lastFileAddress;
    writeFile.pageIndex = 0;
    writeFile.indexNum = 0;
    writeFile.time.tag = 'N';

    writeFile.bufferPtr = FIRST_MEG_LEN;

    writeFile.busy = 1;
    *file = &writeFile;

    return 0;
}

int FILE_writeTime(FILE_Message_t *file, struct RecordTime *time)
{
    if(file == NULL) return -1;

    if(time->tag != 'L' && time->tag != 'R') return -1;
    file->time.tag = time->tag;
    
    file->time.year = time->year;
    file->time.month = time->month;
    file->time.day = time->day;

    file->time.hour = time->hour;
    file->time.minute = time->minute;
    file->time.second = time->second;

    if(file->time.tag == 'L')
    {
        sprintf((char *)&file->dataBuffer[HEAD_TAG_LEN], LTimeFormat, 
                file->time.tag,
                file->time.day,
                file->time.hour,
                file->time.minute,
                file->time.second);
    }
    else if(file->time.tag == 'R')
    {
        sprintf((char *)&file->dataBuffer[HEAD_TAG_LEN], RTimeFormat, 
                file->time.tag,
                file->time.year,
                file->time.month,
                file->time.day,
                file->time.hour,
                file->time.minute,
                file->time.second);
    }
    
    return 0;
}

int FILE_writeFlash(FILE_Message_t *file, int endFlag)
{
    struct File_StartEndAddress tFileAddress;
    uint32_t address;

    if(file == NULL) return -1;

    if(file->pageIndex == 0)
    {
        sprintf((char *)&file->dataBuffer[HEAD_TAG_LEN + TIME_MSG_LEN], PwmFormat,
                    file->pwm.pwm1, file->pwm.pwm2, file->pwm.pwm3,
                    file->pwm.pwm4, file->pwm.pwm5, file->pwm.pwm6);
    }
    else
    {
        sprintf((char *)&file->dataBuffer[HEAD_TAG_LEN], PwmFormat,
                    file->pwm.pwm1, file->pwm.pwm2, file->pwm.pwm3,
                    file->pwm.pwm4, file->pwm.pwm5, file->pwm.pwm6);
    }

    tFileAddress.firstFileAddress = file_Address.firstFileAddress;
    tFileAddress.lastFileAddress = file_Address.lastFileAddress;

    address = file->startAddress + file->pageIndex * PAGE_SIZE;
    if(address >= RECORD_DATA_END_ADDRESS)
    {
        address = address - RECORD_DATA_END_ADDRESS + RECORD_DATA_BEGIN_ADDRESS;
    }
    if((address & (RECORD_SECTOR_SIZE - 1)) == 0)       // new sector
    {
        if((address & ~(RECORD_SECTOR_SIZE - 1)) == (file_Address.firstFileAddress & ~(RECORD_SECTOR_SIZE - 1)))
        {
            FILE_updateFirstAddress(address);
        }
        Flash_eraseSector(address);
    }

    if(endFlag)
    {
        file_Address.lastFileAddress = address + PAGE_SIZE;
		if(file_Address.lastFileAddress >= RECORD_DATA_END_ADDRESS)
		{
			file_Address.lastFileAddress = file_Address.lastFileAddress - RECORD_DATA_END_ADDRESS + RECORD_DATA_BEGIN_ADDRESS;
		}
    }
    if(endFlag || file_Address.firstFileAddress != tFileAddress.firstFileAddress ||
				  file_Address.lastFileAddress != tFileAddress.lastFileAddress)
    {
        //uart_printf("\r\n SaveFileAddr:ST 0x%x SP", file_Address.firstFileAddress, file_Address.lastFileAddress);
        FILE_saveAddress(&file_Address);
    }
    
    if(file->pageIndex == 0)
    {
        file->dataBuffer[0] = FILE_START;
    }
    else if(endFlag)
    {
        file->dataBuffer[0] = FILE_END;
    }
    else
    {
        file->dataBuffer[0] = FILE_COUNT;
    }
    file->dataBuffer[1] = file->indexNum;
    file->dataLength = file->bufferPtr;
    file->dataBuffer[2] = file->dataLength>>8;
    file->dataBuffer[3] = (file->dataLength & 0x0FF);
    //uart_printf("\r\n TO Flah, flag:%d, 0x%x, length:%d", endFlag, address, file->dataLength);
    Flash_writeData(address, &file->dataBuffer[0], file->dataLength);

    file->pageIndex++;
    file->indexNum++;
    file->dataLength = 0;
    file->bufferPtr = OTHER_MSG_LEN;

    if(endFlag) FILE_close(file);

    return 0;
}

int FILE_writeGPSCache(FILE_Message_t *file, char *msg)
{
    int i;

    if(file == NULL) return -1;

    i = strlen(msg);
    if(i > PAGE_SIZE - file->bufferPtr - 1) return 1;
    
    strcpy((char *)&file->dataBuffer[file->bufferPtr], msg);
    file->bufferPtr += i;

    return 0;
}

static int FILE_updateFirstAddress(uint32_t addr)
{
    uint8_t buf[4];
    uint32_t address;

    address = addr + RECORD_SECTOR_SIZE;

	Flash_readData(address, buf, 4);
	if(buf[0] != FILE_START && buf[0] != FILE_COUNT && buf[0] != FILE_END)
	{
        file_Address.firstFileAddress = address;
		return 0;
	}
    if(buf[0] == FILE_START && buf[1] == 0)
    {
        file_Address.firstFileAddress = address;
		return 0;
    }
    do
    {
        address += PAGE_SIZE;
        if(address >= RECORD_DATA_END_ADDRESS)
        {
            address = address - RECORD_DATA_END_ADDRESS + RECORD_DATA_BEGIN_ADDRESS;
        }

        Flash_readData(address, buf, 4);
        if(buf[0] == FILE_START && buf[1] == 0)
        {
            file_Address.firstFileAddress = address;
            return 0;
        }
        else if(buf[0] != FILE_START && buf[0] != FILE_COUNT && buf[0] != FILE_END)
        {
            file_Address.firstFileAddress = address;
			return 0;
        }
    }while(address == file_Address.lastFileAddress);

    file_Address.firstFileAddress = file_Address.lastFileAddress;
    return 0;
}

static int FILE_saveAddress(struct File_StartEndAddress *addr)
{
	if(file_AddressSaveAddress == RECORD_ADDR_BEGIN_ADDRESS || file_AddressSaveAddress + FILE_ADDR_LEN > RECORD_ADDR_END_ADDRESS)
	{
        //uart_printf("\r\n ZJ001: Erease!");
		file_AddressSaveAddress = RECORD_ADDR_BEGIN_ADDRESS;
        Flash_eraseSector(file_AddressSaveAddress);
        
        //uart_printf("\r\n ZJ001: 0x%x, 0x%x", addr->firstFileAddress, addr->lastFileAddress);
        Flash_writeData(file_AddressSaveAddress, (uint8_t *)addr, FILE_ADDR_LEN);
        file_AddressSaveAddress += FILE_ADDR_LEN;
	}
	else
    {
        //uart_printf("\r\n ZJ001: 0x%x, 0x%x", addr->firstFileAddress, addr->lastFileAddress);
        Flash_writeData(file_AddressSaveAddress, (uint8_t *)addr, FILE_ADDR_LEN);
        file_AddressSaveAddress += FILE_ADDR_LEN;
    }
    return 0;
}
