/****************************************************************************************************************************
*
*   文 件 名 ： libfile.h 
*   文件描述 ：  
*   创建日期 ：2019年1月10日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __LIBFILE_H__
#define __LIBFILE_H__

#include "libbase.h"
#include "log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*---------------------------------宏定义---------------------------------*/

#define FILE_READ 0x01
#define FILE_WRITE 0x02

#pragma pack(1)

#pragma pack()

 int file_access(const char * filename,int mode);
 DDWORD get_file_size(const char *filepath);

 int append_buf_to_file(char * buf, int buflen, const char * filename , BYTE syncFlag);
 int read_file_to_buff(const char * filename, char * rwmode, int offset, char * buff, int buflen, BYTE newFlag);
int write_buff_to_file(char * buff, int buflen, const char * filename, char * rwmode, int offset, BYTE newFlag);
 int read_or_write_file(BYTE readwrite, char * rwmode, const char * filename, int offset, char * buff, int buflen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __LIBFILE_H__ */
