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
*   文 件 名 ： compression.h 
*   文件描述 ：VOI数据压缩模块相关功能接口头文件  
*   创建日期 ：2018年9月12日
*   版本号 ： 9.7.2.0.1 
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __COMPRESS_H__
#define __COMPRESS_H__

#include  <common/baselib/baselib.h> //编译时加参数 -I./（即common文件夹所在的路径）

#include "bz2/bz2compress.h"
#include "lz4/lz4compress.h"
#include "lzma/lzmacompress.h"
#include "lzo/lzocompress.h"
#include "zstd/zstdcompress.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*---------------------------------外部变量说明---------------------------------*/




/*---------------------------------外部函数原型说明---------------------------------*/




/*---------------------------------内部函数原型说明---------------------------------*/




/*---------------------------------全局变量---------------------------------*/




/*---------------------------------模块级变量---------------------------------*/




/*---------------------------------常量定义---------------------------------*/




/*---------------------------------宏定义---------------------------------*/

#define COMPRESSION_TYPE_BZ2 0Xfa
#define COMPRESSION_TYPE_LZ4 0Xfb
#define COMPRESSION_TYPE_LZMA 0Xfc
#define COMPRESSION_TYPE_LZO 0Xfd
#define COMPRESSION_TYPE_ZSTD 0Xfe

#define DECOMPRESSION_TYPE_BZ2 COMPRESSION_TYPE_BZ2
#define DECOMPRESSION_TYPE_LZ4 COMPRESSION_TYPE_LZ4
#define DECOMPRESSION_TYPE_LZMA COMPRESSION_TYPE_LZMA
#define DECOMPRESSION_TYPE_LZO COMPRESSION_TYPE_LZO
#define DECOMPRESSION_TYPE_ZSTD COMPRESSION_TYPE_ZSTD


#pragma pack(1)




#pragma pack()


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __COMPRESS_H__ */
