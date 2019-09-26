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
