/****************************************************************************************************************************
*
*   文 件 名 ： openssl.h 
*   文件描述 ： 数据加解密和校验模块头文件
*   创建日期 ：2018年9月12日
*   版本号 ： 
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __V_OPENSSL_H__
#define __V_OPENSSL_H__

#include<openssl/ssl.h>

#include "aes/aes.h"
#include "des/des.h"

#include "base64/base64.h"
//#include "bf/bf.h"
//#include "blowfish/blowfish.h"
//#include "camellia/camellia.h"
//#include "cast/cast.h"
#include "crc/crc.h"
//#include "huffman/huffman.h"
#include "md/md.h"
//#include "rc2/rc2.h"
//#include "rc4/rc4.h"
//#include "rmd160/rmd160.h"
//#include "rsa/rsa.h"
//#include "seed/seed.h"
#include "sha/sha.h"

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




#pragma pack(1)




#pragma pack()


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __OPENSSL_H__ */
