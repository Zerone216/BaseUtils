/****************************************************************************************************************************
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

