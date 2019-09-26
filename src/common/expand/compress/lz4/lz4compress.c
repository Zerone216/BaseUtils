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
*   文 件 名 ： lz4compress.c 
*   文件描述 ：VOI使用LZ4算法进行数据压缩的模块功能接口源文件 
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

#include "lz4compress.h"

/*******************************************************************************************************************
 * 函 数 名  ：  lz4_compress_data
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月13日
 * 函数功能  ：  使用LZ4算法对指定的数据块进行压缩
 * 参数列表  ： 
        BYTE * databuff    要压缩的数据缓冲区指针
        int datasize       要压缩的数据块大小
        int maxDstsize     压缩时分配的最大缓冲区长度
 * 返 回 值  ：  压缩后的数据大小
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
int lz4_compress_data(BYTE * databuff, int datasize, int maxDstsize)
{
	unsigned long in_len = 0;
	unsigned long out_len = 0;
	BYTE * out = NULL;
	
	in_len = datasize;
	out = malloc(maxDstsize);
	
	out_len = LZ4_compress_default((char *)databuff, (char *)out,  in_len, maxDstsize);
	if(out_len <= 0)
	{
		if(out != NULL)
			free(out);
		
		Log("internal error - compression failed: %d", out_len);
		return 0;
	}
	
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
 * 函 数 名  ：  lz4_decompress_data
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月13日
 * 函数功能  ：使用LZ4算法对压缩的数据块进行解压
 * 参数列表  ： 
        BYTE * databuff  要解压的数据缓冲区指针
        int datasize     要解压的数据块大小
        int maxDcprSize  解压时分配的最大数据缓冲区长度
 * 返 回 值  ：解压后的数据块大小  
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
int lz4_decompress_data(BYTE * databuff, int datasize, int maxDcprSize)
{
	unsigned long in_len = 0;
	unsigned long out_len = 0;
	BYTE * out = NULL;
	
	in_len = datasize;
	out = malloc(maxDcprSize);
	memset(out, 0, maxDcprSize);
	
	out_len = LZ4_decompress_safe((char *)databuff, (char *)out, in_len, maxDcprSize);
	if(out_len <= 0)
	{
		if(out != NULL)
			free(out);
		
		Log("internal error - decompression failed: %d", out_len);
		return 0;
	}
	
	memset(databuff, 0, in_len);
	memcpy(databuff, out, out_len);

	free(out);
	return out_len;
}


