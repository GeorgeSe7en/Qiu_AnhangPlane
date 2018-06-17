/*********************************************************
**      GD25Q64CxIGx Çý¶¯»ù´¡¶ÁÐ´ÃüÁî
*********************************************************/

#ifndef __GD25Q64CxIGx_H__
#define __GD25Q64CxIGx_H__
/********************************************************
**  INCLUDES
********************************************************/
#include <stdint.h>

#include "Global.h"
/********************************************************
**  MARICO DEFINE
********************************************************/
//  CMP_0_protection area
/**************GD25Q64C Protected area size (CMP=0)**********/
#define BP0_4_CMP_0_PROTECTION_NONE               0x00
#define BP0_4_CMP_0_PROTECTION_128KB_UPPER        0x01
#define BP0_4_CMP_0_PROTECTION_256KB_UPPER        0x02
#define BP0_4_CMP_0_PROTECTION_512KB_UPPER        0x03
#define BP0_4_CMP_0_PROTECTION_1MB_UPPER          0x04
#define BP0_4_CMP_0_PROTECTION_2MB_UPPER          0x05
#define BP0_4_CMP_0_PROTECTION_4MB_UPPER          0x06
#define BP0_4_CMP_0_PROTECTION_128KB_LOWER        0x09
#define BP0_4_CMP_0_PROTECTION_256KB_LOWER        0x0A
#define BP0_4_CMP_0_PROTECTION_512KB_LOWER        0x0B
#define BP0_4_CMP_0_PROTECTION_1MB_LOWER          0x0C
#define BP0_4_CMP_0_PROTECTION_2MB_LOWER          0x0D
#define BP0_4_CMP_0_PROTECTION_4MB_LOWER          0x0E

#define BP0_4_CMP_0_PROTECTION_8MB_ALL_MASK       0x07
#define BP0_4_CMP_0_PROTECTION_8MB_ALL            0x07

#define BP0_4_CMP_0_PROTECTION_4KB_TOP            0x11
#define BP0_4_CMP_0_PROTECTION_8KB_TOP            0x12
#define BP0_4_CMP_0_PROTECTION_16KB_TOP           0x13
#define BP0_4_CMP_0_PROTECTION_32KB_TOP           0x14
#define BP0_4_CMP_0_PROTECTION_32KB_TOP_1         0x15
#define BP0_4_CMP_0_PROTECTION_32KB_TOP_2         0x16

#define BP0_4_CMP_0_PROTECTION_4KB_BOTTOM         0x19
#define BP0_4_CMP_0_PROTECTION_8KB_BOTTOM         0x1A
#define BP0_4_CMP_0_PROTECTION_16KB_BOTTOM        0x1B
#define BP0_4_CMP_0_PROTECTION_32KB_BOTTOM        0x1C
#define BP0_4_CMP_0_PROTECTION_32KB_BOTTOM_1      0x1D
#define BP0_4_CMP_0_PROTECTION_32KB_BOTTOM_2      0x1E

/**************GD25Q64C Protected area size (CMP=1)**********/
#define BP0_4_CMP_1_PROTECTION_ALL                  0x00
#define BP0_4_CMP_1_PROTECTION_8064KB_LOWER         0x01
#define BP0_4_CMP_1_PROTECTION_7936KB_LOWER         0x02
#define BP0_4_CMP_1_PROTECTION_7680KB_LOWER         0x03
#define BP0_4_CMP_1_PROTECTION_7MB_LOWER            0x04
#define BP0_4_CMP_1_PROTECTION_6MB_LOWER            0x05
#define BP0_4_CMP_1_PROTECTION_4MB_LOWER            0x06
#define BP0_4_CMP_1_PROTECTION_8064KB_UPPER         0x09
#define BP0_4_CMP_1_PROTECTION_7936KB_UPPER         0x0A
#define BP0_4_CMP_1_PROTECTION_7680KB_UPPER         0x0B
#define BP0_4_CMP_1_PROTECTION_7MB_UPPER            0x0C
#define BP0_4_CMP_1_PROTECTION_6MB_UPPER            0x0D
#define BP0_4_CMP_1_PROTECTION_4MB_UPPER            0x0E

#define BP0_4_CMP_1_PROTECTION_NONE_MASK            0x07
#define BP0_4_CMP_1_PROTECTION_NONE                 0x07

#define BP0_4_CMP_1_PROTECTION_8188KB_BOTTOM        0x11
#define BP0_4_CMP_1_PROTECTION_8184KB_BOTTOM        0x12
#define BP0_4_CMP_1_PROTECTION_8176KB_BOTTOM        0x13
#define BP0_4_CMP_1_PROTECTION_8160KB_BOTTOM        0x14
#define BP0_4_CMP_1_PROTECTION_8160KB_BOTTOM_1      0x15
#define BP0_4_CMP_1_PROTECTION_8160KB_BOTTOM_2      0x16

