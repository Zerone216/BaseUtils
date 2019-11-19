/****************************************************************************************************************************
*
*   文 件 名 ： mbrpt.c 
*   文件描述 ：  
*   创建日期 ：2019年5月14日
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

#include "mbrpt.h"

int assign_partinfo_from_parttable(DWORD partTabSec, MBR_PART_TABLE * mbrPartTable, MBR_PART * mbrDp, int itemIndex)
{
	MBR_PART_INFO *pmbrPartInfo = &mbrPartTable->mbrPartInfo[mbrPartTable->partNum];
	//iLog("============= part[%d] ==============", mbrPartTable->partNum);
	
	memcpy(&pmbrPartInfo->mbrPart, mbrDp, sizeof(MBR_PART));
	pmbrPartInfo->partTabSec = partTabSec;
	pmbrPartInfo->itemIndex = itemIndex;
	
	pmbrPartInfo->mbrPart.beginSec += partTabSec; //更新起始扇区为相对于硬盘0扇区的绝对扇区号
	
	mbrPartTable->partNum ++;
	
	//iLog("partTabSec=[%d]", pmbrPartInfo->partTabSec);
	//iLog("beginSec=[%d]", pmbrPartInfo->mbrPart.beginSec);
	//iLog("totalSec=[%d]", pmbrPartInfo->mbrPart.totalSec);
	//iLog("pType=[0x%x]", pmbrPartInfo->mbrPart.pType);
	
	return 0;
}

static int get_mbr_part_info(int diskfd, DWORD partTabSec, MBR_PART_TABLE * mbrPartTable)
{
	BYTE * sector = (BYTE *) malloc(SECTOR_SIZE);
	if(sector == NULL)
		return -1;
	
	if(readwrite_hdisk_sector(DISK_READ, diskfd, partTabSec, 1, sector) < 0)
	{
		safe_free(&sector);
		return -1;
	}

	MBR_SECTOR * pMbrSec = (MBR_SECTOR *)sector;
	if(pMbrSec->EndFlag != 0XAA55)
	{
		safe_free(&sector);
		return -1;
	}

	int i = 0;
	for(i = 0; i < MBR_PRI_PART_NUM_MAX; i++)
	{
		if(pMbrSec->mbrDp[i].beginSec == 0 || pMbrSec->mbrDp[i].totalSec == 0)
			continue;
		
		if(pMbrSec->mbrDp[i].pType == MPT_INVAILD_PART ||\
			pMbrSec->mbrDp[i].pType == MPT_UNKNOWN_PART) //无效和未知的分区类型
			continue;

		//////////////////////////////////////////////////////////////////////////////////////////////////
		assign_partinfo_from_parttable(partTabSec, mbrPartTable, &pMbrSec->mbrDp[i], i);
		
		if(pMbrSec->mbrDp[i].pType == MPT_EXTEND_PART || \
			pMbrSec->mbrDp[i].pType == MPT_WIN_EXTEND_PART || \
			pMbrSec->mbrDp[i].pType == MPT_LINUX_EXTEND_PART) //扩展分区，不计入分区表中
		{
			iLog("find an extend part: beginSec=[%d] pType=[0x%x]", pMbrSec->mbrDp[i].beginSec + partTabSec, pMbrSec->mbrDp[i].pType);

			//////////////////////////////////////////////////////////////////////////////////////////////////
			get_mbr_part_info(diskfd, pMbrSec->mbrDp[i].beginSec + partTabSec, mbrPartTable); //递归调用，深度优先搜索
			//////////////////////////////////////////////////////////////////////////////////////////////////
		}
	}
	
	safe_free(&sector);
	return 0;
}


int get_mbr_parttable(int diskfd, MBR_PART_TABLE * mbrPartTable)
{
	memset(mbrPartTable, 0x00, sizeof(MBR_PART_TABLE));
	return get_mbr_part_info(diskfd, 0, mbrPartTable);
}

static MBR_PARTTABLE * get_mbr_part_data(int diskfd, DWORD partTabSec, MBR_PARTTABLE * pMbrPt)
{	
	BYTE * sector = (BYTE *) malloc(SECTOR_SIZE);
	if(sector == NULL)
		return pMbrPt;
	
	if(readwrite_hdisk_sector(DISK_READ, diskfd, partTabSec, 1, sector) < 0)
	{
		safe_free(&sector);
		return pMbrPt;
	}

	MBR_SECTOR * pMbrSec = (MBR_SECTOR *)sector;
	if(pMbrSec->EndFlag != 0XAA55)
	{
		safe_free(&sector);
		return pMbrPt;
	}

	pMbrPt = (MBR_PARTTABLE *)malloc(sizeof(MBR_PARTTABLE));
	if(pMbrPt == NULL)
	{
		safe_free(&sector);
		return pMbrPt;
	}
	
	memset(pMbrPt, 0x00, sizeof(MBR_PARTTABLE));
	pMbrPt->partTabSec = partTabSec;
	memcpy(&pMbrPt->mbrSector, pMbrSec, sizeof(MBR_SECTOR));
	pMbrPt->next = NULL;
	iLog("find MBR parttable at sector: [%d]", pMbrPt->partTabSec);
	
	int i = 0;
	for(i = 0; i < MBR_PRI_PART_NUM_MAX; i++)
	{
		if(pMbrSec->mbrDp[i].beginSec == 0 || pMbrSec->mbrDp[i].totalSec == 0)
			continue;
		
		if(pMbrSec->mbrDp[i].pType == MPT_INVAILD_PART ||\
			pMbrSec->mbrDp[i].pType == MPT_UNKNOWN_PART) //无效和未知的分区类型
			continue;
		
		if(pMbrSec->mbrDp[i].pType == MPT_EXTEND_PART || \
			pMbrSec->mbrDp[i].pType == MPT_WIN_EXTEND_PART || \
			pMbrSec->mbrDp[i].pType == MPT_LINUX_EXTEND_PART) //扩展分区，不计入分区表中
		{
			pMbrPt->next = get_mbr_part_data(diskfd, pMbrSec->mbrDp[i].beginSec + partTabSec, pMbrPt->next);
		}
	}
	
	safe_free(&sector);
	return pMbrPt;
}

MBR_PARTTABLE * get_mbr_parttable_data_from_disk(int diskfd)
{
	MBR_PARTTABLE * pMbrPt = NULL;
	return get_mbr_part_data(diskfd, 0, pMbrPt);
}

int write_mbr_parttable_data_to_disk(int diskfd, MBR_PARTTABLE * pMbrPt)
{
	MBR_PARTTABLE * pTmpMbrPt = pMbrPt;
	while(pTmpMbrPt)
	{
		MBR_PARTTABLE * pTmp = pTmpMbrPt;
		pTmpMbrPt = pTmpMbrPt->next;
		
		if(readwrite_hdisk_sector(DISK_WRITE, diskfd, pTmp->partTabSec, 1, P_BYTE(&pTmp->mbrSector)) < 0)
			return -1;
	}
	
	return 0;
}

void release_mbr_parttable_data(MBR_PARTTABLE * pMbrPt)
{
	MBR_PARTTABLE * pTmpMbrPt = pMbrPt;

	while(pTmpMbrPt)
	{
		MBR_PARTTABLE * pTmp = pTmpMbrPt;
		pTmpMbrPt = pTmpMbrPt->next;
		
		safe_free(&pTmp);
	}
	
	return;
}


