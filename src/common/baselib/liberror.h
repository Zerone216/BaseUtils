/****************************************************************************************************************************
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