#define BP0_4_CMP_1_PROTECTION_8188KB_TOP           0x19
#define BP0_4_CMP_1_PROTECTION_8184KB_TOP           0x1A
#define BP0_4_CMP_1_PROTECTION_8176KB_TOP           0x1B
#define BP0_4_CMP_0_PROTECTION_8160KB_TOP           0x1C
#define BP0_4_CMP_0_PROTECTION_8160KB_TOP_1         0x1D
#define BP0_4_CMP_0_PROTECTION_8160KB_TOP_2         0x1E

typedef enum 
{
    PROTECTION_NONE,           
    PROTECTION_ALL,       

    PROTECTION_128KB_UPPER,        
    PROTECTION_256KB_UPPER,       
    PROTECTION_512KB_UPPER,       
    PROTECTION_1MB_UPPER,          
    PROTECTION_2MB_UPPER,         
    PROTECTION_4MB_UPPER,      
    
    PROTECTION_128KB_LOWER,       
    PROTECTION_256KB_LOWER,        
    PROTECTION_512KB_LOWER,        
    PROTECTION_1MB_LOWER,         
    PROTECTION_2MB_LOWER,          
    PROTECTION_4MB_LOWER,        

    PROTECTION_4KB_TOP,           
    PROTECTION_8KB_TOP,           
    PROTECTION_16KB_TOP,           
    PROTECTION_32KB_TOP,         
    PROTECTION_4KB_BOTTOM,         
    PROTECTION_8KB_BOTTOM,       
    PROTECTION_16KB_BOTTOM,       
    PROTECTION_32KB_BOTTOM,       
              
    PROTECTION_8064KB_LOWER,        
    PROTECTION_7936KB_LOWER,         
    PROTECTION_7680KB_LOWER,         
    PROTECTION_7MB_LOWER,            
    PROTECTION_6MB_LOWER,           
    //PROTECTION_4MB_LOWER,           

    PROTECTION_8064KB_UPPER,        
    PROTECTION_7936KB_UPPER,        
    PROTECTION_7680KB_UPPER,        
    PROTECTION_7MB_UPPER,           
    PROTECTION_6MB_UPPER,            
    //PROTECTION_4MB_UPPER,          

    PROTECTION_8188KB_BOTTOM,        
    PROTECTION_8184KB_BOTTOM,      
    PROTECTION_8176KB_BOTTOM,      
    PROTECTION_8160KB_BOTTOM,        
    PROTECTION_8188KB_TOP,           
    PROTECTION_8184KB_TOP,          
    PROTECTION_8176KB_TOP,          
    PROTECTION_8160KB_TOP,           

}   SPI_FLASH_ProtectArea_t;

/*****************************************************************
**  Command list
*****************************************************************/
#define FLASH_WRITE_ENABLE     0x06
#define FLASH_WRITE_DISABLE    0x04
#define FLASH_SR_WRITE_ENABLE  0x50
#define FLASH_READ_SR1         0x05
#define FLASH_READ_SR2         0x35
#define FLASH_READ_SR3         0x15
#define FLASH_WRITE_SR1        0x01
#define FLASH_WRITE_SR2        0x31
#define FLASH_WRITE_SR3        0x11
#define FLASH_READ_DATA        0x03
#define FLASH_PAGE_PROGRAM     0x02
#define FLASH_SECTOR_ERASE     0x20
#define FLASH_BLOCK_ERASE_32K  0x52
#define FLASH_BLOCK_ERASE_64K  0xD8
#define FLASH_CHIP_ERASE0      0x60
#define FLASH_CHIP_ERASE1      0xC7

/*****************************************************************
**  FLASH SIZE
*****************************************************************/
#define GD25Q_PAGE_SIZE           256u        // 0x100
#define GD25Q_SECTOR_PAGE_NUM     16u         // 0x10
#define GD25Q_SECTOR_SIZE         (GD25Q_PAGE_SIZE * GD25Q_SECTOR_PAGE_NUM)       // 0x1000

// chip type
#define GD25Q128C
#define GD25Q_FLASH_START_ADDR		    (0x00u)

#ifdef GD25Q64C
#define GD25Q_FLASH_TOTAL_SIZE          (GD25Q_SECTOR_SIZE * 2048u)
#elif defined(GD25Q128C)
#define GD25Q_FLASH_TOTAL_SIZE          (GD25Q_SECTOR_SIZE * 4096u)
#endif

/*****************************************************************
**  FUNCTION
*****************************************************************/
//   initialize spi falsh
Status_t SPI_Flash_Init(void);

//  Erase Sector
Status_t SPI_FLASH_EraseSector(uint32_t addr);

// write data to flash, max len is 256
Status_t SPI_FLASH_WriteData(uint32_t addr, uint8_t *data, uint32_t len, uint32_t *wLen);

// read data from flash, max len is 256
Status_t SPI_FLASH_ReadData(uint32_t addr, uint8_t *data, uint32_t len, uint32_t *rLen);

#endif
