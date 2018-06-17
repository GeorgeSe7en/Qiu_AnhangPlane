/***********************************************************
**	Uart.c
**	Author: Ji Zhou
**	Time:	2018.5.31
***********************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f1xx_hal.h"
#include "Global.h"
/**********************************************************
**	Extern Variable
***********************************************************/
extern UART_HandleTypeDef huart4;		// gps
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;		// debug uart

/**********************************************************
**	type definitions
***********************************************************/
// GGA message
typedef struct gps_gga_cstr_t
{
	char name[5];
	char time[10];		// hhmmss.ss
	char lat[12];		// ddmm.mmmmm N or S
	char lon[13];		// dddmm.mmmmm E or W
	char status[1];		// '0' or '1'
	char noSV[2];		// number of stars
	char hdOP[10];
	char msl[10];
	char M[1];
	char Altref[10];
	char sM[1];
	char DiffAge[10];
	char DiffStatio[10];
	char cs[2];
} gps_gga_cstr_t;

// RMC message
typedef struct gps_rmc_cstr_t
{
	char name[5];
	char time[10];		//hhmmss.ss
	char status[1];		// 'V' or 'A'
	char lat[12];		// ddmm.mmmmm N or S
	char lon[13];		// dddmm.mmmmm E or W
	char speed[10];		// speed, unit knot
	char cog[10];		// speed vector
	char date[6];		// ddmmyy
	char mv[10];
	char mvE[1];
	char mode[1];
	char cs[2];
} gps_rmc_cstr_t;

//time
typedef struct RecordTime  gps_time_t;
/**********************************************************
**	private variable
***********************************************************/
//const data

static gps_gga_cstr_t gps_gga_msg;
static gps_rmc_cstr_t gps_rmc_msg;
static gps_time_t	  gps_time = {'R', 18, 6, 1, 18, 30, 0};
static uint8_t stars = 0;

static uint8_t starsFlag=1;
static uint8_t preStars = 0xFF;
// gps message tempe buffer
//static char gpsMesg[128];

/*********************************************************
**	Extern Function
**********************************************************/
int get_OneMessage(char *msg)
{
	return get_LineFromGPSMsg(msg);
}


/*********************************************************
**	Function
**********************************************************/
uint32_t get_str(const char *src, char *dst, uint32_t dstSize, char ch)
{
	uint32_t size = 0;
	
	while(*src != '\0' && *src != ch && dstSize)
	{
		*dst++ = *src++;
		dstSize--;
		size++;
	}
	if(dstSize) *dst = '\0';
	return size;
}

