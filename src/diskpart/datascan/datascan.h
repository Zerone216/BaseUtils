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
*   文 件 名 ： datascan.h 
*   文件描述 ：  
*   创建日期 ：2019年8月15日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __DATASCAN_H__
#define __DATASCAN_H__

#include <common/baselib/baselib.h>
#include "../diskopt/diskrw.h"
#include "../parttable/dpbase.h"

#include "ExtScan.h"
#include "FatScan.h"
#include "NtfsScan.h"



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




#pragma pack(1)

typedef struct _PART_BITMAP_INFO_
{
	int diskfd;
	DDWORD partBeginSec;
	DDWORD partTotalSec;
	//int SecPerblock;
	int blockNum;
	int bitmapSize;
	BYTE * pBitmap;
	DWORD dataSize; 
} PART_BITMAP_INFO, *pPART_BITMAP_INFO;

typedef struct _DISK_BITMAP_INFO_
{
	int diskfd;
	DDWORD diskTotalSec;
	int SecPerblock;
	int blockNum;
	int bitmapSize;
	BYTE * pBitmap;
	DWORD dataSize; 
} DISK_BITMAP_INFO, *pDISK_BITMAP_INFO;


#pragma pack()

void data_scan_disk_bitmap_info_destory(DISK_BITMAP_INFO ** pDiskBitmapInfo);
DISK_BITMAP_INFO * data_scan_disk_bitmap_info_init(int diskfd, DDWORD diskTotalSec);
void data_scan_part_bitmap_info_destory(PART_BITMAP_INFO ** pPartBitmapInfo);
PART_BITMAP_INFO * data_scan_part_bitmap_info_init(int diskfd, BYTE fsType, DDWORD partBeginSec, DDWORD partTotalSec, BYTE validFlag);
int data_scan_part_mark_on_disk_bitmap(DISK_BITMAP_INFO * pDiskBitmapInfo, BYTE fsType, DDWORD partBeginSec, DDWORD partTotalSec, BYTE validFlag);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __DATASCAN_H__ */
