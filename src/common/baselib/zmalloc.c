/****************************************************************************************************************************
*
*   文 件 名 ： zmalloc.c 
*   文件描述 ：  
*   创建日期 ：2019年6月4日
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
#include <unistd.h>
#include <ctype.h>
#include <regex.h>
#include <stdarg.h>	/* ANSI C header file */
#include <syslog.h> 	/* for syslog() */
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include "zmalloc.h"


void * zs_malloc( char * file, int line, size_t bufLen)
{
	void * pointer = malloc(bufLen);
	if(pointer)
		BZero(pointer, bufLen);

	//printf("[%s, %d] malloc point at:[%x + %d]\n",  file, line, pointer, bufLen);
	
	return pointer;
}

void zs_free(char * file, int line, void ** p)
{
	if(*p != NULL)
	{
		//printf("[%s, %d] free point at:[%x]\n",  file, line, *p);
		
		free(*p);
		*p = NULL;
	}
	
	return ;
}

void * zs_extAlloc(void ** p, size_t curLen, size_t extLen)
{
	void * point = malloc(curLen + extLen);
	if(point)
	{
		BZero(point, curLen + extLen);
		
		if(*p)
		{
			memcpy(point, *p, curLen);
			z_free(p);
		}
	}
	
	return point;
}
