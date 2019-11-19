/****************************************************************************************************************************
*
*   文 件 名 ： aes.c 
*   文件描述 ：使用aes加解密数据的功能接口源文件 
*   创建日期 ：2018年9月18日
*   版本号 ： 
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

#include <openssl/aes.h>

#include "aes.h"


/*******************************************************************************************************************
 * 函 数 名  ：  check_key_bits
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月18日
 * 函数功能  ：  对要生成的密钥长度值进行检测
 * 参数列表  ： 
        int keybits  生成的密钥长度
 * 返 回 值  ：  检测结果
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
static int check_key_bits(int keybits)
{
	if(AES_KEY_BITS_128 == keybits ||AES_KEY_BITS_192 == keybits ||AES_KEY_BITS_256 == keybits)
		return 0;
	else
		return -1;
}


/*******************************************************************************************************************
 * 函 数 名  ：  aes_set_crypt_key
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月18日
 * 函数功能  ：  根据明文密码和加解密标记产生对应的密钥
 * 参数列表  ： 
        char * key        加解密的密码
        int keybits       产生的密钥的长度
        const int crypt   加解密标记   
        AES_KEY * aeskey  产生的密钥
 * 返 回 值  ：  函数执行结果
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
static int aes_set_crypt_key(char * key, int keybits, const int crypt, AES_KEY * aeskey)
{
	if(AES_ENCRYPT == crypt)
	{
		if(AES_set_encrypt_key((unsigned char*)key, keybits, aeskey) < 0)  
		{  
		    return -1;
		}
	}
	else if(AES_DECRYPT == crypt)
	{
		if(AES_set_decrypt_key((unsigned char*)key, keybits, aeskey) < 0)  
		{  
		    return -1;
		}
	}
	else
	{
		return -1;
	}
	
	return 0;
}

/*******************************************************************************************************************
 * 函 数 名  ：  aes_ecb_crypt_data
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月19日
 * 函数功能  ：  使用AES算法ECB模式对block_num个数据块加解密
 * 参数列表  ： 
        const unsigned char *databuf  要加解密的数据缓冲区指针
        unsigned char *outbuf		  加解密后的数据   
        size_t block_num              以AES_BLOCK_SIZE为单位的数据块的个数
        const AES_KEY * aeskey        加解密密钥
        const int crypt               加解密标记（AES_ENCRYPT      加密，AES_DECRYPT 解密）
 * 返 回 值  ：  暂无意义
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
static void aes_ecb_crypt_data(const unsigned char *databuf, unsigned char *outbuf, size_t block_num, const AES_KEY * aeskey , const int crypt)
{
	int i = 0;
	unsigned char out[AES_BLOCK_SIZE];
	int offset = 0;
	
	for(i = 0; i < block_num; i ++)
	{
		offset = i * AES_BLOCK_SIZE;
		AES_ecb_encrypt((const unsigned char *)databuf + offset, (unsigned char *)out, aeskey, crypt);
		memcpy(outbuf + offset, out, AES_BLOCK_SIZE);
	}

	return ;
}

/*******************************************************************************************************************
 * 函 数 名  ：  aes_ecb_crypt
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月18日
 * 函数功能  ：  使用AES算法ECB模式对指定的数据块进行加解密
 * 参数列表  ： 
        const unsigned char *databuf  要加解密的数据缓冲区指针
        unsigned char *outbuf		  加解密后的数据
        size_t datalen     			  要加解密的数据块长度
        const AES_KEY * aeskey        产生的密钥
        int crypt      	              加解密标记（AES_ENCRYPT      加密，AES_DECRYPT 解密）
 * 返 回 值  ：  加解密后的数据长度
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
static unsigned int aes_ecb_crypt(const unsigned char *databuf, unsigned char *outbuf, size_t datalen, const AES_KEY * aeskey, const int crypt)
{
	int block_num = datalen / AES_BLOCK_SIZE;
	int remain = datalen % AES_BLOCK_SIZE;
	
	size_t outlen = datalen;
	//Log("block_num = [%d], remain = [%d]",block_num ,remain);
	
	if(remain == 0)
	{
		aes_ecb_crypt_data(databuf, outbuf, block_num, aeskey, crypt);
	}
	else 	//不是16字节的整数倍则需要PKCS7填充，加密后数据长度增加remain个字节
	{	
		block_num += 1;
		outlen = block_num * AES_BLOCK_SIZE;
		unsigned char padding = AES_BLOCK_SIZE - remain;  //需要填充的数值，也是填充的字节数
		//Log("outlen = [%d], padding = [%d]",outlen, padding);
		
		unsigned char * datatmp = (unsigned char *) malloc(outlen);
		memset(datatmp, padding, outlen);
		memcpy(datatmp, databuf, datalen);
		
		aes_ecb_crypt_data(datatmp, outbuf, block_num, aeskey, crypt);
		free(datatmp);
	}
	
	if(AES_DECRYPT == crypt )  
	{
		unsigned char padding = *(outbuf + outlen -1);

		if(padding >= 0 || padding < 16)
		{
			unsigned char tmp[AES_BLOCK_SIZE];
			memset(tmp, padding, AES_BLOCK_SIZE);
			
			if(memcmp(outbuf + outlen - padding , tmp, padding) == 0) //判断是否为填充
			{
				memset(outbuf + outlen - padding,0x00, padding);
				outlen -= padding;
				//Log("outlen = [%d], padding = [%d]",outlen, padding);
			}
		}
	}
	
	return outlen;
}


/*******************************************************************************************************************
 * 函 数 名  ：  aes_cbc_crypt
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月18日
 * 函数功能  ：  使用AES算法CBC模式对指定的数据块进行加解密
 * 参数列表  ： 
        const unsigned char *databuf  要加解密的数据缓冲区指针
        unsigned char *outbuf		  加解密后的数据
        size_t datalen     			  要加解密的数据块长度
        const AES_KEY * aeskey        产生的密钥
        unsigned char * ivec		  初始化字符串向量
        int crypt      	              加解密标记（AES_ENCRYPT      加密，AES_DECRYPT 解密）
 * 返 回 值  ：  加解密后的数据长度
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
static unsigned int aes_cbc_crypt(const unsigned char *databuf, unsigned char *outbuf, size_t datalen, const AES_KEY * aeskey, unsigned char *ivec, const int crypt)
{
	int block_num = datalen / AES_BLOCK_SIZE;
	int remain = datalen % AES_BLOCK_SIZE;
	unsigned int outlen = datalen;
	//Log("block_num = [%d], remain = [%d]",block_num ,remain);
	
	if(remain == 0)
	{
		AES_cbc_encrypt((const unsigned char *)databuf, (unsigned char *)outbuf, datalen, aeskey, ivec, crypt);
	}
	else
	{	
		block_num += 1;
		outlen = block_num * AES_BLOCK_SIZE;
		
		unsigned char padding = AES_BLOCK_SIZE - remain;
		//Log("outlen = [%d], padding = [%d]",outlen, padding);
		
		unsigned char * datatmp = (unsigned char *) malloc(outlen);
		memset(datatmp, padding, outlen);
		memcpy(datatmp, databuf, datalen);

		datalen = outlen;
		AES_cbc_encrypt((const unsigned char *)datatmp, (unsigned char *)outbuf, datalen, aeskey, ivec, crypt);
		free(datatmp);
	}
	
	
	if(AES_DECRYPT == crypt )  
	{
		unsigned char padding = *(outbuf + outlen -1);
		if(padding >= 0 || padding < 16)
		{
			unsigned char tmp[AES_BLOCK_SIZE];
			memset(tmp, padding, AES_BLOCK_SIZE);
			
			if(memcmp(outbuf + outlen - padding , tmp, padding) == 0) //判断是否为填充
			{
				memset(outbuf + outlen - padding,0x00, padding);
				outlen -= padding;
				//Log("outlen = [%d], padding = [%d]",outlen, padding);
			}
		}
	}
	
	return outlen;
}

/*******************************************************************************************************************
 * 函 数 名  ：  aes_cfb1_crypt
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月18日
 * 函数功能  ：  使用AES算法CFB1模式对指定的数据块进行加解密
 * 参数列表  ： 
        const unsigned char *databuf  要加解密的数据缓冲区指针
        unsigned char *outbuf		  加解密后的数据
        size_t datalen     			  要加解密的数据块长度
        const AES_KEY * aeskey        产生的密钥
        unsigned char * ivec          初始化字符串向量
        int * num                     加解密开始的位置
        int crypt      	              加解密标记（AES_ENCRYPT      加密，AES_DECRYPT 解密）
 * 返 回 值  ：  加解密后的数据长度
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
static unsigned int aes_cfb1_crypt(const unsigned char *databuf, unsigned char *outbuf, size_t datalen, const AES_KEY * aeskey, unsigned char *ivec, int * num, const int crypt)
{
	unsigned int outlen = datalen;
	AES_cfb1_encrypt((const unsigned char *)databuf, (unsigned char *)outbuf, datalen, aeskey, ivec, num, crypt);
	
	return outlen;
}

/*******************************************************************************************************************
 * 函 数 名  ：  aes_cfb8_crypt
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月18日
 * 函数功能  ：  使用AES算法CFB8模式对指定的数据块进行加解密
 * 参数列表  ： 
        const unsigned char *databuf  要加解密的数据缓冲区指针
        unsigned char *outbuf		  加解密后的数据
        size_t datalen     			  要加解密的数据块长度
        const AES_KEY * aeskey        产生的密钥
		unsigned char * ivec		  初始化字符串向量
		int * num					  加解密开始的位置
        int crypt      	              加解密标记（AES_ENCRYPT      加密，AES_DECRYPT 解密）
 * 返 回 值  ：  加解密后的数据长度
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
static unsigned int aes_cfb8_crypt(const unsigned char *databuf, unsigned char *outbuf, size_t datalen, const AES_KEY * aeskey, unsigned char *ivec, int * num, const int crypt)
{
	unsigned int outlen = datalen;
	AES_cfb8_encrypt((const unsigned char *)databuf, (unsigned char *)outbuf, datalen, aeskey, ivec, num, crypt);
	
	return outlen;
}


/*******************************************************************************************************************
 * 函 数 名  ：  aes_cfb128_crypt
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月18日
 * 函数功能  ：  使用AES算法CFB128模式对指定的数据块进行加解密
 * 参数列表  ： 
        const unsigned char *databuf  要加解密的数据缓冲区指针
        unsigned char *outbuf		  加解密后的数据
        size_t datalen     			  要加解密的数据块长度
        const AES_KEY * aeskey        产生的密钥
        unsigned char * ivec          初始化字符串向量
        int * num                     加解密开始的位置
        int crypt      	              加解密标记（AES_ENCRYPT      加密，AES_DECRYPT 解密）
 * 返 回 值  ：  加解密后的数据长度
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
static unsigned int aes_cfb128_crypt(const unsigned char *databuf, unsigned char *outbuf, size_t datalen, const AES_KEY * aeskey, unsigned char *ivec, int * num, const int crypt)
{
	unsigned int outlen = datalen;
	AES_cfb128_encrypt((const unsigned char *)databuf, (unsigned char *)outbuf, datalen, aeskey, ivec, num, crypt);

	return outlen;
}


/*******************************************************************************************************************
 * 函 数 名  ：  aes_ctr128_crypt
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月18日
 * 函数功能  ：  使用AES算法CTR128模式对指定的数据块进行加解密
 * 参数列表  ： 
        const unsigned char *databuf  要加解密的数据缓冲区指针
        unsigned char *outbuf		  加解密后的数据
        size_t datalen     			  要加解密的数据块长度
        const AES_KEY * aeskey        产生的密钥
		unsigned char * ivec		  初始化字符串向量
		int * num					  加解密开始的位置
 * 返 回 值  ：  加解密后的数据长度
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
static unsigned int aes_ctr128_crypt(const unsigned char *databuf, unsigned char *outbuf, size_t datalen, const AES_KEY * aeskey, unsigned char *ivec, int * num)
{
	unsigned int outlen = datalen;
	unsigned char ecount[AES_BLOCK_SIZE];
	memset( ecount, '\0', AES_BLOCK_SIZE);
	AES_ctr128_encrypt((const unsigned char *)databuf, (unsigned char *)outbuf, datalen, aeskey, ivec , ecount, (unsigned int *)num);
	
	return outlen;
}

/*******************************************************************************************************************
 * 函 数 名  ：  aes_ofb128_crypt
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月18日
 * 函数功能  ：  使用AES算法OFB128模式对指定的数据块进行加解密
 * 参数列表  ： 
        const unsigned char *databuf  要加解密的数据缓冲区指针
        unsigned char *outbuf		  加解密后的数据
        size_t datalen     			  要加解密的数据块长度
        const AES_KEY * aeskey        产生的密钥
        unsigned char * ivec          初始化字符串向量
        int * num                     加解密开始的位置
 * 返 回 值  ： 加解密后的数据长度
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
static unsigned int aes_ofb128_crypt(const unsigned char *databuf, unsigned char *outbuf, size_t datalen, const AES_KEY * aeskey, unsigned char *ivec, int * num)
{
	unsigned int outlen = datalen;
	AES_ofb128_encrypt((const unsigned char *)databuf, (unsigned char *)outbuf, datalen, aeskey, ivec, num);
	return outlen;
}


/*******************************************************************************************************************
 * 函 数 名  ：  aes_ige_crypt
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月18日
 * 函数功能  ：  使用AES算法IGE模式对指定的数据块进行加解密
 * 参数列表  ： 
        const unsigned char *databuf  要加解密的数据缓冲区指针
        unsigned char *outbuf		  加解密后的数据
        size_t datalen     			  要加解密的数据块长度
        const AES_KEY * aeskey        产生的密钥
        unsigned char * ivec          初始化字符串向量
        int crypt      	              加解密标记（AES_ENCRYPT      加密，AES_DECRYPT 解密）
 * 返 回 值  ：  加解密后的数据长度
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
static unsigned int aes_ige_crypt(const unsigned char *databuf, unsigned char *outbuf, size_t datalen, const AES_KEY * aeskey, unsigned char *ivec, const int crypt)
{
	unsigned int outlen = datalen;
	AES_ige_encrypt((const unsigned char *)databuf, (unsigned char *)outbuf, datalen, aeskey, ivec, crypt);
	
	return outlen;
}


/*******************************************************************************************************************
 * 函 数 名  ：  aes_bi_ige_crypt
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月18日
 * 函数功能  ：  使用AES算法bi_ige模式对指定的数据块进行加解密
 * 参数列表  ： 
        const unsigned char *databuf  要加解密的数据缓冲区指针
        unsigned char *outbuf		  加解密后的数据
        size_t datalen     			  要加解密的数据块长度
        char * key                    加密的密码（字符串）
        int keybits                   生成的密钥长度（bits）
        const AES_KEY * aeskey        产生的密钥
        unsigned char * ivec          初始化字符串向量
        int crypt      	              加解密标记（AES_ENCRYPT      加密，AES_DECRYPT 解密）
 * 返 回 值  ： 加解密后的数据长度
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
static unsigned int aes_bi_ige_crypt(const unsigned char *databuf, unsigned char *outbuf, size_t datalen, char * key, int keybits, const AES_KEY * aeskey, unsigned char *ivec, const int crypt)
{
	unsigned int outlen = datalen;
	
	AES_KEY aeskey1;
	aes_set_crypt_key(str_reverse(key), keybits, crypt, &aeskey1);
	AES_bi_ige_encrypt((const unsigned char *)databuf, (unsigned char *)outbuf, datalen, aeskey, &aeskey1, ivec, crypt);
	
	return outlen;
}


/*******************************************************************************************************************
 * 函 数 名  ：  aes_crypt_data
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月18日
 * 函数功能  ：  使用AES算法对指定的数据块进行加解密
 * 参数列表  ： 
        BYTE * databuf  要处理的数据缓冲区指针
        int datalen     要处理的数据块长度
        int crypt_mode  加解密模式
        char * key      加密的密码（字符串）
        int keybits     生成的密钥长度（bits）
        int crypt      	加解密标记（AES_ENCRYPT      加密，AES_DECRYPT 解密）
 * 返 回 值  ：  数据加解密执行的结果
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
static int aes_crypt_data(BYTE * databuf, int datalen,int crypt_mode, char * key, int keybits, int crypt, unsigned char * outbuf, unsigned int * outlen)
{  
    if(!databuf || !key || !outbuf)
		return AES_CRYPT_NULL_POINTER;
	
	if(-1 == check_key_bits(keybits))
		return AES_CRYPT_ERROR_KEY_BITS;
	
    AES_KEY aeskey;
	if(aes_set_crypt_key(key, keybits, crypt, &aeskey) < 0)
		return AES_CRYPT_SET_KEY_FAILED;
	
	unsigned char ivec[AES_BLOCK_SIZE] = {0};
	strncpy((char *)ivec, AES_INIT_IVECTOR, AES_BLOCK_SIZE);
	
	int num = 0;
	switch(crypt_mode)
	{
		case MODE_AES_CRYPT_ECB:
			*outlen = aes_ecb_crypt((const unsigned char *)databuf, (unsigned char *)outbuf, datalen, &aeskey, crypt);
			break;
		case MODE_AES_CRYPT_CBC:
			*outlen = aes_cbc_crypt((const unsigned char *)databuf, (unsigned char *)outbuf, datalen, &aeskey, ivec, crypt);
			break;
		case MODE_AES_CRYPT_CFB1:
			*outlen = aes_cfb1_crypt((const unsigned char *)databuf, (unsigned char *)outbuf, datalen, &aeskey, ivec, &num, crypt);
			break;
		case MODE_AES_CRYPT_CFB8:
			*outlen = aes_cfb8_crypt((const unsigned char *)databuf, (unsigned char *)outbuf, datalen, &aeskey, ivec, &num, crypt);
			break;
		case MODE_AES_CRYPT_CFB128:
			*outlen = aes_cfb128_crypt((const unsigned char *)databuf, (unsigned char *)outbuf, datalen, &aeskey, ivec, &num, crypt);
			break;
		case MODE_AES_CRYPT_CTR128:
			*outlen = aes_ctr128_crypt((const unsigned char *)databuf, (unsigned char *)outbuf, datalen, &aeskey, ivec , &num);
			break;
		case MODE_AES_CRYPT_OFB128:
			*outlen = aes_ofb128_crypt((const unsigned char *)databuf, (unsigned char *)outbuf, datalen, &aeskey, ivec, &num);
			break;
		case MODE_AES_CRYPT_IGE:
			*outlen = aes_ige_crypt((const unsigned char *)databuf, (unsigned char *)outbuf, datalen, &aeskey, ivec, crypt);
			break;
		case MODE_AES_CRYPT_BI_IGE:
			*outlen = aes_bi_ige_crypt((const unsigned char *)databuf, (unsigned char *)outbuf, datalen, key, keybits, &aeskey, ivec, crypt);
			break;
		//////////////////////////////////////////////////////////////////////////////
		
		default:
			return AES_CRYPT_ERROR_CRYPT_MODE;
	}

    return AES_CRYPT_OK;
}  


/*******************************************************************************************************************
 * 函 数 名  ：  aes_encrypt_data
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月18日
 * 函数功能  ：  使用AES算法对指定的数据块进行加密
 * 参数列表  ： 
        BYTE * databuf  要加密的数据缓冲区指针
        int datalen     要加密的数据块长度
        int crypt_mode  加密模式
        char * key      加密的密码（字符串）
        int keybits     生成的密钥长度（bits）
 * 返 回 值  ：  数据加密执行的结果
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
int aes_encrypt_data(BYTE * databuf, int datalen,int crypt_mode, char * key, int keybits, unsigned char * outbuf, unsigned int * outlen)
{  
    return aes_crypt_data(databuf, datalen, crypt_mode, key, keybits, AES_ENCRYPT, outbuf, outlen);
}  

/*******************************************************************************************************************
 * 函 数 名  ：  aes_decrypt_data
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月18日
 * 函数功能  ：  使用AES算法对指定的数据块进行解密
 * 参数列表  ： 
        BYTE * databuf  要解密的数据缓冲区指针
        int datalen     要解密的数据块长度
        int crypt_mode  解密模式
        char * key      解密的密码（字符串）
        int keybits     生成的密钥长度（bits）
 * 返 回 值  ： 数据解密执行的结果
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
int aes_decrypt_data(BYTE * databuf, int datalen, int crypt_mode, char * key, int keybits ,unsigned char * outbuf, unsigned int * outlen)
{
	return aes_crypt_data(databuf, datalen, crypt_mode, key, keybits, AES_DECRYPT,outbuf, outlen);
}
