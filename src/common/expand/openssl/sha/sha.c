/****************************************************************************************************************************
*
*   文 件 名 ： sha.c 
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

#include <openssl/sha.h>

#include "sha.h"

unsigned char * sha_sum(int shaType, const unsigned char * data, int len, sha_t output)
{
	unsigned char * sha = &output[0];
	switch(shaType)
	{
		case SHA_CKSUM:
			return SHA(data, len, sha);
		case SHA1_CKSUM:
			return SHA1(data, len, sha);
		case SHA224_CKSUM:
			return SHA224(data, len, sha);
		case SHA256_CKSUM:
			return SHA256(data, len, sha);
		case SHA384_CKSUM:
			return SHA384(data, len, sha);
		case SHA512_CKSUM:
			return SHA512(data, len, sha);
		default:
			printf("Unknown SHA type!");
			return sha;
	}
}

int sha_check(int shaType, const unsigned char * data, int len, const sha_t shasum)
{
	sha_t output;
	unsigned char * sha = &output[0];
	switch(shaType)
	{
		case SHA_CKSUM:
			SHA(data, len, sha);
			break;
		case SHA1_CKSUM:
			SHA1(data, len, sha);
			break;
		case SHA224_CKSUM:
			SHA224(data, len, sha);
			break;
		case SHA256_CKSUM:
			SHA256(data, len, sha);
			break;
		case SHA384_CKSUM:
			SHA384(data, len, sha);
			break;
		case SHA512_CKSUM:
			SHA512(data, len, sha);
			break;
		default:
			printf("Unknown SHA type!");
			break;
	}
	
	if(memcmp(shasum, output, sizeof(sha_t)) == 0)
		return 0;
	else
		return 1;
}

