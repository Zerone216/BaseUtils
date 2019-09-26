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
*   文 件 名 ： diskrw.h 
*   文件描述 ：  
*   创建日期 ：2019年5月14日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __DISKRW_H__
#define __DISKRW_H__

#include <common/baselib/baselib.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


/*---------------------------------外部变量说明---------------------------------*/





/*---------------------------------外部函数原型说明---------------------------------*/





/*---------------------------------内部函数原型说明---------------------------------*/





/*---------------------------------全局变量---------------------------------*/





/*---------------------------------模块级变量---------------------------------*/





/*---------------------------------常量定义---------------------------------*/





/*---------------------------------宏定义---------------------------------*/

#define DISK_CAPABILITY_UNKNOWN -1  //磁盘容量未知

#define SECTOR_SIZE 512

#define DISK_READ 2
#define DISK_WRITE 3

#define RW_DISK_OK 0x00
#define RW_DISK_OVER_BOUND 0x01
#define RW_DISK_LSEEK_ERROR 0x02
#define RW_DISK_READ_ERROR 0x03
#define RW_DISK_WRITE_ERROR 0x04
#define RW_DISK_PARA_ERROR 0x05
#define RW_DISK_TOO_SMALL_BUFFER 0x06
#define RW_DISK_STARTSEC_NON_4K_ALIGN 0x07
#define RW_DISK_SECNUM_NON_4K_ALIGN 0x08
#define RW_DISK_MALLOC_MEM_FAILED 0x09

#pragma pack(1)

//硬盘数据块读写基本结构
typedef struct HDISK_BLOCK_RW
{
	int diskno; //硬盘编号
	BYTE readorwrite; //读或写操作
	int diskfd; //磁盘的文件描述符（必须是已经open的）
	DDWORD startSec; //读写数据块的起始扇区位置
	DDWORD secNum; //读写数据块的扇区总数
	DDWORD endSec ; //读写数据块的结束扇区
	DDWORD diskCapability; //该硬盘的容量，即总扇区数
	BYTE * buf; //读写数据块的缓冲区指针
	DDWORD buflen; //读写数据块的缓冲区大小
	BYTE errCode; //读写操作的结果代码，一般为出错代码
	DDWORD errSec; //读写出错的扇区位置
} HDISK_BLOCK_RW, *pHDISK_BLOCK_RW;


//硬盘读写结构
typedef struct HDISK_RW
{
	HDISK_BLOCK_RW hdiskBlockRw;
	
	BYTE non4kAdjust; ////非4kb读写是否需要调整为4kb的标记
	DDWORD startSec_adjust; //非4kb读写需要调整的起始扇区
	DDWORD secNum_adjust; //非4kb读写需要调整的扇区总数
	DDWORD endSec_adjust; //非4kb读写需要调整的结束扇区
	int (*hdisk_rw_func)(HDISK_BLOCK_RW *); //需要调用的读写接口（4kb对齐或者基础读写接口）
} HDISK_RW, *pHDISK_RW;

#pragma pack()

int readwrite_hdisk_data_block(HDISK_RW * non4kRw) ;
int readwrite_hdisk_sector(BYTE readorwrite, int diskfd, DDWORD logSec, DDWORD secNum, BYTE * buf);
int readwrite_hdisk_block(BYTE readorwrite, int diskno, int diskfd, DDWORD diskCapability, DDWORD startSec, DDWORD secNum, BYTE * buf, DDWORD buflen, BYTE non4kAdjust);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __DISKRW_H__ */
