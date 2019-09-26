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
*   文 件 名 ： diskopt.c 
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

#include "diskopt.h"

int get_disk_part_info(DEF_DISK_INFO * pDiskInfo)
{

	if(pDiskInfo->ptMode == DISK_PART_TABLE_MBR)
	{
		MBR_PART_TABLE mbrPartTable;
		get_mbr_parttable(pDiskInfo->diskfd, &mbrPartTable);
				
		int i = 0;
		for(i = 0; i < mbrPartTable.partNum; i++)
		{
			iLog("--------------part[%d] --------------", i);
			
			DEF_PART_INFO * pPartInfo = &pDiskInfo->partInfo[i];
			MBR_PART_INFO * pMbrPartInfo = &mbrPartTable.mbrPartInfo[i];

			
			pPartInfo->beginSec = pMbrPartInfo->mbrPart.beginSec;
			pPartInfo->sectorNum = pMbrPartInfo->mbrPart.totalSec;
			pPartInfo->endSec = pPartInfo->beginSec + pPartInfo->sectorNum - 1;
			pPartInfo->partTabSec = pMbrPartInfo->partTabSec;

			iLog("beginSec=[%llu]", pPartInfo->beginSec);
			iLog("sectorNum=[%llu]", pPartInfo->sectorNum);
			iLog("endSec=[%llu]", pPartInfo->endSec);
			iLog("partTabSec=[%llu]", pPartInfo->partTabSec);
			
			pPartInfo->bootId = pMbrPartInfo->mbrPart.bootId;
			pPartInfo->beginHead = pMbrPartInfo->mbrPart.beginHead;
			pPartInfo->beginCylnAndSec = pMbrPartInfo->mbrPart.beginCylnAndSec;
			pPartInfo->endHead = pMbrPartInfo->mbrPart.endHead;
			pPartInfo->endCylnAndSec = pMbrPartInfo->mbrPart.endCylnAndSec;
			pPartInfo->partType = pMbrPartInfo->mbrPart.pType;
			
			iLog("bootId=[0x%x]", pPartInfo->bootId);
			iLog("beginHead=[0x%x]", pPartInfo->beginHead);
			iLog("beginCylnAndSec=[0x%x]", pPartInfo->beginCylnAndSec);
			iLog("endHead=[0x%x]", pPartInfo->endHead);
			iLog("endCylnAndSec=[0x%x]", pPartInfo->endCylnAndSec);
			iLog("partType=[0x%x]", pPartInfo->partType);
			
			
			if(pMbrPartInfo->mbrPart.pType == MPT_EXTEND_PART || \
					pMbrPartInfo->mbrPart.pType == MPT_WIN_EXTEND_PART || \
					pMbrPartInfo->mbrPart.pType == MPT_LINUX_EXTEND_PART)
				pPartInfo->partMode = DISK_PART_MODE_EXTEND;
			else
			{
				if(pMbrPartInfo->partTabSec > 0)
					pPartInfo->partMode = DISK_PART_MODE_LOGIC;
				else
					pPartInfo->partMode = DISK_PART_MODE_PRIMARY;
			}
			iLog("partMode=[0x%x]", pPartInfo->partMode);
			
			get_part_dev_name(pPartInfo->beginSec, pDiskInfo->devName, pPartInfo->devName, sizeof(pPartInfo->devName));
			if(strlen(pPartInfo->devName) > 0)
			{
				if(pPartInfo->partMode == DISK_PART_MODE_PRIMARY || \
					pPartInfo->partMode == DISK_PART_MODE_LOGIC) //主分区和逻辑分区
				{
					pPartInfo->fileSys = get_part_filesys(pPartInfo->devName);; //
					iLog("fileSys=[0x%x]", pPartInfo->fileSys);
					
					get_dev_node_info(pPartInfo->devName, &pPartInfo->devNode);
					get_part_label_name(pPartInfo->devName, pPartInfo->partLabel, sizeof(pPartInfo->partLabel));
					//strncpy(pPartInfo->partName, pPartInfo->partLabel, sizeof(pPartInfo->partName));
					if(strlen(pPartInfo->partName) > 0)
						iLog("partName=[%s]", pPartInfo->partName);
					
					get_mbr_part_type_guid(pPartInfo->devName, pPartInfo->typeGuid, sizeof(pPartInfo->typeGuid));
					get_mbr_part_guid(pPartInfo->devName, pPartInfo->partGuid, sizeof(pPartInfo->partGuid));
				}
			}
			
			pDiskInfo->partNum ++;
		}		
	}
	else if(pDiskInfo->ptMode == DISK_PART_TABLE_GPT)
	{
		GPT_PART_TABLE gptTartTable;
		get_gpt_parttable(pDiskInfo->devName, pDiskInfo->diskfd, pDiskInfo->diskCapability, &gptTartTable);

		int i = 0;
		for(i = 0; i < gptTartTable.partNum; i++)
		{
			iLog("============= part[%d] ==============", i);
			
			DEF_PART_INFO * pPartInfo = &pDiskInfo->partInfo[i];
			GPT_PART_INFO * pGptPartInfo = &gptTartTable.gPartInfo[i];

			
			pPartInfo->beginSec = pGptPartInfo->gtpPart.ent_lba_start;
			pPartInfo->endSec = pGptPartInfo->gtpPart.ent_lba_end;
			pPartInfo->sectorNum = pPartInfo->endSec + 1 - pPartInfo->beginSec;
			pPartInfo->partTabSec = 2 + i/4; //从2扇区开始，每个扇区4个表项
			pPartInfo->partAttr = pGptPartInfo->gtpPart.ent_attr;

			unicode_to_ascii(P_UNICODE(pGptPartInfo->gtpPart.ent_name), sizeof(pGptPartInfo->gtpPart.ent_name) / 2, pPartInfo->partName, sizeof(pPartInfo->partName));
			iLog("partName=[%s]", pPartInfo->partName);
			iLog("beginSec=[%llu]", pPartInfo->beginSec);
			iLog("sectorNum=[%llu]", pPartInfo->sectorNum);
			iLog("endSec=[%llu]", pPartInfo->endSec);
			iLog("partTabSec=[%llu]", pPartInfo->partTabSec);

			memcpy(pPartInfo->typeGuid, pGptPartInfo->gtpPart.ent_type, sizeof(pPartInfo->typeGuid));
			memcpy(pPartInfo->partGuid, pGptPartInfo->gtpPart.ent_uuid, sizeof(pPartInfo->partGuid));
			iLog("typeGuid=[" FMT_GUID_UPPER "]", SNIF_GUID(pPartInfo->typeGuid));
			iLog("partGuid=[" FMT_GUID_UPPER "]", SNIF_GUID(pPartInfo->partGuid));
			
			pPartInfo->partType = pGptPartInfo->partType;
			pPartInfo->partMode = DISK_PART_MODE_PRIMARY;
			
			iLog("partType=[0x%x]", pPartInfo->partType);
			iLog("partMode=[0x%x]", pPartInfo->partMode);
			
			get_part_dev_name(pPartInfo->beginSec, pDiskInfo->devName, pPartInfo->devName, sizeof(pPartInfo->devName));
			if(strlen(pPartInfo->devName) > 0)
			{
				pPartInfo->fileSys = get_part_filesys(pPartInfo->devName);
				get_dev_node_info(pPartInfo->devName, &pPartInfo->devNode);
				get_part_label_name(pPartInfo->devName, pPartInfo->partLabel, sizeof(pPartInfo->partLabel));
			}
			
			
			pDiskInfo->partNum ++;
		}
	}
	else
	{
		;
	}

	return 0;
}



