/****************************************************************************************************************************
*
*   文 件 名 ： ExtScan.c 
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

#include "ExtScan.h"

int ext2_datascan_info_display(EXT_DATA_SCAN_INFO * pDataScanInfo)
{
	if(pDataScanInfo == NULL)
		return -1;
	
	iLog("================================================"); 
	iLog("diskfd=[%d]",pDataScanInfo->diskfd); 
	iLog("partBeginSec=[%llu]",pDataScanInfo->partBeginSec); 
	iLog("partTotalSec=[%llu]",pDataScanInfo->partTotalSec); 
	iLog("partName=[%s]",pDataScanInfo->partName);
	iLog("block_size_byte=[%d]",pDataScanInfo->block_size_byte);
	iLog("block_size_sector=[%d]",pDataScanInfo->block_size_sector);
	iLog("group_count=[%d]",pDataScanInfo->group_count);
	iLog("s_first_data_block=[%d]",pDataScanInfo->s_first_data_block);
    iLog("blocks_per_group=[%d]",pDataScanInfo->blocks_per_group);
    iLog("blocks_last_group=[%d]",pDataScanInfo->blocks_last_group); 
    iLog("group_desc_begsec=[%d]",pDataScanInfo->group_desc_begsec); 
    iLog("group_begsec=[%d]",pDataScanInfo->group_begsec);
    iLog("csum_flag=[%d]",pDataScanInfo->csum_flag);
    iLog("s_desc_size=[%d]",pDataScanInfo->s_desc_size);
    iLog("================================================"); 
	
	return 0;
}

EXT_DATA_SCAN_INFO * ext2_datascan_init(int diskfd, DDWORD partBeginSec, DDWORD partTotalSec)
{
	EXT_DATA_SCAN_INFO * pDataScanInfo = (EXT_DATA_SCAN_INFO *)z_malloc(sizeof(EXT_DATA_SCAN_INFO));
	if(pDataScanInfo == NULL)
		return NULL;

	memset(pDataScanInfo, 0x00, sizeof(EXT_DATA_SCAN_INFO));
	pDataScanInfo->diskfd = diskfd;
	pDataScanInfo->partBeginSec = partBeginSec;
	pDataScanInfo->partTotalSec = partTotalSec;
	
	if(readwrite_hdisk_sector(DISK_READ, \
							pDataScanInfo->diskfd, \
							pDataScanInfo->partBeginSec + 2, \
							2, \
							pDataScanInfo->sectorBuf) < 0)
    {
    	z_free(&pDataScanInfo);
    	return NULL;
	}
	
    struct ext2_super_block * super_block = (struct ext2_super_block *)pDataScanInfo->sectorBuf;
	iLog("blockCount=[%d]",super_block->s_blocks_count); 
						
    pDataScanInfo->block_size_byte = 1 << (super_block->s_log_block_size + 10); // 块大小
    pDataScanInfo->block_size_sector = pDataScanInfo->block_size_byte / SECTOR_SIZE;
    pDataScanInfo->group_count = (super_block->s_blocks_count - super_block->s_first_data_block -1) / super_block->s_blocks_per_group + 1; //块组总数
    pDataScanInfo->s_first_data_block = super_block->s_first_data_block; //第一个数据块号
    pDataScanInfo->blocks_per_group = super_block->s_blocks_per_group; //一组块数
	
    //最后一组的块数 总块数 - （块组数-1）×组块数 -块组起始块
    pDataScanInfo->blocks_last_group = super_block->s_blocks_count-(pDataScanInfo->group_count -1)*pDataScanInfo->blocks_per_group -super_block->s_first_data_block;
    pDataScanInfo->group_desc_begsec = (pDataScanInfo->s_first_data_block + 1) * pDataScanInfo->block_size_sector;
    pDataScanInfo->group_begsec = pDataScanInfo->s_first_data_block * pDataScanInfo->block_size_sector;

    if(EXT2_HAS_RO_COMPAT_FEATURE(super_block,EXT4_FEATURE_RO_COMPAT_HUGE_FILE)!=0 ||\
        EXT2_HAS_RO_COMPAT_FEATURE(super_block,EXT4_FEATURE_RO_COMPAT_GDT_CSUM)!=0 ||\
        EXT2_HAS_RO_COMPAT_FEATURE(super_block,EXT4_FEATURE_RO_COMPAT_DIR_NLINK)!=0 ||\
        EXT2_HAS_RO_COMPAT_FEATURE(super_block,EXT4_FEATURE_RO_COMPAT_EXTRA_ISIZE)!=0 ||\
        EXT2_HAS_INCOMPAT_FEATURE(super_block,EXT4_FEATURE_INCOMPAT_64BIT)!=0 ||\
        EXT2_HAS_INCOMPAT_FEATURE(super_block,EXT4_FEATURE_INCOMPAT_MMP)!=0 )
        strncpy(pDataScanInfo->partName, "EXT4", 5);
    else if(EXT2_HAS_COMPAT_FEATURE(super_block,EXT3_FEATURE_COMPAT_HAS_JOURNAL)!=0)
        strncpy(pDataScanInfo->partName, "EXT3", 5);
    else
        strncpy(pDataScanInfo->partName, "EXT2", 5);
    
    if(EXT2_HAS_INCOMPAT_FEATURE(super_block, EXT4_FEATURE_INCOMPAT_64BIT) != 0)
    {
        iLog("support 64 bit");
        pDataScanInfo->s_desc_size = EXT2_MIN_DESC_SIZE_64BIT;
        return pDataScanInfo;
    }
    else
    {
    	hLog("not support 64 bit");
        pDataScanInfo->s_desc_size = EXT2_MIN_DESC_SIZE;
    }
	
    if(EXT2_HAS_INCOMPAT_FEATURE(super_block, EXT4_FEATURE_INCOMPAT_META_BG) != 0)
    {
        iLog("support meta block");
        return pDataScanInfo;    
    }
    else
    {
        hLog("not support meta block");
    }
	
    if (EXT2_HAS_RO_COMPAT_FEATURE(super_block, EXT4_FEATURE_RO_COMPAT_GDT_CSUM) != 0)
        pDataScanInfo->csum_flag = 1;
    else
        pDataScanInfo->csum_flag = 0;
	
    return pDataScanInfo;
}

void ext2_datascan_end(EXT_DATA_SCAN_INFO ** pDataScanInfo)
{
	z_free(pDataScanInfo);
}

//读取组描述符
int ext2_get_group_desc(EXT_DATA_SCAN_INFO * pDataScanInfo, int groupNum, struct ext2_group_desc *ext2GroupDesc)
{
    DWORD groupDescBegsec = groupNum * (sizeof(struct ext2_group_desc)) / SECTOR_SIZE; //组描述符起始扇区
    DWORD groupDescBegbyte = groupNum * (sizeof(struct ext2_group_desc)) % SECTOR_SIZE; //组描述符起始字节
	
    if(readwrite_hdisk_sector(DISK_READ, \
				pDataScanInfo->diskfd, \
				pDataScanInfo->partBeginSec + pDataScanInfo->group_desc_begsec + groupDescBegsec ,\
				1,\
				pDataScanInfo->sectorBuf) < 0)
        return -1;
	
    memcpy(ext2GroupDesc, pDataScanInfo->sectorBuf+ groupDescBegbyte, sizeof(struct ext2_group_desc));
    return 0;
}

/*
 * Get the value of a particular flag for this block group
 */
