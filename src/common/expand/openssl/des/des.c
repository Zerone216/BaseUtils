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
*   文 件 名 ： des.c 
*   文件描述 ：VOI使用des加解密数据的功能接口源文件  
*   创建日期 ：2018年9月18日
*   版本号 ： 9.7.2.0.1 
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <openssl/des.h>
#include <openssl/pkcs7.h>

#include "des.h"

int des_encrypt_data(const char *_key, const char *_vt,char *_raw_ptr,size_t _raw_size, char **_dst_buf, size_t *_dst_size) 
{
    
    return 1;
}

int des_decrypt_data(const char *_key, const char *_vt,char *_raw_ptr,size_t _raw_size, char **_dst_buf, size_t *_dst_size) 
{
    
    return 1;
}

