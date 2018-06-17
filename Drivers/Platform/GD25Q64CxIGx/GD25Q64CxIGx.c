/*******************************************************
**  GD25Q64C 驱动
**  项目：多路PWM测量记录
**  作者：周吉
**  2018年5月21日
*******************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "Global.h"
#include "GD25Q64CxIGx.h"
/******************************************************
**  Macro Define
******************************************************/
#define SPI_CS_LOW()	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET)
#define SPI_CS_HIGH()	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET)

#define CMD_LEN     1u
#define ADDR_LEN    3u
#define WRITE_HEAD  (CMD_LEN+ADDR_LEN)
#define MAX_DATA_LEN    256u

/******************************************************
**  Variable Define
******************************************************/
static uint8_t spi_tx_buffer[CMD_LEN+ADDR_LEN+MAX_DATA_LEN];
static uint8_t spi_rx_buffer[CMD_LEN+ADDR_LEN+MAX_DATA_LEN];

/*******************************************************
**  Extern Function
*******************************************************/
extern SPI_HandleTypeDef hspi2;
static volatile uint8_t TxRxCompleteFlag = 1;
void waitDelay(void);

// interrupt callback
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi == &hspi2)
	{
		TxRxCompleteFlag = 1;
        SPI_CS_HIGH();
	}
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi == &hspi2)
	{
		TxRxCompleteFlag = 1;
        SPI_CS_HIGH();
	}
}

int SPI_Transfer(uint8_t *txData, uint32_t txSize, uint8_t *rxData, uint32_t rxSize)
{
	while(TxRxCompleteFlag == 0)
    {
        __NOP();
    }
    TxRxCompleteFlag = 0;
	SPI_CS_LOW();
	if(rxSize == 0)
	{
		HAL_SPI_Transmit_IT(&hspi2, txData, txSize);
	}
	else
	{
		HAL_SPI_TransmitReceive_IT(&hspi2, txData, rxData, txSize > rxSize ? txSize : rxSize);
	}
    while(TxRxCompleteFlag == 0)
    {
        __NOP();
    }
	return 0;
}

// marco operator
#define WRITE_ENABLE()  \
{   \
    spi_tx_buffer[0] = FLASH_WRITE_ENABLE;  \
    SPI_Transfer(spi_tx_buffer, 1, NULL, 0);   \
}

#define WRITE_DISABLE() \
{   \
    spi_tx_buffer[0] = FLASH_WRITE_DISABLE;  \
    SPI_Transfer(spi_tx_buffer, 1, NULL, 0);   \
}

#define WRITE_SR_ENABLE()   \
{   \
    spi_tx_buffer[0] = FLASH_SR_WRITE_ENABLE;  \
    SPI_Transfer(spi_tx_buffer, 1, NULL, 0);   \
}

#define READ_SR1(val)   \
{   \
    spi_tx_buffer[0] = FLASH_READ_SR1;  \
    SPI_Transfer(spi_tx_buffer, 2, spi_rx_buffer, 2);   \
    val = spi_rx_buffer[1]; \
}

#define READ_SR2(val)   \
{   \
    spi_tx_buffer[0] = FLASH_READ_SR2;  \
    SPI_Transfer(spi_tx_buffer, 2, spi_rx_buffer, 2);   \
    val = spi_rx_buffer[1]; \
}

#define READ_SR3(val)   \
{   \
    spi_tx_buffer[0] = FLASH_READ_SR3;  \
    SPI_Transfer(spi_tx_buffer, 2, spi_rx_buffer, 2);   \
    val = spi_rx_buffer[1]; \
}

#define WRITE_SR1(val)  \
{   \
    spi_tx_buffer[0] = FLASH_WRITE_SR1;  \
    spi_tx_buffer[1] = (val);  \
    SPI_Transfer(spi_tx_buffer, 2, NULL, 0);   \
}

#define WRITE_SR2(val)  \
{   \
    spi_tx_buffer[0] = FLASH_WRITE_SR2;  \
    spi_tx_buffer[1] = (val);  \
    SPI_Transfer(spi_tx_buffer, 2, NULL, 0);   \
}

#define WRITE_SR3(val)  \
{   \
    spi_tx_buffer[0] = FLASH_WRITE_SR3;  \
    spi_tx_buffer[1] = (val);  \
    SPI_Transfer(spi_tx_buffer, 2, NULL, 0);   \
}

#define ERASE_SECTOR(addr)  \
{   \
    spi_tx_buffer[0] = FLASH_SECTOR_ERASE;  \
    spi_tx_buffer[1] = (addr)>>16; spi_tx_buffer[2] = (addr)>>8; spi_tx_buffer[3] = (addr);  \
    SPI_Transfer(spi_tx_buffer, 4, NULL, 0);   \
}

#define SET_WRITE_OPERATE(addr, data, size)    \
{   \
    spi_tx_buffer[0] = FLASH_PAGE_PROGRAM;  \
    spi_tx_buffer[1] = (addr)>>16; spi_tx_buffer[2] = (addr)>>8; spi_tx_buffer[3] = (addr);  \
}

#define DO_WRITE_OPERATE(addr, data, size) \
{   \
    SPI_Transfer(spi_tx_buffer, size + 4, NULL, 0);   \
}

#define SET_READ_OPERATE(addr, data, size)    \
{   \
    spi_tx_buffer[0] = FLASH_READ_DATA;  \
    spi_tx_buffer[1] = (addr)>>16; spi_tx_buffer[2] = (addr)>>8; spi_tx_buffer[3] = (addr);  \
}

