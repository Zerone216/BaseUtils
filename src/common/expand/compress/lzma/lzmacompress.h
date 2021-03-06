/****************************************************************************************************************************
*
*   文 件 名 ： lzmacompress.h 
*   文件描述 ：VOI使用LZMA算法进行数据压缩的模块功能接口头文件  
*   创建日期 ：2018年9月14日
*   版本号 ： 9.7.2.0.1 
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __LZMACOMPRESS_H__
#define __LZMACOMPRESS_H__

#include "lzmalib.h"
#include "../../../baselib/baselib.h"

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




#pragma pack(1)




#pragma pack()

 int lzma_compress_data(BYTE * databuff, 
         	int datasize, 
         	int maxDstsize, 
         	unsigned char *outProps, 
         	size_t *outPropsSize, /* *outPropsSize must be = 5 */
         	int level,      /* 0 <= level <= 9, default = 5 */
         	unsigned dictSize,  /* default = (1 << 24) */
         	int lc,        /* 0 <= lc <= 8, default = 3  */
         	int lp,        /* 0 <= lp <= 4, default = 0  */
         	int pb,        /* 0 <= pb <= 4, default = 2  */
         	int fb,        /* 5 <= fb <= 273, default = 32 */
         	int numThreads /* 1 or 2, default = 2 */
         	);
 int lzma_decompress_data(BYTE * databuff, 
         	int datasize, 
         	int maxDcprSize,
         	const unsigned char * props, 
         	size_t propsSize
         	);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __LZMACOMPRESS_H__ */
