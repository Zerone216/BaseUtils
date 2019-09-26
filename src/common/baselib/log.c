#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <syslog.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <ctype.h>
#include <sys/io.h>
#include <sys/stat.h>

#include "log.h"

static LOG_HEADER logHeader = {LOG_HEADER_SHOW_TYPE |\
						LOG_HEADER_SHOW_TPID |\
						LOG_HEADER_SHOW_TIME |\
						LOG_HEADER_SHOW_FILE_LINE, \
						LOG_DATE_ACCURATE_MDAY, \
						LOG_TIME_ACCURATE_SEC}; //LOG_HEADER_SHOW_DATE

LOG_FILE logFile = {LOG_FILE_NAME, LOG_FILE_DEFALT_MAX_SIZE, 0, TRUE};

void set_log_file_name(const char * filename)
{
	memset(logFile.filename, '\0', sizeof(logFile.filename));
	strncpy(logFile.filename, filename, sizeof(logFile.filename));
	return;	
}

void set_log_file_maxsize( int maxsize)
{
	logFile.maxsize = maxsize;
	return;	
}

void set_log_file_index(int index)
{
	logFile.index = index;
	return;	
}

void set_log_file_write_flag( BYTE writeFlag)
{
	logFile.writeFlag = writeFlag;
	return;	
}

//设置日志文件信息
void set_log_file_info(const char * filename, int maxsize, int index, BYTE writeFlag)
{
	memset(logFile.filename, '\0', sizeof(logFile.filename));
	strncpy(logFile.filename, filename, sizeof(logFile.filename));
	logFile.maxsize = maxsize;
	logFile.index = index;
	logFile.writeFlag = writeFlag;
	return;	
}

//获取日志文件信息
LOG_FILE get_log_file_info()
{
	return logFile;
}

void set_log_header_flag(BYTE flag)
{
	logHeader.header_show = flag;
	return;	
}

//设置调试信息头部日期精确度
void set_log_header_date_acc(BYTE dateAcc)
{
	logHeader.date_accurate = dateAcc;
	return;	
}
//设置调试信息头部时间精确度
void set_log_header_time_acc(BYTE timeAcc)
{
	logHeader.time_accurate = timeAcc;
	return;	
}

//设置调试信息头部类型标记
void set_log_header_type_flag(BYTE typeFlag)
{
	if(typeFlag)
		logHeader.header_show |= ((BYTE)1);
	else
		logHeader.header_show ^= ((BYTE)1); //清除标记
		
	return;	
}

//设置调试信息头部进程线程ID
void set_log_header_tpid_flag(BYTE tpidFlag)
{
	if(tpidFlag)
		logHeader.header_show |= ((BYTE)1 << 1);
	else
		logHeader.header_show ^= ((BYTE)1 << 1); //清除标记
	
	return;	
}


//设置调试信息头部日期标记
void set_log_header_date_flag(BYTE dateFlag)
{
	if(dateFlag)
		logHeader.header_show |= ((BYTE)1 << 2);
	else
		logHeader.header_show ^= ((BYTE)1 << 2); //清除标记
	
	return;	
}

//设置调试信息头部时间标记
void set_log_header_time_flag( BYTE timeFlag)
{
	if(timeFlag)
		logHeader.header_show |= ((BYTE)1 << 3);
	else
		logHeader.header_show ^= ((BYTE)1 << 3); //清除标记
	
	return;	
}

//设置调试信息头部文件名行号标记
void set_log_header_fileline_flag(BYTE filelineFlag)
{
	if(filelineFlag)
		logHeader.header_show |= ((BYTE)1 << 4);
	else
		logHeader.header_show ^= ((BYTE)1 << 4); //清除标记
	
	return;	
}

