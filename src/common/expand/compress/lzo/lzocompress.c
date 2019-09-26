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
*   文 件 名 ： lzocompress.c 
*   文件描述 ：VOI使用lzo算法进行数据压缩的模块功能接口  
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

#include "lzocompress.h"


/*
Get other versions of LZO source please open Link: http://www.oberhumer.com/opensource/lzo/download/
+---------------------------------------------------------------------------+
 | Algorithm				Length		CxB 		ComLen	%Remn 	    Bits			Com K/s 		Dec K/s 	|
 | ---------				------	--- 		------- 	------	    ---- 		 -------		-------  |
 |																																						|
 | memcpy() 				224401		1 		224401		100.0 		8.00			60956.83		59124.58 |
 |																																						|
 | LZO1-1 					224401		1 		117362		53.1			4.25			4665.24 		13341.98 |
 | LZO1-99					224401		1 		101560		46.7			3.73			1373.29 		13823.40 |
 |																																						|
 | LZO1A-1					224401		1 		115174		51.7			4.14			4937.83 		14410.35 |
 | LZO1A-99 				224401		1 		99958 			45.5			3.64			1362.72 		14734.17 |
 |																																						|
 | LZO1B-1					224401		1 		109590		49.6			3.97			4565.53 		15438.34 |
 | LZO1B-2					224401		1 		106235		48.4			3.88			4297.33 		15492.79 |
 | LZO1B-3					224401		1 		104395		47.8			3.83			4018.21 		15373.52 |
 | LZO1B-4					224401		1 		104828		47.4			3.79			3024.48 		15100.11 |
 | LZO1B-5					224401		1 		102724		46.7			3.73			2827.82 		15427.62 |
 | LZO1B-6					224401		1 		101210		46.0			3.68			2615.96 		15325.68 |
 | LZO1B-7					224401		1 		101388		46.0			3.68			2430.89 		15361.47 |
 | LZO1B-8					224401		1 		99453 			45.2			3.62			2183.87 		15402.77 |
 | LZO1B-9					224401		1 		99118 			45.0			3.60			1677.06 		15069.60 |
 | LZO1B-99 				224401		1 		95399 			43.6			3.48			1286.87 		15656.11 |
 | LZO1B-999				224401		1 		83934 			39.1			3.13			232.40			16445.05 |
 |																																						|
 | LZO1C-1					224401		1 		111735		50.4			4.03			4883.08 		15570.91 |
 | LZO1C-2					224401		1 		108652		49.3			3.94			4424.24 		15733.14 |
 | LZO1C-3					224401		1 		106810		48.7			3.89			4127.65 		15645.69 |
 | LZO1C-4					224401		1 		105717		47.7			3.82			3007.92 		15346.44 |
 | LZO1C-5					224401		1 		103605		47.0			3.76			2829.15 		15153.88 |
 | LZO1C-6					224401		1 		102585		46.5			3.72			2631.37 		15257.58 |
 | LZO1C-7					224401		1 		101937		46.2			3.70			2378.57 		15492.49 |
 | LZO1C-8					224401		1 		100779		45.6			3.65			2171.93 		15386.07 |
 | LZO1C-9					224401		1 		100255		45.4			3.63			1691.44 		15194.68 |
 | LZO1C-99 				224401		1 		97252 			44.1			3.53			1462.88 		15341.37 |
 | LZO1C-999				224401		1 		87740 			40.2			3.21			306.44			16411.94 |
 |																																						|
 | LZO1F-1					224401		1 		113412		50.8			4.07			4755.97 		16074.12 |
 | LZO1F-999				224401		1 		89599 			40.3			3.23			280.68			16553.90 |
 |																																						|
 | LZO1X-1(11)			224401		1 		118810		52.6			4.21			4544.42 		15879.04 |
 | LZO1X-1(12)			224401		1 		113675		50.6			4.05			4411.15 		15721.59 |
 | LZO1X-1					224401		1 		109323		49.4			3.95			4991.76 		15584.89 |
 | LZO1X-1(15)			224401		1 		108500		49.1			3.93			5077.50 		15744.56 |
 | LZO1X-999				224401		1 		82854 			38.0			3.04			135.77			16548.48 |
 |																																						|
 | LZO1Y-1					224401		1 		110820		49.8			3.98			4952.52 		15638.82 |
 | LZO1Y-999				224401		1 		83614 			38.2			3.05			135.07			16385.40 |
 |																																						|
 | LZO1Z-999				224401		1 		83034 			38.0			3.04			133.31			10553.74 |
 |																																						|
 | LZO2A-999				224401		1 		87880 			40.0			3.20			301.21			8115.75 	|
 +--------------------------------------------------------------------------+
 */

/*******************************************************************************************************************
 * 函 数 名  ：  lzo_compress_data
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月13日
 * 函数功能  ：  使用lzo算法对指定的数据块进行压缩
 * 参数列表  ： 
        BYTE * databuff  需要压缩的数据缓冲区指针
        int datasize     需要压缩的数据块大小
 * 返 回 值  ：  压缩后的数据大小
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
int lzo_compress_data(BYTE * databuff, int datasize)
{
	int ret;
	unsigned long in_len = 0;
	unsigned long out_len = 0;
	BYTE wrkmem[65536]; //lzo: compression mem only need 64KB
	BYTE * out = NULL;

	in_len = datasize;
	memset(wrkmem, 0, 65536);

	if(lzo_init() != LZO_E_OK)
	{
		Log("internal error - lzo_init  failed !\nThis usually indicates a compiler bug - try recompiling\nwithout optimizations, and enable '-DLZO_DEBUG' for diagnostics.");
		return 0;
	}
	
	out_len = datasize + datasize / 16 + 64 + 3;
	out = malloc(out_len);
	
	ret = lzo1x_1_compress(databuff, in_len, out, &out_len, wrkmem);
	if(ret != LZO_E_OK)
	{
		if(out != NULL) 	
			free(out);

		Log("internal error - compression failed: %d", ret);
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
 * 函 数 名  ：  lzo_decompress_data
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月13日
 * 函数功能  ：  使用lzo算法对压缩的数据进行解压
 * 参数列表  ： 
        BYTE * databuff  需要解压的数据缓冲区指针
        int datasize     需要解压的数据块大小
        int minlen       解压数据后需要的最小缓冲区长度
 * 返 回 值  ：  数据解压后的大小
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
int lzo_decompress_data(BYTE * databuff, int datasize, int minlen)
{
	int ret = 0;
	unsigned long in_len = 0;
	unsigned long out_len = 0;
	BYTE * out = NULL;
	in_len = datasize;

	if(lzo_init() != LZO_E_OK)
	{
		Log("internal error - lzo_init failed !\nThis usually indicates a compiler bug - try recompiling\nwithout optimizations, and enable '-DLZO_DEBUG' for diagnostics.\n");
		return 0;
	}
	
	out = malloc(minlen);
	ret = lzo1x_decompress(databuff, in_len, out, &out_len, NULL);
	if(ret != LZO_E_OK)
	{
		free(out);
		Log("internal error - decompression failed: %d", ret);
		return 0;
	}
	
	memcpy(databuff, out, out_len);
	free(out);
	
	return out_len;
}





