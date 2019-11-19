/****************************************************************************************************************************
*
*   文 件 名 ： libfile.c 
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
#include <fcntl.h>
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

#include "libfile.h"

/*
mode：
	F_OK	// Check for file existence 
	X_OK	// Check for execute permission.
	W_OK	// Check for write permission 
	R_OK	// Check for read permission 
*/
int file_access(const char * filename,int mode)
{
	if(filename == NULL)
        return -1; //参数异常
	
    if( access(filename,mode) == -1 ) //查询mode不生效
    {
       return 0;
    }
	
    return 1;
}


DDWORD get_file_size(const char *filepath)
{
	DDWORD filesize = 0;
	if(file_access(filepath, F_OK) == 0) //文件不存在
	{
		return 0;
	}
	
	struct stat statbuff;
	memset(&statbuff, 0x00, sizeof(struct stat));
		
	if(stat(filepath, &statbuff) != -1)
		filesize = statbuff.st_size;
	
    return filesize;
}

/*
	rwmode:
	r	以只读方式打开文件，该文件必须存在。
	r+	以读/写方式打开文件，该文件必须存在。
	rb+	以读/写方式打开一个二进制文件，只允许读/写数据。
	rt+	以读/写方式打开一个文本文件，允许读和写。
	w	打开只写文件，若文件存在则文件长度清为零，即该文件内容会消失；若文件不存在则创建该文件。
	w+	打开可读/写文件，若文件存在则文件长度清为零，即该文件内容会消失；若文件不存在则创建该文件。
	a	以附加的方式打开只写文件。若文件不存在，则会创建该文件；如果文件存在，则写入的数据会被加到文件尾后，即文件原先的内容会被保留（EOF 符保留）。
	a+	以附加方式打开可读/写的文件。若文件不存在，则会创建该文件，如果文件存在，则写入的数据会被加到文件尾后，即文件原先的内容会被保留（EOF符不保留）。
	wb	以只写方式打开或新建一个二进制文件，只允许写数据。
	wb+	以读/写方式打开或新建一个二进制文件，允许读和写。
	wt+	以读/写方式打开或新建一个文本文件，允许读和写。
	at+	以读/写方式打开一个文本文件，允许读或在文本末追加数据。
	ab+	以读/写方式打开一个二进制文件，允许读或在文件末追加数据。
*/

int if_need_check_exist(char * rwmode)
{
	if(strcmp(rwmode, "r") == 0 ||\
		strcmp(rwmode, "r+") == 0 ||\
		strcmp(rwmode, "rb+") == 0 ||\
		strcmp(rwmode, "rt+") == 0 )
		return 1;
	else
		return 0;
}

int read_file_to_buff(const char * filename, char * rwmode, int offset, char * buff, int buflen, BYTE newFlag)
{
	if(filename == NULL || buff == NULL || rwmode == NULL)
		return NO;
	
	if(if_need_check_exist(rwmode))
	{
		if(file_access(filename, F_OK) == 0)
		{
			if(newFlag)
			{
				wLog("Not found the file[%s], need create new one!", filename);
				FILE * fp = fopen(filename, "wb+");
				if(fp == NULL)
					eLog("Create file[%s] failed!", filename);
				else
				{
					fclose(fp);
					hLog("Create file[%s] succeed!", filename);
				}
			}
			else
				eLog("Not found the file[%s]!", filename);

			return NO; //不存在的文件没有可用的信息,即使新创建的也无法获取数据
		}
		else
		{
			if(file_access(filename, R_OK) == 0)
			{
				eLog("file[%s] is Unreadable!", filename);
				return NO;
			}
		}
	}
	
	FILE * fp = fopen(filename, rwmode);
	if(fp == NULL)
	{
		eLog("Open file[%s] failed!", filename);
		return NO;
	}
	
	if(fseek(fp, offset, SEEK_SET) == -1)
	{
		eLog("fseek set offset to %d faild!", offset);
		
		fclose(fp);
		return NO;
	}
	
	if(fread(buff, buflen, 1, fp) == -1)
	{
		eLog("fread data from file faild");
		fclose(fp);
		return NO;
	}

	fclose(fp);
	return YES;
}


int write_buff_to_file(char * buff, int buflen, const char * filename, char * rwmode, int offset, BYTE newFlag)
{
	if(filename == NULL || buff == NULL || rwmode == NULL)
		return NO;

	if(if_need_check_exist(rwmode) ) 
	{
		if(file_access(filename, F_OK) == 0)
		{
			if(newFlag)
			{
				wLog("Not found the file[%s], need create new one!", filename);
				FILE * fp = fopen(filename, "wb+");
				if(fp == NULL)
				{
					eLog("Create file[%s] failed!", filename);
					return NO;
				}
				else
				{
					hLog("Create file[%s] succeed!", filename);
					fclose(fp);
				}
			}
			else
			{
				eLog("Not found the file[%s]!", filename);
				return NO;
			}
		}

		if(file_access(filename, W_OK) == 0)
		{
			eLog("file[%s] is Unwritable!", filename);
			return NO;
		}
	}
	
	FILE * fp = fopen(filename, rwmode);
	if(fp == NULL)
	{
		eLog("Open file[%s] failed!", filename);
		return NO;
	}
	
	if(fseek(fp, offset, SEEK_SET) == -1)
	{
		eLog("fseek set offset to %d faild!", offset);
		fclose(fp);
		return NO;
	}
	
	if(fwrite(buff, buflen, 1, fp) == -1)
	{
		eLog("fwrite data to file faild!");
		fclose(fp);
		return NO;
	}

	sync();
	
	fclose(fp);
	return YES;
}

int append_buf_to_file(char * buf, int buflen, const char * filename, BYTE syncFlag)
{
	if(filename == NULL || buf == NULL)
		return NO;
	
	FILE * fp = fopen(filename, "a+");
	if(fp == NULL)
	{
		fprintf(stderr, "fopen failed: %s", strerror(errno));
		return NO;
	}

	if(syncFlag) //数据量比较大
	{
		if(fwrite(buf, buflen, 1, fp) == -1)
		{
			fprintf(stderr, "fwrite failed: %s", strerror(errno));
			fclose(fp);
			return NO;
		}
		sync();
	}
	else
		fputs(buf, fp);

	fclose(fp);
	return YES;
}

int read_or_write_file(BYTE readwrite, char * rwmode, const char * filename, int offset, char * buff, int buflen)
{
	if(filename == NULL || buff == NULL || rwmode == NULL)
		return NO;
	
	int ret= 0;
	switch(readwrite)
	{
		case FILE_READ:
			ret = read_file_to_buff(filename, rwmode, offset, buff, buflen, TRUE);
			break;

		case FILE_WRITE:
			ret = write_buff_to_file(buff, buflen, filename, rwmode, offset, TRUE);
			break;

		default:
			break;
	}

	return ret;
}
