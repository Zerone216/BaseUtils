/****************************************************************************************************************************
*
*   文 件 名 ： liberror.c 
*   文件描述 ：  
*   创建日期 ：2019年1月28日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#include "liberror.h"


/* Print a message and return to caller.
 * Caller specifies "errnoflag" and "level". */
static void err_doit(int errnoflag, int level, const char *fmt, va_list ap)
{
	int errno_save = errno;/* value caller might want printed */
	char buf[MAXLEN] = {'\0'};
	
	vsnprintf(buf, sizeof(buf), fmt, ap);	/* this is safe */
	
	int len = strlen(buf);
	if(errnoflag)
		snprintf(buf + len, sizeof(buf) - len, "[Error: %s]", strerror(errno_save));
	
	strcat(buf, "\n");
	syslog(level, "%s", buf);
	return;
}

/* Nonfatal error related to a system call.
 * Print a message and return. */
void err_ret(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, LOG_INFO, fmt, ap);
	va_end(ap);
	
	return;
}

/* Fatal error related to a system call.
 * Print a message and terminate. */
void err_sys(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, LOG_ERR, fmt, ap);
	va_end(ap);
	
	exit(1);
}

/* Fatal error related to a system call.
 * Print a message, dump core, and terminate. */
void err_dump(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, LOG_ERR, fmt, ap);
	va_end(ap);
	
	abort();		/* dump core and terminate */
	exit(1);		/* shouldn't get here */
}

/* Nonfatal error unrelated to a system call.
 * Print a message and return. */
void err_msg(const char *fmt, ...)
{
	va_list ap;
	
	va_start(ap, fmt);
	err_doit(0, LOG_INFO, fmt, ap);
	va_end(ap);
	
	return;
}

/* Fatal error unrelated to a system call.
 * Print a message and terminate. */
void err_quit(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	err_doit(0, LOG_ERR, fmt, ap);
	va_end(ap);
	
	exit(1);
}

