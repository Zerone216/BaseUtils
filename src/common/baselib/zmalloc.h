/****************************************************************************************************************************
*
*   文 件 名 ： zmalloc.h 
*   文件描述 ：  
*   创建日期 ：2019年6月4日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __ZMALLOC_H__
#define __ZMALLOC_H__

#include "libtime.h"
#include "libstring.h"

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

#define BZero(p, s) memset((p), 0, (s))
#define BOne(p, s) memset((p), 1, (s))

#define z_malloc(s) zs_malloc(__FILE__, __LINE__, s)
#define z_free(p) zs_free(__FILE__, __LINE__, DP_VOID(p))  //DP_VOID() 强转为 void **，否则编译有警告
#define z_extAlloc(p, cLen, eLen) zs_extAlloc(DP_VOID(p), cLen, eLen)

#pragma pack(1)

#pragma pack()

void * zs_malloc(char * file, int line, size_t bufLen);
void zs_free(char * file, int line, void ** p);
void * zs_extAlloc(void ** p, size_t curLen, size_t extLen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __ZMALLOC_H__ */
