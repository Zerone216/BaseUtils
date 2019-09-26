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
*   文 件 名 ： libsignal.h 
*   文件描述 ：  
*   创建日期 ：2019年1月18日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __LIBSIGNAL_H__
#define __LIBSIGNAL_H__

#include "log.h"
#include "liberror.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*---------------------------------宏定义---------------------------------*/


#define	WBLKHANG	0	/* Do block waiting.  */

#define BYTE2INT(x) (((x) >= 128) ? ((x)- 256 ): (x))



#pragma pack(1)




#pragma pack()

 void ignore_sig_cld(int pi);
 void sig_cld(int pi);
 void sig_int(int pi);
 void debug_backtrace(void);

 pid_t create_new_process(void (*funcProc)(void *), void * dataProc);
 void child_process_join();
 int wait_process_exit(pid_t pid);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __LIBSIGNAL_H__ */
