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
*   文 件 名 ： FatScan.c 
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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/io.h>

#include "FatScan.h"

//////////////////////////////////////////////下面是CFAT16部份的代码//////////////////////////////////////////////

//获得指定号码的簇标志的值
WORD fat16_get_clusterval (FAT16_DATA_SCAN_INFO * pDataScanInfo, DWORD number)
{
    DWORD sector = number / (2 * SECTORS_PER_CLUSTER);          //1个扇区有256个簇号
    int offset = (number % (2 * SECTORS_PER_CLUSTER)) * 2;      //在扇区中的字节偏移
	
    if (sector+ pDataScanInfo->m_fat1start != pDataScanInfo->m_SecInBuf)
    {
    	DDWORD readSector = pDataScanInfo->partBeginSec+ pDataScanInfo->m_fat1start + sector;
        readwrite_hdisk_sector(DISK_READ, pDataScanInfo->diskfd, readSector, 1, pDataScanInfo->sectorBuf);
        pDataScanInfo->m_SecInBuf = sector + pDataScanInfo->m_fat1start;
    }
	
    return *((WORD*)(pDataScanInfo->sectorBuf + offset));
}

BOOL  fat16_datascan_initial(FAT16_DATA_SCAN_INFO * pDataScanInfo)
{
    if(pDataScanInfo == NULL)
		return FALSE;
	
    BYTE buffer[SECTOR_SIZE];
    if(readwrite_hdisk_sector(DISK_READ, pDataScanInfo->diskfd, pDataScanInfo->partBeginSec + 0, 1, buffer) < 0)
        return FALSE;

	WORD endFlag = *((WORD *)buffer + 0xFF);
    if(SECTOR_END_FLAG != endFlag)
    {
		iLog("CFAT16 endFlag=[0x%X], not 0xAA55!", endFlag);
        return FALSE;
    }

	memcpy(&pDataScanInfo->FAT16boot, buffer, sizeof(FAT16_BOOT_SECTOR));
    if(memcmp(pDataScanInfo->FAT16boot.FileSystem,"FAT16", strlen("FAT16"))!=0)
    {
        eLog("FAT16_BOOT_SECTOR FileSystem error");
        return FALSE;
    }
	
    pDataScanInfo->m_datastart = (pDataScanInfo->FAT16boot.RootEntry >> 4);
    pDataScanInfo->m_datastart += pDataScanInfo->FAT16boot.ResSectors + pDataScanInfo->FAT16boot.SecPerFat * pDataScanInfo->FAT16boot.NumOfFat;
    
    pDataScanInfo->datasec = pDataScanInfo->FAT16boot.BigTotalSec;
    if( pDataScanInfo->datasec == 0 )
        pDataScanInfo->datasec = pDataScanInfo->partTotalSec;
    pDataScanInfo->datasec -= pDataScanInfo->m_datastart;
    
    pDataScanInfo->m_secpercluster = pDataScanInfo->FAT16boot.SecPerClr;                //得到每簇扇区数
    pDataScanInfo->m_clusternum = pDataScanInfo->datasec/pDataScanInfo->m_secpercluster + 2;           //得到总簇数,包括两个保留簇，比WINHEX看到的簇数多2
	
    if(pDataScanInfo->m_secpercluster > SECTORS_PER_CLUSTER)
    {
        eLog("CFAT16: sec per cluster error!");
        return FALSE;
    }
    
    return TRUE;
}

FAT16_DATA_SCAN_INFO * fat16_datascan_init(int diskfd, DDWORD partBeginSec, DDWORD partTotalSec)
{
	FAT16_DATA_SCAN_INFO * pDataScanInfo = (FAT16_DATA_SCAN_INFO *)z_malloc(sizeof(FAT16_DATA_SCAN_INFO));
	if(pDataScanInfo == NULL)
		return NULL;
	
	pDataScanInfo->diskfd = diskfd;
	pDataScanInfo->partBeginSec = partBeginSec;
	pDataScanInfo->partTotalSec = partTotalSec;
	pDataScanInfo->m_SecInBuf = 0xffffffff; //FAT表缓冲
	
   	if( fat16_datascan_initial(pDataScanInfo) == FALSE)
	{
		fat16_datascan_end(&pDataScanInfo);
		return NULL;
	}
	
	return pDataScanInfo;		
}

void fat16_datascan_end(FAT16_DATA_SCAN_INFO ** pDataScanInfo)
{
	z_free(pDataScanInfo);
	return ;
}

