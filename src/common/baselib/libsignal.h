/****************************************************************************************************************************
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
