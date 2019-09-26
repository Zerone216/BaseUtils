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
*   文 件 名 ： diskrw.c 
*   文件描述 ：  
*   创建日期 ：2019年5月14日
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

#include "diskrw.h"

/*******************************************************************************************************************
 * 函 数 名  ：  record_readwrite_error_info
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2019年5月14日
 * 函数功能  ：  记录硬盘读写的错误信息
 * 参数列表  ： 
        HDISK_BLOCK_RW * hdiskBlockRw   
 * 返 回 值  ：   
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
static int record_readwrite_error_info(HDISK_BLOCK_RW * hdiskBlockRw)
{
	switch(hdiskBlockRw->errCode)
	{
		case RW_DISK_OK:
			return 0;
			
		case RW_DISK_OVER_BOUND:
			eLog("RW_DISK_OVER_BOUND");
			break;

		case RW_DISK_LSEEK_ERROR:
			eLog("RW_DISK_LSEEK_ERROR");
			break;
		
		case RW_DISK_READ_ERROR:
			eLog("RW_DISK_READ_ERROR");
			break;
		
		case RW_DISK_WRITE_ERROR:
			eLog("RW_DISK_WRITE_ERROR");
			break;
		
		case RW_DISK_PARA_ERROR:
			eLog("RW_DISK_PARA_ERROR");
			break;
			
		default:
			eLog("RW_DISK_UNKNOWN_ERROR");
			break;
	}

	eLog("readorwrite=[%x] diskno=[%d] diskCapability=[%lld] errCode=[0x%x] errSec=[%lld]", hdiskBlockRw->readorwrite, hdiskBlockRw->diskno, hdiskBlockRw->diskCapability, hdiskBlockRw->errCode, hdiskBlockRw->errSec);
	return 1;
}


/*
* 简单的磁盘读写，没有判断读写越界和缓冲区大小限制，
* 建议最开始初始化时使用（注：在已获取到磁盘容量和缓冲区大小时慎用）
*/
int readwrite_hdisk_sector(BYTE readorwrite, int diskfd, DDWORD logSec, DDWORD secNum, BYTE * buf)
{
	if(buf == NULL)
		return -1;

	if(secNum == 0)
		return 0;
	
	DDWORD offset = logSec * SECTOR_SIZE;
	DDWORD dataSize = secNum * SECTOR_SIZE;
	
	if(lseek64(diskfd, offset, SEEK_SET) == -1)
	{
		eLog("lseek offset failed!");
		assert(0);
		return -1;
	}
	
	//读写硬盘
	if(readorwrite == DISK_READ)
	{
		if(read(diskfd, buf, dataSize) != dataSize)
		{
			eLog("read hdisk sector failed!");
			assert(0);
			return -1;
		}
	}
	else if(readorwrite == DISK_WRITE)
	{
		if(write(diskfd, buf, dataSize) != dataSize)
		{
			eLog("write hdisk sector failed!");
			assert(0);
			return -1;
		}
	}
	else
	{
		eLog("Unknown operate flag[%x]!", readorwrite);
		return -1;
	}
	
	return 0;
}

/*
* 判断读写扇区是否越界
*/
static int judge_if_readwrite_overboundry(HDISK_BLOCK_RW * hdiskBlockRw)
{
	BYTE overflag = 0;

	if(DISK_CAPABILITY_UNKNOWN == hdiskBlockRw->diskCapability)
		return 0;
	
	if(hdiskBlockRw->startSec > hdiskBlockRw->diskCapability)
	{
		hdiskBlockRw->errSec = hdiskBlockRw->startSec;
		overflag = 1;
	}
	else if(hdiskBlockRw->secNum > hdiskBlockRw->diskCapability)
	{
		hdiskBlockRw->errSec = hdiskBlockRw->secNum;
		overflag = 1;
	}
	else if((hdiskBlockRw->startSec + hdiskBlockRw->secNum) > hdiskBlockRw->diskCapability)
	{
		hdiskBlockRw->errSec = hdiskBlockRw->startSec + hdiskBlockRw->secNum;
		overflag = 1;
	}
	else
		overflag = 0;

	return overflag;
}


