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
*   文 件 名 ： liberror.h 
*   文件描述 ：  
*   创建日期 ：2019年1月28日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __LIBERROR_H__
#define __LIBERROR_H__

#include "libbase.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


#define MAXLEN 10240

#define ErrClean() errno = 0
#define ErrCmp(err) ((errno) == (err) ? 1 : 0)
#define ErrExp() strerror(errno)

#pragma pack(1)

#pragma pack()

 void err_dump(const char *fmt, ...);
 void err_msg(const char *fmt, ...);
 void err_quit(const char *fmt, ...);
 void err_ret(const char *fmt, ...);
 void err_sys(const char *fmt, ...);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __LIBERROR_H__ */
