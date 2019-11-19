/****************************************************************************************************************************
*
*   文 件 名 ： lzmacompress.c 
*   文件描述 ：VOI使用LZMA算法进行数据压缩的模块功能接口源文件 
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

#include "lzmacompress.h"

/*******************************************************************************************************************
 * 函 数 名  ：  deal_error_code
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月14日
 * 函数功能  ：  处理LZMA压缩出现的错误码，打印对应的错误信息
 * 参数列表  ： 
        int errcode  错误码
 * 返 回 值  ：  暂无意义
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
static void deal_error_code(int errcode)
{
	switch(errcode)
	{
		case SZ_OK:
			Log("OK!");
			break;
		case SZ_ERROR_MEM:
			Log(" Memory allocation error!");
			break;
		case SZ_ERROR_PARAM:
			Log("Incorrect paramater!");
			break;
		case SZ_ERROR_OUTPUT_EOF:
			Log("Output buffer overflow!");
			break;
		case SZ_ERROR_THREAD:
			Log("Errors in multithreading functions (only for Mt version)!");
			break;
		case SZ_ERROR_DATA:
			Log("Data error!");
			break;
		case SZ_ERROR_UNSUPPORTED:
			Log("Unsupported properties!");
			break;
		case SZ_ERROR_INPUT_EOF:
			Log("It needs more bytes in input buffer (src)!");
			break;
		case SZ_ERROR_CRC :
			Log("CRC check error!");
			break;
		case SZ_ERROR_READ :
			Log("Read error!");
			break;
		case SZ_ERROR_WRITE :
			Log("Write error!");
			break;
		case SZ_ERROR_PROGRESS :
			Log("Progress error!");
			break;
		case SZ_ERROR_FAIL :
			Log("Operate failed!");
			break;
		case SZ_ERROR_ARCHIVE :
			Log("Archive error!");
			break;
		case SZ_ERROR_NO_ARCHIVE :
			Log("No archive error!");
			break;
		
		default:
			Log("Unknown error!");
			break;
	}

	return;
}


/*******************************************************************************************************************
 * 函 数 名  ：  lzma_compress_data
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月13日
 * 函数功能  ：  使用LZMA算法对指定的数据块进行压缩
 * 参数列表  ： 
        BYTE * databuff    要压缩的数据缓冲区指针
        int datasize       要压缩的数据块大小
        int maxDstsize     压缩时分配的最大缓冲区长度
        unsigned char *outProps   
		size_t *outPropsSize  must be = 5
        int level          压缩级别: 0 ~ 9
		unsigned dictSize  The dictionary size in bytes. The maximum value is
				128 MB = (1 << 27) bytes for 32-bit version
				1 GB = (1 << 30) bytes for 64-bit version
				The default value is 16 MB = (1 << 24) bytes.
				It's recommended to use the dictionary that is larger than 4 KB and
				that can be calculated as (1 << N) or (3 << N) sizes.

		int lc             The number of literal context bits (high bits of previous literal).
				It can be in the range from 0 to 8. The default value is 3.
				Sometimes lc=4 gives the gain for big files.

		int lp             The number of literal pos bits (low bits of current position for literals).
				It can be in the range from 0 to 4. The default value is 0.
				The lp switch is intended for periodical data when the period is equal to 2^lp.
				For example, for 32-bit (4 bytes) periodical data you can use lp=2. Often it's
				better to set lc=0, if you change lp switch.

		int pb             The number of pos bits (low bits of current position).
				It can be in the range from 0 to 4. The default value is 2.
				The pb switch is intended for periodical data when the period is equal 2^pb.

		int fb - Word size (the number of fast bytes).
		     It can be in the range from 5 to 273. The default value is 32.
		     Usually, a big number gives a little bit better compression ratio and
		     slower compression process.
		
		int numThreads - The number of thereads. 1 or 2. The default value is 2.
		     Fast mode (algo = 0) can use only 1 thread.

		Please see the chart below:
		|--------------------------------------------
		|	level	dictSize	algo   	fb			
		|	0:     	16 KB   	0     	32			
		|	1:     	64 KB   	0    	32
		|	2:     	256 KB  	0    	32
		|	3:     	1 MB     	0    	32
		|	4:     	4 MB    	0    	32
		|	5:     	16 MB   	1    	32
		|	6:     	32 MB   	1    	32
		|	7+:   	64 MB   	1    	64
		| 
		|  The default value for "level" is 5.
		|
		|  algo = 0 means fast method
		|  algo = 1 means normal method
		---------------------------------------------|
		
 * 返 回 值  ：  压缩后的数据大小
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
int lzma_compress_data(BYTE * databuff, 
	int datasize, 
	int maxDstsize, 
	unsigned char *outProps, 
	size_t * outPropsSize, /* *outPropsSize must be = 5 */
	int level,      /* 0 <= level <= 9, default = 5 (LZMA_PROPS_SIZE)*/
	unsigned dictSize,  /* default = (1 << 24) */
	int lc,        /* 0 <= lc <= 8, default = 3  */
	int lp,        /* 0 <= lp <= 4, default = 0  */
	int pb,        /* 0 <= pb <= 4, default = 2  */
	int fb,        /* 5 <= fb <= 273, default = 32 */
	int numThreads /* 1 or 2, default = 2 */
	)
{
	unsigned long in_len = 0;
	unsigned long out_len = 0;
	BYTE * out = NULL;
	
	in_len = datasize;
	out = malloc(maxDstsize);
	
	int ret = LzmaCompress((unsigned char *)out, (size_t * )&out_len, (const unsigned char *)databuff, in_len, outProps, outPropsSize, level, dictSize, lc, lp, pb, fb, numThreads);
	if(ret != SZ_OK)
	{
		if(out != NULL)
			free(out);
		
		deal_error_code(ret);
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
 * 函 数 名  ：  lzma_decompress_data
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月13日
 * 函数功能  ：使用LZMA算法对压缩的数据块进行解压
 * 参数列表  ： 
        BYTE * databuff  要解压的数据缓冲区指针
        int datasize     要解压的数据块大小
        int maxDcprSize  解压时分配的最大数据缓冲区长度
 * 返 回 值  ：解压后的数据块大小  
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
int lzma_decompress_data(BYTE * databuff, 
	int datasize, 
	int maxDcprSize,
	const unsigned char * props, 
	size_t propsSize
	)
{
	unsigned long in_len = 0;
	unsigned long out_len = 0;
	BYTE * out = NULL;
	
	in_len = datasize;
	out = malloc(maxDcprSize);
	memset(out, 0, maxDcprSize);

	int ret = LzmaUncompress((unsigned char *) out, (size_t *) &out_len, (const unsigned char *) databuff, (SizeT *)&in_len, props, propsSize);
	if(ret <= 0)
	{
		if(out != NULL)
			free(out);
		
		deal_error_code(ret);
		return 0;
	}
	
	memset(databuff, 0, in_len);
	memcpy(databuff, out, out_len);

	free(out);
	return out_len;
}