/*
* 比较安全的基本磁盘读写接口，有判断读写越界和缓冲区大小限制
*/
static int hdisk_readorwrite(HDISK_BLOCK_RW * hdiskBlockRw)
{
	if(hdiskBlockRw->buf == NULL ||\
		hdiskBlockRw->secNum == 0 || \
		hdiskBlockRw->buflen == 0)
	{
		hdiskBlockRw->errSec = hdiskBlockRw->startSec;
		hdiskBlockRw->errCode = RW_DISK_PARA_ERROR;
		return -1;
	}
	
	if(judge_if_readwrite_overboundry(hdiskBlockRw))
	{
		eLog("readwrite out of  boundry!");
		hdiskBlockRw->errCode = RW_DISK_OVER_BOUND;
		return -1;
	}
	
	DDWORD offset = hdiskBlockRw->startSec * SECTOR_SIZE;
	DDWORD dataSize = hdiskBlockRw->secNum * SECTOR_SIZE;
	if(dataSize > hdiskBlockRw->buflen) //读写扇区的数据量大于缓冲区长度，防止内存访问越界崩溃
	{
		eLog("buflen is too small!");
		hdiskBlockRw->errCode = RW_DISK_TOO_SMALL_BUFFER;
		hdiskBlockRw->errSec = hdiskBlockRw->startSec;
		return -1;
	}
	
	if(lseek64(hdiskBlockRw->diskfd, offset, SEEK_SET) == -1)
	{
		eLog("lseek offset failed!");
		assert(0);
		hdiskBlockRw->errCode = RW_DISK_LSEEK_ERROR;
		hdiskBlockRw->errSec = hdiskBlockRw->startSec;
		return -1;
	}
	
	//读写硬盘
	if(hdiskBlockRw->readorwrite == DISK_READ)
	{
		if(read(hdiskBlockRw->diskfd, hdiskBlockRw->buf, dataSize) != dataSize)
		{
			eLog("read hdisk sector failed!");
			assert(0);
			hdiskBlockRw->errCode = RW_DISK_READ_ERROR;
			hdiskBlockRw->errSec = hdiskBlockRw->startSec;
			return -1;
		}
	}
	else if(hdiskBlockRw->readorwrite == DISK_WRITE)
	{
		if(write(hdiskBlockRw->diskfd, hdiskBlockRw->buf, dataSize) != dataSize)
		{
			eLog("write hdisk sector failed!");
			assert(0);
			hdiskBlockRw->errCode = RW_DISK_WRITE_ERROR;
			hdiskBlockRw->errSec = hdiskBlockRw->startSec;
			return -1;
		}
	}
	else
	{
		eLog("Unknown operate flag[%x]!", hdiskBlockRw->readorwrite);
		hdiskBlockRw->errCode = RW_DISK_PARA_ERROR;
		return -1;
	}
	
	hdiskBlockRw->errCode = RW_DISK_OK;
	return 0;
}

/*****************************************************************************
 * 函 数 名  : hdisk_4kblock_readwrite
 * 负 责 人  : Zhangsheng
 * 创建日期  : 2017年9月28日
 * 函数功能  :  64扇区数据写硬盘的接口，已优化
 * 输入参数  :
 				fp_disk		磁盘对应的文件描述符
 				logsec 		需要写到硬盘的起始扇区号
 				blocksize 	写入硬盘数据的大小
 				buff			 数据缓存

 * 输出参数  : 无
 * 返 回 值  :
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
static int hdisk_4kblock_readwrite(HDISK_BLOCK_RW * hdiskBlockRw)
{
	if(MOD(hdiskBlockRw->startSec, 8) != 0)
	{
		eLog("Start sector is not 4kb align!");
		hdiskBlockRw->errCode = RW_DISK_STARTSEC_NON_4K_ALIGN;
		hdiskBlockRw->errSec = hdiskBlockRw->startSec;
		return -1;
	}
	
	if(MOD(hdiskBlockRw->secNum, 8) != 0)
	{
		eLog("Sector number is not 4kb align!");
		hdiskBlockRw->errCode = RW_DISK_SECNUM_NON_4K_ALIGN;
		hdiskBlockRw->errSec = hdiskBlockRw->secNum;
		return -1;
	}
	
	return hdisk_readorwrite(hdiskBlockRw);
}

/*
* 对非4kb对齐的扇区参数进行调整，满足4kb读写的要求
*/
static int adjust_non4kblock_parameters(HDISK_RW * non4kRw)
{
	if(MOD(non4kRw->hdiskBlockRw.startSec, 8) == 0)	//起始扇区是4kb对齐
		non4kRw->startSec_adjust = non4kRw->hdiskBlockRw.startSec;
	else 						//起始扇区非4kb对齐
		non4kRw->startSec_adjust = non4kRw->hdiskBlockRw.startSec < 8 ? 0 : FLOOR(non4kRw->hdiskBlockRw.startSec, 8);//起始扇区向前扩展到最近的4kb

	if(MOD(non4kRw->hdiskBlockRw.endSec + 1, 8) == 0) 	//结束扇区数是4kb-1对齐
		non4kRw->endSec_adjust = non4kRw->hdiskBlockRw.endSec;
	else 						//结束扇区数非4kb对齐
		non4kRw->endSec_adjust = CEILING(non4kRw->hdiskBlockRw.endSec + 1, 8 ) - 1; ////结束扇区向后扩展到最近的4kb
	
	non4kRw->secNum_adjust = non4kRw->endSec_adjust + 1 -non4kRw->startSec_adjust; //重新计算总扇区数
	if((non4kRw->endSec_adjust + 1) > non4kRw->hdiskBlockRw.diskCapability) //调整后判断结束扇区有没有超出硬盘容量，超过的话不能使用4k读写接口
	{
		non4kRw->endSec_adjust = non4kRw->hdiskBlockRw.diskCapability - 1;
		non4kRw->secNum_adjust = non4kRw->hdiskBlockRw.diskCapability -non4kRw->startSec_adjust;
		
		hLog("non4k adjust:[%llu ->%llu<- %llu] ==> [%llu ->%llu<- %llu]",  non4kRw->hdiskBlockRw.startSec, non4kRw->hdiskBlockRw.secNum, non4kRw->hdiskBlockRw.endSec, non4kRw->startSec_adjust, non4kRw->secNum_adjust, non4kRw->endSec_adjust);
		return 1;
	}
	else //调整后起始扇区和扇区总数均为4kb对齐，可以直接调用4k读写接口
	{
		hLog("non4k adjust:[%llu ->%llu<- %llu] ==> [%llu ->%llu<- %llu]",  non4kRw->hdiskBlockRw.startSec, non4kRw->hdiskBlockRw.secNum, non4kRw->hdiskBlockRw.endSec, non4kRw->startSec_adjust, non4kRw->secNum_adjust, non4kRw->endSec_adjust);
		return 0;
	}
}