#define DO_READ_OPERATE(addr, data, size)   \
{   \
    SPI_Transfer(spi_tx_buffer, 4, spi_rx_buffer, size + 4);   \
}

// wait complete
#define WAIT_OPERATOR() \
do{   \
    waitDelay();    \
    spi_tx_buffer[0] = FLASH_READ_SR1;  \
    SPI_Transfer(spi_tx_buffer, 2, spi_rx_buffer, 2);   \
}while(spi_rx_buffer[1] & 0x01)
/*******************************************************
**  Function
*******************************************************/
//  write status register 1
static void writeStatusReg1(uint8_t value)
{
    WRITE_ENABLE();
    WRITE_SR_ENABLE();

    WRITE_SR1(value);       // write status register  1

    WAIT_OPERATOR();        //wait for operator complete

    WRITE_DISABLE();
}
//  write status register 2
static void writeStatusReg2(uint8_t value)
{
    WRITE_ENABLE();
    WRITE_SR_ENABLE();

    WRITE_SR2(value);       // write status register  2

    WAIT_OPERATOR();        //wait for operator complete
    
    WRITE_DISABLE();
}
//  write status register 3
static void writeStatusReg3(uint8_t value)
{
    WRITE_ENABLE();
    WRITE_SR_ENABLE();

    WRITE_SR3(value);       // write status register  3
    WAIT_OPERATOR();        //wait for operator complete
    
    WRITE_DISABLE();
}

//  erase sector
static void EraseSector(uint32_t addr)
{
    WRITE_ENABLE();

    ERASE_SECTOR(addr);     // erase
    WAIT_OPERATOR();        //wait for operator complete

    WRITE_DISABLE();
}

//  write data
static int WriteData(uint32_t addr, uint8_t *data, uint32_t size)
{
    uint32_t i, num, ret = 0;
    uint8_t *pTxBuf;

    if(size == 0) return ret;

    do
    {
        num = 256u - (addr&0xFF);
        if(num > size) num = size;

        WRITE_ENABLE();

        SET_WRITE_OPERATE(addr, data, num);
        pTxBuf = &spi_tx_buffer[WRITE_HEAD];
        
        // move data to user buffer
        for(i=0;i<num;i++)
        {
            *pTxBuf++ = *data++;
        }
        DO_WRITE_OPERATE(addr, data, num);

        WAIT_OPERATOR();        //wait for operator complete
        WRITE_DISABLE();

        size -= num;
        addr += num;
        ret += num;
    }while(size);

    return ret;
}

//  read data
static int ReadData(uint32_t addr, uint8_t *data, uint32_t size)
{
    uint8_t *pRxBuf;
    uint32_t i, num, ret = 0;

    if(size == 0) return ret;

    do
    {
        if(size > MAX_DATA_LEN)
        {
            num = MAX_DATA_LEN;
        }
        else
        {
            num = size;
        }

        SET_READ_OPERATE(addr, data, num);
        DO_READ_OPERATE(addr, data, num);

        //  move data
        pRxBuf = &spi_rx_buffer[WRITE_HEAD];
        for(i=0;i<num;i++)
        {
            *data++ = *pRxBuf++;
        }

        size -= num;
        addr += num;
        ret += num;
    }while(size);

    return ret;
}

//  delay function
void waitDelay(void)
{
    int i;
    for(i=1000; i>0; i--)
    {
        // do nothing
        __NOP();
    }
}

/****************************************************************
**  Public Interface
****************************************************************/
//   initialize spi falsh
Status_t SPI_Flash_Init(void)
{
    uint8_t stuReg;

    //  读SR3寄存器
    READ_SR3(stuReg);
    if(stuReg & 0x40)
    {
        // WPS位不为0，直接重写保护位
        stuReg &= ~0x40;
        writeStatusReg3(stuReg);
    }
    
    //  读SR2寄存器
    READ_SR2(stuReg);
    if(stuReg & 0x40)
    {
        // CMP位不为0，直接重写保护位
        stuReg &= ~0x40;
        writeStatusReg2(stuReg);
    }

    //  读SR0寄存器
    READ_SR1(stuReg);
    if(stuReg & 0x1C)
    {
        //  BP0~2不为0，直接重写保护位
        stuReg &= ~0x1C;
        writeStatusReg1(stuReg);
    }

    return STATUS_OK;
}

//  Erase Sector
Status_t SPI_FLASH_EraseSector(uint32_t addr)
{

    // check params
    if(addr >= GD25Q_FLASH_TOTAL_SIZE)
    {
        return STATUS_FLASH_PARAM_INVALID;
    }

    addr &= ~(GD25Q_SECTOR_SIZE -1);

    EraseSector(addr); // erase sector

    return STATUS_OK;
}

// write data to flash, max len is 256
Status_t SPI_FLASH_WriteData(uint32_t addr, uint8_t *data, uint32_t len, uint32_t *wLen)
{
    uint32_t wSize;

    // write data to flash
    wSize = WriteData(addr, data, len);

    if(wLen != NULL)
    {
        *wLen = wSize;
    }

    return STATUS_OK;
}

// read data from flash, max len is 256
Status_t SPI_FLASH_ReadData(uint32_t addr, uint8_t *data, uint32_t len, uint32_t *rLen)
{
    uint32_t rSize;

    // read data
    rSize = ReadData(addr, data, len);

    if(rLen != NULL)
    {
        *rLen = rSize;
    }

    return STATUS_OK;
}
