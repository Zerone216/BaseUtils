/****************************************************************************************************************************
*
*   文 件 名 ： fsFat32.c 
*   文件描述 ：  
*   创建日期 ：2019年9月4日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
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

#include "fsFat32.h"


static void * Fat32_malloc(size_t size){void * p=malloc(size); if(p) memset(p, 0x00, size); return p;}
static void Fat32_free(void ** p){if(*p) { free(*p); *p = NULL;}}
static char * Fat32_StringCleanAssignEndof(char * src , int len, char asg)
{
	int pos = len - 1;
	while(pos >= 0)
	{
		if(src[pos] == asg)
			pos --;
		else
			break;
	}
	
	src[pos + 1] = '\0';
	return src;
}

static char * Fat32_ByteToString(char * dst, int dstLen, BYTE * src, int assignLen)
{
	int cpLen = MIN(dstLen - 1, assignLen);
	memset(dst, '\0', dstLen); 
	memcpy(dst, src, cpLen);
	
	return Fat32_StringCleanAssignEndof(dst, cpLen, ' ');
}

static BYTE Fat32_CalcCheckValue(BYTE * shortName){BYTE chknum = 0;int i,j = 0; for (i = 11; i >0; i--) chknum=((chknum & 1) ? 0x80 : 0) + (chknum >> 1) + shortName[j++]; return chknum;}

static int Fat32_OpenDev(const char * devname, int rwMode){return open(devname, rwMode);}
static void Fat32_CloseDev(int diskfd){close(diskfd);}

static int Fat32_ReadSector(int diskfd, DDWORD partStartSec, DDWORD offsetLogicSec, int secNum, BYTE * buff)
{
	DDWORD offset = (partStartSec + offsetLogicSec) * SECTORSIZE;
	DDWORD dataSize = secNum * SECTORSIZE;
	if(buff == NULL && dataSize == 0)
		return -1;

	DDWORD realStartSec = partStartSec + offsetLogicSec;
	//iLog("Fat32_ReadSector:[%llu + %d]", realStartSec, secNum);
	
	if(lseek64(diskfd, offset, SEEK_SET) == -1)
	{
		eLog("lseek offset failed!");
		assert(0);
		return -1;
	}
	
	if(read(diskfd, buff, dataSize) != dataSize)
	{
		eLog("read hdisk sector failed!");
		assert(0);
		return -1;
	}
	
	return 0;
}

static int Fat32_WriteSector(int diskfd, DDWORD partStartSec, DDWORD offsetLogicSec, BYTE * buff, DDWORD dataSize)
{
	if(buff == NULL && dataSize == 0)
		return -1;
	
	DDWORD offset = (partStartSec + offsetLogicSec) * SECTORSIZE;	
	if(lseek64(diskfd, offset, SEEK_SET) == -1)
	{
		eLog("lseek offset failed!");
		assert(0);
		return -1;
	}
		
	if(write(diskfd, buff, dataSize) != dataSize)
	{
		eLog("write hdisk sector failed!");
		assert(0);
		return -1;
	}
	
	return 0;
}

static BYTE * Fat32_DumpSector(int diskfd, DDWORD partStartSec, DDWORD offsetLogicSec, int secNum)
{
	DDWORD dataSize = secNum * SECTORSIZE;
	if(dataSize == 0)
		return NULL;
	
	BYTE * buff = (BYTE *) Fat32_malloc(dataSize);
	if(buff)
	{
		if(Fat32_ReadSector(diskfd, partStartSec, offsetLogicSec, secNum, buff) == -1)
		{
			Fat32_free((void **)buff);
			return NULL;
		}
	}
	
	return buff;
}

static void Fat32_ReleaseSector(BYTE ** buff){Fat32_free((void **) buff);}
int Fat32_read_data_block(int diskfd, DDWORD partStartSec, DDWORD StartSec, int secNum, BYTE * buff){return Fat32_ReadSector(diskfd, partStartSec, StartSec, secNum, buff);}
int Fat32_write_data_block(int diskfd, DDWORD partStartSec, DDWORD StartSec, int secNum, BYTE * buff){return Fat32_WriteSector(diskfd, partStartSec, StartSec, secNum, buff);}

static char * Fat32_unicode_to_utf8 (WORD * unicode, int wlen, char * utf8, int osize){memset(utf8, '\0', osize); int utf8Len = sconv_unicode_to_utf8((const wchar *)unicode, wlen, utf8, osize); utf8[utf8Len] = 0; return utf8;}
static WORD * Fat32_utf8_to_unicode (char * utf8, int slen, WORD * unicode, int osize){memset(unicode, '\0', osize); sconv_utf8_to_unicode((const char *)utf8, slen, unicode, osize); return unicode;}

static char * Fat32_WcharToUniString(WORD *wcSrcStr, int wlen, char * DstStr, int osize)
{
    int i;
    BYTE * src = (BYTE *)wcSrcStr;
	char * dst = DstStr;
    for(i = 0; i < MIN(wlen, osize) ;i ++)
    {
    	if(*src > 0x7e)
			*dst = 0;
		else
        	*dst = *src;
		
        src += 2;
        dst ++;
    }
	
    return DstStr;
}



int Fat32_StringFindFirstOf(char * baseStr, char assign) {int i, pos = -1; for(i = 0;i < strlen(baseStr); i++) if(*(baseStr+i) == assign){pos = i;break;}return pos;}

int Fat32_get_file_name_from_path_by_deepth(char * filePath, int deepth, char * fileName, int bufLen)
{
	char * pFpath = filePath;
	int fileIndex = 0;
	int pathLen = strlen(pFpath);
	int offset = 0;

	if(deepth <= 0)
		return -1;
	
	while(pFpath)
	{
		int posLen = Fat32_StringFindFirstOf(pFpath, '/');
		if(posLen >= 0)
		{
			if(posLen > 0)
			{
				fileIndex ++;
				if(fileIndex >= deepth)
				{
					memset(fileName, '\0', bufLen);
					memcpy(fileName, pFpath, posLen);
					//iLog("fileName=[%s]", fileName);
					return 0;
				}
			}
			
			pFpath += (posLen + 1);
			offset += (posLen + 1);
		}
		else
			break;
	}

	if(fileIndex == deepth - 1 && pathLen > offset)
	{
		memset(fileName, '\0', bufLen);
		memcpy(fileName, pFpath, pathLen - offset);
		//iLog("fileName=[%s]", fileName);
		return 1;
	}
	else
		return -1;
}

int Fat32_get_file_path_deepth(char * filePath)
{
	char * pFpath = filePath;
	int deepth = 0;
	int pathLen = strlen(pFpath);
	int offset = 0;
	
	while(pFpath)
	{
		int posLen = Fat32_StringFindFirstOf(pFpath, '/');
		if(posLen >= 0)
		{
			if(posLen > 0)
				deepth ++;
			
			pFpath += (posLen + 1);
			offset += (posLen + 1);
		}
		else
			break;
	}

	if(pathLen > offset)
		deepth ++;
	
	return deepth;
}

DWORD Fat32_CalcCluster(WORD high16VCN, WORD low16VCN)
{
	DWORD startVCN = high16VCN;
	startVCN <<= 16;
	startVCN += low16VCN;
	return startVCN;
}

DWORD Fat32_ClusterToSector(DWORD Cluster, DWORD RootDirSecStart, WORD SecPerCluster)
{
    if (Cluster < 2)
        return 0;
    return (RootDirSecStart + SecPerCluster * (Cluster -2));
}

int Fat32_show_short_fdt_info(SHORT_FDT32 * pShortFdt)
{
	char NameBuf[128];
	char SfxBuf[64];

	Fat32_ByteToString(NameBuf, sizeof(NameBuf), pShortFdt->fileName, sizeof(pShortFdt->fileName));
	Fat32_ByteToString(SfxBuf, sizeof(SfxBuf), pShortFdt->extName, sizeof(pShortFdt->extName));
	
	if(FAT_ENTRY_VOLUME(pShortFdt->attribute))
		iLog("VOLUME=[%s%s]",NameBuf, SfxBuf);
	else if(FAT_ENTRY_DIR(pShortFdt->attribute))
	{
		if(strlen(SfxBuf))
			iLog("dirName=[%s.%s]",NameBuf, SfxBuf);
		else
			iLog("dirName=[%s]",NameBuf);
	}
	else if(FAT_ENTRY_FILE(pShortFdt->attribute))
	{
		if(strlen(SfxBuf))
			iLog("fileName=[%s.%s]",NameBuf, SfxBuf);
		else
			iLog("fileName=[%s]",NameBuf);
	}
	else
		;
	
	//iLog("fileName=[%-8.8s.%-3.3s]",pShortFdt->fileName, pShortFdt->extName);
	//iLog("CheckValue=[0x%x]", Fat32_CalcCheckValue(&pShortFdt->fileName[0]));
	iLog("attribute=[0x%x]", pShortFdt->attribute);
	iLog("reserved=[0x%x]", pShortFdt->reserved);
	iLog("milliTime=[0x%x]", pShortFdt->milliTime);
	iLog("createTime=[%04d-%02d-%02d %02d:%02d:%02d]", pShortFdt->createDate.year + 1980, \
														pShortFdt->createDate.month, \
														pShortFdt->createDate.day,\
														pShortFdt->createTime.hour + 8, \
														pShortFdt->createTime.min, \
														pShortFdt->createTime.twoSec*2);
	iLog("lastVisitDate=[%04d-%02d-%02d]", pShortFdt->lastVisitDate.year + 1980, pShortFdt->lastVisitDate.month, pShortFdt->lastVisitDate.day);
	iLog("modifyTime=[%04d-%02d-%02d %02d:%02d:%02d]", pShortFdt->modifyDate.year + 1980, \
														pShortFdt->modifyDate.month, \
														pShortFdt->modifyDate.day, \
														pShortFdt->modifyTime.hour + 8, \
														pShortFdt->modifyTime.min, \
														pShortFdt->modifyTime.twoSec*2);
	iLog("high16VCN=[%d]", pShortFdt->high16VCN);
	iLog("low16VCN=[%d]", pShortFdt->low16VCN);
	iLog("fileSzie=[%d]", pShortFdt->fileSzie);
	
	return 0;
}

char * Fat32_generate_assign_string(WORD * wStr, int assignLen, char * outStr, int outSize)
{
	//Fat32_WcharToUniString(wStr, assignLen, outStr, outSize);
	//Fat32_unicode_to_utf8(wStr, assignLen, outStr, outSize);
	
	StrUtf16ToUtf8(wStr, assignLen, outStr, outSize);
	return outStr;
}

static char * Fat32_get_long_fdt_filename(LONG_FDT32 * pLongFdt, char * fileName, int bufLen)
{
	memset(fileName, '\0', bufLen);
	
	int sLen = 0;
	int tLen = 0;
	char nameBuf[256] = {0};
	sLen = ARRLEN(pLongFdt->longFileName1);
	Fat32_generate_assign_string(pLongFdt->longFileName1, sLen, nameBuf, sizeof(nameBuf));
	if(strlen(nameBuf) > 0) 
		strncat(fileName, nameBuf, bufLen);
	tLen += sLen;
	
	sLen = ARRLEN(pLongFdt->longFileName2);
	Fat32_generate_assign_string(pLongFdt->longFileName2, sLen, nameBuf, sizeof(nameBuf));
	if(strlen(nameBuf) > 0) 
		strncat(fileName, nameBuf, bufLen);
	tLen += sLen;

	sLen = ARRLEN(pLongFdt->longFileName3); 
	Fat32_generate_assign_string(pLongFdt->longFileName3, sLen, nameBuf, sizeof(nameBuf));
	if(strlen(nameBuf) > 0) 
		strncat(fileName, nameBuf, bufLen);
	tLen += sLen;
	
	fileName[tLen] = 0;
	//Fat32_StringCleanAssignEndof(fileName, strlen(fileName), ' ');
	
	
	return fileName;
}

static char * Fat32_get_whole_filename(BYTE * fdtBuf, int itemNum, char * wholeName, int NameLen)
{
	if(!wholeName)
		return NULL;
	
	memset(wholeName, '\0', NameLen);
	if(!fdtBuf || itemNum == 0)
		return wholeName;
	
	FDT_ITEM * pFdtItem = (FDT_ITEM * ) fdtBuf;
	char fileName[256] = {0};
	
	int i =0;
	for(i = itemNum - 1; i >= 0; i --)
	{
		if(FAT_ENTRY_LONG(pFdtItem[i].longFdt.attribute))
		{
			Fat32_get_long_fdt_filename(&pFdtItem[i].longFdt, fileName, sizeof(fileName));
			strncat(wholeName, fileName, NameLen);
		}
		else
		{
			if(FAT_ENTRY_DIR(pFdtItem[i].longFdt.attribute) && itemNum == 1) //
			{
				Fat32_ByteToString(wholeName, sizeof(wholeName), pFdtItem[i].shortFdt.fileName, sizeof(pFdtItem[i].shortFdt.fileName));
				break;
			}
		}
	}
	
	return wholeName;
}


int Fat32_show_long_fdt_info(LONG_FDT32 * pLongFdt)
{
	char fileName[256] = {0};
	
	iLog("dirId=[0x%x]", pLongFdt->dirId);
	//iLog("longFileName=[%s]", Fat32_get_long_fdt_filename(pLongFdt, fileName, sizeof(fileName)));
	//iLog("attribute=[0x%x]", pLongFdt->attribute);
	//iLog("reserved=[0x%x]", pLongFdt->reserved);
	iLog("checkValue=[0x%x]", pLongFdt->checkValue);	
	iLog("startVCN=[%d]", pLongFdt->startVCN);
	
	return 0;
}

BYTE Fat32_get_check_value_from_fdt_item(FDT_ITEM * pFdtItem)
{
	if(FAT_ENTRY_LONG(pFdtItem->longFdt.attribute))
		return pFdtItem->longFdt.checkValue;
	else
		return Fat32_CalcCheckValue(&pFdtItem->shortFdt.fileName[0]);
}


FAT32_PART_INFO * Fat32_get_part_info(const char * devName, UINT64 partStartSec, UINT64 partTotalSec)
{
	FAT32_PART_INFO * pFat32PartInfo = (FAT32_PART_INFO *) Fat32_malloc(sizeof(FAT32_PART_INFO));
	if(!pFat32PartInfo)
		return NULL;
	
	iLog("sizeof(FAT32_BOOT_INFO)=[%d]", sizeof(FAT32_BOOT_INFO));
	iLog("sizeof(FDT_TIME)=[%d]", sizeof(FDT_TIME));
	iLog("sizeof(FDT_DATE)=[%d]", sizeof(FDT_DATE));
	
	iLog("sizeof(SHORT_FDT32)=[%d]", sizeof(SHORT_FDT32));
	iLog("sizeof(LONG_FDT32)=[%d]", sizeof(LONG_FDT32));
	
	pFat32PartInfo->partStartSec = partStartSec;
	pFat32PartInfo->partTotalSec = partTotalSec;
	pFat32PartInfo->devfd = Fat32_OpenDev(devName, O_RDWR);
	if(pFat32PartInfo->devfd < 0)
	{
		Fat32_free((void **)pFat32PartInfo);
		return NULL;
	}

	Fat32_ReadSector(pFat32PartInfo->devfd, pFat32PartInfo->partStartSec, 0, 2, pFat32PartInfo->bootSec);
	pFat32PartInfo->pFat32BootInfo = (FAT32_BOOT_INFO *)pFat32PartInfo->bootSec;
	pFat32PartInfo->pFilesysSectorInfo = (FILESYS_SECTOR_INFO *)(pFat32PartInfo->bootSec + sizeof(FAT32_BOOT_INFO));
	
	/////////////////////////////////////////////////////////////////////////////////////
	char strBuf[512];
	iLog("Jumpto=[0x%x]", pFat32PartInfo->pFat32BootInfo->Jumpto);
	iLog("OemID=[%s]", Fat32_ByteToString(strBuf, sizeof(strBuf), pFat32PartInfo->pFat32BootInfo->OemID, sizeof(pFat32PartInfo->pFat32BootInfo->OemID)));
	iLog("BytesPerSector=[%d]", pFat32PartInfo->pFat32BootInfo->BytesPerSector);
	iLog("SecPerClr=[%d]", pFat32PartInfo->pFat32BootInfo->SecPerClr);
	iLog("ReservedSectors=[%d]", pFat32PartInfo->pFat32BootInfo->ReservedSectors);
	iLog("NumOfFat=[%d]", pFat32PartInfo->pFat32BootInfo->NumOfFat);
	iLog("MediaID=[0x%x]", pFat32PartInfo->pFat32BootInfo->MediaID);
	iLog("SecPerTrk=[%d]", pFat32PartInfo->pFat32BootInfo->SecPerTrk);
	iLog("BigTotalSec=[%d]", pFat32PartInfo->pFat32BootInfo->BigTotalSec);
	iLog("BigSecPerFat=[%d]", pFat32PartInfo->pFat32BootInfo->BigSecPerFat);
	iLog("BPB_RootClus=[%d]", pFat32PartInfo->pFat32BootInfo->BPB_RootClus);
	iLog("DISKLabel=[%s]", Fat32_ByteToString(strBuf, sizeof(strBuf), pFat32PartInfo->pFat32BootInfo->DISKLabel, sizeof(pFat32PartInfo->pFat32BootInfo->DISKLabel)));
	iLog("FileSystem=[%s]", Fat32_ByteToString(strBuf, sizeof(strBuf), pFat32PartInfo->pFat32BootInfo->FileSystem, sizeof(pFat32PartInfo->pFat32BootInfo->FileSystem)));
	//iLog("guidCode=[%s]", Fat32_ByteToString(strBuf, sizeof(strBuf), pFat32PartInfo->pFat32BootInfo->FillInfo, sizeof(pFat32PartInfo->pFat32BootInfo->guidCode)));
	iLog("endFlag=[0x%x]", pFat32PartInfo->pFat32BootInfo->endFlag);
	/////////////////////////////////////////////////////////////////////////////////////
	iLog("extGuidTag=[%s]", Fat32_ByteToString(strBuf, sizeof(strBuf), pFat32PartInfo->pFilesysSectorInfo->extGuidTag, sizeof(pFat32PartInfo->pFilesysSectorInfo->extGuidTag)));
	iLog("fileSysSign=[%s]", Fat32_ByteToString(strBuf, sizeof(strBuf), pFat32PartInfo->pFilesysSectorInfo->fileSysSign, sizeof(pFat32PartInfo->pFilesysSectorInfo->fileSysSign)));
	iLog("emptyClusterNum=[%d]", pFat32PartInfo->pFilesysSectorInfo->emptyClusterNum);
	iLog("nextEmptyVCN=[%d]", pFat32PartInfo->pFilesysSectorInfo->nextEmptyVCN);
	iLog("endFlag=[0x%x]", pFat32PartInfo->pFilesysSectorInfo->endFlag);
	/////////////////////////////////////////////////////////////////////////////////////
	
	int i = 0;
	for(i= 0; i < pFat32PartInfo->pFat32BootInfo->NumOfFat; i ++)
	{
		iLog("------------------------ FAT[%d] ------------------------", i);
		pFat32PartInfo->fatTableInfo[i].StartAddrSec = pFat32PartInfo->pFat32BootInfo->ReservedSectors + i * pFat32PartInfo->pFat32BootInfo->BigSecPerFat;
		pFat32PartInfo->fatTableInfo[i].TotalSecNum = pFat32PartInfo->pFat32BootInfo->BigSecPerFat;
		iLog("StartAddrSec=[%llu]", pFat32PartInfo->fatTableInfo[i].StartAddrSec);
		iLog("TotalSecNum=[%llu]", pFat32PartInfo->fatTableInfo[i].TotalSecNum);
	}

	pFat32PartInfo->fatDataZoneInfo.DataStartSec = pFat32PartInfo->pFat32BootInfo->ReservedSectors + pFat32PartInfo->pFat32BootInfo->NumOfFat * pFat32PartInfo->pFat32BootInfo->BigSecPerFat;
	iLog("DataStartSec=[%llu]", pFat32PartInfo->fatDataZoneInfo.DataStartSec);
	pFat32PartInfo->fatDataZoneInfo.DataSecNum = pFat32PartInfo->pFat32BootInfo->BigTotalSec - pFat32PartInfo->fatDataZoneInfo.DataStartSec;	
	iLog("DataSecNum=[%llu]", pFat32PartInfo->fatDataZoneInfo.DataSecNum);
	
    pFat32PartInfo->fatDataZoneInfo.ClusterNum = pFat32PartInfo->fatDataZoneInfo.DataSecNum / pFat32PartInfo->pFat32BootInfo->SecPerClr + 2;   //得到总簇数,包括两个保留簇，比WINHEX看到的簇数多2
	iLog("ClusterNum=[%d]", pFat32PartInfo->fatDataZoneInfo.ClusterNum);
	
	//pFat32PartInfo->BytesPerSector = pFat32PartInfo->pFat32BootInfo->BytesPerSector;           //一般是512
    //pFat32PartInfo->SectorsPerCluster = pFat32PartInfo->pFat32BootInfo->SecPerClr; 			//一般是8
   	//pFat32PartInfo->SectorsPerFAT = pFat32PartInfo->pFat32BootInfo->SecPerFat;          
    //pFat32PartInfo->fat_count = pFat32PartInfo->pFat32BootInfo->NumOfFat;                               //一般是两个紧挨着，第二个应该是备份的，跟第一个一模一样
    //pFat32PartInfo->fat_start = pFat32PartInfo->pFat32BootInfo->ReservedSectors * pFat32PartInfo->BytesPerSector;     //保留扇区之后便是 FAT
    //pFat32PartInfo->RootDirStart = pFat32PartInfo->fat_start + pFat32PartInfo->fat_count * pFat32PartInfo->SectorsPerFAT * pFat32PartInfo->BytesPerSector;       //FAT之后便是根目录
    //pFat32PartInfo->FATDiscriPerSection = pFat32PartInfo->BytesPerSector / sizeof(ULONG);
	
	return pFat32PartInfo;
}

void Fat32_release_part_info(FAT32_PART_INFO ** pFat32PartInfo)
{
	if(*pFat32PartInfo)
	{
		Fat32_CloseDev((*pFat32PartInfo)->devfd);
		Fat32_free((void **) pFat32PartInfo);
	}
}

DWORD Fat32_search_file_in_direction(char * filePath, int deepIndex, FAT32_PART_INFO * pFat32PartInfo, DWORD startVSN, DWORD * fileSize)
{
	if(strlen(filePath) == strlen("/") && \
		strncmp(filePath, "/", strlen("/")) == 0) //根目录
		return startVSN;
	
	char fileName[256];
	int PathDeep = Fat32_get_file_path_deepth(filePath);	
	if(Fat32_get_file_name_from_path_by_deepth(filePath, deepIndex, fileName, sizeof(fileName)) == -1)
		return 0;
	
	iLog("deepIndex=[%d/%d],fileName=[%s]", deepIndex, PathDeep , fileName);
	
	int itemNum = 0;
	int breakFlag = 0;
	BYTE CurItemChackVal = 0;
	BYTE CmpItemChackVal = 0;
	int cpOffset = 0;
	
	FDT_ITEM * pFdtItem = NULL;
	BYTE * SecBuf = (BYTE *) Fat32_malloc(SECTORSIZE);
	if(!SecBuf)
	{
		eLog("malloc error");
		return 0;
	}
	
	BYTE * FdtBuf = (BYTE *) Fat32_malloc(SECTORSIZE);
	if(!FdtBuf)
	{
		eLog("malloc error");
		Fat32_free(DP_VOID(&SecBuf));
		return 0;
	}
	
	int i, j ;
	for(j = 0; j < pFat32PartInfo->pFat32BootInfo->SecPerClr; j ++)
	{
		if(breakFlag)
			break;
		
		Fat32_ReadSector(pFat32PartInfo->devfd, pFat32PartInfo->partStartSec, startVSN + j, 1, SecBuf);
		pFdtItem = (FDT_ITEM * )SecBuf ;
		
		dump_mem_to_file(SecBuf, SECTORSIZE, "DT_SEC", FALSE);
		
		if(CmpItemChackVal == 0) //针对第一项
			CmpItemChackVal = Fat32_get_check_value_from_fdt_item(pFdtItem);
				
		for(i = 0; i < (SECTORSIZE / sizeof(FDT_ITEM)); i ++)
		{
			CurItemChackVal = Fat32_get_check_value_from_fdt_item(pFdtItem);
			
			iLog("[0x%x / 0x%x]", CmpItemChackVal, CurItemChackVal);
			if(CurItemChackVal != CmpItemChackVal) //出现新的fdt
			{
				CmpItemChackVal = CurItemChackVal;
				iLog("======================================");
								
				char wholeName[256];
				Fat32_get_whole_filename(FdtBuf, itemNum, wholeName, sizeof(wholeName));
				if(strlen(wholeName) > 0)
				{
					iLog("wholeName=[%s]", wholeName);
					FDT_ITEM * pTmpFdtItem = (FDT_ITEM *)FdtBuf ;
					Fat32_show_long_fdt_info(&pTmpFdtItem->longFdt);
					
					pTmpFdtItem = (FDT_ITEM *)FdtBuf + itemNum - 1;
					Fat32_show_short_fdt_info(&pTmpFdtItem->shortFdt);
					
					if(strlen(wholeName) == strlen(fileName) && \
						strncmp(wholeName, fileName, strlen(fileName)) == 0) //匹配到文件
					{
						DWORD DataStartVSN = Fat32_ClusterToSector(Fat32_CalcCluster(pTmpFdtItem->shortFdt.high16VCN, pTmpFdtItem->shortFdt.low16VCN), pFat32PartInfo->fatDataZoneInfo.DataStartSec, pFat32PartInfo->pFat32BootInfo->SecPerClr);
						if(deepIndex >= PathDeep)
						{
							*fileSize = pTmpFdtItem->shortFdt.fileSzie;
							iLog("find file[%s], fileSzie=[%d], DataStartVSN=[%d]", fileName, *fileSize, DataStartVSN);
							Fat32_free(DP_VOID(&FdtBuf));
							Fat32_free(DP_VOID(&SecBuf));
							return DataStartVSN;
						}
						else
						{
							if (!FAT_ENTRY_DELETED(pTmpFdtItem->longFdt.dirId) \
			            		&& !FAT_ENTRY_END(pTmpFdtItem->longFdt.dirId) \
			            		&& !FAT_ENTRY_LONG(pTmpFdtItem->longFdt.attribute)\
			            		&& FAT_ENTRY_DIR(pTmpFdtItem->longFdt.attribute))
		            		{
			            		if(strlen(wholeName) == strlen(".") && \
									strncmp(wholeName, ".", strlen(wholeName)) == 0) //Current direction
									;
								else if(strlen(wholeName) == strlen("..") && \
									strncmp(wholeName, "..", strlen(wholeName)) == 0) //Parent direction
									;
								else
								{
									dump_mem_to_file(FdtBuf, itemNum * sizeof(FDT_ITEM) , "DT_DATA", FALSE);
									
									Fat32_free(DP_VOID(&FdtBuf));
									Fat32_free(DP_VOID(&SecBuf));
									iLog("find dir[%s], DataStartVSN=[%d]", fileName, DataStartVSN);
									
									return Fat32_search_file_in_direction(filePath, ++deepIndex, pFat32PartInfo, DataStartVSN, fileSize);
								}

		            		}
						
						}
							
					}
				}			
				itemNum = 0;
			}

			cpOffset = itemNum * sizeof(FDT_ITEM);
			if(FAT_ENTRY_END(pFdtItem->longFdt.dirId) ||\
				cpOffset + sizeof(FDT_ITEM) >= SECTORSIZE) //CurItemChackVal == 0 
			{
				iLog("dirId=[%x]", pFdtItem->longFdt.dirId);
				iLog("CurItemChackVal=[0x%x]", CurItemChackVal);
				iLog("cpOffset=[%d / %d]", cpOffset, SECTORSIZE);
				iLog("break");
				breakFlag = 1;
				break;
			}
			
			memcpy(FdtBuf + cpOffset, pFdtItem, sizeof(FDT_ITEM));
			itemNum ++;
			pFdtItem ++;
		}
	}
	
	Fat32_free(DP_VOID(&FdtBuf));
	Fat32_free(DP_VOID(&SecBuf));
	return 0;
}

static DWORD Fat32_sacn_direction(char * filePath, FAT32_PART_INFO * pFat32PartInfo, DWORD startVSN, int CurDeepth, int limitDeepth)
{
	if(CurDeepth ++ >= limitDeepth) //当前深度达到限制深度时
		return 0;
	
	int itemNum = 0;
	int breakFlag = 0;
	BYTE CurItemChackVal = 0;
	BYTE CmpItemChackVal = 0;
	int cpOffset = 0;
	
	FDT_ITEM * pFdtItem = NULL;
	BYTE * SecBuf = (BYTE *) Fat32_malloc(SECTORSIZE);
	if(!SecBuf)
	{
		eLog("malloc error");
		return 0;
	}
	
	BYTE * FdtBuf = (BYTE *) Fat32_malloc(SECTORSIZE);
	if(!FdtBuf)
	{
		eLog("malloc error");
		Fat32_free(DP_VOID(&SecBuf));
		return 0;
	}
	
	int i, j ;
	for(j = 0; j < pFat32PartInfo->pFat32BootInfo->SecPerClr; j ++)
	{
		if(breakFlag)
			break;
		
		Fat32_ReadSector(pFat32PartInfo->devfd, pFat32PartInfo->partStartSec, startVSN + j, 1, SecBuf);
		pFdtItem = (FDT_ITEM * )SecBuf ;
		
		if(CmpItemChackVal == 0) //针对第一项
			CmpItemChackVal = Fat32_get_check_value_from_fdt_item(pFdtItem);
			
		for(i = 0; i < (SECTORSIZE / sizeof(FDT_ITEM)); i ++)
		{
			CurItemChackVal = Fat32_get_check_value_from_fdt_item(pFdtItem);
			if(CurItemChackVal != CmpItemChackVal) //出现新的fdt
			{
				CmpItemChackVal = CurItemChackVal;
				
				char wholeName[256];
				Fat32_get_whole_filename(FdtBuf, itemNum, wholeName, sizeof(wholeName));
				if(strlen(wholeName) > 0)
				{
					if(strlen(wholeName) == strlen(".") && \
						strncmp(wholeName, ".", strlen(wholeName)) == 0) //Current direction
						;
					else if(strlen(wholeName) == strlen("..") && \
						strncmp(wholeName, "..", strlen(wholeName)) == 0) //Parent direction
						;
					else
					{	
						char fileName[256];
						if(filePath[strlen(filePath) - 1] == '/')
							sprintf(fileName, "%s%s", filePath, wholeName);
						else
							sprintf(fileName, "%s/%s", filePath, wholeName);
						iLog("fileName=[%s]",fileName);

						FDT_ITEM * pTmpFdtItem = (FDT_ITEM *)FdtBuf ;
						//Fat32_show_long_fdt_info(&pTmpFdtItem->longFdt);
						
						pTmpFdtItem = (FDT_ITEM *)FdtBuf + itemNum - 1;
						//Fat32_show_short_fdt_info(&pTmpFdtItem->shortFdt);
					
						if (!FAT_ENTRY_DELETED(pTmpFdtItem->longFdt.dirId) \
		            		&& !FAT_ENTRY_END(pTmpFdtItem->longFdt.dirId) \
		            		&& !FAT_ENTRY_LONG(pTmpFdtItem->longFdt.attribute)\
		            		&& FAT_ENTRY_DIR(pTmpFdtItem->longFdt.attribute)) //正常的目录
		        		{
							DWORD DataStartVSN = Fat32_ClusterToSector(Fat32_CalcCluster(pTmpFdtItem->shortFdt.high16VCN, pTmpFdtItem->shortFdt.low16VCN), pFat32PartInfo->fatDataZoneInfo.DataStartSec, pFat32PartInfo->pFat32BootInfo->SecPerClr);
							Fat32_sacn_direction(fileName, pFat32PartInfo, DataStartVSN, CurDeepth, limitDeepth);	
		        		}
					}
				}
						
				itemNum = 0;
			}

			cpOffset = itemNum * sizeof(FDT_ITEM);
			if(FAT_ENTRY_END(pFdtItem->longFdt.dirId) ||\
				cpOffset + sizeof(FDT_ITEM) >= SECTORSIZE) //CurItemChackVal == 0 ||
			{
				breakFlag = 1;
				break;
			}
			
			memcpy(FdtBuf + cpOffset, pFdtItem, sizeof(FDT_ITEM));
			itemNum ++;
			pFdtItem ++;
		}
	}
	
	Fat32_free(DP_VOID(&FdtBuf));
	Fat32_free(DP_VOID(&SecBuf));
	return 0;
}

int Fat32_scan_file_in_direction(char * filePath, FAT32_PART_INFO * pFat32PartInfo, int limitDeepth)
{
	DWORD filesize = 0;
	return Fat32_sacn_direction(filePath, pFat32PartInfo, Fat32_search_file_in_direction(filePath, 1, pFat32PartInfo, pFat32PartInfo->fatDataZoneInfo.DataStartSec, &filesize), 0, limitDeepth);
}

int Fat32_read_file_by_data_info(char * newFile, FAT32_FILE_DATA_INFO * pFat32FileDataInfo)
{
	if(newFile == NULL || pFat32FileDataInfo == NULL)
		return -1;

	if(pFat32FileDataInfo->fileRealSize <= 0)
		return -1;
	
	FILE * fp = fopen(newFile, "wb");
	if(fp == NULL)
	{
		eLog("fopen file[%s] failed: %s", newFile, ErrExp());
		return -1;
	}
	
	DWORD readTotalSec = 0;
	int leftDataSize = pFat32FileDataInfo->fileRealSize;
	BYTE * dataBuf = (BYTE *) Fat32_malloc(pFat32FileDataInfo->ClusterSize);
	if(dataBuf == NULL)
	{
		eLog("malloc failed: %s", ErrExp());
		fclose(fp);
		return -1;
	}
	
	while(leftDataSize > 0)
	{
		//iLog("leftDataSize=[%d | %d]", leftDataSize, blockSize);
		int wrtieDataSize = 0;
		if(leftDataSize > pFat32FileDataInfo->ClusterSize)
		{
			Fat32_read_data_block(pFat32FileDataInfo->devfd, pFat32FileDataInfo->partStartSec, pFat32FileDataInfo->fileDataVSN + readTotalSec, pFat32FileDataInfo->SecPerClr, dataBuf);
			readTotalSec += pFat32FileDataInfo->SecPerClr;
			wrtieDataSize = pFat32FileDataInfo->ClusterSize;
		}
		else
		{
			int readSecNum = (leftDataSize + SECTORSIZE - 1) / SECTORSIZE;
			Fat32_read_data_block(pFat32FileDataInfo->devfd, pFat32FileDataInfo->partStartSec, pFat32FileDataInfo->fileDataVSN + readTotalSec, readSecNum, dataBuf);
			readTotalSec += readSecNum;
			wrtieDataSize = leftDataSize;
		}
		
		if(fwrite(dataBuf, wrtieDataSize, 1, fp) != 1)
		{
			eLog("fwrite failed: %s", ErrExp());
			Fat32_free(DP_VOID(&dataBuf));
			fclose(fp);
			return -1;
		}
		
		leftDataSize -= wrtieDataSize;
	}
	
	Fat32_free(DP_VOID(&dataBuf));
	fclose(fp);
	return 0;
}


FAT32_FILE_DATA_INFO * Fat32_get_file_data_info(char * filePath, FAT32_PART_INFO * pFat32PartInfo)
{
	FAT32_FILE_DATA_INFO * pFat32FileDataInfo = (FAT32_FILE_DATA_INFO *) Fat32_malloc(sizeof(FAT32_FILE_DATA_INFO));
	if(!pFat32FileDataInfo)
		return NULL;

	pFat32FileDataInfo->devfd = pFat32PartInfo->devfd;
	pFat32FileDataInfo->partStartSec = pFat32PartInfo->partStartSec;
	pFat32FileDataInfo->BytesPerSec = pFat32PartInfo->pFat32BootInfo->BytesPerSector;
	pFat32FileDataInfo->SecPerClr = pFat32PartInfo->pFat32BootInfo->SecPerClr;
	pFat32FileDataInfo->DataZoneAddr = pFat32PartInfo->fatDataZoneInfo.DataStartSec;
	
	pFat32FileDataInfo->fileDataVSN = Fat32_search_file_in_direction(filePath, 1, pFat32PartInfo, pFat32PartInfo->fatDataZoneInfo.DataStartSec, &pFat32FileDataInfo->fileRealSize);
	if(pFat32FileDataInfo->fileRealSize <= 0 || pFat32FileDataInfo->fileDataVSN < pFat32PartInfo->fatDataZoneInfo.DataStartSec)
	{
		Fat32_free(DP_VOID(&pFat32FileDataInfo));
		return NULL;
	}
	
	pFat32FileDataInfo->ClusterSize = pFat32FileDataInfo->BytesPerSec * pFat32FileDataInfo->SecPerClr;
	pFat32FileDataInfo->fileAllocClrNum = (pFat32FileDataInfo->fileRealSize +  pFat32FileDataInfo->ClusterSize - 1) / pFat32FileDataInfo->ClusterSize;
	iLog("ClusterSize=[%d], fileAllocClrNum=[%d]", pFat32FileDataInfo->ClusterSize, pFat32FileDataInfo->fileAllocClrNum);
	
	return pFat32FileDataInfo;
}

void Fat32_release_file_data_info(FAT32_FILE_DATA_INFO ** pFat32FileDataInfo){Fat32_free((void **) pFat32FileDataInfo);}

int Fat32_read_file_from_part(char * devName, UINT64 partStartSec, UINT64 partTotalSec, char * filePath, char * newFile)
{
	FAT32_PART_INFO * pFat32PartInfo = Fat32_get_part_info(devName, partStartSec, partTotalSec);
	if(!pFat32PartInfo)
		return -1;

	Fat32_scan_file_in_direction(filePath, pFat32PartInfo, 10);
	
	FAT32_FILE_DATA_INFO * pFat32FileDataInfo = Fat32_get_file_data_info(filePath, pFat32PartInfo);
	if(!pFat32FileDataInfo)
	{
		Fat32_release_part_info(&pFat32PartInfo);
		return -1;
	}
	
	/////////////////////////////////////////////////////
	Fat32_read_file_by_data_info(newFile, pFat32FileDataInfo);
	/////////////////////////////////////////////////////

	Fat32_release_file_data_info(&pFat32FileDataInfo);
	Fat32_release_part_info(&pFat32PartInfo);
	return 0;
	
	//Fat32_scan_file_in_direction(filePath, pFat32PartInfo, 5);
	//Fat32_release_part_info(&pFat32PartInfo);
	//return 0;
}