//设置调试信息头部结构标记 TRUE or FALSE
void set_log_header_info_flag(BYTE typeFlag, BYTE tpidFlag, BYTE dateFlag, BYTE timeFlag, BYTE filelineFlag, BYTE dateAcc, BYTE timeAcc)
{
	logHeader.header_show = (typeFlag |  (tpidFlag << 1) |(dateFlag << 2) |(timeFlag << 3) |(filelineFlag << 4));
	logHeader.date_accurate = dateAcc;
	logHeader.time_accurate = timeAcc;
	return;	
}


//得到调试信息头部信息
LOG_HEADER get_log_header()
{
	return logHeader;
}

//生成头部类型信息
static char * generate_type_info(BYTE type, char * typebuf, int buflen)
{
	const char *typestr[] = {"", "[INFO]", "[HINT]", "[WARNING]", "[ASSERT]", "[ERROR]", "[FATAL]"};
	strncpy(typebuf, typestr[type], buflen);
	return typebuf;
}

//生成头部类型信息
static char * generate_tpid_info(char * tpidbuf, int buflen)
{
	sprintf(tpidbuf, "[%d,%ld]", getpid(), gettid());
	return tpidbuf;
}

//生成头部文件名和行号信息
static char * generate_file_and_line_info(const char * file, int line, char * flbuf)
{
	file += (str_find_last_of(file, '/') + 1);
	sprintf(flbuf, "[%s,%d]", file, line);
	return flbuf;
}

//生成头部日期信息
static char * generate_date_info(BYTE dateAcc, struct tm * ptimes, char * datebuf)
{
	char wkday[16];
	switch(dateAcc)
	{
		case LOG_DATE_ACCURATE_YEAR:
			sprintf(datebuf, "[%04d]", \
				ptimes->tm_year + 1900);
			break;
			
		case LOG_DATE_ACCURATE_MONTH:
			sprintf(datebuf, "[%04d-%02d]", \
				ptimes->tm_year + 1900, \
				ptimes->tm_mon + 1);
			break;
			
		case LOG_DATE_ACCURATE_MDAY:
			sprintf(datebuf, "[%04d-%02d-%02d]", \
				ptimes->tm_year + 1900, \
				ptimes->tm_mon + 1, \
				ptimes->tm_mday);
			break;
			
		case LOG_DATE_ACCURATE_WDAY:
			sprintf(datebuf, "[%04d-%02d-%02d %s]", 
				ptimes->tm_year + 1900, \
				ptimes->tm_mon + 1, \
				ptimes->tm_mday, \
				trans_to_weekday(ptimes->tm_wday, wkday, sizeof(wkday)));
			break;
			
		default:
			break;
	}

	return datebuf;
}

//生成头部时间信息
static char * generate_time_info(BYTE timeAcc, struct tm * ptimes, char * timebuf)
{
	struct timeval utctime;
	long nsec;
	
	switch(timeAcc)
	{
		case LOG_TIME_ACCURATE_HOUR:
			sprintf(timebuf, "[%02d]", \
				ptimes->tm_hour + 8);
			break;
			
		case LOG_TIME_ACCURATE_MIN:
			sprintf(timebuf, "[%02d:%02d]", \
				ptimes->tm_hour + 8, \
				ptimes->tm_min);
			break;
			
		case LOG_TIME_ACCURATE_SEC:
			sprintf(timebuf, "[%02d:%02d:%02d]", \
				ptimes->tm_hour + 8, \
				ptimes->tm_min, \
				ptimes->tm_sec);
			break;
			
		case LOG_TIME_ACCURATE_MSEC:
			utctime = get_utc_time();
			sprintf(timebuf, "[%02d:%02d:%02d:%03d]", \
				ptimes->tm_hour + 8, \
				ptimes->tm_min, \
				ptimes->tm_sec, \
				approximate_by_int((double)utctime.tv_usec, 3)/1000);
			break;
		
		case LOG_TIME_ACCURATE_USEC:
			utctime = get_utc_time();
			sprintf(timebuf, "[%02d:%02d:%02d:%06ld]", \
				ptimes->tm_hour + 8, \
				ptimes->tm_min, \
				ptimes->tm_sec, \
				utctime.tv_usec);
			break;
		
		case LOG_TIME_ACCURATE_NSEC:
			nsec = get_nano_second();
			sprintf(timebuf, "[%02d:%02d:%02d:%09ld]", \
				ptimes->tm_hour + 8, \
				ptimes->tm_min, \
				ptimes->tm_sec, \
				nsec);
			break;
		
		default:
			break;
	}

	return timebuf;
}

