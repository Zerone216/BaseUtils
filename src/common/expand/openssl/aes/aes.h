/****************************************************************************************************************************
*
*   文 件 名 ： xdelta.h 
*   文件描述 ：使用aes加解密数据的功能接口头文件 
*   创建日期 ：2018年9月18日
*   版本号 ： 
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __AES_H__
#define __AES_H__

#include  <common/baselib/baselib.h> //编译时加参数 -I./（即common文件夹所在的路径）


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


//AES初始化字符向量
#define AES_INIT_IVECTOR "SamingTC-836595"

//AES加密模式
#define MODE_AES_CRYPT_ECB 0xa1
#define MODE_AES_CRYPT_CBC 0xa2
/* ------------注意：后面的加密模式未修改调试完成，勿用----------*/
#define MODE_AES_CRYPT_CFB1 0xa3
#define MODE_AES_CRYPT_CFB8 0xa4
#define MODE_AES_CRYPT_CFB128 0xa5
#define MODE_AES_CRYPT_OFB128 0xa6
#define MODE_AES_CRYPT_CTR128 0xa7
#define MODE_AES_CRYPT_IGE 0xa8
#define MODE_AES_CRYPT_BI_IGE 0xa9

//AES密钥长度类型
#define AES_KEY_BITS_128 128
#define AES_KEY_BITS_192 192
#define AES_KEY_BITS_256 256

//AES加解密结果返回值
#define AES_CRYPT_OK 0X00 
#define AES_CRYPT_NULL_POINTER 0X01
#define AES_CRYPT_SET_KEY_FAILED  0X02
#define AES_CRYPT_ERROR_KEY_BITS  0X03
#define AES_CRYPT_ERROR_CRYPT_MODE  0X04
#define AES_CRYPT_ERROR_SRC_DATA_LEN  0X05
#define AES_CRYPT_ERROR_CRYPT_FLAG 	0X06
 
#pragma pack(1)



#pragma pack()

int aes_encrypt_data(BYTE * databuf, int datalen,int crypt_mode, char * key, int keybits,unsigned char * outbuf, unsigned int * outlen);
int aes_decrypt_data(BYTE * databuf, int datalen,int crypt_mode, char * key, int keybits,unsigned char * outbuf, unsigned int * outlen);


#ifdef __cplusplus
#if __cplusplus
 }
#endif
#endif /* __cplusplus */
 
 
#endif /* __AES_H__ */
 
