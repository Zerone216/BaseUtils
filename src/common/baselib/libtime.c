/****************************************************************************************************************************
*
*   文 件 名 ： libtime.c 
*   文件描述 ：  
*   创建日期 ：2019年1月10日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include <linux/capability.h> 
#include <unistd.h>
#include <ctype.h>
#include <regex.h>
#include <stdarg.h>	/* ANSI C header file */
#include <syslog.h> 	/* for syslog() */
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <sys/wait.h>

#include "libtime.h"

int GetDaysOfMonth(int year, int month)
{
	int days = 0;
	switch(month)
	{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			days = 31;
			break;
		case 2:
			days = (LEAP_YEAR(year) ? 29 : 28);
			break;
		case 4:
		case 6:
		case 9:
		case 11:
			days = 30;
			break;
	}
	
	return days;
}

int GetDaysOfYear(int year){return (LEAP_YEAR(year) ? 366 : 365);}

int CalcDaysPassOfYear(int year, int month, int day)
{
	int dayPass = 0;
	int i = 1;
	for(i = 1; i < month; i ++)
		dayPass += GetDaysOfMonth(year, i);
	
	dayPass += (day - 1);
	return dayPass;
}

DDWORD CalcSecondsPassOfYear(struct tm time)
{
	DDWORD timePass = CALC_SECONDS_PASS_OF_DAY(time.tm_hour, time.tm_min, time.tm_sec) +\
						CalcDaysPassOfYear(time.tm_year, time.tm_mon, time.tm_mday) * SECONDS_PER_DAY;
	return timePass;
}


//Time Expression: "2019-03-12 09:32:57" or "2019-3-12 9:32:57", "2019/03/12 09:32:57" or "2019/3/12 9:32:57", "2019_03_12 09:32:57" or "2019_3_12 9:32:57"
struct tm GetTimeInfoFromExpress(char * timeExp)
{
	struct tm time;
	memset(&time, 0x00, sizeof(struct tm));
	
	char timeExpStr[64] = {'\0'};
	if(get_substr_by_regex_match(timeExp, "[0-9]{4}-[0-9]{1,2}-[0-9]{1,2} [0-9]{1,2}:[0-9]{1,2}:[0-9]{1,2}", timeExpStr, sizeof(timeExpStr) )== 0)
		sscanf(timeExpStr, "%d%*[-]%d%*[-]%d %d%*[:]%d%*[:]%d" , &time.tm_year, &time.tm_mon, &time.tm_mday, &time.tm_hour, &time.tm_min, &time.tm_sec);
	else if(get_substr_by_regex_match(timeExp, "[0-9]{4}/[0-9]{1,2}/[0-9]{1,2} [0-9]{1,2}:[0-9]{1,2}:[0-9]{1,2}", timeExpStr, sizeof(timeExpStr)) == 0)
		sscanf(timeExpStr, "%d%*[/]%d%*[/]%d %d%*[:]%d%*[:]%d" , &time.tm_year, &time.tm_mon, &time.tm_mday, &time.tm_hour, &time.tm_min, &time.tm_sec);
	else if(get_substr_by_regex_match(timeExp, "[0-9]{4}_[0-9]{1,2}_[0-9]{1,2} [0-9]{1,2}:[0-9]{1,2}:[0-9]{1,2}", timeExpStr, sizeof(timeExpStr)) == 0)
		sscanf(timeExpStr, "%d%*[_]%d%*[_]%d %d%*[:]%d%*[:]%d" , &time.tm_year, &time.tm_mon, &time.tm_mday, &time.tm_hour, &time.tm_min, &time.tm_sec);
	else
		;
	
	return time;
}

//指定的时间段来计算经过的时间（以秒计算）
DDWORD CalcTimePassByAssignPeriod(struct tm start, struct tm end)
{
	DDWORD secondPass = 0;
	
	int i = 0;
	for(i = start.tm_year; i < end.tm_year; i ++)
		secondPass += GetDaysOfYear(i) * SECONDS_PER_DAY;

	secondPass += (long long)(CalcSecondsPassOfYear(end) - CalcSecondsPassOfYear(start));
	return secondPass;
}

//从过去的一个时间点开始加上经过的时间来计算新的时间点
struct tm CalcTimeFromPassPoint(struct tm timeStart, DDWORD timePass)
{
	DDWORD timePassStart = CalcSecondsPassOfYear(timeStart);
	DDWORD totalSecLeft = timePass + timePassStart; //
	DDWORD totalDayLeft = (totalSecLeft + SECONDS_PER_DAY - 1) / SECONDS_PER_DAY;
	DDWORD secLeftCurDay = totalSecLeft % SECONDS_PER_DAY;

	struct tm NewTime;
	memset(&NewTime, 0x00, sizeof(struct tm));
	NewTime.tm_year = timeStart.tm_year;
	NewTime.tm_mon = 1;
	NewTime.tm_mday = 1;
	