int get_disk_info_by_index(DEF_DISK_INFO * diskInfo, int index)
{
	DEF_DISK_INFO * pDiskInfo = &diskInfo[index];
	
	get_disk_devname_by_index(pDiskInfo->devName, index);
	get_disk_serial_num(pDiskInfo->devName, pDiskInfo->serialNum, sizeof(pDiskInfo->serialNum));
	pDiskInfo->interfaceType = get_disk_interface_type(pDiskInfo->devName);
	pDiskInfo->removable = get_disk_removable(pDiskInfo->devName);
	get_dev_node_info(pDiskInfo->devName, &pDiskInfo->devNode);
	
	pDiskInfo->diskfd = open_disk(pDiskInfo->devName, O_RDWR);
	if(pDiskInfo->diskfd == -1)
		return-1;
	
	pDiskInfo->diskChs = get_disk_geometry(pDiskInfo->devName, pDiskInfo->diskfd);
	pDiskInfo->diskCapability = pDiskInfo->diskChs.size;
	iLog("diskCapability=[%llu]", pDiskInfo->diskCapability);
	
	pDiskInfo->ptMode = get_disk_parttable_mode(pDiskInfo->devName);
	get_disk_guid(pDiskInfo->ptMode, pDiskInfo->diskfd, pDiskInfo->diskGuid);
	
	///////////////////////////////////////////
	get_disk_part_info(pDiskInfo);
	///////////////////////////////////////////
	
	return 0;
}

LOCAL_DISK_INFO * local_disk_info_init()
{
	LOCAL_DISK_INFO * pLocalDisk = (LOCAL_DISK_INFO *)z_malloc(sizeof(LOCAL_DISK_INFO));
	if(pLocalDisk == NULL)
		return NULL;
	
	iLog("sizeof(LOCAL_DISK_INFO)=[%d]", sizeof(LOCAL_DISK_INFO));
	
	int diskNum = get_local_disk_num();
	for(pLocalDisk->diskNum = 0; pLocalDisk->diskNum < diskNum; )
	{
		iLog("========================== disk[%d]==============================", pLocalDisk->diskNum);
		if(get_disk_info_by_index(pLocalDisk->diskInfo, pLocalDisk->diskNum) == -1)
			continue;

		pLocalDisk->diskNum ++;
	}
	
	return pLocalDisk;
}

void local_disk_info_release(LOCAL_DISK_INFO ** plocalDisk)
{
	LOCAL_DISK_INFO * localDisk = *plocalDisk;
	int i = 0;
	for(i = 0; i < localDisk->diskNum; i ++)
		close_disk(&localDisk->diskInfo[i].diskfd);
	
	safe_free(plocalDisk);
	return ;
}