static int hdisk_non4k_readwrite(HDISK_RW * non4kRw)
{
	int ret = 0;
	int startSec_offset = non4kRw->hdiskBlockRw.startSec - non4kRw->startSec_adjust;
	DDWORD rwLen = non4kRw->hdiskBlockRw.secNum * SECTOR_SIZE;
	if(rwLen > non4kRw->hdiskBlockRw.buflen) //读写扇区的数据量大于缓冲区长度，防止内存访问越界崩溃
	{
		eLog("buflen is too small!");
		non4kRw->hdiskBlockRw.errCode = RW_DISK_TOO_SMALL_BUFFER;
		non4kRw->hdiskBlockRw.errSec = non4kRw->hdiskBlockRw.startSec;
		return -1;
	}
	
	DDWORD tmpLen = non4kRw->secNum_adjust * SECTOR_SIZE;
	DDWORD tmpOffset = startSec_offset * SECTOR_SIZE;
	BYTE * tmpBuf = (BYTE *) malloc(tmpLen * sizeof(BYTE));
	if(tmpBuf == NULL)
	{
		eLog("tmpbuf malloc [%s] failed!", tmpLen);
		assert(0);
		non4kRw->hdiskBlockRw.errCode = RW_DISK_MALLOC_MEM_FAILED;
		return -1;
	}

	HDISK_BLOCK_RW hdiskBlockRw;
	memcpy(&hdiskBlockRw, &non4kRw->hdiskBlockRw, sizeof(HDISK_BLOCK_RW));
	//需要将调整后的扇区参数更新到核心读写参数区
	hdiskBlockRw.startSec = non4kRw->startSec_adjust;
	hdiskBlockRw.endSec = non4kRw->endSec_adjust;
	hdiskBlockRw.secNum = non4kRw->secNum_adjust;
	hdiskBlockRw.buf = tmpBuf;
	hdiskBlockRw.buflen = tmpLen;
	
	if(hdiskBlockRw.readorwrite == DISK_READ)
	{
		//hLog("readdisk: [%llu + %llu]", hdiskBlockRw.startSec, hdiskBlockRw.secNum);
		ret = non4kRw->hdisk_rw_func(&hdiskBlockRw);
		if(ret == -1)
		{
			if(tmpBuf != NULL)
				free(tmpBuf);

			return -1;
		}
		
		memcpy(non4kRw->hdiskBlockRw.buf, tmpBuf + tmpOffset, rwLen); //从tmpBuf中截取要读取的对应的扇区数据
		//hLog("pull out buffer: -->[%llu + %llu]", tmpOffset, rwLen);
	}
	else if(non4kRw->hdiskBlockRw.readorwrite == DISK_WRITE)
	{
		hdiskBlockRw.readorwrite = DISK_READ;
		//hLog("readdisk: [%llu + %llu]", hdiskBlockRw.startSec, hdiskBlockRw.secNum);
		ret = non4kRw->hdisk_rw_func(&hdiskBlockRw);
		if(ret == -1)
		{
			if(tmpBuf != NULL)
				free(tmpBuf);

			return -1;
		}
		
		memcpy(tmpBuf + tmpOffset, non4kRw->hdiskBlockRw.buf, rwLen); //将要写入的数据嵌入tmpBuf对应的位置
		//hLog("insert buffer: -->[%llu + %llu]", tmpOffset, rwLen);
		
		hdiskBlockRw.readorwrite = DISK_WRITE;
		//hLog("writedisk: [%llu + %llu]", hdiskBlockRw.startSec, hdiskBlockRw.secNum);
		ret = non4kRw->hdisk_rw_func(&hdiskBlockRw);
		if(ret == -1)
		{
			if(tmpBuf != NULL)
				free(tmpBuf);

			return -1;
		}
	}

	if(tmpBuf != NULL)
		free(tmpBuf);

	return 0;
}

