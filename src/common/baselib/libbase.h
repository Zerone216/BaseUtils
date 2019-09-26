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
*   文 件 名 ： libbase.h 
*   文件描述 ：  
*   创建日期 ：2019年1月10日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __LIBBASE_H__
#define __LIBBASE_H__

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
#include <fcntl.h>
#include <regex.h>

#include <getopt.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <asm/types.h>
#include <arpa/inet.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <linux/ethtool.h>
#include <linux/rtnetlink.h>
#include <linux/sockios.h>
#include <linux/if_ether.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/io.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>

#include "stdtypes.h"
#include "units.h"
#include "zmalloc.h"
#include "uconv.h"
#include "libutf.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*---------------------------------宏定义---------------------------------*/

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCEED
#define SUCCEED 0
#endif
#ifndef FAILED
#define FAILED 1
#endif

#ifndef YES
#define YES 1
#endif
#ifndef NO
#define NO 0
#endif
#ifndef NEW
#define NEW 2
#endif

#ifndef OK
#define OK 1
#endif
#ifndef CANCEL
#define CANCEL 0
#endif

#ifndef MIN
#define MIN(a,b) (((a)>(b))?(b):(a))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef MOD
#define MOD(x , y)  ((x) % (y))
#endif

#ifndef CEILING
//#define CEILING(x , y)  ((((x) +  (y) - 1) / (y)) * (y))
#define CEILING(x , y)  (((x) / (y) + 1) * (y))
#endif

#ifndef FLOOR
#define FLOOR(x , y)  (((x)  / (y)) * (y))
#endif

#define CheckPara( Para, ErrVal, ErrRet)\
	if((Para) == (ErrVal))\
		return (ErrRet);\
		
#define DOUBLE(x) ((double)((x)*1.0))
#define BitVal(a, p) ((a) & (BYTE)1 << (p))

#define gettid() syscall(__NR_gettid)

#define ARRLEN(arr) (sizeof(arr) / sizeof(arr[0]))

#define _XFF(t) ((t)&0xFF)

#define FMT_DOS_UUID_UPPER "%02X%02X%02X%02X"
#define FMT_DOS_UUID_LOWER "%02x%02x%02x%02x"
#define SNIF_DOS_UUID(p)     \
    _XFF((p)[3]),_XFF((p)[2]),_XFF((p)[1]),_XFF((p)[0])

#define FMT_GUID_UPPER "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X"
#define FMT_GUID_LOWER "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x"
#define SNIF_GUID(p)     \
    _XFF((p)[3]),_XFF((p)[2]),_XFF((p)[1]),_XFF((p)[0]),_XFF((p)[5]),_XFF((p)[4]),_XFF((p)[7]),_XFF((p)[6]),_XFF((p)[8]),_XFF((p)[9]),_XFF((p)[10]),_XFF((p)[11]),_XFF((p)[12]),_XFF((p)[13]),_XFF((p)[14]),_XFF((p)[15])

#define FMT_MAC_ADDR "%02X-%02X-%02X-%02X-%02X-%02X"
#define SNIF_MAC_ADDR(p)     \
    _XFF((p)[0]),_XFF((p)[1]),_XFF((p)[2]),_XFF((p)[3]),_XFF((p)[4]),_XFF((p)[5])

#define Auto_trans_unit(x) ((x) > (1024*1024*1024) ?  "GB" : ((x) > (1024*1024) ? "MB" :((x) > 1024 ? "KB" : "B" )))

#define safe_free(p) safeFree(DP_VOID(p))

#pragma pack(1)

typedef struct EFI_GUID
{
	DWORD data1;
	WORD  data2;
	WORD  data3;
	BYTE  data[8];
} EFI_GUID , *pEFI_GUID;

#pragma pack()

int init_program_pid_and_tid();
int check_if_new_thread();
int check_if_new_process();
void safeFree(void ** point);

int approximate_by_int(double x, BYTE bitlvl);
double approximate_by_double(double x, BYTE bitlvl);

int systemf(const char *fmt, ...);

char * get_cmd_result(const char *fmt, ...);
void free_cmd_result(char ** result);

double auto_trans_size(DDWORD x);
int create_new_file(const char * filename);
int dump_mem_to_file(BYTE * membuff, int bufflen, const char * filename, BYTE appendFlag);
int dump_file_to_mem(const char * filename, BYTE * membuff, int bufflen);
BYTE * file_dump(const char * filePath, int *datalen);


void xterm();
void touch(char * filename);
int mount(char * option, char * devname, char * pointer);
int ntfs_3g(char * option, char * devname, char * pointer);
int delete_file(char * filepath, char * filename);

int umount(char * pointer);
int partprobe(char * devname);
void sync();
void reboot();
void poweroff();
int copy(char * option, char * srcFile, char * distFile);
int ifconfig(char * netdev, char * addrSetting);
int route(char * netdev, char * operating);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __LIBBASE_H__ */
