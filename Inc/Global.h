/************************************************************
**  全局定义
**  项目：多路PWM测量记录
**  作者：周吉
**  2018年5月22日
************************************************************/
#ifndef __GLOBAL_H__
#define __GLOBAL_H__
/***********************************************************
**  INCLUDES
***********************************************************/
#include <stdint.h>
#include <stdbool.h>

#include "stm32f1xx_hal.h"
/************************************************************
**  MARCO DEFINITION
************************************************************/
//watchdog enable
#define WATCHDOG_ENABLE  0
// power on pin
#define POWER_HOLD_HIGH()     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET)
#define POWER_HOLD_LOW()      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET)
/************************************************************
**  TYPES REFERENCE
************************************************************/
typedef enum
{
    STATUS_UNKNOW = -2,
    STATUS_ERROR = -1,
    STATUS_OK = 0,
    STATUS_BUSY = 1,
    STATUS_READY = 2,
    STATUS_FLASH_PARAM_INVALID = 0x80,

}   Status_t;
/************************************************************
**  EXTERN REFERENCE
************************************************************/
void POWER_OffCallback(void);

/************************************************************
**  VARIABLE DEFINITION
************************************************************/
#define PRINT_BUF_LEN	128u		// print string buffer length
#define CMD_BUF_LEN		8u		// cmd recive buffer length
#define GPS_SEND_BUF_LEN	32u	// gps rx buffer
#define GPS_RECIVE_BUF_LEN	16u	//

//cmd direct
typedef struct Direct
{
	uint8_t cmd;
	
	uint8_t year;
	uint8_t month;
	uint8_t day;
	
}	Direct_t;


// time
struct RecordTime
{
    /* data */
    char tag;
    uint8_t year;
    uint8_t month;
    uint8_t day;

    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

/*! Debug Print Macro ----------------------------------------*/
#define printlog(fmt, ...)
#define printmsg(fmt, ...)
#define printerr(fmt, ...)
/************************************************************
**  FUNCTION DEFINITION
************************************************************/
static inline uint32_t Time_GetCount(void)
{
    return HAL_GetTick();
}

static inline void Time_DelayMs(uint32_t x)
{
    uint32_t time = HAL_GetTick();

    while(HAL_GetTick() - time < x);

    return;
}

// user uart print format string
extern int uart_printf(const char *fmt, ...);   // format character string 
extern int uart_printStatus(void);              // 1 busy; 0 ready
extern int uart_print(const char *str);                     // output character string 
extern int uart_send(const uint8_t *dat, uint32_t len);    // output data

// recive data init
void uart_rx_init(void);
// recive data processing
void uart_rx_processing(void);
// read a msg
int get_LineFromGPSMsg(char *buf);

////
void GPS_MessgeDecode(void);
int get_gpsLevel(uint8_t *st);
void get_LatLon(char *buf);

// uart recive cmd
// cmd RX
void CMD_receive(void);
void CMD_SendRXError(void);
uint8_t CMD_getFlag(void);
void CMD_clearFlag(uint8_t cflag);
bool CMD_getDirect(Direct_t *cmd);

// application
void global_updatetime(struct RecordTime *time);
void time_increase(void);

int get_mytime(struct RecordTime *time);
int get_timeDate(char *time, char *date);
int get_timeString(char *str);

#endif
/****************************END****************************/
