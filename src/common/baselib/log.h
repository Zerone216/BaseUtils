/****************************************************************************************************************************
*
*	Copyright (c) 1998-2018  XI'AN SAMING TECHNOLOGY Company 
*	西安三茗科技股份有限公司  版权所有 1998-2018 
*
*   PROPRIETARY RIGHTS of XI'AN SAMING TECHNOLOGY Company are involved in  the subject matter of this material.       
*   All manufacturing, reproduction,use, and sales rights pertaining to this subject matter are governed bythe license            
*   agreement. The recipient of this software implicitly accepts the terms of the license.	
*
*   本软件文档资料是西安三茗科技股份有限责任公司的资产,任何人士阅读和使用本资料必须获得相应的书面
*   授权,承担保密责任和接受相应的法律约束. 								     
*
*----------------------------------------------------------------------------------------------------------------------------
*
*   文 件 名 ： log.h 
*   文件描述 ：  
*   创建日期 ：2019年1月9日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __LOG_H__
#define __LOG_H__

#include "libbase.h"
#include "libstring.h"
#include "libfile.h"
#include "libtime.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


//单个调试信息的类型
#define LOG_TYPE_NULL 0x00	//空类型
#define LOG_TYPE_INFO 0x01	//信息类型
#define LOG_TYPE_HINT 0x02	//提示类型
#define LOG_TYPE_WARNING 0x03	//警告类型
#define LOG_TYPE_ASSERT 0x04	//断言类型
#define LOG_TYPE_ERROR 0x05	//错误类型
#define LOG_TYPE_FATAL 0x06	//致命类型

//调试信息头部日期信息的精确度
#define LOG_DATE_ACCURATE_YEAR 0x01	//年
#define LOG_DATE_ACCURATE_MONTH 0x02	//月
#define LOG_DATE_ACCURATE_MDAY 0x03	//月内日
#define LOG_DATE_ACCURATE_WDAY 0x04	//周内日


//调试信息头部时间信息的精确度
#define LOG_TIME_ACCURATE_HOUR 0x01	//时
#define LOG_TIME_ACCURATE_MIN 0x02	//分
#define LOG_TIME_ACCURATE_SEC 0x03	//秒
#define LOG_TIME_ACCURATE_MSEC 0x04	//毫秒
#define LOG_TIME_ACCURATE_USEC 0x05	//微秒
#define LOG_TIME_ACCURATE_NSEC 0x06	//纳秒

//调试信息头部结构显示的种类
#define LOG_HEADER_SHOW_TYPE 0x01	//显示信息类型
#define LOG_HEADER_SHOW_TPID 0x02	//显示进程线程ID
#define LOG_HEADER_SHOW_DATE 0x04	//显示信息日期
#define LOG_HEADER_SHOW_TIME 0x08	//显示信息时间
#define LOG_HEADER_SHOW_FILE_LINE 0x10	//显示信息文件名和行号

#define Log(format, args...)  print_debug_info(LOG_TYPE_NULL , __FILE__, __LINE__, format, ##args) //pure log: 纯粹的调试信息（不带任何修饰）
#define iLog(format, args...) print_debug_info(LOG_TYPE_INFO, __FILE__, __LINE__, format, ##args) //info log: 正常调试信息
#define hLog(format, args...) print_debug_info(LOG_TYPE_HINT, __FILE__, __LINE__, format, ##args) //hint log: 提示调试信息
#define wLog(format, args...) print_debug_info(LOG_TYPE_WARNING, __FILE__, __LINE__, format, ##args) //warning log: 警告调试信息
#define aLog(format, args...) print_debug_info(LOG_TYPE_ASSERT, __FILE__, __LINE__, format, ##args) //assert log: 断言调试信息
#define eLog(format, args...) print_debug_info(LOG_TYPE_ERROR, __FILE__, __LINE__, format, ##args) //error log: 错误调试信息
#define fLog(format, args...) print_debug_info(LOG_TYPE_FATAL, __FILE__, __LINE__, format, ##args) //fatal log: 致命调试信息

#define Assert(x) if (!(x)) {print_debug_info(LOG_TYPE_ASSERT, __FILE__, __LINE__, "%s", ErrExp());ErrClean();}
#define assert(x) Assert(x)

#define LOG_FILE_NAME "/tmp/printLog.log"
#define LOG_FILE_DEFALT_MAX_SIZE 20971520 //20MB

#pragma pack(1)

//日志调试信息头部结构
typedef struct LOG_FILE
{
	char filename[512];
	int maxsize;
	int index;
	BYTE writeFlag;
} LOG_FILE, *pLOG_FILE;


//日志调试信息头部结构
typedef struct LOG_HEADER
{
	BYTE header_show; //头部信息显示种类的标记
	BYTE date_accurate;//日期的精确度
	BYTE time_accurate;//时间的精确度
} LOG_HEADER, *pLOG_HEADER;


#pragma pack()

void set_log_file_name(const char * filename);
void set_log_file_maxsize( int maxsize);
void set_log_file_index(int index);
void set_log_file_write_flag( BYTE writeFlag);
void set_log_file_info(const char * filename, int maxsize, int index, BYTE writeFlag);
LOG_FILE get_log_file_info();

void set_log_header_flag(BYTE flag);
void set_log_header_date_acc(BYTE dateAcc);
void set_log_header_time_acc(BYTE timeAcc);
void set_log_header_type_flag(BYTE typeFlag);
void set_log_header_tpid_flag(BYTE tpidFlag);
void set_log_header_date_flag(BYTE dateFlag);
void set_log_header_time_flag( BYTE timeFlag);
void set_log_header_fileline_flag(BYTE filelineFlag);
void set_log_header_info_flag(BYTE typeFlag, BYTE tpidFlag, BYTE dateFlag, BYTE timeFlag, BYTE filelineFlag, BYTE dateAcc, BYTE timeAcc);

void print_debug_info(BYTE type,const char * file, int line, const char *fmt, ...);
int backup_log_file( char * newpath, char * newname);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __LOG_H__ */
