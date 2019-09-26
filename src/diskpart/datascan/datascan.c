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
*   文 件 名 ： datascan.c 
*   文件描述 ：  
*   创建日期 ：2019年8月15日
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

#include "datascan.h"

PART_BITMAP_INFO * data_scan_part_bitmap_info_init(int diskfd, BYTE fsType, DDWORD partBeginSec, DDWORD partTotalSec, BYTE validFlag)
{
	PART_BITMAP_INFO * pPartBitmapInfo = (PART_BITMAP_INFO *) z_malloc(sizeof(PART_BITMAP_INFO));
	if(pPartBitmapInfo == NULL)
		return NULL;

	pPartBitmapInfo->diskfd = diskfd;
	pPartBitmapInfo->partBeginSec = partBeginSec;
	pPartBitmapInfo->partTotalSec = partTotalSec;
	pPartBitmapInfo->blockNum = (partTotalSec + SECTORS_PER_BLOCK - 1) / SECTORS_PER_BLOCK;
	pPartBitmapInfo->bitmapSize = (pPartBitmapInfo->blockNum + 8 -1) / 8;
	
	iLog("start scan part[0x%02X]: [--> %llu + %llu]", fsType, partBeginSec, partTotalSec);
	iLog("blockNum=[%d],bitmapSize=[%d]", pPartBitmapInfo->blockNum, pPartBitmapInfo->bitmapSize);
	
	BYTE *(*genBitmap)(int, DDWORD, DDWORD, int); 
	
	if(validFlag == FALSE) //非有效数据，即全部扇区
		genBitmap = unknown_fs_generate_part_bitmap;
	else
	{
		switch(fsType)
		{
			case PT_FS_FAT16:
				genBitmap = fat16_generate_part_bitmap;
				break;
			case PT_FS_FAT32:
				genBitmap = fat32_generate_part_bitmap;
				break;
			case PT_FS_NTFS:
				genBitmap = ntfs_generate_part_bitmap;
				break;
			case PT_FS_LINUX_EXT2:
			case PT_FS_LINUX_EXT3:
			case PT_FS_LINUX_EXT4:
				genBitmap = ext_generate_part_bitmap;
				break;
			default:
				genBitmap = unknown_fs_generate_part_bitmap;
				break;
		}
	}
	
	pPartBitmapInfo->pBitmap = genBitmap(diskfd, partBeginSec, partTotalSec, pPartBitmapInfo->bitmapSize);
	pPartBitmapInfo->dataSize = calc_valid_data_size(pPartBitmapInfo->pBitmap, pPartBitmapInfo->bitmapSize);
	
	iLog("dataSize=[%d]", pPartBitmapInfo->dataSize);
	
	return pPartBitmapInfo;
}

void data_scan_part_bitmap_info_destory(PART_BITMAP_INFO ** pPartBitmapInfo)
{
	if((*pPartBitmapInfo)->pBitmap)
		z_free(&((*pPartBitmapInfo)->pBitmap));
	
	z_free(pPartBitmapInfo);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
DISK_BITMAP_INFO * data_scan_disk_bitmap_info_init(int diskfd, DDWORD diskTotalSec)
{
	DISK_BITMAP_INFO * pDiskBitmapInfo = (DISK_BITMAP_INFO *) z_malloc(sizeof(DISK_BITMAP_INFO));
	if(pDiskBitmapInfo == NULL)
		return NULL;
	
	pDiskBitmapInfo->diskfd = diskfd;
	pDiskBitmapInfo->diskTotalSec = diskTotalSec;
	pDiskBitmapInfo->blockNum = (diskTotalSec + SECTORS_PER_BLOCK - 1) / SECTORS_PER_BLOCK;
	pDiskBitmapInfo->bitmapSize = (pDiskBitmapInfo->blockNum + 8 -1) / 8;
	pDiskBitmapInfo->pBitmap = (BYTE *) z_malloc(pDiskBitmapInfo->bitmapSize);
	if(pDiskBitmapInfo->pBitmap == NULL)
	{
		free(pDiskBitmapInfo);
		return NULL;
	}
	
	memset(pDiskBitmapInfo->pBitmap, 0x00, pDiskBitmapInfo->bitmapSize);
	return pDiskBitmapInfo;
}

int data_scan_part_mark_on_disk_bitmap(DISK_BITMAP_INFO * pDiskBitmapInfo, BYTE fsType, DDWORD partBeginSec, DDWORD partTotalSec, BYTE validFlag)
{
	int (* markBitmap)(BYTE *, int, DDWORD, DDWORD, int);
	
	if(validFlag == FALSE) //非有效数据，即全部扇区
		markBitmap = unknown_fs_mark_part_on_disk_bitmap;
	else
	{
		switch(fsType)
		{
			case PT_FS_FAT16:
				markBitmap = fat16_part_mark_on_disk_bitmap;
				break;
			case PT_FS_FAT32:
				markBitmap = fat32_part_mark_on_disk_bitmap;
				break;
			case PT_FS_NTFS:
				markBitmap = ntfs_part_mark_on_disk_bitmap;
				break;
			case PT_FS_LINUX_EXT2:
			case PT_FS_LINUX_EXT3:
			case PT_FS_LINUX_EXT4:
				markBitmap = ext_part_mark_on_disk_bitmap;
				break;
			default:
				markBitmap = unknown_fs_mark_part_on_disk_bitmap;
				break;
		}
	}
	
	return (int)markBitmap(pDiskBitmapInfo->pBitmap, pDiskBitmapInfo->diskfd, partBeginSec, partTotalSec, pDiskBitmapInfo->bitmapSize);
}


void data_scan_disk_bitmap_info_destory(DISK_BITMAP_INFO ** pDiskBitmapInfo)
{
	if((*pDiskBitmapInfo)->pBitmap)
		z_free(&((*pDiskBitmapInfo)->pBitmap));
	
	z_free(pDiskBitmapInfo);
}