int ext2_get_val_by_bgflags(struct ext2_group_desc ext2_gdp, WORD bgFlag)
{
    struct ext4_group_desc gdp;
    memcpy(&gdp, &ext2_gdp, sizeof(struct ext2_group_desc));
	
    return (gdp.bg_flags & bgFlag);
}


//读取块位图
int ext2_get_block_bitmap(EXT_DATA_SCAN_INFO * pDataScanInfo, int groupNum, BYTE * buff, int bufSize)
{
	if(pDataScanInfo == NULL || buff == NULL)

		return -1;

	memset(buff, 0x00, bufSize);
	
    struct ext2_group_desc ext2GroupDesc;
    if(ext2_get_group_desc(pDataScanInfo, groupNum,&ext2GroupDesc) == -1) //读取组描述符
		return -1;
	
    if( pDataScanInfo->csum_flag != 0 && \
		ext2_get_val_by_bgflags(ext2GroupDesc, EXT2_BG_BLOCK_UNINIT))
        return -1;
	
	DWORD bitmapBegsec = ext2GroupDesc.bg_block_bitmap * pDataScanInfo->block_size_sector;
    if(readwrite_hdisk_sector(DISK_READ, \
				pDataScanInfo->diskfd, \
				pDataScanInfo->partBeginSec + bitmapBegsec, \
				pDataScanInfo->block_size_sector, \
				buff) < 0)
        return -1;
	
    return 0;
}

