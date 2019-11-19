/****************************************************************************************************************************
*
*   文 件 名 ： libtime.h 
*   文件描述 ：  
*   创建日期 ：2019年1月10日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __LIBTIME_H__
#define __LIBTIME_H__

#include "libbase.h"
#include "log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*---------------------------------宏定义---------------------------------*/

#define RTC_SET_TIME _IOW('p', 0x0a, struct rtc_time) /* Set RTC time    */

#define LEAP_YEAR(y) (((0 == ((y) % 4) &&  ((y) % 100) != 0) || (0== ((y) % 400))) ? YES : NO) //判断是否为闰年

#define SECONDS_PER_MINUTE 60
#define MINUTES_PER_HOUR 60
#define SECONDS_PER_HOUR (MINUTES_PER_HOUR * SECONDS_PER_MINUTE)
#define HOURS_PER_DAY 24
#define SECONDS_PER_DAY (HOURS_PER_DAY * SECONDS_PER_HOUR)
#define CALC_SECONDS_PASS_OF_DAY(h, m, s) ((h) * SECONDS_PER_HOUR + (m) * SECONDS_PER_MINUTE + (s))
 
#pragma pack(1)

#pragma pack()

int GetDaysOfMonth(int year, int month);
int GetDaysOfYear(int year);
int CalcDaysPassOfYear(int year, int month, int day);
DDWORD CalcSecondsPassOfYear(struct tm time);
struct tm GetTimeInfoFromExpress(char * timeExp);

//指定的时间段来计算经过的时间（以秒计算）
DDWORD CalcTimePassByAssignPeriod(struct tm start, struct tm end);

//从过去的一个时间点开始加上经过的时间来计算新的时间点
struct tm CalcTimeFromPassPoint(struct tm timeStart, DDWORD timePass);

 struct timeval time_parse(double time);
 struct tm * get_sys_time();
 int set_sys_time(struct timeval * time_tv);
 struct timeval calc_left_time(struct timeval * begintime, struct timeval * currenttime, struct timeval *  totaltime);

 DDWORD time_trans_msec(struct timeval * timev);

 void set_hwclock_from_sysclock(int utc);
 int update_systime_to_hwtime();

 char * trans_to_weekday(int wkindex, char * weekstr, int buflen);

 struct timeval get_utc_time();
 struct timespec get_nano_time();
 long get_nano_second();

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __LIBTIME_H__ */
