/****************************************************************************************************************************
*
*   文 件 名 ： fsNtfs.c 
*   文件描述 ：  
*   创建日期 ：2019年8月28日
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
#include <ctype.h>
#include <fcntl.h>

#include "fsNtfs.h"

/*
 * Limitations:
 *  1. Don't support >1K MFT record size, >4K INDEX record size
 *  2. Don't support encrypted file
 *  3. Don't support >4K non-resident attribute list and $BITMAP
 *
 */

///////////////////////////////////////////////////////////////////////////////////////////

static int Ntfs_search_file_in_index_allocation(char *, NTFS_PART_INFO *, BYTE *, int, DWORD, DWORD *, DDWORD *, DDWORD *);
static int Ntfs_scan_direction_in_index_allcation(char *, NTFS_PART_INFO *, BYTE *, int, DWORD , int, int);
static int Ntfs_scan_direction(char *, NTFS_PART_INFO *, DWORD, DWORD, int, int);


static void * Ntfs_malloc(size_t size){void * p=malloc(size); if(p) memset(p, 0x00, size); return p;}
static void Ntfs_free(void ** p){if(*p) { free(*p); *p = NULL;}}

static int Ntfs_OpenDev(const char * devname, int rwMode){return open(devname, rwMode);}
static void Ntfs_CloseDev(int diskfd){close(diskfd);}

