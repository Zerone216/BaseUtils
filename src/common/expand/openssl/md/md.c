/****************************************************************************************************************************
*
*	Copyright (c) 1998-2019  XI'AN SAMING TECHNOLOGY Co., Ltd
*	西安三茗科技股份有限公司  版权所有 1998-2019 
*
*   PROPRIETARY RIGHTS of XI'AN SAMING TECHNOLOGY Co., Ltd are involved in  the subject matter of this material.       
*   All manufacturing, reproduction,use, and sales rights pertaining to this subject matter are governed bythe license            
*   agreement. The recipient of this software implicitly accepts the terms of the license.	
*
*   本软件文档资料是西安三茗科技股份有限公司的资产,任何人士阅读和使用本资料必须获得相应的书面
*   授权,承担保密责任和接受相应的法律约束. 								     
*
*----------------------------------------------------------------------------------------------------------------------------
*
*   文 件 名 ： md.c 
*   文件描述 ：  
*   创建日期 ：2019年5月13日
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

#include <openssl/md5.h>
#include <openssl/md4.h>

#include "md.h"


#define MD4_CKSUM 0X01
#define MD5_CKSUM 0X02

///////////////////////////////////////////////////////////////////////////////////////
unsigned char * md_sum(int mdType, const unsigned char * data, int len, md_t output)
{
	unsigned char * md = &output[0];
	switch(mdType)
	{
		case MD4_CKSUM :
			return MD4(data, len, md);
		case MD5_CKSUM :
			return MD5(data, len, md);
		default:
			printf("Unknown MD type!");
			return md;
	}
}

int md_check(int mdType, const unsigned char * data, int len, const md_t md4sum)
{
	md_t output;
	unsigned char * md = &output[0];
	switch(mdType)
	{
		case MD4_CKSUM :
			MD4(data, len, md);
			break;
		case MD5_CKSUM :
			MD5(data, len, md);
			break;
		default:
			printf("Unknown MD type!");
			break;
	}
	
	if(memcmp(md4sum, output, sizeof(md_t)) == 0)
		return 0;
	else
		return 1;
}
