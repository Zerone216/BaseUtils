/****************************************************************************************************************************
*
*   文 件 名 ： gptpt.c 
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

#include "gptpt.h"

int get_gpt_parttable(char * diskDevName, int diskfd, DDWORD diskCapability, GPT_PART_TABLE * gptTartTable)
{
	int i = 0;
	memset(gptTartTable, 0x00, sizeof(GPT_PART_TABLE));
	
	BYTE * tmpbuf = malloc(GPT_PART_TABLE_SIZE);
	if(tmpbuf == NULL)
		return -1;
	
	if(readwrite_hdisk_sector(DISK_READ, diskfd, 0, 1, P_BYTE(&gptTartTable->pMbrSec)) == -1)
	{
		safe_free(&tmpbuf);
		return -1;
	}
	
	if(readwrite_hdisk_sector(DISK_READ, diskfd, 1, 1, P_BYTE(&gptTartTable->gptHeader)) == -1)
	{
		safe_free(&tmpbuf);
		return -1;
	}

	if(readwrite_hdisk_sector(DISK_READ, diskfd, diskCapability - 1, 1, P_BYTE(&gptTartTable->gptHeaderBak)) == -1)
	{
		safe_free(&tmpbuf);
		return -1;
	}
	
	memset(tmpbuf, 0x00, GPT_PART_TABLE_SIZE);
	if(readwrite_hdisk_sector(DISK_READ, diskfd, 2, 32, tmpbuf) == -1)
	{
		safe_free(&tmpbuf);
		return -1;
	}
	
	for(i = 0; i < GPT_PART_NUM_MAX; i++)
	{
		int offset = GPT_PART_INFO_SIZE * i;

		GPT_PART_INFO * pPartInfo = &gptTartTable->gPartInfo[i];
		
		memcpy(&pPartInfo->gtpPart, tmpbuf + offset, GPT_PART_INFO_SIZE);
		
		if(pPartInfo->gtpPart.ent_lba_start != 0 && \
			pPartInfo->gtpPart.ent_lba_end != 0)
		{
			gptTartTable->partNum = i + 1;
			
			get_part_dev_name(pPartInfo->gtpPart.ent_lba_start, diskDevName, pPartInfo->devName, sizeof(pPartInfo->devName));
			
			if(memcmp(pPartInfo->gtpPart.ent_type, &(EFI_GUID)GPT_ENT_TYPE_MS_RECOVERY_GUID, 16) == 0) //RECOVERY
			{
				pPartInfo->partType = GPT_RECOVERY_PART;
				iLog("RECOVERY part.");
			}

			if(memcmp(pPartInfo->gtpPart.ent_type, &(EFI_GUID)GPT_ENT_TYPE_MS_ESP_GUID, 16) == 0) //ESP
			{
				pPartInfo->partType = GPT_ESP_PART;
				iLog("ESP part.");
			}

			if(memcmp(pPartInfo->gtpPart.ent_type, &(EFI_GUID)GPT_ENT_TYPE_MS_RESERVED_GUID, 16) == 0) //MSR
			{
				pPartInfo->partType = GPT_MSR_PART;
				iLog("MSR part.");
			}

			if(memcmp(pPartInfo->gtpPart.ent_type, &(EFI_GUID)GPT_ENT_TYPE_BLANK_PART_GUID, 16) == 0) //BLANK
			{
				pPartInfo->partType = GPT_BLANK_PART;
				iLog("BLANK part.");
			}
			
			if(memcmp(pPartInfo->gtpPart.ent_type, &(EFI_GUID)GPT_ENT_TYPE_MS_BASIC_PART_GUID, 16) == 0) //BASIC
			{
				pPartInfo->partType = GPT_BASIC_PART;
				iLog("BASIC part.");
			}

			if(memcmp(pPartInfo->gtpPart.ent_type, &(EFI_GUID)GPT_ENT_TYPE_SRV_ESP_GUID, 16) == 0) //SRV
			{
				pPartInfo->partType = GPT_SRV_ESP_PART;
				iLog("SRV part.");
			}
						
		}
	}
	
	//show_part_info_by_parttable(parttable);
	return 0;
}




