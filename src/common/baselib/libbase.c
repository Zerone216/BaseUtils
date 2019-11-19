/****************************************************************************************************************************
*
*   文 件 名 ： libbase.c 
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
#include <unistd.h>
#include <ctype.h>
#include <regex.h>
#include <stdarg.h>	/* ANSI C header file */
#include <syslog.h> 	/* for syslog() */
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <math.h>

#include "libbase.h"
#include "log.h"
//#include "libsignal.h"

pthread_t g_mainTid;
pid_t g_mainPid;

int init_program_pid_and_tid()
{
	g_mainTid = gettid();
	g_mainPid = getpid();
	
	return 0;
}

int check_if_new_thread()
{
	pthread_t cur_tid = gettid();
	//Log("get current tid = [%d]", cur_tid);
	
	if(g_mainTid == cur_tid)
		return 0;
	else
		return 1;
}

int check_if_new_process()
{
	pid_t cur_pid = getpid();
	//iLog("get current pid = [%d]", cur_pid);
	
	if(g_mainPid == cur_pid)
		return 0;
	else
		return 1;
}


void safeFree(void ** point)
{
	if(*point != NULL)
	{
		//iLog("free point at addr: [0x%x]", *point);
		free(*point);
		*point = NULL;
	}
	
	return ;
}

int approximate_by_int(double x, BYTE bitlvl)
{
	int divisor = pow(10, bitlvl);
	return (int)(x / divisor + 0.5) * divisor;
}

double approximate_by_double(double x, BYTE bitlvl)
{
	int divisor = pow(10, bitlvl);
	return (int)(x * divisor + 0.5) * 1.0 / divisor;
}

static int system_s(const char *cmd)
{
	int ret = 0;
	sighandler_t old_handler;
	old_handler = signal(SIGCLD, SIG_DFL);
	ret = system(cmd);
	signal(SIGCLD, old_handler);
	return ret;
}

int systemf(const char *fmt, ...)
{
	char cmdbuf[1024] = {'\0'};
	
	va_list ap;
	va_start(ap, fmt);
	vsprintf(cmdbuf, fmt, ap);
	va_end(ap);

	int ret = system_s(cmdbuf);
	iLog("run cmd[%d]:[%s]", ret, cmdbuf);
	
	return ret;
}

char * get_cmd_result(const char *fmt, ...)
{
	if(fmt == NULL)
	{
		eLog("Invalid command!");
		return NULL;
	}
	
	char cmd[1024] = {'\0'};
	
	va_list ap;
	va_start(ap, fmt);
	vsprintf(cmd, fmt, ap);
	va_end(ap);
	
	int file_len = S_1MiB;//限制输出最大为1MB
	char * result = (char *) z_malloc(file_len);
	if(result == NULL)
	{
		eLog("malloc failed!");
		return NULL;
	}
	
	//hLog("run cmd: %s", cmd);
	
	FILE * fstream = popen(cmd , "r");
	if(fstream == NULL)
	{
		eLog("fstream is NULL!");
		str_free(&result);
		return NULL;
	}
	
	fseek(fstream, 0, SEEK_SET);
	if(fread(result, file_len, 1, fstream) < 0)
	{
		eLog("fread failed!");
		pclose(fstream);
		str_free(&result);
		return NULL;
	}

	int len = strlen(result);
	if(len == 0) //没获取到任何信息
	{
		pclose(fstream);
		str_free(&result);
		return NULL;
	}
	
	result[len - 1] = '\0';
	pclose(fstream);
	//child_process_join();
	
	return result;
}

void free_cmd_result(char ** result)
{
	str_free(result);
}

double auto_trans_size(DDWORD x)
{
	if(x > (1024 * 1024 * 1024))
		return x * 1.0 / (1024 * 1024 * 1024);

	if(x > (1024 * 1024))
		return x * 1.0 / (1024 * 1024);

	if(x > 1024)
		return x * 1.0 / 1024;
	else
		return x;
}

int create_new_file(const char * filename)
{
	if(filename ==NULL)
		return -1;
	
	FILE * fp = fopen(filename, "wb");
	if(fp == NULL)
	{
		iLog("fopen %s error!", filename);
		return -1;
	}
	
	fclose(fp);
	return 0;
}

