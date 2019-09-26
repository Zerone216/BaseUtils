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
*   文 件 名 ： libsignal.c 
*   文件描述 ：  
*   创建日期 ：2019年1月18日
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
#include <getopt.h>
#include <asm/types.h>
#include <linux/kernel.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/io.h>
#include <execinfo.h>

#include "libsignal.h"


void sig_int(int pi)
{
	fLog("The child process[%d] is interrupted!", getpid());	
	return;
}

void sig_cld(int pi)
{
	pid_t pid;
	int status;
	
	hLog("Detected signal that the child process exit!");
	while((pid = waitpid(-1, &status, WNOHANG)) > 0)
	{
		if(WIFEXITED(status))
			iLog("The child process[%d] has exited normally!", pid);
		else
			wLog("The child process[%d] has exited with the exception[%d]!", pid, status);
	}
	
	return;
}

pid_t create_new_process(void (*funcProc)(void *), void * dataProc)
{
	//////////////////////////父进程要处理的数据//////////////////////////////
	iLog("current process id = [%d]", getpid());
	
	///////////////////////////子进程/////////////////////////////
	pid_t pid = fork();
	if(pid < 0)
		err_sys("fork error");
	else if(pid == 0)
	{
		iLog("child process id = [%d]", getpid());
		funcProc(dataProc); //子进程
	}
	
	return pid;
}

void child_process_join()
{
	pid_t pid = 0;
	int status = 0;
	hLog("wait process exit!");
	int times = 10; //设置次数，防止卡死
	
	while(times --)
	{
		pid = waitpid(-1, &status, WNOHANG);
		if(pid > 0)
		{
			if(WIFEXITED(status))
				iLog("The child process[%d] has exited normally!", pid);
			else
				wLog("The child process[%d] has exited with the exception[%d]!", pid, status);

			break;
		}
	}
	
	return;
}

int wait_process_exit(pid_t pid)
{
	int retvalue = 0;
	int status;
    if(waitpid(pid, &status, WBLKHANG) > 0) //阻塞模式等待子进程退出
   		retvalue = BYTE2INT((int)WEXITSTATUS(status));
	else
        retvalue = -1;

	return retvalue;
}

void ignore_sig_cld(int pi)
{
	return ;
}

//SIGSEGV信号的处理函数，回溯栈，打印函数的调用关系
void debug_backtrace(void)
{
	#define TRACE_SIZE 100
	#define SEGMENT_FILE "/tmp/Segmentfault"
	
	void * array[TRACE_SIZE];
	int size,i;
	char ** strings;

	unlink(SEGMENT_FILE);
	
	set_log_header_flag(0);
	set_log_file_name(SEGMENT_FILE);
	Log("--------------------------------------- Segmentation fault ---------------------------------------\n");
	size = backtrace(array, TRACE_SIZE);
	Log("Backtrace (%d deep):\n", size);
	
	strings = backtrace_symbols(array, TRACE_SIZE);

	for(i = size - 1; i > 1; i --)
		Log("%d: %s\n", size - i - 1, strings[i]);
	Log("--------------------------------------- Segmentation fault ---------------------------------------\n");
	
	free(strings);
	exit(-1);
}

