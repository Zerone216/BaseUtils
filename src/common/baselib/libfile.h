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