int dump_mem_to_file(BYTE * membuff, int bufflen, const char * filename, BYTE appendFlag)
{
	if(membuff == NULL || filename ==NULL)
		return -1;
	
	FILE * fp = NULL;

	if(appendFlag == TRUE) //追加
		fp = fopen(filename, "ab+");
	else
		fp = fopen(filename, "wb");
	if(fp == NULL)
	{
		iLog("fopen %s error!", filename);
		return -1;
	}
		
	if(fwrite(membuff, bufflen, 1, fp) != 1)
	{
		iLog("fwrite %s error!", filename);
		return -1;
	}

	fclose(fp);
	return 0;
}


int dump_file_to_mem( const char * filename, BYTE * membuff, int bufflen)
{
	if(membuff == NULL || filename ==NULL)
		return -1;
	
	FILE * fp = NULL;
	/***检查文件是否存在,不存在则创建文件***/
	if(file_access(filename, F_OK) == 0)
	{
		iLog("The %s do not exit!", filename);
		return -1;
	}

	/***检查wakeup文件是否可读***/
	if(file_access(filename, R_OK) == 0)
	{
		iLog("The %s can not be read ...\n", filename);
		return -1;
	}

	/***读写方式打开2进制文件***/
	if((fp = fopen(filename, "rb+")) == NULL)
	{
		iLog("Open %s error ...", filename);
		return -1;
	}

	if(fseek(fp, 0, SEEK_SET) == -1)
	{
		iLog(" fseek %s faild\n", filename);
		fclose(fp);
		return -1;
	}
	
	if(fread((void *)membuff, bufflen, 1, fp)  != 1)
	{
		iLog(" fread %s faild!", filename);
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

BYTE * file_dump(const char * filePath, int *datalen)
{
	if(filePath == NULL)
	{
		*datalen = 0;
		return NULL;
	}
	
	*datalen = get_file_size(filePath);
	if(*datalen <= 0)
	{
		*datalen = 0;
		return NULL;
	}

	BYTE * filedata = (BYTE *) malloc(*datalen);
	if(filedata == NULL)
	{
		*datalen = 0;
		return NULL;
	}
	
	if(dump_file_to_mem(filePath, filedata, *datalen) == -1)
	{
		*datalen = 0;
		safe_free(&filedata);
		return NULL;
	}

	return filedata;
}

void xterm()
{
	system_s("xterm -geometry 160x55");
}

void touch(char * filename)
{
	systemf("touch %s", filename);
	chmod(filename, 777);
	return;
}

static int delete_dir_or_file(char * dirorfile)
{
	if(dirorfile == NULL)
		return -1;

	if(file_access(dirorfile, F_OK) == 0) //不存在
	{
		eLog("[%s] is not exist!", dirorfile);
		return 0;
	}
	else
	{
		struct stat fstat;
		stat(dirorfile, &fstat);
		eLog("delete [%s]", dirorfile);

		if(S_ISREG(fstat.st_mode))
		{
			return unlink(dirorfile);
		}

		if(S_ISDIR(fstat.st_mode))
		{
			return systemf("rm -rf -- \'%s\'", dirorfile);  //rmdir()只能删除空目录
		}
		
		return -1;
	}
}

int delete_file(char * filepath, char * filename)
{
	if(filename == NULL && filepath == NULL)
		return -1;
	else if(filename == NULL && filepath != NULL)
		return delete_dir_or_file(filepath);
	else if(filename != NULL && filepath == NULL)
		return delete_dir_or_file(filename);
	else
	{
		char strtotal[256] = {0};

		if(filepath[strlen(filepath) - 1] == '/') //以"/" 结尾
			sprintf(strtotal, "%s%s", filepath, filename);
		else
			sprintf(strtotal, "%s/%s", filepath, filename);
		
		return delete_dir_or_file(strtotal);
	}
}

int mount(char * option, char * devname, char * pointer)
{
	return systemf("mount %s %s %s", option, devname, pointer);
}

int ntfs_3g(char * option, char * devname, char * pointer)
{
	return systemf("ntfs-3g %s %s %s", option, devname, pointer);
}

int umount(char * pointer)
{
	return systemf("umount %s", pointer);
}

int partprobe(char * devname)
{
	return systemf("partprobe %s", devname);
}

void sync()
{
	system("sync");
}

int ifconfig(char * netdev, char * addrSetting)
{
	return systemf("ifconfig %s %s", netdev, addrSetting);
}

int route(char * netdev, char * operating)
{
	return systemf("route %s %s", operating, netdev);
}

int copy(char * option, char * srcFile, char * distFile)
{
	return systemf("cp %s %s %s", option, srcFile, distFile);
}

void reboot()
{
	system("reboot");
}

void poweroff()
{
	system("poweroff");
}