	while(totalDayLeft > 0)
	{
		//iLog("[%04d-%02d-%02d] totalDayLeft=[%llu]", NewTime.tm_year, NewTime.tm_mon, NewTime.tm_mday, totalDayLeft);
		DDWORD totalDayCurYear = GetDaysOfYear(NewTime.tm_year);
		if(totalDayLeft < totalDayCurYear)
			break;
		
		totalDayLeft -= totalDayCurYear;
		NewTime.tm_year ++;
	}
		
	while(totalDayLeft > 0)
	{
		//iLog("[%04d-%02d-%02d] totalDayLeft=[%llu]", NewTime.tm_year, NewTime.tm_mon, NewTime.tm_mday, totalDayLeft);
		DDWORD totalDayCurMonth = GetDaysOfMonth(NewTime.tm_year, NewTime.tm_mon);
		if(totalDayLeft < totalDayCurMonth)
			break;
		
		totalDayLeft -= totalDayCurMonth;
		NewTime.tm_mon ++;
	}
	
	NewTime.tm_mday = totalDayLeft;
	//iLog("[%04d-%02d-%02d] totalDayLeft=[%llu]", NewTime.tm_year, NewTime.tm_mon, NewTime.tm_mday, totalDayLeft);
	
	NewTime.tm_hour = secLeftCurDay / SECONDS_PER_HOUR;
	NewTime.tm_min = (secLeftCurDay % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE;
	NewTime.tm_sec = (secLeftCurDay % SECONDS_PER_HOUR) % SECONDS_PER_MINUTE;
	
	return NewTime;
}


struct timeval time_parse(double time)
{
	struct timeval tv;
	tv.tv_sec = (int)time;
	tv.tv_usec = ((time - tv.tv_sec) * 1000000) / 1000000;
	//iLog("time parse: %.6lfsec", time);
	
	return tv;
}

DDWORD time_trans_msec(struct timeval * timev)
{
	DDWORD timeTrans = timev->tv_sec * 1000 + timev->tv_usec / 1000;
	return timeTrans;
}

struct timeval calc_left_time(struct timeval * begintime, struct timeval * currenttime, struct timeval *  totaltime)
{
	DDWORD time_total = totaltime->tv_sec * 1000000 + totaltime->tv_usec;
	DDWORD time_pass = (currenttime->tv_sec * 1000000 + currenttime->tv_usec ) - (begintime->tv_sec * 1000000 + begintime->tv_usec );
	DDWORD time_left = (time_total > time_pass) ? (time_total - time_pass) : 0;
	
	struct timeval tv;
	tv.tv_sec = time_left / 1000000;
	tv.tv_usec = time_left % 1000000;
	
	return tv;
}

struct tm * get_sys_time()
{
	time_t plogtime;
	struct tm * ptimes = NULL;
	time(&plogtime);
	ptimes = gmtime(&plogtime);

	return ptimes;
}

int set_sys_time(struct timeval * time_tv)
{
    int ret = settimeofday(time_tv, NULL);
    if(ret != 0)
    {
        fprintf(stderr, "settimeofday failed\n");
        return -2;
    }
	
	return 0;
}

static int rtc_xopen(const char **default_rtc, int flags)
{
	int rtc;
	
	if (!*default_rtc) {
		*default_rtc = "/dev/rtc";
		rtc = open(*default_rtc, flags);
		if (rtc >= 0)
			return rtc;
		*default_rtc = "/dev/rtc0";
		rtc = open(*default_rtc, flags);
		if (rtc >= 0)
			return rtc;
		*default_rtc = "/dev/misc/rtc";
	}

	return open(*default_rtc, flags);
}

static void rtc_xclose(int rtc)
{
	close(rtc);
}

static void rtc_xwrite(int rtc, time_t t, int utc)
{
	struct tm tm_s = *(utc ? gmtime(&t) : localtime(&t));
	tm_s.tm_isdst = 0;
	ioctl(rtc, RTC_SET_TIME, &tm_s);
}

void set_hwclock_from_sysclock(int utc)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	
	const char *rtcname;
	int rtc = rtc_xopen(&rtcname, O_WRONLY);
	rtc_xwrite(rtc, tv.tv_sec, utc);
	rtc_xclose(rtc);
}

char * trans_to_weekday(int wkindex, char * weekstr, int buflen)
{
	const char *weekday[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
	memset(weekstr, '\0', buflen);
	
	if(wkindex >= 0 && wkindex <= 6)
		strncpy(weekstr,weekday[wkindex], buflen);

	return weekstr;
}

int update_systime_to_hwtime()
{
	system("hwclock --systohc");
	system("hwclock -w");
	return 0;
}

struct timeval get_utc_time()
{
	struct timeval utctime;
	memset(&utctime, 0x00, sizeof(struct timeval));
	
	gettimeofday(&utctime, NULL);
	return utctime;
}

struct timespec get_nano_time()
{
	struct timespec nano_time={0, 0};
	clock_gettime(CLOCK_REALTIME, &nano_time);

	return nano_time;
}

long get_nano_second()
{
	struct timespec ntime = get_nano_time();

	return ntime.tv_nsec;
}