//生成调试信息头部信息
static void generate_log_header(BYTE type, const char * file, int line, char * headbuf, int buflen)
{
	LOG_HEADER loghead = get_log_header();
	struct tm * ptimes = get_sys_time();
	BYTE bit = 0;
	for(bit = 1; bit < 64; bit <<= 1)
	{
		char tmp[512] = {'\0'};
		
		BYTE flag = loghead.header_show & bit;
		switch(flag)
		{
			case LOG_HEADER_SHOW_TYPE:
				generate_type_info(type, tmp, sizeof(tmp));
				break;
			case LOG_HEADER_SHOW_TPID:
				generate_tpid_info( tmp, sizeof(tmp));
				break;
			case LOG_HEADER_SHOW_DATE:
				generate_date_info(loghead.date_accurate, ptimes, tmp);
				break;
			case LOG_HEADER_SHOW_TIME:
				generate_time_info(loghead.time_accurate, ptimes, tmp);
				break;
			case LOG_HEADER_SHOW_FILE_LINE:
				generate_file_and_line_info(file, line, tmp);
				break;
			default:
				break;
		}
		
		strncat(headbuf, tmp, buflen);
	}
	
	return ;
}

static void append_debug_info_to_log_file(char * buf, int buflen)
{
	LOG_FILE logfile = get_log_file_info();
	
	int filesize = get_file_size(logfile.filename);
	if(filesize >= logfile.maxsize)
	{
		/*
		logfile.index ++;
		set_log_file_index(logfile.index);

		int position = str_find_last_of(logfile.filename, '-'); //找到最后一个“-”
		if(position != -1)
			str_erase(logfile.filename, position, strlen(logfile.filename) - position, sizeof(logfile.filename)); //擦除后缀“-[num]”
		
		char tmpfile[512] = {'\0'};
		sprintf(tmpfile, "%s-%d", logfile.filename, logfile.index);
		strncpy(logfile.filename, tmpfile, sizeof(logfile.filename));
		set_log_file_name(logfile.filename);
		*/
		
		//原来是加索引标记，文件数量太多耗内存，改为删除
		char strbuf[512];
		sprintf(strbuf, "rm -f %s", logfile.filename);
		system(strbuf);
	}
	
	if(logfile.writeFlag)
		append_buf_to_file(buf, strlen(buf) + 1, logfile.filename, FALSE);
	
	return;
}

//打印输出调试信息
void print_debug_info(BYTE    type,const char * file, int line, const char *fmt, ...)
{
    char strbuf[10240] = {'\0'};
    char buf[10240] = {'\0'};
	
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(strbuf, sizeof(strbuf), fmt, ap);
    va_end(ap);
	
	generate_log_header(type, file, line, buf, sizeof(buf));
    strncat(buf, strbuf, sizeof(buf));
    if(*(buf + strnlen(buf,sizeof(buf)) - 1) != '\n')
        strncat(buf, "\n", sizeof(buf));
	
    fputs(buf, stdout);
	append_debug_info_to_log_file(buf, sizeof(buf));
	
    return;
}

int backup_log_file( char * newpath, char * newname)
{
	if(file_access(newpath, F_OK) == 0)
		mkdir(newpath, 777);
	
	char newLogfile[512] = {'\0'};
	sprintf(newLogfile, "%s/%s", newpath, newname);

	LOG_FILE logfile = get_log_file_info();
	copy("-f", logfile.filename, newLogfile);
	sync();
	
	return 0;
}