void GPS_MessgeDecode(void)
{
	char gpsMesg[128] = {0};
	uint32_t len, index;
	uint8_t flag;
	
	while(get_OneMessage(gpsMesg))
	{
		flag = 0;
		// GGA Message
		if(strncmp("$GNGGA", gpsMesg, 6) == 0)
		{
			// name
			memcpy(gps_gga_msg.name, "GNGGA", 5);
			index = 7;
			//time
			len = get_str(&gpsMesg[index], gps_gga_msg.time, 10, ',');
			index += len + 1;		// jump the ','
			// lat
			len = get_str(&gpsMesg[index], gps_gga_msg.lat, 10, ',');
			index += len + 1;		// jump the ','
			// north or south himisphere
			gps_gga_msg.lat[len+1] = '\0';
			len = get_str(&gpsMesg[index], &gps_gga_msg.lat[len], 1, ',');
			index += len + 1;		// jump the ','
			
			// lon
			len = get_str(&gpsMesg[index], gps_gga_msg.lon, 11, ',');
			index += len + 1;		// jump the ','
			// East and west longitude
			gps_gga_msg.lon[len+1] = '\0';
			len = get_str(&gpsMesg[index], &gps_gga_msg.lon[len], 1, ',');
			index += len + 1;		// jump the ','
			
			//gps status
			len = get_str(&gpsMesg[index], gps_gga_msg.status, 1, ',');
			index += len + 1;		// jump the ','
			
			//number of stars
			len = get_str(&gpsMesg[index], gps_gga_msg.noSV, 2, ',');
			index += len + 1;		// jump the ','
			if(len) flag = 1;
			//HDOP
			len = get_str(&gpsMesg[index], gps_gga_msg.hdOP, 10, ',');
			index += len + 1;		// jump the ','
			
			//MSL
			len = get_str(&gpsMesg[index], gps_gga_msg.msl, 10, ',');
			if(len == 10) gps_gga_msg.msl[9] = '\0';
			index += len + 1;		// jump the ','
			
			// M
			len = get_str(&gpsMesg[index], gps_gga_msg.M, 1, ',');
			index += len + 1;		// jump the ','
			
			// Altref
			len = get_str(&gpsMesg[index], gps_gga_msg.Altref, 10, ',');
			index += len + 1;		// jump the ','
			
			// sM
			len = get_str(&gpsMesg[index], gps_gga_msg.sM, 1, ',');
			index += len + 1;		// jump the ','
			
			// DiffAge
			len = get_str(&gpsMesg[index], gps_gga_msg.DiffAge, 10, ',');
			index += len + 1;		// jump the ','
			
			// Diffstatio
			len = get_str(&gpsMesg[index], gps_gga_msg.DiffStatio, 10, '*');
			index += len + 1;		// jump the ','
			
			// cs
			len = get_str(&gpsMesg[index], gps_gga_msg.cs, 2, '\r');
			
			if(flag == 1)
			{
				// get number of stars
				stars = gps_gga_msg.noSV[0] - '0';
				stars *= 10;
				stars += gps_gga_msg.noSV[1] - '0';
				
				if(preStars != stars)
				{
					preStars = stars;
					starsFlag = 1;
				}
			}
		}
		
		// RMC Message
		else if(strncmp("$GNRMC", gpsMesg, 6) == 0)
		{
			// name
			memcpy(gps_rmc_msg.name, "GNRMC", 5);
			index = 7;
			//time
			len = get_str(&gpsMesg[index], gps_rmc_msg.time, 10, ',');
			index += len + 1;		// jump the ','
			if(len) flag |= 0x01;
			// status 'V or 'A'
			len = get_str(&gpsMesg[index], gps_rmc_msg.status, 1, ',');
			index += len + 1;		// jump the ','
			
			// lat
			len = get_str(&gpsMesg[index], gps_rmc_msg.lat, 10, ',');
			index += len + 1;		// jump the ','
			// north or south himisphere
			gps_gga_msg.lat[len+1] = '\0';
			len = get_str(&gpsMesg[index], &gps_rmc_msg.lat[len], 1, ',');
			index += len + 1;		// jump the ','
			
			// lon
			len = get_str(&gpsMesg[index], gps_rmc_msg.lon, 11, ',');
			index += len + 1;		// jump the ','
			// East and west longitude
			gps_gga_msg.lon[len+1] = '\0';
			len = get_str(&gpsMesg[index], &gps_rmc_msg.lon[len], 1, ',');
			index += len + 1;		// jump the ','
			
			// speed
			len = get_str(&gpsMesg[index], gps_rmc_msg.speed, 10, ',');
			index += len + 1;		// jump the ','
			
			//cog
			len = get_str(&gpsMesg[index], gps_rmc_msg.cog, 10, ',');
			index += len + 1;		// jump the ','
			
			//date
			len = get_str(&gpsMesg[index], gps_rmc_msg.date, 6, ',');
			index += len + 1;		// jump the ','
			if(len) flag |= 0x02;
			// Mv
			len = get_str(&gpsMesg[index], gps_rmc_msg.mv, 10, ',');
			index += len + 1;		// jump the ','
			
			// Altref
			len = get_str(&gpsMesg[index], gps_rmc_msg.mvE, 1, ',');
			index += len + 1;		// jump the ','
			
			// mode
			len = get_str(&gpsMesg[index], gps_rmc_msg.mode, 1, '*');
			index += len + 1;		// jump the ','
			
			// cs
			len = get_str(&gpsMesg[index], gps_rmc_msg.cs, 10, ',');
			
			if(flag & 0x01)
			{
				// hour
				gps_time.hour = gps_rmc_msg.time[0] - '0';
				gps_time.hour *= 10;
				gps_time.hour += gps_rmc_msg.time[1] - '0';
				
				// minute
				gps_time.minute = gps_rmc_msg.time[2] - '0';
				gps_time.minute *= 10;
				gps_time.minute += gps_rmc_msg.time[3] - '0';
				
				// second
				gps_time.second = gps_rmc_msg.time[4] - '0';
				gps_time.second *= 10;
				gps_time.second += gps_rmc_msg.time[5] - '0';
			}
			
			if(flag & 0x02)
			{
				// year
				gps_time.year = gps_rmc_msg.date[4] - '0';
				gps_time.year *= 10;
				gps_time.year += gps_rmc_msg.date[5] - '0';
				
				// month
				gps_time.month = gps_rmc_msg.date[2] - '0';
				gps_time.month *= 10;
				gps_time.month += gps_rmc_msg.date[3] - '0';
				
				// day
				gps_time.day = gps_rmc_msg.date[0] - '0';
				gps_time.day *= 10;
				gps_time.day += gps_rmc_msg.date[1] - '0';
			}
			
			if((flag & 0x01) && (flag & 0x02))
			{
				global_updatetime(&gps_time);
			}
		}
	}
}

// longitude and latitude
void get_LatLon(char *buf)
{
	int i;
	
	buf[0] = gps_rmc_msg.status[0];
	buf[1] = ':';
	i = strlen(strcpy(&buf[2], gps_rmc_msg.lat));		// 纬度
	buf[i + 2] = ' ';
	i += strlen(strcpy(&buf[i+3], gps_rmc_msg.lon));		// 经度
	buf[i + 3] = ' ';
	i += strlen(strcpy(&buf[i+4], gps_gga_msg.msl));			// 海拨
	buf[i+4] = '\0';
}

int get_gpsLevel(uint8_t *st)
{
	if(starsFlag == 0) return 0;
	
	*st = stars;
	starsFlag = 0;
	return 1;
}