static int Ntfs_ReadSector(int diskfd, DDWORD partStartSec, DDWORD offsetLogicSec, int secNum, BYTE * buff)
{
	DDWORD offset = (partStartSec + offsetLogicSec) * SECTORSIZE;
	DDWORD dataSize = secNum * SECTORSIZE;
	if(buff == NULL && dataSize == 0)
		return -1;
	
	//iLog("Ntfs_ReadSector:[%llu + %d]", (partStartSec + offsetLogicSec), secNum);
	
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

static int Ntfs_WriteSector(int diskfd, DDWORD partStartSec, DDWORD offsetLogicSec, int secNum, BYTE * buff)
{
	if(buff == NULL && secNum == 0)
		return -1;

	DDWORD dataSize = secNum * SECTORSIZE;
	DDWORD offset = (partStartSec + offsetLogicSec) * SECTORSIZE;
	//iLog("Ntfs_WriteSector:[%llu + %d]", (partStartSec + offsetLogicSec), secNum);
	
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

static BYTE * Ntfs_DumpSector(int diskfd, DDWORD partStartSec, DDWORD offsetLogicSec, int secNum)
{
	DDWORD dataSize = secNum * SECTORSIZE;
	if(dataSize == 0)
		return NULL;
	
	BYTE * buff = (BYTE *) Ntfs_malloc(dataSize);
	if(buff)
	{
		if(Ntfs_ReadSector(diskfd, partStartSec, offsetLogicSec, secNum, buff) == -1)
		{
			Ntfs_free(DP_VOID(&buff));
			return NULL;
		}
	}
	
	return buff;
}

static void Ntfs_ReleaseSector(BYTE ** buff){Ntfs_free(DP_VOID( buff));}

int Ntfs_read_data_block(int diskfd, DDWORD partStartSec, DDWORD StartSec, int secNum, BYTE * buff){return Ntfs_ReadSector(diskfd, partStartSec, StartSec, secNum, buff);}
int Ntfs_write_data_block(int diskfd, DDWORD partStartSec, DDWORD StartSec, int secNum, BYTE * buff){return Ntfs_WriteSector(diskfd, partStartSec, StartSec, secNum, buff);}

static char * Ntfs_StringCleanAssignEndof(char * src , int len, char asg)
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

static char * Ntfs_ByteToString(char * dst, int dstLen, BYTE * src, int assignLen)
{
	int cpLen = MIN(dstLen - 1, assignLen);
	memset(dst, '\0', dstLen); 
	memcpy(dst, src, cpLen);
	
	return Ntfs_StringCleanAssignEndof(dst, cpLen, ' ');
}

static char * Ntfs_unicode_to_utf8 (WORD * unicode, int wlen, char * utf8, int osize){memset(utf8, '\0', osize); sconv_unicode_to_utf8((const wchar *)unicode, wlen, utf8, osize); return utf8;}
static WORD * Ntfs_utf8_to_unicode (char * utf8, int slen, WORD * unicode, int osize){memset(unicode, '\0', osize); sconv_utf8_to_unicode((const char *)utf8, slen, unicode, osize); return unicode;}

static char * Ntfs_WcharToUniString(WORD *wcSrcStr, int wlen, char * DstStr, int osize)
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

char * Ntfs_generate_assign_string(WORD * wStr, int assignLen, char * outStr, int outSize)
{
	memset(outStr, '\0', outSize);
	Ntfs_WcharToUniString(wStr, assignLen, outStr, outSize);
	outStr[outSize] = 0;
	return outStr;
}

int Ntfs_StringFindFirstOf(char * baseStr, char assign) {int i, pos = -1; for(i = 0;i < strlen(baseStr); i++) if(*(baseStr+i) == assign){pos = i;break;}return pos;}

int Ntfs_get_file_name_from_path_by_deepth(char * filePath, int deepth, char * fileName, int bufLen)
{
	char * pFpath = filePath;
	int fileIndex = 0;
	int pathLen = strlen(pFpath);
	int offset = 0;

	if(deepth <= 0)
		return -1;
	
	while(pFpath)
	{
		int posLen = Ntfs_StringFindFirstOf(pFpath, '/');
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

int Ntfs_get_file_path_deepth(char * filePath)
{
	char * pFpath = filePath;
	int deepth = 0;
	int pathLen = strlen(pFpath);
	int offset = 0;
	
	while(pFpath)
	{
		int posLen = Ntfs_StringFindFirstOf(pFpath, '/');
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


const char * Ntfs_metafile_to_strimg(UINT8 fileId)
{
	static const char * metaFileString[] = {"$MFT", "$MFTMirr", "$LogFile", "$Volume", "$AttrDef", \
									".", "$Bitmap", "$Boot", "$BadClus", "$Secure", "$UpCase", \
									"$Extended metadata directory", "$Extend\\$Reparse", "$Extend\\UsnJrnl", \
									"$Extend\\Quota", "$Extend\\ObjId"};
	return metaFileString[fileId];
}

static int Ntfs_get_member_form_runs(BYTE * pRuns, LONGLONG * llStAddr, ULONGLONG * ulSize)
{
    BYTE ucbSize = LOWBYTE ( pRuns[0] );
    BYTE ucbAddr = HIGBYTE( pRuns[0] );
	//iLog("pRuns=[%d, %d]", ucbSize, ucbAddr);
	
    pRuns += 1;
    switch (ucbSize)
    {
    case 1:
        *ulSize = (ULONGLONG ) *((UINT8 *) pRuns);
        break;
    case 2:
        *ulSize = (ULONGLONG ) *((UINT16 *) pRuns);
        break;
    case 3:
        *ulSize = (ULONGLONG ) *((UINT32 *) pRuns);
        *ulSize &= 0xFFFFFF;
        break;
    case 4:
        *ulSize = (ULONGLONG ) *((UINT32 *) pRuns);
        break;
    case 5:
        *ulSize = (ULONGLONG ) *((UINT64 *) pRuns);
        *ulSize &= 0xFFFFFFFFFF;
        break;
    case 6:
        *ulSize = (ULONGLONG ) *((UINT64 *) pRuns);
        *ulSize &= 0xFFFFFFFFFFFF;
        break;
    case 7:
        *ulSize = (ULONGLONG ) *((UINT64 *) pRuns);
        *ulSize &= 0xFFFFFFFFFFFFFF;
        break;
    case 8:
        *ulSize = (ULONGLONG ) *((UINT64 *) pRuns);
        break;
    default:
        *ulSize = 0;
		break;
        
    }

	pRuns += ucbSize;
    switch (ucbAddr)
    {
    case 1:
        *llStAddr = (LONGLONG ) *((UINT8 *) pRuns);
        break;
    case 2:
        *llStAddr = (LONGLONG ) *(( UINT16 *) pRuns);
        break;
    case 3:
        {
            long lAddr = *((UINT32 *) pRuns);
            if(0 != (lAddr & 0x800000))
                *llStAddr = lAddr |= 0xFF000000;
            else
                *llStAddr = lAddr &= 0xFFFFFF;
        }
        break;
    case 4:
        *llStAddr = (LONGLONG ) *((UINT32 *) pRuns);
        break;
    case 5:
        {
            __int64 lAddr = *((UINT64 *) pRuns);
            if(0 != (lAddr & 0x8000000000LL))
                *llStAddr = lAddr |= 0xFFFFFF0000000000LL;
            else
                *llStAddr = lAddr &= 0xFFFFFFFFFFLL;
        }
    case 6:
        {
            __int64 lAddr = *((UINT64 *) pRuns);
            if(0 != (lAddr & 0x800000000000LL))
                *llStAddr = lAddr |= 0xFFFF000000000000LL;
            else
                *llStAddr = lAddr &= 0xFFFFFFFFFFFFLL;
        }
    case 7:
        {
            __int64 lAddr = *((UINT64 *) pRuns);
            if(0 != ( lAddr & 0x80000000000000LL))
                *llStAddr = lAddr |= 0xFF00000000000000LL;
            else
                *llStAddr = lAddr &= 0xFFFFFFFFFFFFFFLL;
        }
    case 8:
        *llStAddr = (LONGLONG ) *(( __int64*) pRuns);
        break;
    default:
        *llStAddr = 0;
        break;
    }
	
	return 0;
}

//根据Fixup字节检查记录是否合法
//修正最后两个字节
int Ntfs_fixup_mft_info(BYTE * pMft, ULONG lcbMft)
{
    int i = 0;
    MFT_HEADER * pMftHead = (MFT_HEADER *)pMft;
	
    //判断 MFT记录的分配空间是否合法；
    if(lcbMft < (ULONG)((pMftHead->UsnCount - 1) * SECTORSIZE))
        return -1;
	
    WORD * pwFix = (WORD)(pMft + pMftHead->UsnOffset);
    for(i = 1; i < pMftHead->UsnCount; i++ )
    {
        WORD * ptrFix = (WORD *)(pMft + (i * SECTORSIZE) -sizeof(WORD));
        if(pwFix[0] != ptrFix[0])
            return -1;
		
        ptrFix[0] = pwFix[i];//修正每个扇区最后两个字节
    }
	
    return 0;
}

//得到所有run的占用的字节数
static UINT64 Ntfs_get_data_runs_total_size(BYTE SecPerClr, WORD BytePerSec, BYTE * pRuns)
{
    int iseek = 0;
    LONGLONG llstAddr = 0; //non use
    ULONGLONG ullSize = 0 ;
	UINT64 ullAllSize = 0;
	//iLog("SecPerClr=[%d],BytePerSec=[%d]", SecPerClr, BytePerSec);
	
    while(0 != *pRuns)
    {
        iseek = HIGBYTE(*pRuns) + LOWBYTE(*pRuns) + 1;
        Ntfs_get_member_form_runs(pRuns, &llstAddr, &ullSize);
		ullSize *= SecPerClr;
		
        ullAllSize += ullSize;
        pRuns += iseek;
    }
	
    return ullAllSize * BytePerSec;
}

static MftAttributeItem * Ntfs_get_attritem_by_attrtype(MFT_FILE_INFO * pMftFileInfo, UINT32 AttrType)
{
	MftAttributeItem * pMftAttrItem = NULL;
	
	int i = 0;
	for(i = 0; i < pMftFileInfo->MftAttrNum; i ++)
	{
		if(pMftFileInfo->MftAttrItem[i].pAttrUnion->AttrHeader.ATTR_Type == AttrType)
		{
			pMftAttrItem = &pMftFileInfo->MftAttrItem[i];
			break;
		}
	}
	
	return pMftAttrItem;
}

static BYTE * Ntfs_get_file_runs(MFT_FILE_INFO * pMftFileInfo, UINT32 AttrType, int * RunsLen, int * dataType)
{	
	MftAttributeItem * pMftAttrItem = Ntfs_get_attritem_by_attrtype(pMftFileInfo, AttrType);
	if(pMftAttrItem ==NULL)
		return NULL;
	
	*dataType = pMftAttrItem->pAttrUnion->AttrHeader.ATTR_ResFlag; //
	if(*dataType == 1)
	    *RunsLen = pMftAttrItem->pAttrUnion->AttrHeader.ATTR_Size -pMftAttrItem->pAttrUnion->AttrBody.NonRsdAttrBody.ATTR_DataOff;
	else
		*RunsLen = pMftAttrItem->pAttrUnion->AttrHeader.ATTR_Size -pMftAttrItem->pAttrUnion->AttrBody.RsdAttrBody.ATTR_DataOffset;
	
	return pMftAttrItem->pAttrData;
}

BYTE * Ntfs_get_mft_file_data_runs(MFT_FILE_INFO * pMftFileInfo, int * pRunsLen, int * dataType)
{
	if(pMftFileInfo->pMftHeader->Resident != RESIDENT_NORMAL_FILE) //
        return NULL;
	
	return Ntfs_get_file_runs(pMftFileInfo, ATTR_DATA, pRunsLen, dataType);
}

BYTE * Ntfs_get_direction_index_runs(MFT_FILE_INFO * pMftFileInfo, int * pRunsLen, int * dataType)
{
    if(pMftFileInfo->pMftHeader->Resident != RESIDENT_NORMAL_DIR) //
        return NULL;
	
	return Ntfs_get_file_runs(pMftFileInfo, ATTR_INDEX_ALLOCATION, pRunsLen, dataType);
}

BYTE * Ntfs_get_mft_file_bitmap_runs(MFT_FILE_INFO * pMftFileInfo, int * pRunsLen, int * dataType)
{
	return Ntfs_get_file_runs(pMftFileInfo, ATTR_BITMAP, pRunsLen, dataType);
}


static int Ntfs_CheckOemId(BYTE * OemID)
{
	if (( 'N' != OemID[0]) || ( 'T' != OemID[1]) || ( 'F' != OemID[2]) ||( 'S' != OemID[3]))
	{
		iLog("OemID=[%c%c%c%c], Not NTFS!", OemID[0], OemID[1], OemID[2], OemID[3]);
    	return FALSE;
    }
	
	return TRUE;
}

static int Ntfs_CheckIndexId(UINT8 * IndexFlag)
{
	if ( 'I' != IndexFlag[0] ||'N' != IndexFlag[1] ||'D' != IndexFlag[2] || 'X' != IndexFlag[3] )
    {
    	iLog("IndexFlag=[%c%c%c%c], not INDX!", IndexFlag[0], IndexFlag[1], IndexFlag[2], IndexFlag[3]);
        return -1;
    }
	
	return 0;
}

static int Ntfs_CheckMftHaedId(BYTE * MftHeadID)
{
	if (( 'F' != MftHeadID[0]) || ( 'I' != MftHeadID[1]) || ( 'L' != MftHeadID[2]) ||( 'E' != MftHeadID[3]))
	{
		iLog("MftHeadID=[%c%c%c%c], Not FILE!", MftHeadID[0], MftHeadID[1], MftHeadID[2], MftHeadID[3]);
    	return FALSE;
    }
	
	return TRUE;
}


NTFS_BOOT_INFO * Ntfs_get_boot_info(int devfd, DDWORD partStartSec)
{
	NTFS_BOOT_INFO * pNtfsBootInfo = (NTFS_BOOT_INFO *) Ntfs_malloc(sizeof(NTFS_BOOT_INFO));
	if(pNtfsBootInfo)
	{
		BYTE * secBuf = Ntfs_DumpSector(devfd, partStartSec, 0, 1);
		if(secBuf)
		{
			memcpy(pNtfsBootInfo, secBuf, sizeof(NTFS_BOOT_INFO));
			Ntfs_ReleaseSector(&secBuf);
		}
	}
	
	return pNtfsBootInfo;
}

void Ntfs_release_boot_info(NTFS_BOOT_INFO ** pNtfsBootInfo){Ntfs_free(DP_VOID(pNtfsBootInfo));}

#define NTFS_TIME_CALC_START "1601-01-01 08:00:00"

static int Ntfs_analyse_mft_attribute_item(MftAttributeItem * pMftAttrItem)
{
	if(pMftAttrItem == NULL)
	{
		eLog("Para is NULL: pMftAttrItem=[%x]", pMftAttrItem);
		return -1;
	}
	
	//char fileName[64];
	
	switch(pMftAttrItem->pAttrUnion->AttrHeader.ATTR_Type)
	{
		case ATTR_STANDARD_INFORMATION:
			pMftAttrItem->pStdInfoAttr = (STANDARD_INFORMATION_ATTR *)pMftAttrItem->pAttrData;
			
			/*
			struct tm timeStart = GetTimeInfoFromExpress(NTFS_TIME_CALC_START);
			struct tm NewTime = CalcTimeFromPassPoint(timeStart, (pMftAttrItem->pStdInfoAttr->CreateTime.DateTime / 10000000));
			iLog("CreateTime=[%04d-%02d-%02d %02d:%02d:%02d]", NewTime.tm_year, NewTime.tm_mon, NewTime.tm_mday, NewTime.tm_hour, NewTime.tm_min, NewTime.tm_sec);

			NewTime = CalcTimeFromPassPoint(timeStart, (pMftAttrItem->pStdInfoAttr->ModifyTime.DateTime / 10000000));
			iLog("ModifyTime=[%04d-%02d-%02d %02d:%02d:%02d]", NewTime.tm_year, NewTime.tm_mon, NewTime.tm_mday, NewTime.tm_hour, NewTime.tm_min, NewTime.tm_sec);

			NewTime = CalcTimeFromPassPoint(timeStart, (pMftAttrItem->pStdInfoAttr->MftChangeTime.DateTime / 10000000));
			iLog("MftChangeTime=[%04d-%02d-%02d %02d:%02d:%02d]", NewTime.tm_year, NewTime.tm_mon, NewTime.tm_mday, NewTime.tm_hour, NewTime.tm_min, NewTime.tm_sec);

			NewTime = CalcTimeFromPassPoint(timeStart, (pMftAttrItem->pStdInfoAttr->LatVisitedTime.DateTime / 10000000));
			iLog("LatVisitedTime=[%04d-%02d-%02d %02d:%02d:%02d]", NewTime.tm_year, NewTime.tm_mon, NewTime.tm_mday, NewTime.tm_hour, NewTime.tm_min, NewTime.tm_sec);
			*/
			
			break;
		case ATTR_ATTRIBUTE_LIST:
			pMftAttrItem->pAttrListAttr = (ATTRIBUTE_LIST_ATTR *)pMftAttrItem->pAttrData;
			break;
		case ATTR_FILENAME:
			pMftAttrItem->pFileNameAttr = (FILENAME_ATTR *)pMftAttrItem->pAttrData;

			//StrUnicodeToUtf8(P_VOID(pMftAttrItem->pFileNameAttr->FileName), pMftAttrItem->pFileNameAttr->NameSz, fileName, sizeof(fileName));
			//Ntfs_unicode_to_utf8(P_WORD(pMftAttrItem->pFileNameAttr->FileName), pMftAttrItem->pFileNameAttr->NameSz, fileName, sizeof(fileName));
			//iLog("FileName=[%s]", fileName);
			//iLog("DOSAttr=[0x%x]", pMftAttrItem->pFileNameAttr->DOSAttr);
			break;
		case ATTR_OBJECT_ID:
			pMftAttrItem->pObjIdAttr = (OBJECT_ID_ATTR *)pMftAttrItem->pAttrData;
			break;
		case ATTR_SECURITY_DESCRIPTOR:
			break;
		case ATTR_VOLUME_NAME:
			pMftAttrItem->pVolNameAttr = (VOLUME_NAME_ATTR *)pMftAttrItem->pAttrData;
			break;
		case ATTR_VOLUME_INFORMATION:
			pMftAttrItem->pVolInforAttr = (VOLUME_INFORMATION_ATTR *)pMftAttrItem->pAttrData;
			break;
		case ATTR_DATA:
			pMftAttrItem->pDataAttr = (DATA_ATTR *)pMftAttrItem->pAttrData;
			break;
		case ATTR_INDEX_ROOT:
			pMftAttrItem->pIndexRootAttr = (INDEX_ROOT_ATTR *)pMftAttrItem->pAttrData;
			break;
		case ATTR_INDEX_ALLOCATION:
			
			break;
		case ATTR_BITMAP:
			pMftAttrItem->pBitmapAttr = (INDEX_ROOT_ATTR *)pMftAttrItem->pAttrData;
			break;
		case ATTR_REPARSE_POINT:
			break;
		case ATTR_EA_INFORMATION:
			break;
		case ATTR_EA:
			break;
		case ATTR_LOGGED_UTILITY_STREAM:
			break;
		default:
			break;
	}

	return 0;
}

static int Ntfs_get_mft_attribute_item(UINT8 * pBufStart, MftAttributeItem * pMftAttrItem)
{
	if(pBufStart == NULL || pMftAttrItem == NULL)
	{
		eLog("Para is NULL: pBufStart=[%x], pMftAttrItem=[%x]", pBufStart, pMftAttrItem);
		return -1;
	}

	pMftAttrItem->pAttrUnion = (AttributeUnion *)pBufStart;
	
	//iLog("ATTR_Type=[0x%x]", pMftAttrItem->pAttrUnion->AttrHeader.ATTR_Type);
	//iLog("ATTR_Size=[%d]", pMftAttrItem->pAttrUnion->AttrHeader.ATTR_Size);
	//iLog("ATTR_ResFlag=[%d]", pMftAttrItem->pAttrUnion->AttrHeader.ATTR_ResFlag);
	//iLog("ATTR_NameSize=[%d]", pMftAttrItem->pAttrUnion->AttrHeader.ATTR_NameSize);
	//iLog("ATTR_NameOffset=[%d]", pMftAttrItem->pAttrUnion->AttrHeader.ATTR_NameOffset);
	//iLog("ATTR_Flags=[0x%x]", pMftAttrItem->pAttrUnion->AttrHeader.ATTR_Flags);
	//iLog("ATTR_Id=[0x%x]", pMftAttrItem->pAttrUnion->AttrHeader.ATTR_Id);
	
	if(pMftAttrItem->pAttrUnion->AttrHeader.ATTR_ResFlag == 0) //常驻属性
	{
		
		pMftAttrItem->pAttrData = pBufStart + pMftAttrItem->pAttrUnion->AttrBody.RsdAttrBody.ATTR_DataOffset;
		
		//iLog("ATTR_DataSize=[%d]", pMftAttrItem->pAttrUnion->AttrBody.RsdAttrBody.ATTR_DataSize);
		//iLog("ATTR_DataOffset=[%d]", pMftAttrItem->pAttrUnion->AttrBody.RsdAttrBody.ATTR_DataOffset);
		//iLog("ATTR_Index=[%d]", pMftAttrItem->pAttrUnion->AttrBody.RsdAttrBody.ATTR_Index);
		//iLog("ATTR_Resvd=[%d]", pMftAttrItem->pAttrUnion->AttrBody.RsdAttrBody.ATTR_Resvd);

		Ntfs_analyse_mft_attribute_item( pMftAttrItem);
		
		return 0;
	}
	else if(pMftAttrItem->pAttrUnion->AttrHeader.ATTR_ResFlag == 1)//非常驻属性
	{
		pMftAttrItem->pAttrData = pBufStart + pMftAttrItem->pAttrUnion->AttrBody.NonRsdAttrBody.ATTR_DataOff;

		//iLog("ATTR_StartVCN=[%llu]", pMftAttrItem->pAttrUnion->AttrBody.NonRsdAttrBody.ATTR_StartVCN);
		//iLog("ATTR_EndVCN=[%llu]", pMftAttrItem->pAttrUnion->AttrBody.NonRsdAttrBody.ATTR_EndVCN);
		//iLog("ATTR_DataOff=[%d]", pMftAttrItem->pAttrUnion->AttrBody.NonRsdAttrBody.ATTR_DataOff);
		//iLog("ATTR_CmpressZ=[%x]", pMftAttrItem->pAttrUnion->AttrBody.NonRsdAttrBody.ATTR_CmpressZ);
		//iLog("ATTR_Resvd=[%x]", pMftAttrItem->pAttrUnion->AttrBody.NonRsdAttrBody.ATTR_Resvd);
		//iLog("ATTR_AllocSize=[%llu]", pMftAttrItem->pAttrUnion->AttrBody.NonRsdAttrBody.ATTR_AllocSize);
		//iLog("ATTR_ValidSize=[%llu]", pMftAttrItem->pAttrUnion->AttrBody.NonRsdAttrBody.ATTR_ValidSize);
		//iLog("ATTR_InitedSize=[%llu]", pMftAttrItem->pAttrUnion->AttrBody.NonRsdAttrBody.ATTR_InitedSize);

		Ntfs_analyse_mft_attribute_item(pMftAttrItem);
		
		return 0;
	}
	else
	{
		eLog("Unknown ATTR_ResFlag=[0x%x]", pMftAttrItem->pAttrUnion->AttrHeader.ATTR_ResFlag);
		return -1;
	}
}

int Ntfs_analyse_mft_file_dataruns(MFT_FILE_INFO * pMftFileInfo, UINT8 SecPerClr)
{
	pMftFileInfo->mftDataRuns.pDataRuns = Ntfs_get_mft_file_data_runs(pMftFileInfo, &pMftFileInfo->mftDataRuns.pDataRunLen, &pMftFileInfo->mftDataRuns.pDataRunType);
	if(!pMftFileInfo->mftDataRuns.pDataRuns)
		return -1;
	
	//iLog("pDataRunLen=[%d], pDataRunType=[%d]", pMftFileInfo->mftDataRuns.pDataRunLen, pMftFileInfo->mftDataRuns.pDataRunType);
	
	if(pMftFileInfo->mftDataRuns.pDataRunType == 1) //非常驻属性，为真实的dataruns，需要解析
	{
		int iseek;
	    __int64 llstAddr, llAddr = 0;
	    ULONGLONG ullSize;
		BYTE * pDataRuns = pMftFileInfo->mftDataRuns.pDataRuns;
		
	    while(0 != *pDataRuns)
	    {
	        iseek = HIGBYTE(*pDataRuns) + LOWBYTE(*pDataRuns) + 1;
	        Ntfs_get_member_form_runs(pDataRuns, &llstAddr, &ullSize);
			ullSize *= SecPerClr;
	        llAddr += (llstAddr * SecPerClr);
			
			pMftFileInfo->mftDataRuns.pFileDataRun[pMftFileInfo->mftDataRuns.DataRunNum].VSNStart = llAddr;
			pMftFileInfo->mftDataRuns.pFileDataRun[pMftFileInfo->mftDataRuns.DataRunNum].VSNNum = ullSize;
			
			/*
			iLog("pFileDataRun[%d] = [-->%llu + %llu]", pMftFileInfo->mftDataRuns.DataRunNum, \
				pMftFileInfo->mftDataRuns.pFileDataRun[pMftFileInfo->mftDataRuns.DataRunNum].VSNStart, \
				pMftFileInfo->mftDataRuns.pFileDataRun[pMftFileInfo->mftDataRuns.DataRunNum].VSNNum);
			*/
			
			pMftFileInfo->mftDataRuns.DataRunNum ++;
	        pDataRuns += iseek;
	    }
	}
	
	return 0;
}

int Ntfs_analyse_mft_file_bitmap(MFT_FILE_INFO * pMftFileInfo, UINT8 SecPerClr)
{
	pMftFileInfo->DataBitmap.pBitmap = Ntfs_get_mft_file_bitmap_runs(pMftFileInfo, &pMftFileInfo->DataBitmap.bitmapLen, &pMftFileInfo->DataBitmap.pDataBitmapType);
	if(!pMftFileInfo->DataBitmap.pBitmap)
		return -1;
	
	//iLog("pDataRunLen=[%d], pDataRunType=[%d]", pMftFileInfo->mftDataRuns.pDataRunLen, pMftFileInfo->mftDataRuns.pDataRunType);
	
	if(pMftFileInfo->DataBitmap.pDataBitmapType == 1) //非常驻属性，为真实的dataruns，需要解析
	{
		int iseek;
	    __int64 llstAddr, llAddr = 0;
	    ULONGLONG ullSize;
		BYTE * pDataRuns = pMftFileInfo->DataBitmap.pBitmap;
		
	    while(0 != *pDataRuns)
	    {
	        iseek = HIGBYTE(*pDataRuns) + LOWBYTE(*pDataRuns) + 1;
	        Ntfs_get_member_form_runs(pDataRuns, &llstAddr, &ullSize);
			ullSize *= SecPerClr;
	        llAddr += (llstAddr * SecPerClr);
			
			pMftFileInfo->DataBitmap.pBitmapDataRun[pMftFileInfo->DataBitmap.DataBitmapNum].VSNStart = llAddr;
			pMftFileInfo->DataBitmap.pBitmapDataRun[pMftFileInfo->DataBitmap.DataBitmapNum].VSNNum = ullSize;
			
			/*
			iLog("pFileDataRun[%d] = [-->%llu + %llu]", pMftFileInfo->mftDataRuns.DataRunNum, \
				pMftFileInfo->mftDataRuns.pFileDataRun[pMftFileInfo->mftDataRuns.DataRunNum].VSNStart, \
				pMftFileInfo->mftDataRuns.pFileDataRun[pMftFileInfo->mftDataRuns.DataRunNum].VSNNum);
			*/
			
			pMftFileInfo->DataBitmap.DataBitmapNum ++;
	        pDataRuns += iseek;
	    }
	}
	
	return 0;
}


static int Ntfs_get_mft_attribute_info(MFT_FILE_INFO * pMftFileInfo, UINT8 SecPerClr) 
{
	if(pMftFileInfo == NULL)
		return -1;

	AttributeHeader * pAttrHeader = NULL;
	int pAttrOffset = 0; //在属性区域的偏移
	pMftFileInfo->MftAttrNum = 0;
	
	UINT8 * pAttrBuf = pMftFileInfo->SectorData + pMftFileInfo->pMftHeader->AttributeOffset;  // 先偏移到第一个属性区起始位置
	while(TRUE)
	{
		if(pAttrOffset > pMftFileInfo->pMftHeader->MFTRealSize) //超过MFT大小
			break;
		
		pAttrHeader = (AttributeHeader *)(pAttrBuf + pAttrOffset);
		if(valueat(pAttrBuf, pAttrOffset, UINT16) == MFT_END_FLAG || \
			pAttrHeader->ATTR_Type == 0 || \
			pAttrHeader->ATTR_Size == 0) //结束标记
			break;

		//iLog("-----------------------attribute_item[%d]-----------------------", pMftFileInfo->MftAttrNum);
		if(Ntfs_get_mft_attribute_item(pAttrBuf + pAttrOffset, &pMftFileInfo->MftAttrItem[pMftFileInfo->MftAttrNum]) == -1)
			break;
		
		pMftFileInfo->MftAttrNum ++;
		pAttrOffset += pAttrHeader->ATTR_Size;//偏移到下一个属性起始位置
	}
	
	//iLog("MftAttrNum=[%d]", pMftFileInfo->MftAttrNum);

	Ntfs_analyse_mft_file_dataruns(pMftFileInfo, SecPerClr);
	Ntfs_analyse_mft_file_bitmap(pMftFileInfo, SecPerClr);
	
	return (pMftFileInfo->MftAttrNum > 0 ? 0 : -1);
}

MFT_FILE_INFO * Ntfs_get_mft_file_info (int devfd, UINT64 partStartSec, DWORD MftId, MFT_DATA_RUNS * pMftDataRuns)
{
	DDWORD leftSecNum = MftId * 2;
	DDWORD MftAddrSec = pMftDataRuns->pFileDataRun[0].VSNStart + leftSecNum;
	
	int i = 0;
	for(i = 0; i < pMftDataRuns->DataRunNum; i ++)
	{
		if(leftSecNum < pMftDataRuns->pFileDataRun[i].VSNNum)
		{
			MftAddrSec = pMftDataRuns->pFileDataRun[i].VSNStart + leftSecNum;
			break;
		}
		
		leftSecNum -= pMftDataRuns->pFileDataRun[i].VSNNum;
	}
	
	MFT_FILE_INFO * pMftFileInfo = (MFT_FILE_INFO *) Ntfs_malloc(sizeof(MFT_FILE_INFO));
	if(!pMftFileInfo)
	{
		assert(0);
		eLog("Ntfs_malloc failed!");
		return NULL;
	}
	
	if(Ntfs_ReadSector(devfd, partStartSec, MftAddrSec, 2, pMftFileInfo->SectorData) == -1)
	{
		Ntfs_free(DP_VOID(&pMftFileInfo));
		return NULL;
	}
	
	pMftFileInfo->pMftHeader = (MFT_HEADER *)pMftFileInfo->SectorData; //MFT头指向数据起始位置
	if (Ntfs_CheckMftHaedId( pMftFileInfo->pMftHeader->HeadID) == FALSE)
	{
		Ntfs_free(DP_VOID(&pMftFileInfo));
		return NULL;
    }
	
	if(pMftFileInfo->pMftHeader->MFTAllocSize < SECTORSIZE || (pMftFileInfo->pMftHeader->MFTAllocSize % SECTORSIZE) != 0)
	{
		Ntfs_free(DP_VOID(&pMftFileInfo));
		return NULL;
	}

	//if(Ntfs_fixup_mft_info((BYTE *)pMftFileInfo->pMftHeader, pMftFileInfo->pMftHeader->MFTAllocSize) == -1)
	//{
	//	Ntfs_free(DP_VOID(&pMftFileInfo));
	//	return NULL;
	//}
	
	//iLog("AttributeOffset=[%d]", pMftFileInfo->pMftHeader->AttributeOffset);
	//iLog("Resident=[0x%x]", pMftFileInfo->pMftHeader->Resident);
	
	if(Ntfs_get_mft_attribute_info(pMftFileInfo, 8) == -1)
	{
		Ntfs_free(DP_VOID(&pMftFileInfo));
		return NULL;
	}

	//dump_mem_to_file(pMftFileInfo->SectorData, SECTORSIZE * 2, "MFT_SEC", FALSE);
	
    return pMftFileInfo;
}

MFT_FILE_INFO * Ntfs_get_assign_mft_file_info (int devfd, UINT64 partStartSec, DWORD MftId, MFT_DATA_RUNS * pMftDataRuns){ return Ntfs_get_mft_file_info(devfd, partStartSec, MftId, pMftDataRuns);}

void Ntfs_release_mft_file_info(MFT_FILE_INFO ** pMftFileInfo){Ntfs_free(DP_VOID(pMftFileInfo));}

NTFS_PART_INFO * Ntfs_get_part_info(const char * devName, UINT64 partStartSec, UINT64 partTotalSec)
{
	iLog("start NTFS_get_part_info");
	iLog("sizeof(NTFS_PART_INFO)=[%d]", sizeof(NTFS_PART_INFO));
	
	NTFS_PART_INFO * pNtfsPartInfo = (NTFS_PART_INFO *) Ntfs_malloc(sizeof(NTFS_PART_INFO));
	if(!pNtfsPartInfo)
	{
		assert(0);
		eLog("Ntfs_malloc failed!");
		return NULL;
	}
	
	pNtfsPartInfo->partStartSec = partStartSec;
	pNtfsPartInfo->partTotalSec = partTotalSec;
	pNtfsPartInfo->devfd = Ntfs_OpenDev(devName, O_RDWR);
	if(pNtfsPartInfo->devfd < 0)
	{
		Ntfs_release_part_info(&pNtfsPartInfo);
		return NULL;
	}
	
	pNtfsPartInfo->pNtfsBootInfo = Ntfs_get_boot_info(pNtfsPartInfo->devfd, pNtfsPartInfo->partStartSec);
	if(!pNtfsPartInfo->pNtfsBootInfo)
	{
		Ntfs_release_part_info(&pNtfsPartInfo);
		return NULL;
	}
	
	if (Ntfs_CheckOemId( pNtfsPartInfo->pNtfsBootInfo->OemID) == FALSE)
	{
		Ntfs_release_part_info(&pNtfsPartInfo);
		return NULL;
    }

	pNtfsPartInfo->MftAddrSec = pNtfsPartInfo->pNtfsBootInfo->MFTAddrClr * pNtfsPartInfo->pNtfsBootInfo->SecPerClr;
	iLog("MftAddrSec=[%llu * %d = %llu]", pNtfsPartInfo->pNtfsBootInfo->MFTAddrClr, pNtfsPartInfo->pNtfsBootInfo->SecPerClr, pNtfsPartInfo->MftAddrSec);
		
	int i = 0;
	int MftAllocSec = 0;
	
	for(i = 0; i < MFT_FILE_NUM; i ++)
	{
		MFT_FILE_INFO * pMftFileInfo = &pNtfsPartInfo->MftFileInfo[i];
		//iLog("===============================MFT[%d]===============================", i);
		if(Ntfs_ReadSector(pNtfsPartInfo->devfd, pNtfsPartInfo->partStartSec, pNtfsPartInfo->MftAddrSec + MftAllocSec, 2, pMftFileInfo->SectorData) == -1)
			break;
		
		pMftFileInfo->pMftHeader = (MFT_HEADER *)pMftFileInfo->SectorData; //MFT头指向数据起始位置
		if (Ntfs_CheckMftHaedId( pMftFileInfo->pMftHeader->HeadID) == FALSE)
			break;
		
		if(pMftFileInfo->pMftHeader->MFTAllocSize < SECTORSIZE || (pMftFileInfo->pMftHeader->MFTAllocSize % SECTORSIZE) != 0)
			break;
		
		//if(Ntfs_fixup_mft_info((BYTE *)pMftFileInfo->pMftHeader, pMftFileInfo->pMftHeader->MFTAllocSize) == -1)
		//	break;
		
		//iLog("Resident=[0x%x]", pMftFileInfo->pMftHeader->Resident);
		
		if(Ntfs_get_mft_attribute_info(pMftFileInfo, pNtfsPartInfo->pNtfsBootInfo->SecPerClr) == -1)
			break;
		
		MftAllocSec += (pMftFileInfo->pMftHeader->MFTAllocSize / SECTORSIZE);
	}
	
	return pNtfsPartInfo;
}

void Ntfs_release_part_info(NTFS_PART_INFO ** pNtfsPartInfo)
{
	if(*pNtfsPartInfo)
	{
		Ntfs_CloseDev((*pNtfsPartInfo)->devfd);
		Ntfs_release_boot_info(&(*pNtfsPartInfo)->pNtfsBootInfo);
		Ntfs_free(DP_VOID(pNtfsPartInfo));
	}
}

//将目录所有运行的数据读到内存,需要修正一下Fixup位置
int Ntfs_read_direction_runs_to_mem(NTFS_PART_INFO * pNtfsPartInfo, BYTE * pRuns, BYTE * pMpptr)
{
    int iseek;
    __int64 llstAddr, llAddr = 0;
    ULONGLONG ullSize;
	int runsNum = 0;
	
    while(0 != *pRuns)
    {
        iseek = HIGBYTE(*pRuns) + LOWBYTE(*pRuns) + 1;
        Ntfs_get_member_form_runs(pRuns, &llstAddr, &ullSize);
		ullSize *= pNtfsPartInfo->pNtfsBootInfo->SecPerClr;
        llAddr += (llstAddr * pNtfsPartInfo->pNtfsBootInfo->SecPerClr);
		
		if(Ntfs_ReadSector(pNtfsPartInfo->devfd, pNtfsPartInfo->partStartSec, (DWORD)llAddr, (WORD)ullSize, pMpptr) == -1)
            return -1;
		
        //if(Ntfs_check_mft_fixup(pMpptr, (WORD)ullSize * SECTORSIZE) != 0)//用这个函数修正INDX中的FIXUP位置，效果一样 
        //    return dwRes;
		
        pRuns += iseek;
        pMpptr += (ullSize * SECTORSIZE);
    }
	
    return runsNum;
}

static int Ntfs_search_file_in_child_index_allocation(char * fileName, NTFS_PART_INFO * pNtfsPartInfo, DDWORD RunsVSN, DWORD * MftFileId, DDWORD * fileAllocSize, DDWORD * fileRealSize)
{
	if(RunsVSN == 0)
		return -1;

	BYTE * pIndexDataBuf = (BYTE *)Ntfs_malloc(SECTORSIZE * pNtfsPartInfo->pNtfsBootInfo->SecPerClr);
    if(NULL == pIndexDataBuf)
		return -1;
	
	int indxId = -1;
	int retvalue = -1;
	
	while(TRUE)
	{
		if(Ntfs_read_data_block(pNtfsPartInfo->devfd, pNtfsPartInfo->partStartSec, RunsVSN, pNtfsPartInfo->pNtfsBootInfo->SecPerClr, pIndexDataBuf) == -1)
			break;
		
		
		STD_INDEX_HEADER * pStdIndxHead = (STD_INDEX_HEADER *)pIndexDataBuf;
		if(++ indxId != pStdIndxHead->IndexCacheVCN)
			break;
		
		if(pStdIndxHead->IndexEntryAllocSize == 0)
			break;
		
		if(Ntfs_CheckIndexId(pStdIndxHead->IndexFlag) == -1)
			break;

		if(Ntfs_search_file_in_index_allocation(fileName, \
										pNtfsPartInfo, \
										pIndexDataBuf + INDEX_EXT_OFFSET + pStdIndxHead->IndexEntryOffset , \
										pStdIndxHead->IndexEntryAllocSize, \
										RunsVSN, \
										MftFileId, \
										fileAllocSize, \
										fileRealSize) == 0)//已经找到
		{
			retvalue = 0;
    		break;
		}
		
		RunsVSN += pNtfsPartInfo->pNtfsBootInfo->SecPerClr;
	}
	
	Ntfs_free(DP_VOID(&pIndexDataBuf));
	return retvalue;
}

static int Ntfs_search_file_in_index_allocation(char * fileName, NTFS_PART_INFO * pNtfsPartInfo, BYTE * indexBuf, int indexDataLen, DWORD RunsVSN, DWORD * MftFileId, DDWORD * fileAllocSize, DDWORD * fileRealSize)
{
	int IndxAttrOffset = 0;
	char szName[256] = {'\0'};
	STD_INDEX_ENTRY * pIndxAttr = NULL;
	int searchRst = -1;
	DWORD oldMftId = 0;
	
	while(IndxAttrOffset < indexDataLen)
	{
		//iLog("IndxAttrOffset=[%d / %d]", IndxAttrOffset, indexDataLen);
		pIndxAttr = (STD_INDEX_ENTRY *)(indexBuf + IndxAttrOffset);
		if(pIndxAttr->IndexEntrySize == 0x10) //特殊情况,无子节点，直接退出
			break;

		dump_mem_to_file(pIndxAttr, pIndxAttr->IndexEntrySize, "INDEX_DATA", FALSE);
		
		if(pIndxAttr->IndexEntrySize == 0x18) //特殊情况，
		{
			//iLog("MFTReferNumber=[%d]", (DWORD)pIndxAttr->FatherDirMFTReferNumber);
			if(pIndxAttr->IndexFlag == 0x03) //有子节点
			{
				DWORD IndxNodeVSNOffset = (DWORD)pIndxAttr->FatherDirMFTReferNumber;
				if(IndxNodeVSNOffset > 0)
				{
					RunsVSN += IndxNodeVSNOffset* pNtfsPartInfo->pNtfsBootInfo->SecPerClr;
					//iLog("RunsVSN=[%d]", RunsVSN);
					if(Ntfs_search_file_in_child_index_allocation(fileName, pNtfsPartInfo, RunsVSN, MftFileId, fileAllocSize, fileRealSize) == 0)
						searchRst = 0;
				}
			}

			break;
		}

		if((DWORD)pIndxAttr->MFTReferNumber == oldMftId)
		{	
			IndxAttrOffset += pIndxAttr->IndexEntrySize;
			continue;
		}

		oldMftId = (DWORD)pIndxAttr->MFTReferNumber;
		
		//if(pIndxAttr->FileNamespace == 3)//只显示UNICODE文件名，不显示DOS等文件名
        {
	        StrUnicodeToUtf8(P_VOID(pIndxAttr->FileNameAndFill), pIndxAttr->FileNameSize, szName, sizeof(szName));
			//Ntfs_unicode_to_utf8(P_WORD(pIndxAttr->FileNameAndFill), pIndxAttr->FileNameSize, szName, sizeof(szName));
			
			iLog("szName=[%d][%d][%x][%s]", (DWORD)pIndxAttr->MFTReferNumber, pIndxAttr->IndexFlag, pIndxAttr->FileNamespace, szName);
			
       	 	if(strlen(szName) == strlen(fileName) && \
				strncmp(szName, fileName, strlen(szName)) == 0)
       	 	{
           		*MftFileId = (DWORD)pIndxAttr->MFTReferNumber;

				*fileRealSize = pIndxAttr->FileRealSize;
				*fileAllocSize = pIndxAttr->FileAllocSize;
				if(pIndxAttr->FileRealSize > pIndxAttr->FileAllocSize)
					*fileRealSize = (DWORD)pIndxAttr->FileRealSize;
				
				searchRst = 0;
				iLog("find file[%s]!  MftFileId=[%d]", szName, *MftFileId);
				break;
            }
		}
		
		IndxAttrOffset += pIndxAttr->IndexEntrySize;
	}
	
	return searchRst;
}

static MftAttributeItem * Ntfs_get_index_root_attritem(MFT_FILE_INFO * pMftFileInfo){return Ntfs_get_attritem_by_attrtype(pMftFileInfo, ATTR_INDEX_ROOT);}

//从目录查找文件的Run,递归查找,返回值为文件的MFT标号，返回0则查找失败
DWORD Ntfs_search_file_in_direction(char * filePath, int deepIndex, NTFS_PART_INFO * pNtfsPartInfo, DDWORD RunsVSN, DWORD mftFileId, DDWORD * fileAllocSize, DDWORD * fileRealSize)
{
	if(strlen(filePath) == strlen("/") && strncmp(filePath, "/", strlen(filePath)) == 0)
		return mftFileId;
	
	char fileName[256];
	int PathDeep = Ntfs_get_file_path_deepth(filePath);	
	if(Ntfs_get_file_name_from_path_by_deepth(filePath, deepIndex, fileName, sizeof(fileName)) == -1)
		return 0;
	
	iLog("deepIndex=[%d/%d],fileName=[%s]", deepIndex, PathDeep , fileName);
	
	MFT_FILE_INFO * pMftFileInfo = Ntfs_get_mft_file_info(pNtfsPartInfo->devfd, pNtfsPartInfo->partStartSec, mftFileId, &pNtfsPartInfo->MftFileInfo[FILE_MFT].mftDataRuns);
	if(pMftFileInfo == NULL)
		return 0;
	
	dump_mem_to_file(pMftFileInfo->SectorData, 1024 , "MFT_DATA", FALSE);
	
    STD_INDEX_HEADER * pStdIndxHead;
    UINT32 StdIndexHeadOffset = 0;
	DWORD MftFileId = 0;
	int pRunsLen = 0;
	int dataType = 0;
	
	MftAttributeItem * pMftAttrItem = Ntfs_get_index_root_attritem(pMftFileInfo);
	if(pMftAttrItem)
	{
		int IndexEntrySize = pMftAttrItem->pIndexRootAttr->IndexHeader.TalSzOfEntries - pMftAttrItem->pIndexRootAttr->IndexHeader.EntryOff;
		iLog("IndexEntrySize=[%d]", IndexEntrySize);
		if(Ntfs_search_file_in_index_allocation(fileName, pNtfsPartInfo, pMftAttrItem->pIndexRootAttr->IndexEntry, IndexEntrySize, RunsVSN, &MftFileId, fileAllocSize, fileRealSize) == 0)
		{
			Ntfs_release_mft_file_info(&pMftFileInfo);
			if(deepIndex >= PathDeep) //已经找到
				return MftFileId;
			else
				return Ntfs_search_file_in_direction(filePath, ++deepIndex, pNtfsPartInfo, RunsVSN, MftFileId, fileAllocSize, fileRealSize);
		}
	}
	
	BYTE * pRuns = Ntfs_get_direction_index_runs(pMftFileInfo, &pRunsLen, &dataType);
	if(pRuns == NULL || dataType == 0)
	{
		Ntfs_release_mft_file_info(&pMftFileInfo);
		return 0;
	}
	
    ULONGLONG ullAllSize = Ntfs_get_data_runs_total_size(pNtfsPartInfo->pNtfsBootInfo->SecPerClr, pNtfsPartInfo->pNtfsBootInfo->SecInByte, pRuns);
    if(ullAllSize <= 0)
	{
		Ntfs_release_mft_file_info(&pMftFileInfo);
		return 0;
	}

	BYTE * pIndexDataBuf = (BYTE *)Ntfs_malloc((int)ullAllSize);
    if(NULL == pIndexDataBuf)
	{
		Ntfs_release_mft_file_info(&pMftFileInfo);
		return 0;
	}

	///////////////////////////////////////////////
    /*
    if(Ntfs_read_direction_runs_to_mem(pNtfsPartInfo, pRuns, pIndexDataBuf, RunsVSN) != 0)
    {
        Ntfs_free(DP_VOID(&pIndexDataBuf));
		Ntfs_release_mft_file_info(&pMftFileInfo);
        return 0;
    }
	
	dump_mem_to_file(pIndexDataBuf, ullAllSize, "INDEX_DATA", FALSE);
	*/
	//////////////////////////////////////////////
	
	int iseek = 0;
    LONGLONG llstAddr, llAddr = 0;
    ULONGLONG ullSize = 0;
	
    while(0 != *pRuns)
    {
        iseek = HIGBYTE(*pRuns) + LOWBYTE(*pRuns) + 1;
        Ntfs_get_member_form_runs(pRuns, &llstAddr, &ullSize);
		ullSize *= pNtfsPartInfo->pNtfsBootInfo->SecPerClr;
        llAddr += (llstAddr * pNtfsPartInfo->pNtfsBootInfo->SecPerClr);
		
		if(Ntfs_read_data_block(pNtfsPartInfo->devfd, pNtfsPartInfo->partStartSec, (DWORD)llAddr, ullSize, pIndexDataBuf) == -1)
		{
			Ntfs_free(DP_VOID(&pIndexDataBuf));
			Ntfs_release_mft_file_info(&pMftFileInfo);
			return 0;
		}
		
		//dump_mem_to_file(pIndexDataBuf, ullSize * SECTORSIZE, "INDEX_DATA", FALSE);
		
		RunsVSN = llAddr;
		
		DDWORD nextRunsVSN = 0;
		///////////////////////////////////////////////////////////////////////////////
		while(StdIndexHeadOffset < ullAllSize)
		{
			//iLog("StdIndexHeadOffset=[%d/%llu]", StdIndexHeadOffset, ullAllSize);
	    	pStdIndxHead = (STD_INDEX_HEADER *)(pIndexDataBuf + StdIndexHeadOffset);
			if(pStdIndxHead->IndexEntryAllocSize == 0)
				break;
			
			if(Ntfs_CheckIndexId(pStdIndxHead->IndexFlag) == -1)
			{
				Ntfs_free(DP_VOID(&pIndexDataBuf));
				Ntfs_release_mft_file_info(&pMftFileInfo);
	        	return 0;
			}
			
			nextRunsVSN = RunsVSN + pStdIndxHead->IndexCacheVCN * pNtfsPartInfo->pNtfsBootInfo->SecPerClr;
			
			if(Ntfs_search_file_in_index_allocation(fileName, \
											pNtfsPartInfo, \
											pIndexDataBuf + StdIndexHeadOffset + INDEX_EXT_OFFSET + pStdIndxHead->IndexEntryOffset , \
											pStdIndxHead->IndexEntryAllocSize, \
											nextRunsVSN, \
											&MftFileId, \
											fileAllocSize, \
											fileRealSize) == 0)
			{
				Ntfs_free(DP_VOID(&pIndexDataBuf));
				Ntfs_release_mft_file_info(&pMftFileInfo);
				
	    		if(deepIndex >= PathDeep) //已经找到
					return MftFileId;
				else
					return Ntfs_search_file_in_direction(filePath, ++deepIndex, pNtfsPartInfo, nextRunsVSN, MftFileId, fileAllocSize, fileRealSize);
			}
			
			StdIndexHeadOffset += (pStdIndxHead->IndexEntryAllocSize + INDEX_EXT_OFFSET);
		}

		pRuns += iseek;
    }	
	
    Ntfs_free(DP_VOID(&pIndexDataBuf));
	Ntfs_release_mft_file_info(&pMftFileInfo);
	
    return 0;
}

static int Ntfs_scan_direction_in_child_index_allocation(char * PathName, NTFS_PART_INFO * pNtfsPartInfo, DDWORD RunsVSN, int CurDeepth, int limitDeepth)
{
	if(RunsVSN == 0)
		return -1;

	int indxId = -1;
	int retvalue = -1;
	
	BYTE * pIndexDataBuf = (BYTE *)Ntfs_malloc(SECTORSIZE * pNtfsPartInfo->pNtfsBootInfo->SecPerClr);
    if(NULL == pIndexDataBuf)
		return -1;
	
	while(TRUE)
	{
		if(Ntfs_read_data_block(pNtfsPartInfo->devfd, pNtfsPartInfo->partStartSec, RunsVSN, pNtfsPartInfo->pNtfsBootInfo->SecPerClr, pIndexDataBuf) == -1)
			break;
		
		STD_INDEX_HEADER * pStdIndxHead = (STD_INDEX_HEADER *)pIndexDataBuf;
		if(++ indxId != pStdIndxHead->IndexCacheVCN)
			break;
		
		if(pStdIndxHead->IndexEntryAllocSize == 0)
			break;
		
		if(Ntfs_CheckIndexId(pStdIndxHead->IndexFlag) == -1)
			break;
		
		//////////////////////////////////////////////////////////////////////
		Ntfs_scan_direction_in_index_allcation(PathName, pNtfsPartInfo, \
				pIndexDataBuf + INDEX_EXT_OFFSET + pStdIndxHead->IndexEntryOffset, \
				pStdIndxHead->IndexEntryAllocSize, RunsVSN, CurDeepth, limitDeepth);
		//////////////////////////////////////////////////////////////////////
			
		RunsVSN += pNtfsPartInfo->pNtfsBootInfo->SecPerClr;
	}
	
	Ntfs_free(DP_VOID(&pIndexDataBuf));
	return retvalue;
}

static int Ntfs_scan_direction_in_index_allcation(char * PathName, NTFS_PART_INFO * pNtfsPartInfo, BYTE * indexBuf, int indexDataLen, DWORD RunsVSN , int CurDeepth, int limitDeepth)
{
	int IndxAttrOffset = 0;
	char szName[256] = {'\0'};
	STD_INDEX_ENTRY * pIndxAttr = NULL;
	int searchRet = 0;
	
	while(IndxAttrOffset < indexDataLen)
	{
		//iLog("IndxAttrOffset=[%d / %d]", IndxAttrOffset, indexDataLen);
		pIndxAttr = (STD_INDEX_ENTRY *)(indexBuf + IndxAttrOffset);
		if(pIndxAttr->IndexEntrySize == 0 || \
			pIndxAttr->IndexEntrySize == 0x10) //特殊情况,无子节点，直接退出
			break;

		//////////////////////////////////////////////////////////////////////////////////////////////////////
		if(pIndxAttr->IndexEntrySize == 0x18) //特殊情况，
		{
			//iLog("MFTReferNumber=[%d]", (DWORD)pIndxAttr->FatherDirMFTReferNumber);
			if(pIndxAttr->IndexFlag == 0x03) //有子节点
			{
				DWORD IndxNodeVSNOffset = (DWORD)pIndxAttr->FatherDirMFTReferNumber;
				if(IndxNodeVSNOffset > 0)
				{
					RunsVSN += IndxNodeVSNOffset * pNtfsPartInfo->pNtfsBootInfo->SecPerClr;
					//iLog("RunsVSN=[%d]", RunsVSN);
					
					////////////////////////////////////////////////////////////////////////////////
					Ntfs_scan_direction_in_child_index_allocation(PathName, pNtfsPartInfo, RunsVSN, CurDeepth, limitDeepth);
					////////////////////////////////////////////////////////////////////////////////
				}
			}
			
			break;
		}
		//////////////////////////////////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////////////////////////////////
		//if(pIndxAttr->FileNamespace)//只显示UNICODE文件名，不显示DOS等文件名
        {
        	StrUnicodeToUtf8(P_VOID(pIndxAttr->FileNameAndFill), pIndxAttr->FileNameSize, szName, sizeof(szName));
			//Ntfs_unicode_to_utf8(P_WORD(pIndxAttr->FileNameAndFill), pIndxAttr->FileNameSize, szName, sizeof(szName));
			
			if(strlen(szName) == strlen(".") && \
				strncmp((const char*)szName, ".", strlen(szName)) == 0) //（避免与$ROOT混淆）根目录出现有以 “.”开头的文件名，如： .start
			{
				IndxAttrOffset += pIndxAttr->IndexEntrySize;
				continue;
			}
			
			if(strlen(szName))
			{
				char fileName[256];
				if(PathName[strlen(PathName) - 1] == '/')
					sprintf(fileName, "%s%s", PathName, szName);
				else
					sprintf(fileName, "%s/%s", PathName, szName);
				
				//iLog("szName=[%d][%x][%s]", (DWORD)pIndxAttr->MFTReferNumber, pIndxAttr->FileNamespace, szName);
				iLog("szName=[%s]",fileName);

				/////////////////////////////////////////////////////////////////////////////////////
				if(Ntfs_scan_direction(fileName, pNtfsPartInfo, (DWORD)pIndxAttr->MFTReferNumber, RunsVSN, CurDeepth, limitDeepth) == -1)
					searchRet = -1;
				/////////////////////////////////////////////////////////////////////////////////////
			}
		}
		//////////////////////////////////////////////////////////////////////////////////////////////////////
		
		IndxAttrOffset += pIndxAttr->IndexEntrySize;
	}
	
    return searchRet;
}

static int Ntfs_scan_direction(char * PathName, NTFS_PART_INFO * pNtfsPartInfo, DWORD mftFileId, DWORD RunsVSN, int CurDeepth, int limitDeepth)
{
	if(CurDeepth ++ >= limitDeepth) //当前深度达到限制深度时
		return 0;
	
	STD_INDEX_HEADER * pStdIndxHead;
    UINT32 StdIndexHeadOffset = 0;
	int pRunsLen = 0;
	int dataType = 0;
	
	MFT_FILE_INFO * pMftFileInfo = Ntfs_get_mft_file_info(pNtfsPartInfo->devfd, pNtfsPartInfo->partStartSec, mftFileId, &pNtfsPartInfo->MftFileInfo[FILE_MFT].mftDataRuns);
	if(pMftFileInfo == NULL)
		return -1;
	
	MftAttributeItem * pMftAttrItem = Ntfs_get_index_root_attritem(pMftFileInfo); //90H属性
	if(pMftAttrItem)
	{
		int IndexEntrySize = pMftAttrItem->pIndexRootAttr->IndexHeader.TalSzOfEntries - pMftAttrItem->pIndexRootAttr->IndexHeader.EntryOff;
		//iLog("IndexEntrySize=[%d]", IndexEntrySize);
		Ntfs_scan_direction_in_index_allcation(PathName, pNtfsPartInfo, pMftAttrItem->pIndexRootAttr->IndexEntry, IndexEntrySize, RunsVSN, CurDeepth, limitDeepth);
	}
	
	BYTE * pRuns = Ntfs_get_direction_index_runs(pMftFileInfo, &pRunsLen, &dataType); //A0H属性
	if(pRuns == NULL || dataType == 0)
	{
		Ntfs_release_mft_file_info(&pMftFileInfo);
		return -1;
	}
	
    ULONGLONG ullAllSize = Ntfs_get_data_runs_total_size(pNtfsPartInfo->pNtfsBootInfo->SecPerClr, pNtfsPartInfo->pNtfsBootInfo->SecInByte, pRuns);
	if(ullAllSize <= 0)
	{
		Ntfs_release_mft_file_info(&pMftFileInfo);
		return -1;
	}
	
    BYTE * pIndexDataBuf = (BYTE *)Ntfs_malloc((int)ullAllSize);
    if(NULL == pIndexDataBuf)
        return -1;

	////////////////////////////////////////////////////////////////////////////////////////////
	int iseek = 0;
    LONGLONG llstAddr, llAddr = 0;
    ULONGLONG ullSize = 0;
    while(0 != *pRuns)
    {
        iseek = HIGBYTE(*pRuns) + LOWBYTE(*pRuns) + 1;
        Ntfs_get_member_form_runs(pRuns, &llstAddr, &ullSize);
		ullSize *= pNtfsPartInfo->pNtfsBootInfo->SecPerClr;
        llAddr += (llstAddr * pNtfsPartInfo->pNtfsBootInfo->SecPerClr);
		
		if(Ntfs_read_data_block(pNtfsPartInfo->devfd, pNtfsPartInfo->partStartSec, (DWORD)llAddr, (WORD)ullSize, pIndexDataBuf) == -1)
		{
			Ntfs_free(DP_VOID(&pIndexDataBuf));
			Ntfs_release_mft_file_info(&pMftFileInfo);
			return 0;
		}
		
		dump_mem_to_file(pIndexDataBuf, ullSize, "INDEX_DATA", FALSE);
		RunsVSN = llAddr;
		
		///////////////////////////////////////////////////////////////////////////////
		DDWORD nextRunsVSN = 0;
		while(StdIndexHeadOffset < ullAllSize)
		{
			//iLog("StdIndexHeadOffset=[%d/%llu]", StdIndexHeadOffset, ullAllSize);
	    	pStdIndxHead = (STD_INDEX_HEADER *)(pIndexDataBuf + StdIndexHeadOffset);
			if(pStdIndxHead->IndexEntryAllocSize == 0)
				break;
			
			if(Ntfs_CheckIndexId(pStdIndxHead->IndexFlag) == -1)
			{
				Ntfs_free(DP_VOID(&pIndexDataBuf));
				Ntfs_release_mft_file_info(&pMftFileInfo);
	        	return 0;
			}
			
			nextRunsVSN = RunsVSN + pStdIndxHead->IndexCacheVCN * pNtfsPartInfo->pNtfsBootInfo->SecPerClr;

			//////////////////////////////////////////////////////////////////////
			Ntfs_scan_direction_in_index_allcation(PathName, pNtfsPartInfo, \
					pIndexDataBuf + StdIndexHeadOffset + INDEX_EXT_OFFSET + pStdIndxHead->IndexEntryOffset, \
					pStdIndxHead->IndexEntryAllocSize, nextRunsVSN, CurDeepth, limitDeepth);
			//////////////////////////////////////////////////////////////////////
			
			StdIndexHeadOffset += (pStdIndxHead->IndexEntryAllocSize + INDEX_EXT_OFFSET); 
		}
		///////////////////////////////////////////////////////////////////////////////
		
		pRuns += iseek;
    }
	////////////////////////////////////////////////////////////////////////////////////////////
	
    Ntfs_free(DP_VOID(&pIndexDataBuf));
	Ntfs_release_mft_file_info(&pMftFileInfo);
	
    return 0;
}


int Ntfs_scan_file_in_direction(char * PathName, NTFS_PART_INFO * pNtfsPartInfo, int limitDeepth)
{
	DDWORD fileAllocSize = 0;
	DDWORD fileRealSize = 0;
	
	return Ntfs_scan_direction(PathName, pNtfsPartInfo, Ntfs_search_file_in_direction(PathName, 1, pNtfsPartInfo, 0, FILE_ROOT, &fileAllocSize, &fileRealSize), 0, 0, limitDeepth);
}

NTFS_FILE_DATA_INFO * Ntfs_get_file_data_info(char * filePath, NTFS_PART_INFO * pNtfsPartInfo)
{
	NTFS_FILE_DATA_INFO * pNtfsFileDataInfo = (NTFS_FILE_DATA_INFO *) Ntfs_malloc(sizeof(NTFS_FILE_DATA_INFO));
	if(!pNtfsFileDataInfo)
		return NULL;

	pNtfsFileDataInfo->devfd = pNtfsPartInfo->devfd;
	pNtfsFileDataInfo->partStartSec = pNtfsPartInfo->partStartSec;
	pNtfsFileDataInfo->SecInByte = pNtfsPartInfo->pNtfsBootInfo->SecInByte;
	pNtfsFileDataInfo->SecPerClr = pNtfsPartInfo->pNtfsBootInfo->SecPerClr;
	pNtfsFileDataInfo->ClusterSize = pNtfsFileDataInfo->SecInByte * pNtfsFileDataInfo->SecPerClr;
	pNtfsFileDataInfo->MftAddrSec = pNtfsPartInfo->MftAddrSec;
	
	pNtfsFileDataInfo->MftId = Ntfs_search_file_in_direction(filePath, 1, pNtfsPartInfo, 0, FILE_ROOT, &pNtfsFileDataInfo->fileAllocSize, &pNtfsFileDataInfo->fileRealSize);
	if(pNtfsFileDataInfo->MftId == 0)
		return NULL;
	
	iLog("MftId = [%d], fileSize = [%llu / %llu]", pNtfsFileDataInfo->MftId, pNtfsFileDataInfo->fileRealSize, pNtfsFileDataInfo->fileAllocSize);
	
	pNtfsFileDataInfo->pMftFileInfo = Ntfs_get_assign_mft_file_info(pNtfsFileDataInfo->devfd, pNtfsFileDataInfo->partStartSec, pNtfsFileDataInfo->MftId, &pNtfsPartInfo->MftFileInfo[FILE_MFT].mftDataRuns);
	if(!pNtfsFileDataInfo->pMftFileInfo)
	{
		Ntfs_release_file_data_info(&pNtfsFileDataInfo);
		return NULL;
	}
	
	dump_mem_to_file(pNtfsFileDataInfo->pMftFileInfo->SectorData, 1024 , "MFT_DATA", FALSE);
	return pNtfsFileDataInfo;
}

void Ntfs_release_file_data_info(NTFS_FILE_DATA_INFO ** pNtfsFileDataInfo)
{
	if(*pNtfsFileDataInfo)
	{
		if((*pNtfsFileDataInfo)->pMftFileInfo)
			Ntfs_free(DP_VOID(&(*pNtfsFileDataInfo)->pMftFileInfo));
		
		Ntfs_free(DP_VOID( pNtfsFileDataInfo));
	}
}

int Ntfs_read_file_by_data_info(NTFS_FILE_DATA_INFO * pNtfsFileDataInfo, BYTE * databuf, int bufLen)
{
	if(pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pDataRunType == 1)
	{
		int bufOffset = 0;
		DDWORD readSecStart = 0;
		DDWORD readSecSize = 0;
		
		int i = 0;
		for(i = 0; i < pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.DataRunNum; i ++)
		{
			readSecStart = pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pFileDataRun[i].VSNStart;
			readSecSize = pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pFileDataRun[i].VSNNum * SECTORSIZE;

			if(bufOffset >= bufLen)
				break;
			else if(bufOffset + readSecSize > bufLen)
			{
				DDWORD readSecNum = (bufLen - bufOffset) / SECTORSIZE;
				readSecSize = readSecNum * SECTORSIZE;
				
				Ntfs_read_data_block(pNtfsFileDataInfo->devfd, pNtfsFileDataInfo->partStartSec, readSecStart, readSecNum, databuf + bufOffset);
				bufOffset += readSecSize;
			}
			else
			{
				Ntfs_read_data_block(pNtfsFileDataInfo->devfd, pNtfsFileDataInfo->partStartSec, readSecStart, pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pFileDataRun[i].VSNNum, databuf + bufOffset);
				bufOffset += readSecSize;
			}
		}
	}
	else if(pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pDataRunType == 0)
	{
		memcpy(databuf, pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pDataRuns, MIN(bufLen, pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pDataRunLen));
	}
	else
		return -1;
	
	return 0;
}


int Ntfs_read_file_from_data_info(char * newFile, NTFS_FILE_DATA_INFO * pNtfsFileDataInfo)
{
	if(newFile == NULL || pNtfsFileDataInfo == NULL)
		return -1;

	if(pNtfsFileDataInfo->fileRealSize <= 0)
		return -1;
	
	FILE * fp = fopen(newFile, "wb");
	if(fp == NULL)
	{
		eLog("fopen file[%s] failed: %s", newFile, ErrExp());
		return -1;
	}

	if(pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pDataRunType == 0) //常驻属性，文件数据比较小，在MFT扇区中可以存储
	{
		if(fwrite(pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pDataRuns, MIN(pNtfsFileDataInfo->fileRealSize, pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pDataRunLen), 1, fp) != 1)
		{
			eLog("fwrite failed: %s", ErrExp());
			fclose(fp);
			return -1;
		}
	}
	else if(pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pDataRunType == 1)//非常驻属性，dataRuns，文件数据比较大，需要在其他地方开辟空间
	{
		UINT64 totalWriteDataSize = 0;
		BYTE * dataBuf = (BYTE *) Ntfs_malloc(pNtfsFileDataInfo->ClusterSize);
		if(dataBuf == NULL)
		{
			eLog("malloc failed: %s", ErrExp());
			fclose(fp);
			return -1;
		}

		int i = 0, j = 0;
		for(i = 0; i < pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.DataRunNum; i ++)
		{
			for(j = 0; j < pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pFileDataRun[i].VSNNum; j += pNtfsFileDataInfo->SecPerClr)
			{
				
				Ntfs_read_data_block(pNtfsFileDataInfo->devfd, pNtfsFileDataInfo->partStartSec, pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pFileDataRun[i].VSNStart + j, pNtfsFileDataInfo->SecPerClr, dataBuf);
				int wrtieDataSize = ((totalWriteDataSize + pNtfsFileDataInfo->ClusterSize) < pNtfsFileDataInfo->fileRealSize ? pNtfsFileDataInfo->ClusterSize : (pNtfsFileDataInfo->fileRealSize - totalWriteDataSize));

				//iLog("wrtieDataSize=[%d]", wrtieDataSize);
				if(fwrite(dataBuf, wrtieDataSize, 1, fp) != 1)
				{
					eLog("fwrite failed: %s", ErrExp());
					Ntfs_free(DP_VOID(&dataBuf));
					fclose(fp);
					return -1;
				}
				
				totalWriteDataSize += wrtieDataSize;
			}
		}
		
		Ntfs_free(DP_VOID(&dataBuf));
	}
	else
	{
		eLog("Error pDataRunType=[%d]", pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pDataRunType);
		fclose(fp);
		return -1;
	}
	
	fclose(fp);
	return 0;
}

int Ntfs_write_file_by_data_info(NTFS_FILE_DATA_INFO * pNtfsFileDataInfo, BYTE * databuf, int bufLen)
{
	if(pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pDataRunType == 1)
	{
		int bufOffset = 0;
		DDWORD writeSecStart = 0;
		DDWORD writeSecSize = 0;
		
		int i = 0;
		for(i = 0; i < pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.DataRunNum; i ++)
		{
			writeSecStart = pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pFileDataRun[i].VSNStart;
			writeSecSize = pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pFileDataRun[i].VSNNum * SECTORSIZE;
			
			if(bufOffset >= bufLen)
				break;
			else if(bufOffset + writeSecSize > bufLen)
			{
				DDWORD WriteSecNum = (bufLen - bufOffset) / SECTORSIZE;
				writeSecSize = WriteSecNum * SECTORSIZE;
				
				Ntfs_write_data_block(pNtfsFileDataInfo->devfd, pNtfsFileDataInfo->partStartSec, writeSecStart, WriteSecNum, databuf + bufOffset);
				bufOffset += writeSecSize;
			}
			else
			{
				Ntfs_write_data_block(pNtfsFileDataInfo->devfd, pNtfsFileDataInfo->partStartSec, writeSecStart, pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pFileDataRun[i].VSNNum, databuf + bufOffset);
				bufOffset += writeSecSize;
			}
		}
	}
	else if(pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pDataRunType == 0)
	{
		memcpy(pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pDataRuns, databuf, MIN(bufLen, pNtfsFileDataInfo->pMftFileInfo->mftDataRuns.pDataRunLen));
		Ntfs_write_data_block(pNtfsFileDataInfo->devfd, pNtfsFileDataInfo->partStartSec, pNtfsFileDataInfo->MftAddrSec + pNtfsFileDataInfo->MftId * 2, 2, (BYTE *)pNtfsFileDataInfo->pMftFileInfo);
	}
	else
		return -1;
	
	return 0;
}

int Ntfs_analyse_bitmap(BYTE * pBitmap, int pLen)
{
	int validBlockNum = 0;
	BYTE byteValue = 0;
	BYTE bitValue = 0;
	
	int i = 0;
	int j = 0;
	for(i = 0; i < pLen; i ++)
	{
		byteValue = *(pBitmap + i);
		if(byteValue == 0x00)
			continue;
        else if(byteValue == 0xFF)
        	validBlockNum += 8;
        else
        {
            for ( j = 0; j < 8; j++ )
        	{
        		bitValue = BitVal(byteValue, j);
	            if(bitValue)
	                validBlockNum ++;
        	}
        }
	}
	
	return validBlockNum;
}

int Ntfs_judge_cluster_if_used(BYTE * pBitmap, int pLen, DWORD clusterNo)
{
	BYTE BytePos = clusterNo / 8;
	BYTE BitPos = clusterNo % 8;
	
	if(BytePos > pLen)
		return -1;
	
	return (BitVal(*(pBitmap + BytePos), BitPos) ? 1 : 0);
}