static int ext_mark_bitmap(BYTE * pBitmap, int diskfd, DDWORD bitmapBeginSec, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize)
{
	EXT_DATA_SCAN_INFO * pDataScanInfo = ext2_datascan_init( diskfd, partBeginSec, partTotalSec);
	if(pDataScanInfo)
	{
		DDWORD beginsec = 0;
		BYTE blockBitmap[4096];
		DWORD usedBlock = 0;
		DWORD freeBlock = 0;

		ext2_datascan_info_display(pDataScanInfo);
		
		mark_bitmap_by_sector_info(pBitmap, bitmapBeginSec, 4);
		
		//////////////////////////////解析前面group_count-1组有效数据///////////////////////////////////
		int i = 0;
		for(i = 0; i < pDataScanInfo->group_count -1; i ++)
		{
			usedBlock = 0;
			
            memset(blockBitmap, 0x00, sizeof(blockBitmap));
            ext2_get_block_bitmap(pDataScanInfo, i, blockBitmap,sizeof(blockBitmap)); //读取块位图
			
            int j = 0;
            for(j = 0; j < pDataScanInfo->blocks_per_group / 8; j ++)
            {
                beginsec = bitmapBeginSec + \
						pDataScanInfo->group_begsec + \
						i * pDataScanInfo->blocks_per_group * pDataScanInfo->block_size_sector + \
						j * pDataScanInfo->block_size_sector * 8;
				
                if(blockBitmap[j] == 0xff)
                {
                    usedBlock += 8;
					mark_bitmap_by_sector_info(pBitmap, beginsec, pDataScanInfo->block_size_sector * 8);
                }
                else
                {
                	int k = 0;
                    for(k=0; k<8; k++)
                    {
                        if(blockBitmap[j] & (1 << k))     //对应块被使用
                        {
                            usedBlock += 1;
							mark_bitmap_by_sector_info(pBitmap, beginsec, pDataScanInfo->block_size_sector);
                        }
						
                        beginsec += pDataScanInfo->block_size_sector;
                    }
                }
            }
			
            beginsec = bitmapBeginSec + \
            		pDataScanInfo->group_begsec + \
            		i * pDataScanInfo->blocks_per_group * pDataScanInfo->block_size_sector + \
            		j * pDataScanInfo->block_size_sector ;
			
			int k = 0;
            for(k = 0; k < (pDataScanInfo->blocks_per_group % 8); k ++)
            {
                if(blockBitmap[j] & (1 << k))     //对应块被使用
                {
                    usedBlock += 1;
					mark_bitmap_by_sector_info(pBitmap, beginsec, pDataScanInfo->block_size_sector);
                }
				
				beginsec += pDataScanInfo->block_size_sector;       
            }
			
            freeBlock += (pDataScanInfo->blocks_per_group -usedBlock);
        }
		
        //////////////////////////////解析最后一组有效数据///////////////////////////////////
        usedBlock = 0;
        memset(blockBitmap, 0x00, sizeof(blockBitmap));
        ext2_get_block_bitmap(pDataScanInfo, pDataScanInfo->group_count -1, blockBitmap,sizeof(blockBitmap)); //读取块位图
		
		int j = 0;
        for(j = 0; j < (pDataScanInfo->blocks_last_group / 8); j ++)
        {
            beginsec = bitmapBeginSec + \
					pDataScanInfo->group_begsec + \
					(pDataScanInfo->group_count -1) * pDataScanInfo->blocks_per_group * pDataScanInfo->block_size_sector + \
					j * pDataScanInfo->block_size_sector * 8;

            if(blockBitmap[j] == 0xff)
            {
                usedBlock += 8;
				mark_bitmap_by_sector_info(pBitmap, beginsec, pDataScanInfo->block_size_sector * 8);
            }
            else
            {
            	int k = 0;
                for(k=0; k<8; k++)
                {
                    if(blockBitmap[j] & (1 << k))//对应块被使用
                    {
                        usedBlock += 1;
						mark_bitmap_by_sector_info(pBitmap, beginsec, pDataScanInfo->block_size_sector);
                    }
					
                    beginsec += pDataScanInfo->block_size_sector;
                }
            }                
        }

		beginsec = bitmapBeginSec + \
					pDataScanInfo->group_begsec + \
					(pDataScanInfo->group_count -1) * pDataScanInfo->blocks_per_group * pDataScanInfo->block_size_sector + \
					j * pDataScanInfo->block_size_sector;

		int k = 0;
		for(k = 0; k < (pDataScanInfo->blocks_last_group % 8); k ++)
        {
            if(blockBitmap[j] & (1 << k))     //对应块被使用
            {
                usedBlock += 1;
				mark_bitmap_by_sector_info(pBitmap, beginsec, pDataScanInfo->block_size_sector);
            }

			beginsec += pDataScanInfo->block_size_sector;
        }
		
        freeBlock += (pDataScanInfo->blocks_per_group -usedBlock);

		ext2_datascan_end(&pDataScanInfo);
    }
	else
		mark_bitmap_by_sector_info(pBitmap, bitmapBeginSec, partTotalSec);
	
	return 0;
}

int ext_part_mark_on_disk_bitmap(BYTE * pBitmap, int diskfd, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize)
{
	return ext_mark_bitmap(pBitmap, diskfd, partBeginSec, partBeginSec, partTotalSec, bitmapSize);
}

BYTE * ext_generate_part_bitmap(int diskfd, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize)
{
	BYTE * pBitmap = (BYTE *) z_malloc(bitmapSize);
	if(pBitmap == NULL)
		return NULL;
	
	memset(pBitmap, 0x00, bitmapSize);
	ext_mark_bitmap(pBitmap, diskfd, 0, partBeginSec, partTotalSec, bitmapSize);
	
	return pBitmap;
}