static int fat16_mark_bitmap(BYTE * pBitmap, int diskfd, DDWORD bitmapBeginSec, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize)
{
	iLog("FAT16 part scan: mark bitmap...");
	
	FAT16_DATA_SCAN_INFO * pDataScanInfo = fat16_datascan_init( diskfd, partBeginSec, partTotalSec);
	if(pDataScanInfo)
	{
		mark_bitmap_by_sector_info(pBitmap, bitmapBeginSec, pDataScanInfo->m_datastart); //DBR保留扇区 + FAT1 + FAT1-bak
		
		int i = 0, j = 0;
		for ( i = 2, j = 0; i < pDataScanInfo->m_clusternum; i++)
		{
			if (fat16_get_clusterval(pDataScanInfo, i) != 0)
			{
			    DDWORD beginSec = bitmapBeginSec + pDataScanInfo->m_datastart + pDataScanInfo->m_secpercluster * (i -2);
			    mark_bitmap_by_sector_info(pBitmap, bitmapBeginSec + beginSec, pDataScanInfo->m_secpercluster);
			    j++;
			}
		}
		
		fat16_datascan_end(&pDataScanInfo);
    }
	else
		mark_bitmap_by_sector_info(pBitmap, bitmapBeginSec, partTotalSec);
	
	return 0;
}

BYTE * fat16_generate_part_bitmap(int diskfd, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize)
{
	iLog("FAT16 part scan: generate part bitmap...");
	BYTE * pBitmap = (BYTE *) z_malloc(bitmapSize);
	if(pBitmap == NULL)
		return NULL;
	
	fat16_mark_bitmap(pBitmap, diskfd, 0, partBeginSec, partTotalSec, bitmapSize);	
	return pBitmap;
}

int fat16_part_mark_on_disk_bitmap(BYTE * pBitmap, int diskfd, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize)
{
	return fat16_mark_bitmap(pBitmap, diskfd, partBeginSec, partBeginSec, partTotalSec, bitmapSize);
}


//////////////////////////////////////////////下面是CFAT32部份的代码//////////////////////////////////////////////
//获得指定号码的簇标志的值
DWORD fat32_get_clusterval (FAT32_DATA_SCAN_INFO * pDataScanInfo, DDWORD number)
{
    DDWORD sector = number / SECTORS_PER_CLUSTER;          //1个扇区有128个簇号
    int offset = (number % SECTORS_PER_CLUSTER) * 4;      //在扇区中的字节偏移
	
    if (sector + pDataScanInfo->m_fat1start != pDataScanInfo->m_SecInBuf)
    {
		DDWORD readSector = pDataScanInfo->partBeginSec + pDataScanInfo->m_fat1start + sector;		
        readwrite_hdisk_sector(DISK_READ, \
							pDataScanInfo->diskfd,\
							readSector,\
							1, \
							pDataScanInfo->sectorBuf);
		
        pDataScanInfo->m_SecInBuf = sector + pDataScanInfo->m_fat1start;
    }

	return *((DWORD *)(pDataScanInfo->sectorBuf + offset));
}

