/*
 *  FileManage.h
 *  The header file manage, defined relationship marco.
 *  
 * ----------------------
 *   Paddle Studio      *
 *                      *
 *   Auther: Zhou Ji    *
 *   Date:   2018.6.8   *
 * ----------------------
 */

#ifndef __FILEMANAGE_H__
#define __FILEMANAGE_H__

/*! Includes ----------------------------------------*/
#include <stdint.h>

#include "Flash.h"
/*! Macro -------------------------------------------*/
// flash base message
#define MYFLASH_MIN_ADDR          Flash_getStartAddress()
#define MYFLASH_MAX_ADDR          Flash_getEndAddress()
#define MYFLASH_SECTOR_SIZE       Flash_getSectorSize()
#define MYFLASH_PAGE_SIZE         Flash_getPageSize()

// record segment
#define RECORD_SECTOR_SIZE        MYFLASH_SECTOR_SIZE
#define RECORD_SECTOR_ADDR        MYFLASH_MIN_ADDR

// File Tag
#define FILE_HEAD_TAG           0xFEEF
#define FILE_OPEN_MODE_WRITE    0x01
#define FILE_OPEN_MODE_READ     0x02
/*! Type --------------------------------------------*/
typedef struct file_header 
{
    uint16_t tag;           // default: 0xFEEF
    uint16_t fileName;      // file name crc value
    uint32_t fileLen;       // the size of file (include file header) (unit bytes)
}   file_header_t;

typedef struct file_list
{
	uint32_t preFileAddress;
	uint8_t flag;
	uint8_t year;
	uint8_t month;
	uint8_t day;
}	file_list_t;



typedef struct MY_FILE
{
    uint32_t fileAddr;
    uint32_t fileSize;
    uint32_t readptr;
	
	uint16_t fileName;
}   MY_FILE_t;

/*! Function ----------------------------------------*/
void FileManage_Initialize(void);

MY_FILE_t * FileManage_open(char *name, uint8_t mode);
void FileManage_close(MY_FILE_t *file);

int FileManage_files(void);
int FileManage_seek(MY_FILE_t *file, uint32_t seek);
int FileManage_write(MY_FILE_t *file, uint8_t *data, uint32_t len);
int FileManage_read(MY_FILE_t *file, uint8_t *data, uint32_t len);

#endif
/******************************END*******************/
