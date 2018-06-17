/*
 *  Record.h
 *  
 *  Auther: Zhou JI
 *  Date:   2018-06-14 20:33
 */
#ifndef __RECORD_H__
#define __RECORD_H__

/*! Include ------------------------------------------------------*/
#include "Flash.h"


/*! Macro --------------------------------------------------------*/
// record data area
#define RECORD_ADDR_BEGIN_ADDRESS       (Flash_getStartAddress())
#define RECORD_ADDR_END_ADDRESS         (Flash_getSectorSize())

// record address area
#define RECORD_DATA_BEGIN_ADDRESS       (Flash_getStartAddress() + Flash_getSectorSize())
#define RECORD_DATA_END_ADDRESS         (Flash_getEndAddress())

// srctor
#define RECORD_SECTOR_SIZE              (Flash_getSectorSize())
// one page size
#define RECORD_PAGE_SIZE                (Flash_getPageSize())

#define PAGE_SIZE       				(RECORD_PAGE_SIZE * 4u)
/*! Type --------------------------------------------------------*/

struct PwmCounter
{
    /* data */
    uint32_t pwm1;
    uint32_t pwm2;
    uint32_t pwm3;
    uint32_t pwm4;
    uint32_t pwm5;
    uint32_t pwm6;
};

typedef struct FILE_Message
{
    /* data */
    struct RecordTime time;
    struct PwmCounter pwm;
    uint32_t startAddress;
    uint32_t pageIndex;
    uint8_t dataBuffer[PAGE_SIZE];
    uint16_t dataLength;
    uint16_t bufferPtr;
    uint8_t indexNum;
    uint8_t busy;
}   FILE_Message_t;

/*! Function --------------------------------------------------*/
/*
 *  Initialize file system
 */
void FILE_Init(void);

/*
 *  Clear the record
 */
int FILE_clearRecord(void);

/*
 *  Find File
 */
int FILE_findLast(uint32_t *addr);
int FILE_findFirst(uint32_t *addr);
int FILE_findFront(uint32_t *addr);
int FILE_findBack(uint32_t *addr);

/*
 *  close file
 */
int FILE_close(FILE_Message_t *file);

/*
 *  read file
 */
int FILE_readInit(uint32_t address, FILE_Message_t **pfile);
int FILE_readPage(FILE_Message_t *file);

/*
 *  write file operate
 */
int FILE_writeInit(FILE_Message_t **file);
int FILE_writeTime(FILE_Message_t *file, struct RecordTime *time);
int FILE_writeFlash(FILE_Message_t *file, int endFlag);
int FILE_writeGPSCache(FILE_Message_t *file, char *msg);


#endif