BOOL fat32_datascan_initial(FAT32_DATA_SCAN_INFO * pDataScanInfo)
{
	if(pDataScanInfo == NULL)
		return FALSE;
	
    BYTE buffer[SECTOR_SIZE];
    if(readwrite_hdisk_sector(DISK_READ, pDataScanInfo->diskfd, pDataScanInfo->partBeginSec + 0, 1, buffer) < 0)
        return FALSE;
	
	WORD endFlag = *((WORD *)buffer + 0xFF);
    if(SECTOR_END_FLAG != endFlag)
    {
    	iLog("CFAT32 endFlag=[0x%X], not 0xAA55!", endFlag);
        return FALSE;
    }
	
	//iLog("sizeof(FAT32_BOOT_SECTOR)=[%d]", sizeof(FAT32_BOOT_SECTOR));
	memcpy(&pDataScanInfo->FAT32boot, buffer, sizeof(FAT32_BOOT_SECTOR));
	if(memcmp(pDataScanInfo->FAT32boot.FileSystem, "FAT32", strlen("FAT32"))!=0)
    {
        eLog("FAT32_BOOT_SECTOR FileSystem error");
        return FALSE;
    }
	
    pDataScanInfo->m_fat1start = pDataScanInfo->FAT32boot.ReservedSectors;
    pDataScanInfo->m_datastart = pDataScanInfo->FAT32boot.ReservedSectors + pDataScanInfo->FAT32boot.BigSecPerFat * pDataScanInfo->FAT32boot.NumOfFat;

	iLog("m_fat1start=[%d]", pDataScanInfo->m_fat1start);
	iLog("m_datastart=[%d]", pDataScanInfo->m_datastart);
	
    pDataScanInfo->datasec = pDataScanInfo->FAT32boot.BigTotalSec;
    if(pDataScanInfo->datasec == 0)
        pDataScanInfo->datasec = pDataScanInfo->partTotalSec;
	
    pDataScanInfo->datasec -= pDataScanInfo->m_datastart;
	iLog("datasec=[%d]", pDataScanInfo->datasec);
	
    pDataScanInfo->m_secpercluster = pDataScanInfo->FAT32boot.SecPerClr;              //得到每簇扇区数
    pDataScanInfo->m_clusternum = pDataScanInfo->datasec / pDataScanInfo->m_secpercluster + 2;          //得到总簇数,包括两个保留簇，比WINHEX看到的簇数多2
	iLog("m_secpercluster=[%d]", pDataScanInfo->m_secpercluster);
	iLog("m_clusternum=[%d]", pDataScanInfo->m_clusternum);
	
    if(pDataScanInfo->m_secpercluster > SECTORS_PER_CLUSTER)
    {
        eLog("CFAT32: sec per cluster error!");
        return FALSE;
    }
	
    return TRUE;
}

FAT32_DATA_SCAN_INFO * fat32_datascan_init(int diskfd, DDWORD partBeginSec, DDWORD partTotalSec)
{
	FAT32_DATA_SCAN_INFO * pDataScanInfo = (FAT32_DATA_SCAN_INFO *) z_malloc(sizeof(FAT32_DATA_SCAN_INFO));
	if(pDataScanInfo == NULL)
		return NULL;
	
	pDataScanInfo->diskfd = diskfd;
	pDataScanInfo->partBeginSec = partBeginSec;
	pDataScanInfo->partTotalSec = partTotalSec;
	pDataScanInfo->m_SecInBuf = 0xffffffff; //FAT表缓冲
	
   	if( fat32_datascan_initial(pDataScanInfo) == FALSE)
	{
		fat32_datascan_end(&pDataScanInfo);
		return NULL;
	}
	
	return pDataScanInfo;		
}

void fat32_datascan_end(FAT32_DATA_SCAN_INFO ** pDataScanInfo)
{
	z_free(pDataScanInfo);
	return ;
}

static int fat32_mark_bitmap(BYTE * pBitmap, int diskfd, DDWORD bitmapBeginSec, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize)
{
	iLog("FAT32 part scan: mark bitmap...");
	
	FAT32_DATA_SCAN_INFO * pDataScanInfo = fat32_datascan_init( diskfd, partBeginSec, partTotalSec);
	if(pDataScanInfo != NULL)
	{		
		mark_bitmap_by_sector_info(pBitmap, bitmapBeginSec, pDataScanInfo->m_datastart); //DBR保留扇区 + FAT1 + FAT1-bak
		
		int i = 0, j = 0;
		for ( i = 2, j = 0; i < pDataScanInfo->m_clusternum; i++)
		{
			if (fat32_get_clusterval(pDataScanInfo, i) != 0)
			{
			    DDWORD beginSec = bitmapBeginSec + pDataScanInfo->m_datastart + pDataScanInfo->m_secpercluster * (i -2);
			    mark_bitmap_by_sector_info(pBitmap, bitmapBeginSec + beginSec, pDataScanInfo->m_secpercluster);
			    j++;
			}
		}
		
		fat32_datascan_end(&pDataScanInfo);
    }
	else
		mark_bitmap_by_sector_info(pBitmap, bitmapBeginSec, partTotalSec);
	
	return 0;
}

BYTE * fat32_generate_part_bitmap(int diskfd, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize)
{
	iLog("FAT32 part scan: generate part bitmap...");
	BYTE * pBitmap = (BYTE *) z_malloc(bitmapSize);
	if(pBitmap == NULL)
		return NULL;
	
	fat32_mark_bitmap(pBitmap, diskfd, 0, partBeginSec, partTotalSec, bitmapSize);
	return pBitmap;
}

int fat32_part_mark_on_disk_bitmap(BYTE * pBitmap, int diskfd, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize)
{
	return fat32_mark_bitmap(pBitmap, diskfd, partBeginSec, partBeginSec, partTotalSec, bitmapSize);
}
