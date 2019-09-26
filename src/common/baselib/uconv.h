/****************************************************************************************************************************
*
*	Copyright (c) 1998-2019  XI'AN SAMING TECHNOLOGY Co., Ltd
*	西安三茗科技股份有限公司  版权所有 1998-2019 
*
*   PROPRIETARY RIGHTS of XI'AN SAMING TECHNOLOGY Co., Ltd are involved in  the subject matter of this material.       
*   All manufacturing, reproduction,use, and sales rights pertaining to this subject matter are governed bythe license            
*   agreement. The recipient of this software implicitly accepts the terms of the license.	
*
*   本软件文档资料是西安三茗科技股份有限公司的资产,任何人士阅读和使用本资料必须获得相应的书面
*   授权,承担保密责任和接受相应的法律约束. 								     
*
*----------------------------------------------------------------------------------------------------------------------------
*
*   文 件 名 ： uconv.h 
*   文件描述 ：  
*   创建日期 ：2019年9月6日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __UCONV_H__
#define __UCONV_H__


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

/* Return code if invalid input after a shift sequence of n bytes was read.
(xxx_mbtowc) */
#define RET_SHIFT_ILSEQ(n)  (-1-2*(n))
/* Return code if invalid. (xxx_mbtowc) */
#define RET_ILSEQ           RET_SHIFT_ILSEQ(0)
/* Return code if only a shift sequence of n bytes was read. (xxx_mbtowc) */
#define RET_TOOFEW(n)       (-2-2*(n))

/* Return code if invalid. (xxx_wctomb) */
#define RET_ILUNI      -1
/* Return code if output buffer is too small. (xxx_wctomb, xxx_reset) */
#define RET_TOOSMALL   -2

typedef unsigned int ucs4_t;
typedef unsigned short  wchar;

#pragma pack(1)



#pragma pack()

 // @slen: string lenth, in characters. 
 // 	   if this parameter is -1, the function processes the entire input string, 
 // 		 excluding the terminating null character. 
 // @osize: the size of outbuf, in bytes.
 // return : if outbuf is null, return needed buf size, in bytes.
 // 		 otherwise return the size of converted data filled in outbuf, in bytes.
 int sconv_gbk_to_unicode(const char *gbkstr, int slen, wchar *outbuf, int osize);
 int sconv_unicode_to_gbk(const wchar *wstr, int wlen, char *outbuf, int osize);
 int sconv_unicode_to_utf8(const wchar *wstr, int wlen, char *outbuf, int osize);
 int sconv_utf8_to_unicode(const char *utf8str, int slen, wchar *outbuf, int osize);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __UCONV_H__ */
