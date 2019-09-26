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
*   文 件 名 ： libstring.h 
*   文件描述 ：  
*   创建日期 ：2019年1月10日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __LIBSTRING_H__
#define __LIBSTRING_H__

#include <locale.h>
#include <wchar.h>
#include "libbase.h"
#include "log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define UCS_UNKNOWN 0x00
#define UCS_2 0x02
#define UCS_4 0x04

#define SET_POINT_TYPE(point, type) ((type *)(point))
#define GET_UCS_TYPE(ucs) ((ucs == UCS_4) ? DWORD : WORD)
#define GET_VAL_OF_UCS(word, dword, ucsType) (((ucsType) == UCS_2) ? (word) : (dword))

#pragma pack(1)

typedef struct NODE_SUBSTR
{
	char * substr;
	int offset;
	int len;
	struct NODE_SUBSTR * next;
} NODE_SUBSTR, *pNODE_SUBSTR;

typedef struct MATCH_RESULT
{
	int number;
	struct NODE_SUBSTR * matchList;
} MATCH_RESULT, *pMATCH_RESULT;


#pragma pack()

int GetUnicodeUcsType(const void * Unicode);
int GetUnicodeLen(const void * Unicode);

int StrUnicodeToUtf8(const void * Unicode, int UniSize, char * Utf8, int Utf8Size);
int StrUtf8ToUnicode(const char * Utf8, int Utf8Size, int UcsType, void * Unicode, int UniSize);

wchar_t * wcs_dup(wchar_t * src, int maxlen);
void wcs_free(wchar_t ** p);

char * str_dup(char *src, int maxlen);
 char * str_dup_printf(int maxlen, const char *fmt, ...);
 int str_cat(char * srcstr, char * catstr, int bounder);
 char * str_clear(char * s);
 char * str_erase(char * srcstr, int start, int eraselen, int buflen);
 int str_replace_assign_index_substr(char * basepath,char * srcStr, char * dstStr, int buflen, int index);
 int str_replace(char * basepath,char * srcStr, char * dstStr, int buflen, int indexFlag);
 int str_find_first_of(const char * basepath,char asg);
 int str_find_last_of(const char * basepath,char asg);
 void str_free(char ** p);
 char * str_insert(char * srcstr, char * instr, int position, int bounder);
 char * str_reverse(char * strarr);
 int array_reverse(BYTE * start, int len);
 int swap_mem(void * a, void *b, int len);
 void swap_value(BYTE *a, BYTE *b);
 char * unicode_to_ascii(CHAR16 *unicode, int unicode_size, char *string, int bufLen);

 void init_next_table(BYTE * parten, int * next, int len);
 int kmp_match(BYTE * src, int slen, BYTE * parten, int plen, int * next);

 int search_all_match_by_kmp(BYTE * src, int slen, BYTE * parten, int plen, int * position, int ptnum);
 int search_all_match_violent(BYTE * src, int slen, BYTE * parten, int plen, int * position, int ptnum);
 int search_assign_index_match_position(BYTE * src, int slen, BYTE * parten, int plen, int index);
 int get_assign_substr_by_regex_match(char * srcStr, const char * pattern, char * substring, int substr_len, int index);
 int get_substr_by_regex_match(char * srcStr, const char * pattern, char * substring, int substr_len);
 MATCH_RESULT * regex_match_all_substr(char * srcStr, const char * pattern);
 void show_substr(MATCH_RESULT * match_result);
 void destroy_substr(MATCH_RESULT ** match_result);

 char * itoa(int num, int radix, BYTE Itely, char * numstr,int buflen);
 char * byte2string(BYTE * bString, int len);



 void * memmove(void *dest, const void *src, size_t n);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __LIBSTRING_H__ */