/*****************************************************************************
 * 函 数 名  : hdisk_4kblock_readwrite
 * 负 责 人  : Zhangsheng
 * 创建日期  : 2017年9月28日
 * 函数功能  :  64扇区数据写硬盘的接口，已优化
 * 输入参数  :
 				fp_disk		磁盘对应的文件描述符
 				logsec 		需要写到硬盘的起始扇区号
 				blocksize 	写入硬盘数据的大小
 				buff			 数据缓存

 * 输出参数  : 无
 * 返 回 值  :
 * 调用关系  :
 * 其    它  :
*****************************************************************************/
DDWORD hdisk_non4kblock_readwrite(HDISK_RW * non4kRw)
{
	int ret = adjust_non4kblock_parameters(non4kRw);	
	if(ret == 0) //这种情况下的读写接口全部使用4kb对齐
		non4kRw->hdisk_rw_func = hdisk_4kblock_readwrite;
	else //这种情况下的读写接口全部使用基本读写接口
		non4kRw->hdisk_rw_func = hdisk_readorwrite;
	
	return hdisk_non4k_readwrite(non4kRw);
}


/*******************************************************************************************************************
 * 函 数 名  ：  readwrite_hdisk_data_block
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2019年5月14日
 * 函数功能  ：  支持4kb对齐和非4kb对齐的硬盘读写接口
 * 参数列表  ： 
        HDISK_RW * hdiskRw  硬盘读写参数结构指针
 * 返 回 值  ：  硬盘数据读写结果
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
int readwrite_hdisk_data_block(HDISK_RW * hdiskRw) 
{
	if(MOD(hdiskRw->hdiskBlockRw.startSec, 8) == 0 && MOD(hdiskRw->hdiskBlockRw.secNum, 8) == 0 ) 
		hdisk_4kblock_readwrite(&hdiskRw->hdiskBlockRw);//4kb对齐读写
	else
	{
		if(hdiskRw->non4kAdjust == TRUE)
			hdisk_non4kblock_readwrite(hdiskRw); //非4kb对齐（调整为4kb对齐后读写）
		else
			hdisk_readorwrite(&hdiskRw->hdiskBlockRw);//直接读写
	}
	
	return record_readwrite_error_info(&hdiskRw->hdiskBlockRw);  //错误记录在errCode里，此处无需返回值
}

/*******************************************************************************************************************
 * 函 数 名  ：  readwrite_hdisk_block
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2019年5月14日
 * 函数功能  ：  硬盘扇区数据块读写接口
 * 参数列表  ： 
        BYTE readorwrite       读或写操作
        int diskno             硬盘的索引（区分多硬盘）
        int diskfd             要读写的硬盘的描述符
        DDWORD diskCapability  要读写的硬盘的容量
        DDWORD startSec        读写数据的起始扇区
        DDWORD secNum          要读写的数据的扇区数量
        BYTE * buf             要读写的数据缓冲区
        DDWORD buflen          数据缓冲区的大小
        BYTE non4kAdjust       非4kb读写时是否需要调整为对齐
 * 返 回 值  ：  函数执行的结果
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
int readwrite_hdisk_block(BYTE readorwrite, int diskno, int diskfd, DDWORD diskCapability, DDWORD startSec, DDWORD secNum, BYTE * buf, DDWORD buflen, BYTE non4kAdjust)
{
	HDISK_RW hdiskRw;
	memset(&hdiskRw, 0x00, sizeof(HDISK_RW));
	hdiskRw.hdiskBlockRw.diskno = diskno;
	hdiskRw.hdiskBlockRw.readorwrite = readorwrite;
	hdiskRw.hdiskBlockRw.diskfd = diskfd;
	hdiskRw.hdiskBlockRw.startSec = startSec;
	hdiskRw.hdiskBlockRw.secNum = secNum;
	hdiskRw.hdiskBlockRw.endSec = hdiskRw.hdiskBlockRw.startSec + hdiskRw.hdiskBlockRw.secNum -1;
	hdiskRw.hdiskBlockRw.diskCapability = diskCapability;
	hdiskRw.hdiskBlockRw.buf = buf;
	hdiskRw.hdiskBlockRw.buflen = buflen;
	hdiskRw.non4kAdjust = non4kAdjust;
	
	return readwrite_hdisk_data_block(&hdiskRw);
}

