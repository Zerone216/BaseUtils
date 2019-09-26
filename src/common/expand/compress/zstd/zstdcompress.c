/****************************************************************************************************************************
*
*	Copyright (c) 1998-2018  XI'AN SAMING TECHNOLOGY Company 
*	西安三茗科技股份有限公司  版权所有 1998-2018 
*
*   PROPRIETARY RIGHTS of XI'AN SAMING TECHNOLOGY Company are involved in  the subject matter of this material.       
*   All manufacturing, reproduction,use, and sales rights pertaining to this subject matter are governed bythe license            
*   agreement. The recipient of this software implicitly accepts the terms of the license.	
*
*   本软件文档资料是西安三茗科技股份有限责任公司的资产,任何人士阅读和使用本资料必须获得相应的书面
*   授权,承担保密责任和接受相应的法律约束. 								     
*
*----------------------------------------------------------------------------------------------------------------------------
*
*   文 件 名 ： zstdcompress.c 
*   文件描述 ：VOI使用zstd算法进行数据压缩的模块功能接口 
*   创建日期 ：2018年9月13日
*   版本号 ： 9.7.2.0.1 
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <linux/hdreg.h>

#include "zstdcompress.h"

/*******************************************************************************************************************
 * 函 数 名  ：  zstd_compress_data
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月13日
 * 函数功能  ：  使用zstd压缩算法对指定的缓冲区数据进行压缩
 * 参数列表  ： 
        BYTE * databuff  要压缩的数据缓冲区指针
        int datasize     要压缩的数据缓冲区大小
        BYTE level       压缩级别（1~21，级别越高，压缩率越大，压缩速度越慢）
 * 返 回 值  ：  压缩后的数据大小
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
int zstd_compress_data(BYTE * databuff, int datasize, BYTE level)
{
	int ret;
	unsigned long in_len = 0;
	unsigned long out_len = 0;
	BYTE * out = NULL;

	in_len = datasize;
	out_len = ZSTD_compressBound(in_len);
	out = malloc(out_len);

	int compressionLevel = level; // range of compressionLevel: 1~21
	ret = ZSTD_compress(out, out_len, databuff, in_len , compressionLevel);

	if(ZSTD_isError(ret))
	{
		if(out != NULL)
			free(out);

		Log("internal error - compression failed: %d", ret);
		return 0;
	}

	out_len = ret;

	if(out_len >= in_len)
	{
		Log("This block contains incompressible data.");

		if(out != NULL)
			free(out);

		return 0;
	}

	memcpy(databuff, out, out_len);
	memset(databuff + out_len, 0, in_len - out_len);
	
	if(out != NULL)
		free(out);

	return out_len;
}



/*******************************************************************************************************************
 * 函 数 名  ：  zstd_decompress_data
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月13日
 * 函数功能  ：  使用zstd压缩算法对指定的压缩数据进行解压
 * 参数列表  ： 
        BYTE * databuff  要解压的数据缓冲区指针
        int datasize     要解压的数据缓冲区大小
        int minlen       指定的解压后分配的最小内存
 * 返 回 值  ：  解压后的数据大小
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
int zstd_decompress_data(BYTE * databuff, int datasize, int minlen)
{
	unsigned long in_len = 0;
	unsigned long out_len = 0;
	BYTE * out = NULL;

	in_len = datasize;
	out_len = ZSTD_getDecompressedSize(databuff, in_len); //MM_SIZE

	if(out_len < minlen)
		out_len = minlen;
	
	out = malloc(out_len);
	memset(out, 0, out_len);

	int dstsize = ZSTD_decompress(out, out_len, databuff, in_len);
	if(ZSTD_isError(dstsize))
	{
		if(out != NULL)
			free(out);

		Log("internal error - decompression failed: %d", dstsize);
		return 0;
	}
	
	memset(databuff, 0, in_len);
	memcpy(databuff, out, dstsize);

	free(out);
	return dstsize;
}

