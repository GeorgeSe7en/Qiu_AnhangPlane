/******************************************************/
/*  Application.c                                     */
/*  Auther : Zhou Ji                                  */
/*  Date:   2018-06-15  21:40                         */
/******************************************************/

/*! Include ------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f1xx_hal.h"

#include "Global.h"
//#include "application.h"

/*! Macro --------------------------------------------*/
#define ISLeapYear(x)	(!((x)&0x03))


/*! variable -----------------------------------------*/
static const uint8_t numberOfMonth[] =
{
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

struct RecordTime mytime = 
{
    'L', 18, 6, 15, 20, 30, 0
};

static uint8_t preMinute = 60;
/*! Function -----------------------------------------*/
void time_increase(void)
{
    mytime.second += 1;
	if(mytime.second >= 60)
	{
		mytime.second = 0;
		mytime.minute += 1;
		if(mytime.minute >= 60)
		{
	    	mytime.minute = 0;
			mytime.hour += 1;
			if(mytime.hour >= 24)
			{
				mytime.hour = 0;
				mytime.day += 1;
				if(ISLeapYear(mytime.year) && mytime.month == 2)		// leap year and February
				{
					if(mytime.day > numberOfMonth[mytime.month-1] + 1)
					{
						mytime.day = 1;
						mytime.month += 1;
						if(mytime.month > 12)
						{
							mytime.month  = 1;
							mytime.year += 1;
						}
					}
				}
				else
				{
					if(mytime.day > numberOfMonth[mytime.month-1])
					{
						mytime.day = 1;
						mytime.month += 1;
						if(mytime.month > 12)
						{
							mytime.month  = 1;
							mytime.year += 1;
						}
					}
				}
			}
		}
	}
}

void global_updatetime(struct RecordTime *time)
{
    static uint32_t updatetime = 0;

    if(mytime.tag == 'L' && time->tag == 'R')
    {
UPDATE:
        if(time->hour > 23 || time->minute > 59 || time->second > 59)
        {
            return;
        }
        if(ISLeapYear(mytime.year) && mytime.month == 2)		// leap year and February
		{
            if(time->day < 1 || time->day > numberOfMonth[mytime.month-1] + 1)
            {
                return;
            }
        }
        else
        {
            if(time->month < 1 || time->month > 12 ||
                time->day < 1 || time->day > numberOfMonth[mytime.month-1])
            {
                return;
            }
        }

        updatetime = HAL_GetTick();
        __disable_irq();
        mytime.tag = time->tag;
        mytime.year = time->year;
        mytime.month = time->month;
        mytime.day = time->day;

        mytime.hour = time->hour;
        mytime.minute = time->minute;
        mytime.second = time->second;

        mytime.hour += 8;
		if(mytime.hour >= 24)
		{
			mytime.hour -= 24;
			// update date!
			mytime.day += 1;
			if(ISLeapYear(mytime.year) && mytime.month == 2)		// leap year and February
			{
				if(mytime.day > numberOfMonth[mytime.month-1] + 1)
				{
					mytime.day = 1;
					mytime.month += 1;
					if(mytime.month > 12)
					{
						mytime.month  = 1;
						mytime.year += 1;
					}
				}
			}
			else
			{
				if(mytime.day > numberOfMonth[mytime.month-1])
				{
					mytime.day = 1;
					mytime.month += 1;
					if(mytime.month > 12)
					{
						mytime.month  = 1;
						mytime.year += 1;
					}
				}
			}
		}
        __enable_irq();
		
		preMinute = 60;
    }
    else
    {
        if(HAL_GetTick() - updatetime > (1000*60*60))
        {
            goto UPDATE;
        }
    }
    
    return;
}

int get_mytime(struct RecordTime *time)
{
    __disable_irq();
    memcpy(time, &mytime, sizeof(struct RecordTime));
    __enable_irq();

    return 0;
}

int get_timeDate(char *time, char *date)
{
    if(preMinute != mytime.minute)
    {
        __disable_irq();
        preMinute = mytime.minute;
        sprintf(time, "%02d:%02d", mytime.hour, mytime.minute);
        sprintf(date, "%d-%02d-%02d", mytime.year, mytime.month, mytime.day);
        __enable_irq();
        return 0;
    }

    return -1;
}

int get_timeString(char *str)
{
    __disable_irq();
    sprintf(str, "%c:%d/%02d/%02d/%02d-%02d:&02d:%02d", 
            mytime.tag,
            mytime.year + 2000,
            mytime.month,
            mytime.day,
            mytime.hour, 
            mytime.minute,
            mytime.second);
    __enable_irq();
    return 0;
}
