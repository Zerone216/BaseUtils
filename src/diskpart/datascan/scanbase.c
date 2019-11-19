/****************************************************************************************************************************
*
*   文 件 名 ： scanbase.c 
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

#include "scanbase.h"

//根据扇区获得在位表中所在的偏移
DWORD get_byte_pos_in_bitmap(DDWORD logSec)
{
    return (DWORD)(logSec / BITMAP_BYTE_SECTOR);
}

BYTE get_bit_pos_in_bitmap(DDWORD logSec)
{
    DWORD PosByte = (DWORD)(logSec / BITMAP_BYTE_SECTOR);
    return (BYTE)((logSec -PosByte *BITMAP_BYTE_SECTOR ) / BITMAP_BIT_SECTOR);
}

//根据扇区号logsector计算出在位表中的位置
//将获得信息放入BITMAP结构中
SCAN_BITMAP_INFO * init_bitmap_info(BYTE * pBitmap, DDWORD logSec)
{
	SCAN_BITMAP_INFO * pScanBitmapInfo = (SCAN_BITMAP_INFO *) malloc(sizeof(SCAN_BITMAP_INFO));
	if(pScanBitmapInfo == NULL)
		return NULL;
	
    //pScanBitmapInfo->pBitmap = pBitmap;
    pScanBitmapInfo->PosByte = get_byte_pos_in_bitmap(logSec);
    pScanBitmapInfo->PosBit = get_bit_pos_in_bitmap(logSec);
    pScanBitmapInfo->tmpByte = *(pBitmap + pScanBitmapInfo->PosByte);
	
    return pScanBitmapInfo;
}

void release_bitmap_info(SCAN_BITMAP_INFO ** pScanBitmapInfo)
{
	if(*pScanBitmapInfo)
	{
		free(*pScanBitmapInfo);
		*pScanBitmapInfo = NULL;
	}
}

//标记位表中的某字节某位为1
void mark_bit_on_bitmap(BYTE * pBitmap, DWORD byte, BYTE bit)
{
    if (bit > 7)
	{
		eLog("mask_bit_on_bitmap: bit=[%d] > 7", bit);
		return ;
	}
	
    *(pBitmap + byte) |= (1 << bit);
    return ;
}

//标记位表中的某字节为0xff
void mark_byte_on_bitmap(BYTE * pBitmap, DWORD byte)
{
    *(pBitmap + byte) = 0xFF;
    return ;
}

//判断位表中的某字节某位是否标记过
BOOL check_bit_on_bitmap_marked(BYTE * pBitmap, DWORD byte, BYTE bit)
{
   	if (bit > 7)
	{
		eLog("check_bit_on_bitmap_masked: bit=[%d] > 7", bit);
		return NO;
	}
   	
    if((*(pBitmap + byte) & (1 << bit))== 0)
        return NO;
 	else
    	return YES;
}

//判断位表中的某字节是否标记过
BOOL check_byte_on_bitmap_marked(BYTE * pBitmap, DWORD byte)
{
    if(*(pBitmap + byte) == 0xFF)
        return YES;
 	else
    	return NO;
}


//bitmap中的某字节某位的起始扇区
DDWORD get_begin_sector_from_bitmap(DWORD byte, BYTE bit)
{
	if (bit > 7)
	{
		eLog("get_beginsec_from_bitmap: bit=[%d] > 7", bit);
		return -1;
	}
	
	DDWORD beginSec = (DDWORD)byte * (DDWORD)BITMAP_BYTE_SECTOR + (DDWORD)bit * (DDWORD)BITMAP_BIT_SECTOR - 64; //回该bit表示的64个扇区的起始位置
	return beginSec; 
}

//根据扇区号判断在位表中对应的位是1还是0
BOOL check_sector_on_bitmap_marked(BYTE * pBitmap, DDWORD logSec)
{
    if((*(pBitmap + get_byte_pos_in_bitmap(logSec)) & (1 << get_bit_pos_in_bitmap(logSec))) == 0)
        return NO;
	else
    	return YES;
}

//获得有效数据大小，精确到扇区
//底层划分的一个分区的最大容量为500G, 共1048576000个扇区, 一个unsigned int 最大可以表示4294967296.
DWORD calc_valid_data_size(BYTE * pBitmap, DWORD bitmapSize)
{
    DWORD i = 0, j = 0;
    DWORD blockNum = 0;
	BYTE * pTmp = pBitmap;
	
    //分析位置中bit值为1的个数
    for ( i = 0; i < bitmapSize ; i ++, pTmp ++)
    {
    	if(*pTmp == 0)
			continue;
        else if(*pTmp == 0xFF)
            blockNum += 8;
        else
        {
            for ( j = 0; j < 8; j++ )
                if((*pTmp & (1 << j)) != 0)
                    blockNum ++;
        }
    }
	
    return blockNum;
}

//根据有效数据起始扇区和扇区个数生成位表
int mark_bitmap_by_sector_info(BYTE * pBitmap, DDWORD beginSec, DDWORD secNum)
{
	if(pBitmap == NULL || secNum == 0)
        return -1;
	
    DWORD i = 0, j = 0;
    SCAN_BITMAP_INFO * pBitmapInfoBegin = init_bitmap_info(pBitmap, beginSec);
	if(pBitmapInfoBegin == NULL)
		return -1;
	
	SCAN_BITMAP_INFO * pBitmapInfoEnd = init_bitmap_info(pBitmap, beginSec + secNum - 1);
	if(pBitmapInfoBegin == NULL)
	{
		release_bitmap_info(&pBitmapInfoBegin);
		return -1;
	}
	
    if(pBitmapInfoBegin->PosByte == pBitmapInfoEnd->PosByte) //一个字节内的标记
    {
        for(j = pBitmapInfoBegin->PosBit; j < pBitmapInfoEnd->PosBit + 1; j++)
            mark_bit_on_bitmap(pBitmap, pBitmapInfoBegin->PosByte, j);
    }
	
    for(i = pBitmapInfoBegin->PosByte; i < pBitmapInfoEnd->PosByte + 1; i++)
    {
        if(i == pBitmapInfoBegin->PosByte)//处理到第一个字节
        {
            for(j = pBitmapInfoBegin->PosBit; j < 8; j ++)
                mark_bit_on_bitmap(pBitmap, i, j);
        }
        else if(i == pBitmapInfoEnd->PosByte)//处理到最后一个字节
        {
            for(j = 0; j < pBitmapInfoBegin->PosBit + 1; j++)
                mark_bit_on_bitmap(pBitmap, i, j);
        }
        else //处理普通字节
            mark_byte_on_bitmap(pBitmap, i);
    }

	release_bitmap_info(&pBitmapInfoEnd);
	release_bitmap_info(&pBitmapInfoBegin);
		
    return 0;
}

static int unknown_part_mark_bitmap(BYTE * pBitmap, DDWORD bitmapBeginSec, DDWORD partTotalSec)
{
	return mark_bitmap_by_sector_info(pBitmap, bitmapBeginSec, partTotalSec);
}

BYTE * unknown_fs_generate_part_bitmap(int diskfd, DDWORD partBegiSsec, DDWORD partTotalSec, int bitmapSize)
{
	BYTE * pBitmap = (BYTE *) malloc(bitmapSize);
	if(pBitmap == NULL)
		return NULL;
	
	memset(pBitmap, 0x00, bitmapSize);
	unknown_part_mark_bitmap(pBitmap, 0, partTotalSec);
	
	return pBitmap;
}

int unknown_fs_mark_part_on_disk_bitmap(BYTE * pBitmap, int diskfd, DDWORD partBegiSsec, DDWORD partTotalSec, int bitmapSize)
{
	return unknown_part_mark_bitmap(pBitmap, partBegiSsec, partTotalSec);
}

