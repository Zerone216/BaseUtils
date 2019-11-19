/****************************************************************************************************************************
*
*   文 件 名 ： libstring.c 
*   文件描述 ：  
*   创建日期 ：2019年1月10日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#include "libstring.h"


int GetUnicodeUcsType(const void * Unicode)
{
	if(Unicode == NULL)
		return UCS_UNKNOWN;
	
	WORD * uniVal2 = (WORD *)Unicode;
	DWORD * uniVal4 = (DWORD *)Unicode;
	
	if(*uniVal4 == 0)
		return UCS_UNKNOWN;
	else if(*uniVal4 > 0x7FFFFFFF) //超过UCS-4编码范围，高位两个字节和低位两个字节均在0x0000~0xFFFF范围内，是两个UCS-2字符
		return UCS_2;
	else //(0x00000001~ 0x7FFFFFFF)
	{
		if(*(uniVal2 + 1) == 0x00)
			return UCS_4;
		else
			return UCS_2;	//return GetUnicodeUcsType(Unicode + sizeof(DWORD));
	}
}

/*******************************************************************************************************************
 * 函 数 名  ：  GetUnicodeBytes
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2019年9月11日
 * 函数功能  ：获取一个Unicode字符转换为utf8对应的字节长度
 * 参数列表  ： 
        const unsigned short * Unicode   一个需要转换的Unicode字符
 * 返 回 值  ：  一个Unicode字符转换为utf8对应的字节长度
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
int GetUnicodeBytes(int UcsType, const void * Unicode)
{
	if(Unicode == NULL || UcsType == UCS_UNKNOWN)
		return 0;

	if(*(DWORD *)Unicode == 0xFFFFFFFF) //UCS-2出现0xFFFF
		return 0;
	
	DWORD uniVal = GET_VAL_OF_UCS(*(WORD *)Unicode, *(DWORD *)Unicode, UcsType);
	if(uniVal >= 0x00000001 &&uniVal <= 0x0000007F) //注：uniVal=0无意义,('\0')表示字符终结，1~127为ASCII
        return 1; 				// * U-00000000 - U-0000007F: 0xxxxxxx
    else if(uniVal >= 0x00000080 && uniVal <= 0x000007FF)
        return 2; 				// * U-00000080 - U-000007FF: 110xxxxx 10xxxxxx
    else if(uniVal >= 0x00000800 && uniVal <= 0x0000FFFF)
        return 3; 				// * U-00000800 - U-0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx
    else if(uniVal >= 0x00010000 && uniVal <= 0x001FFFFF)
        return 4; 				// * U-00010000 - U-001FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    else if(uniVal >= 0x00200000 && uniVal <= 0x03FFFFFF)
        return 5; 				// * U-00200000 - U-03FFFFFF: 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    else if(uniVal >= 0x04000000 && uniVal <= 0x7FFFFFFF)
        return 6; 				// * U-04000000 - U-7FFFFFFF: 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
   	else
    	return 0;
}


int GetUnicodeLen(const void * Unicode)
{
	if(Unicode == NULL)
		return 0;
	
	int uniLen = 0;
	int ucsType = GetUnicodeUcsType(Unicode);
	if(ucsType == UCS_UNKNOWN)
		return 0;
	
	int typeSize = GET_VAL_OF_UCS(sizeof(WORD), sizeof(DWORD), ucsType);
	while(GetUnicodeBytes(ucsType, Unicode + uniLen * typeSize))
		uniLen ++;
	
	return uniLen;
}


/*******************************************************************************************************************
 * 函 数 名  ：  UnicodeToUtf8
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2019年9月11日
 * 函数功能  ：  将一个Unicode字符转换为若干个UTF8字符(UTF8为变长字符编码，转换比范围为1 -> (1~6))
 * 参数列表  ： 
        const unsigned short * Unicode 	需要转换的Unicode
        char * Utf8      		接收转换后的uft8字符串的指针
        int Utf8Size              		接收转换后的uft8字符串的数组长度（数组的长度，
        															并非内存长度，防止越界）
 * 返 回 值  ：   
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
int UnicodeToUtf8(int UcsType, const void * Unicode, char * Utf8, int Utf8Size) 
{
	if(Unicode == NULL ||Utf8 == NULL || Utf8Size == 0 || UcsType == UCS_UNKNOWN)
		return 0;
	
	int uniBytes = GetUnicodeBytes(UcsType, Unicode);
	if(uniBytes > Utf8Size) //
		return 0;
	
	DWORD uniVal = GET_VAL_OF_UCS(*(WORD *)Unicode, *(DWORD *)Unicode, UcsType);
	
	switch(uniBytes)
	{
		case 1:
			*Utf8 = (uniVal & 0x7F);
			break;
		
		case 2:
			*(Utf8 + 1) = (uniVal & 0x3F) |0x80;
			*Utf8 = ((uniVal >> 6) & 0x1F) |0xC0;
			break;
		
		case 3:
			*(Utf8+2) = (uniVal & 0x3F) |0x80;
			*(Utf8+1) = ((uniVal >> 6) & 0x3F) |0x80;
			*Utf8 = ((uniVal >> 12) & 0x0F) |0xE0;
			break;
		
		case 4:
			*(Utf8+3) = (uniVal & 0x3F) | 0x80;
			*(Utf8+2) = ((uniVal >> 6) & 0x3F) | 0x80;
			*(Utf8+1) = ((uniVal >> 12) & 0x3F) | 0x80;
			*Utf8 = ((uniVal >> 18) & 0x07) | 0xF0;
			break;
		
		case 5:
			*(Utf8+4) = (uniVal & 0x3F) |0x80;
			*(Utf8+3) = ((uniVal >> 6) & 0x3F) |0x80;
			*(Utf8+2) = ((uniVal >> 12) & 0x3F) |0x80;
			*(Utf8+1) = ((uniVal >> 18) & 0x3F) |0x80;
			*Utf8 = ((uniVal >> 24) & 0x03) |0xF8;
			break;
		
		case 6:
			*(Utf8 + 5) = (uniVal & 0x3F) |0x80;
			*(Utf8 + 4) = ((uniVal >> 6) & 0x3F) |0x80;
			*(Utf8 + 3) = ((uniVal >> 12) & 0x3F) |0x80;
			*(Utf8 + 2) = ((uniVal >> 18) & 0x3F) |0x80;
			*(Utf8 + 1) = ((uniVal >> 24) & 0x3F) |0x80;
			*Utf8 = ((uniVal >> 30) & 0x01) |0xFC;
			break;
		
		default:
			uniBytes = 0;
			break;
	}
	
	return uniBytes;
} 

/*******************************************************************************************************************
 * 函 数 名  ：  StrUnicodeToUtf8
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2019年9月11日
 * 函数功能  ：  将一个字符的Unicode(UCS-2和UCS-4)编码转换成UTF-8编码.
 * 参数列表  ： 
        WORD * Unicode   	需要转换的源Unicode字符编码
        int UniSize     	Unicode字符编码的长度
        char * Utf8  		输出的uft8字符串编码
        int Utf8Size    	接收转换后的uft8字符串的数组长度（数组的长度，并非内存长度，
        																		防止越界）
 * 返 回 值  ：  转换后的utf8字符串指针
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
 * 注意:
 *     1. UTF8没有字节序问题, 但是Unicode有字节序要求;
 *        字节序分为大端(Big Endian)和小端(Little Endian)两种;
 *        在Intel处理器中采用小端法表示, 在此采用小端法表示. (低地址存低位)
 *     2. 请保证 pOutput 缓冲区有最少有 6 字节的空间大小!
*******************************************************************************************************************/
int StrUnicodeToUtf8(const void * Unicode, int UniSize, char * Utf8, int Utf8Size)
{
	if(Unicode == NULL || Utf8 == NULL || UniSize == 0 || Utf8Size == 0)
		return 0;
	
	int UcsType = GetUnicodeUcsType(Unicode);
	if(UcsType == UCS_UNKNOWN)
		return 0;
	
	int uft8Len = 0;
	int uniBytes = 0;
	memset(Utf8, '\0', Utf8Size);
	int typeSize = GET_VAL_OF_UCS(sizeof(WORD), sizeof(DWORD), UcsType);
	
	int i, j;
	for(i = 0, j = 0; i  < UniSize &&  j < Utf8Size; i ++, j += uniBytes)
	{
		if((uniBytes = UnicodeToUtf8(UcsType, Unicode + i * typeSize, Utf8 + j, Utf8Size - j)) <= 0)
		{
			memset(Utf8 + j, 0x00, sizeof(char));
			break;
		}
		
		uft8Len += uniBytes;
	}
	
	return uft8Len;
}



/*******************************************************************************************************************
 * 函 数 名  ：  GetUtf8Bytes
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2019年9月11日
 * 函数功能  ：  获取UFT-8字符转成Unicode对应的字节数
 * 参数列表  ： 
        unsigned char Utf8  输入的uft8字符
 * 返 回 值  ：  该字符转成Unicode对应的字节数
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
 * 注意:
*******************************************************************************************************************/
static int GetUtf8Bytes(unsigned char Utf8)
{
	if(Utf8 < 0x80) 
		return 0;      // 0xxxxxxx 返回0
	else if(Utf8 >= 0x80 && Utf8 < 0xC0) 
		return -1;     // 10xxxxxx 返回-1
	else if(Utf8 >= 0xC0 && Utf8 < 0xE0) 
		return 2;      // 110xxxxx 返回2
	else if(Utf8 >= 0xE0 && Utf8 < 0xF0) 
		return 3;      // 1110xxxx 返回3
	else if(Utf8 >= 0xF0 && Utf8 < 0xF8) 
		return 4;      // 11110xxx 返回4
	else if(Utf8 >= 0xF8 && Utf8 < 0xFC) 
		return 5;      // 111110xx 返回5
	else 
		return 6;      // 1111110x 返回6
}

/*******************************************************************************************************************
 * 函 数 名  ：  Utf8ToUnicode
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2019年9月11日
 * 函数功能  ：  将若干个（范围1~6）utf8字符转换为一个Unicode字符
 * 参数列表  ： 
        const char * Utf8         需要转换的utf8字符串指针
        unsigned short * Unicode  转换后的Unicode字符的指针
 * 返 回 值  ：  本次编码转换处理的utf8字符串的个数
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
 int Utf8ToUnicode(const char * Utf8, void * Unicode)
 {
 	if(Unicode == NULL ||Utf8 == NULL)
		return 0;
	
	char b1, b2, b3, b4, b5, b6; // b1 表示UTF-8编码的pInput中的高字节, b2 表示次高字节, ... 
	char * pInput = (char *)Utf8;
	unsigned char * pOutput = (unsigned char *)Unicode;
	int utfBytes = GetUtf8Bytes(*pInput);
	
	switch(utfBytes) 
	{
		case 0:
			*pOutput = *pInput;
			utfBytes += 1;
			break;
			
		case 2:
			b1 = *pInput;
			b2 = *(pInput + 1);
			if((b2 & 0xE0) != 0x80)
				return 0;

			*pOutput = (b1 << 6) + (b2 & 0x3F);
			*(pOutput+1) = (b1 >> 2) & 0x07;
			break;

		case 3:
			b1 = *pInput;
			b2 = *(pInput + 1);
			b3 = *(pInput + 2);
			if(((b2 & 0xC0) != 0x80) ||((b3 & 0xC0) != 0x80)) 
				return 0;

			*pOutput = (b2 << 6) + (b3 & 0x3F);
			*(pOutput+1) = (b1 << 4) + ((b2 >> 2) & 0x0F);
			break;

		case 4:
			b1 = *pInput;
			b2 = *(pInput + 1);
			b3 = *(pInput + 2);
			b4 = *(pInput + 3);
			if(((b2 & 0xC0) != 0x80) ||((b3 & 0xC0) != 0x80) ||\
				((b4 & 0xC0) != 0x80))
				return 0;
			
			*pOutput = (b3 << 6) + (b4 & 0x3F);
			*(pOutput+1) = (b2 << 4) + ((b3 >> 2) & 0x0F);
			*(pOutput+2) = ((b1 << 2) & 0x1C) + ((b2 >> 4) & 0x03);
			break;

		case 5:
			b1 = *pInput;
			b2 = *(pInput + 1);
			b3 = *(pInput + 2);
			b4 = *(pInput + 3);
			b5 = *(pInput + 4);
			if(((b2 & 0xC0) != 0x80) ||((b3 & 0xC0) != 0x80) ||\
				((b4 & 0xC0) != 0x80) ||((b5 & 0xC0) != 0x80))
				return 0;

			*pOutput = (b4 << 6) + (b5 & 0x3F);
			*(pOutput+1) = (b3 << 4) + ((b4 >> 2) & 0x0F);
			*(pOutput+2) = (b2 << 2) + ((b3 >> 4) & 0x03);
			*(pOutput+3) = (b1 << 6);
			break;

		case 6:
			b1 = *pInput;
			b2 = *(pInput + 1);
			b3 = *(pInput + 2);
			b4 = *(pInput + 3);
			b5 = *(pInput + 4);
			b6 = *(pInput + 5);
			if(((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) ||\
				((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80) ||\
				((b6 & 0xC0) != 0x80))
				return 0;
			
			*pOutput = (b5 << 6) + (b6 & 0x3F);
			*(pOutput+1) = (b5 << 4) + ((b6 >> 2) & 0x0F);
			*(pOutput+2) = (b3 << 2) + ((b4 >> 4) & 0x03);
			*(pOutput+3) = ((b1 << 6) & 0x40) + (b2 & 0x3F);
			break;
		
		default:
			utfBytes = 0;
			break;
	} 
	
	return utfBytes; 
 } 

 /*******************************************************************************************************************
 * 函 数 名  ：  StrUtf8ToUnicode
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2019年9月11日
 * 函数功能  ：  将一个字符的UTF8编码转换成Unicode(UCS-2和UCS-4)编码.
 * 参数列表  ： 
        const char * Utf8 	需要转换的源uft8字符编码
        int Utf8Size     	uft8字符编码的长度
        WORD * Unicode  	输出的Unicode字符串编码
        int UniSize    	输出的Unicode字符串的长度（数组的容量，防止越界）
 * 返 回 值  ：  转换后的Unicode字符串指针
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
 * 注意:
 *	   1. UTF8没有字节序问题, 但是Unicode有字节序要求;
 * 	   字节序分为大端(Big Endian)和小端(Little Endian)两种;
 * 	   在Intel处理器中采用小端法表示, 在此采用小端法表示. (低地址存低位)
*******************************************************************************************************************/
int StrUtf8ToUnicode(const char * Utf8, int Utf8Size, int UcsType, void * Unicode, int UniSize)
{
	if(Unicode == NULL || Utf8 == NULL || UniSize == 0 || Utf8Size == 0)
		return 0;

	if(UcsType == UCS_UNKNOWN)
		return 0;
	
	int uniLen = 0;
	int utf8Bytes = 0;
	int typeSize = GET_VAL_OF_UCS(sizeof(WORD), sizeof(DWORD), UcsType);
	memset(Unicode, '\0', UniSize * typeSize);// 把 outPutStr 初始化为全零
	
	int i, j;
	for(i = 0, j = 0; i < Utf8Size && j < UniSize; i += utf8Bytes, j ++)
	{
		if((utf8Bytes = Utf8ToUnicode(Utf8 + i, Unicode + j * typeSize)) <= 0)
		{
			memset(Unicode + j * typeSize, 0x00, typeSize);
			break;
		}
		
		uniLen ++;
	}
	
    return uniLen;
}

char * StrUnicodeToAscii(WORD * inPutStr, int inPutSize, char * outPutStr , int outPutSize)
{
	unsigned int *unicode = (unsigned int *) inPutStr;
	char *ascii = outPutStr;
	memset(outPutStr, '\0', outPutSize);// 把 outPutStr 初始化为全零
	
	inPutSize /= (sizeof(unsigned int)/sizeof(WORD));
	
	int i, j;
    for(i = 0; i < inPutSize && j < outPutSize; i ++)
    {
        if (unicode[i] <= 0x80)  //0-128
        {
            ascii[j ++] = unicode[i];
        }
        else if (unicode[i] < 0x800) //129-2048 Two-byte UTF-8
        {
            ascii[j ++] = 0xc0 | ((unicode[i] >> 6) & 0x1f);
            ascii[j ++] = 0x80 | (unicode[i] & 0x3f);
        }
        else if (unicode[i] < 0x10000)//Three-byte UTF-8
        {
            ascii[j ++] = 0xe0 | (unicode[i] >> 12);
            ascii[j ++] = 0x80 | ((unicode[i] >> 6) & 0x3f);
            ascii[j ++] = 0x80 | (unicode[i] & 0x3f);
        }
        else //Four-byte UTF-8
        {
            ascii[j ++] = 0xf0 | (unicode[i] >> 18);
            ascii[j ++] = 0x80 | ((unicode[i] >> 12) & 0x3f);
            ascii[j ++] = 0x80 | ((unicode[i] >> 6) & 0x3f);
            ascii[j ++] = 0x80 | (unicode[i] & 0x3f);
        }
    }
	
    return outPutStr;
}
 
WORD * StrAsciiToUnicode(char * inPutStr, int inPutSize, WORD * outPutStr , int outPutSize)
{
	char *ascii = inPutStr;
	unsigned int *unicode = (unsigned int *) outPutStr;
	memset(outPutStr, '\0', outPutSize);// 把 outPutStr 初始化为全零

	outPutSize /= (sizeof(unsigned int)/sizeof(WORD));
	
    int i, j;
    for(i = 0; i < inPutSize && j < outPutSize; j ++)
    {
        if ((ascii[i] & 0x80) == 0)
        {
            unicode[j] = ascii[i ++];
        }
        else if ((ascii[i] & 0xE0) == 0xB0)
        {
            unicode[j] = (ascii[i ++] & 0x1F) << 6;
            unicode[j] |= (ascii[i ++] & 0x3F);
        }
        else if ((ascii[i] & 0xF0) == 0xE0)
        {
            unicode[j] = (ascii[i ++] & 0x0F) << 12;
            unicode[j] |= ((ascii[i ++] & 0x3F) << 6);
            unicode[j] |= (ascii[i ++] & 0x3F);
        }
        else if ((ascii[i] & 0xF8) == 0xF0)
        {
            unicode[j] = (ascii[i ++]& 0x07) << 18;
            unicode[j] |= ((ascii[i ++] & 0x3F) << 12);
            unicode[j] |= ((ascii[i ++] & 0x3F) << 6);
            unicode[j] |= (ascii[i ++] & 0x3F);
        }
		else
			break;
    }
	
    return outPutStr;
}


/****************************************************************************
 * 	unicode_to_ascii
 *	Turns a string from Unicode into ASCII.
 *	Doesn't do a good job with any characters that are outside the normal
 *	ASCII range, but it's only for debugging...
 ****************************************************************************/
char * unicode_to_ascii(CHAR16 *unicode, int unicode_size, char *string, int bufLen)
{
	int i = 0;
	memset(string, '\0', bufLen);
	
	for(i = 0; i < unicode_size; ++i)
	{
		if(i >= bufLen)
			break;
		
		string[i] = (char)(unicode[i]);
	}
	
	return string;
}

int swap_mem(void * a, void *b, int len)
{
	BYTE * tmp = (BYTE *) malloc(len);
	if(tmp == NULL)
		return -1;
	
	memset(tmp, 0x00, len);
	memcpy(tmp, a, len);
	memcpy(a, b, len);
	memcpy(b, tmp, len);
	
	free(tmp);
	return 0;
}

void swap_value(BYTE *a, BYTE *b)
{
	BYTE tmp = *a;
	*a = *b;
	*b = tmp;

	return;
}

int str_find_first_of(const char * basepath,char asg)
{
	int len = strlen(basepath);
	int i = 0;
	int position = -1;
	
	for(i = 0;i < len; i++)
	{
		if(basepath[i] == asg)
		{
			position = i;
			break;
		}
	}
	
	return position;
}

int str_find_last_of(const char * basepath,char asg)
{
	int len = strlen(basepath);
	int i = 0;
	int position = -1;
	
	for(i = len - 1;i >= 0; i --)
	{
		if(basepath[i] == asg)
		{
			position = i;
			break;
		}
	}
	
	return position;
}

int str_replace_assign_index_substr(char * basepath,char * srcStr, char * dstStr, int buflen, int index)
{
	int deltaLen = strlen(dstStr) - strlen(srcStr);
	
	if(strnlen(basepath, buflen) + deltaLen >= buflen)
		return -1;
	else
	{
		int position = search_assign_index_match_position((BYTE * )basepath, strnlen(basepath, buflen), (BYTE * )srcStr, strlen(srcStr), index);
		if(position == -1)
		{
			iLog("Not found the index[%d] of \"%s\"!", index, srcStr);
			return -1;
		}
		else
		{
			str_erase(basepath, position, strlen(srcStr), buflen);
			str_insert(basepath, dstStr, position, buflen);
			return 0;
		}
	}
}


int str_replace(char * basepath,char * srcStr, char * dstStr, int buflen, int indexFlag)
{
	if(indexFlag > 0)
		return str_replace_assign_index_substr(basepath, srcStr, dstStr, buflen, indexFlag);
	else //all
	{
		while(TRUE)
		{
			if(-1 == str_replace_assign_index_substr(basepath, srcStr, dstStr, buflen, 1))
				return -1;
		}

		return 0;
	}
}

char * str_erase(char * srcstr, int start, int eraselen, int buflen)
{
	char * str = srcstr;
	int strlen = strnlen(srcstr, buflen);
	int end = start + eraselen - 1;
	
	if(eraselen > 0 && buflen > start)
	{
		if(strlen > end)
		{
			int cutlen = strlen - end - 1;
			memcpy(srcstr + start, srcstr + end + 1, cutlen);
			memset(srcstr + start + cutlen, '\0', eraselen);
		}
		else
		{
			memset(srcstr + start, '\0', strlen -start);
		}
	}
	
	return str;
}

char * str_dup(char *src, int maxlen)
{
	int len = strlen(src);

	char * str = (char *)malloc(maxlen);
	if(str == NULL)
		return NULL;

	//iLog("malloc pointer at addr: [-->0x%x +%d]", str, maxlen);
	memset(str, '\0', maxlen);
	memcpy(str, src, MIN(len, maxlen - 1));
	return str;
}

wchar_t * wcs_dup(wchar_t * src, int maxlen)
{
	int len = wcslen(src);
	maxlen *= sizeof(wchar_t);
	wchar_t * wcstr = (wchar_t *)malloc(maxlen);
	if(wcstr == NULL)
		return NULL;
	
	//iLog("malloc pointer at addr: [-->0x%x +%d]", str, maxlen);
	memset(wcstr, '\0', maxlen);	
	wcsncpy(wcstr, src, MIN(len, maxlen - 1));
	
	return wcstr;
}

void wcs_free(wchar_t ** p)
{
	if(*p != NULL)
	{
		//iLog("free strPointer at addr: [0x%x]", *p);
		free(*p);
		*p = NULL;
	}
	
	return;
}

char * str_dup_printf(int maxlen, const char *fmt, ...)
{
	char * strbuf = (char *)malloc(maxlen);
	if(strbuf == NULL)
		return NULL;
	
	memset(strbuf, '\0', maxlen);
	
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(strbuf, maxlen, fmt, ap);
    va_end(ap);

	return strbuf;
}

void str_free(char ** p)
{
	if(*p != NULL)
	{
		//iLog("free strPointer at addr: [0x%x]", *p);
		free(*p);
		*p = NULL;
	}
	
	return;
}

char * str_clear(char * s)
{
	char * str = s;
	memset(str, '\0', strlen(str) + 1);
	return str;
}

int str_cat(char * srcstr, char * catstr, int bounder)
{
	int srclen = strlen(srcstr);
	int catlen = strlen(catstr);
	
	int cpylen = MIN(catlen, bounder - srclen);
	memcpy(srcstr + srclen, catstr,cpylen);
	
	return cpylen;
}

char * str_insert(char * srcstr, char * instr, int position, int bounder)
{
	if(srcstr == NULL)
		return NULL;
	
	char * str = srcstr;
	if(instr == NULL )
		return str;

	int srclen = strlen(str);
	int inslen = strlen(instr);
	if(inslen == 0 || position >= bounder)
		return str;

	int spacelen = position - srclen;
	if(spacelen >= 0)
	{
		memset(str + srclen,' ', spacelen);
		str_cat(str,instr,bounder);
	}
	else
	{
		int offset = 0;
		if(position + inslen >= bounder)
		{
			memset(str + position, '\0', bounder - position);
			str_cat(str,instr,bounder);
		}
		else
		{	
			int tmplen = srclen - position + 1;
			offset += position;
			
			char * tmp = str_dup(str + offset, tmplen);
			memcpy(str + offset, instr, inslen);
			
			offset += inslen;
			memset(str + offset, '\0', bounder - offset);
			str_cat(str,tmp,bounder);
			
			str_free(&tmp);
		}
	}
	
	return srcstr;
}

char * itoa(int num, int radix, BYTE Itely, char * numstr,int buflen)
{
	memset(numstr,'\0',buflen);
	
	/*索引表*/
	char index[17] = {'\0'};
	
	if(Itely == TRUE)
		strncpy(index, "0123456789ABCDEF", 17);
	else
		strncpy(index, "0123456789abcdef", 17);

	unsigned unum;/*中间变量*/
	int i = 0, j, k;

	/*确定unum的值*/
	if(radix == 10 && num < 0) /*十进制负数*/
	{
		unum = (unsigned) - num;
		numstr[i++] = '-';
	}
	else
		unum = (unsigned)num; /*其他情况*/

	/*转换*/
	do
	{
		numstr[i++] = index[unum % (unsigned)radix];
		unum /= radix;
	}
	while(unum);

	numstr[i] = '\0';

	/*逆序*/
	if(numstr[0] == '-')
		k = 1; /*十进制负数*/
	else
		k = 0;

	char temp;
	for(j = k; j <= (i - 1) / 2; j++)
	{
		temp = numstr[j];
		numstr[j] = numstr[i - 1 + k - j];
		numstr[i - 1 + k - j] = temp;
	}

	return numstr;
}

char * str_reverse(char * strarr)
{
	int len = strlen(strarr);
	char * left= strarr;
	char * right = strarr + len - 1;
	
	while(left < right)
	{
		swap_mem(left,right, sizeof(char));
		
		left ++;
		right --;
	}
	
	return strarr;
}

int array_reverse(BYTE * start, int len)
{
	BYTE * left= start;
	BYTE * right = start + len - 1;
	
	while(left < right)
	{
		swap_mem(left, right, sizeof(BYTE));
		
		left ++;
		right --;
	}
	
	return 0;
}


char * ByteToString(char * dst, int dstLen, BYTE * src, int assignLen)
{
	int cpLen = MIN(dstLen - 1, assignLen);
	memset(dst, '\0', dstLen); 
	memcpy(dst, src, cpLen);
	
	return dst;
}

void init_next_table(BYTE *parten, int * next, int pLen)
{
	int preIndex = -1;
	int sufIndex = 0;
	next[0] = -1;
	while(sufIndex < pLen - 1)
	{
		if(preIndex == -1 || parten[preIndex] == parten[sufIndex])
			next[++ sufIndex] = ++ preIndex;
		else
			preIndex = next[preIndex];
	}
	
	return;
}

int kmp_match(BYTE * src, int slen, BYTE * parten, int plen, int * next)
{
	if(src ==NULL || parten == NULL || next == NULL)
		return -1;

	if(plen <= 0 || slen <= 0 ||plen > slen)
		return -1;
	
    int s_i = 0, p_i = 0;
    while( s_i < slen && p_i < plen )
    {
        if( src[ s_i ] == parten[ p_i ] )
        {
            s_i++;
            p_i++;
        }
        else
        {
            if( p_i == 0 )
                s_i++;
            else
                p_i = next[ p_i];
        }
    }
	
    return ( p_i == plen ) ? ( s_i - plen ) : -1;
}

//使用KMP算法搜索，高效率
int search_all_match_by_kmp(BYTE * src, int slen, BYTE * parten, int plen, int * position, int ptnum)
{
	if(src ==NULL || parten == NULL || position == NULL)
		return 0;

	if(plen <= 0 || slen <= 0 ||plen > slen)
		return 0;

	int next_len = plen;
	int * next = (int *) malloc(sizeof(int) * next_len);
	memset(next ,0x00, sizeof(int) * next_len);
	init_next_table(parten, next, next_len);
	
	int offset = 0;
	int num = 0;
	
	while(TRUE)
	{
		if(offset > (slen -plen) || num > ptnum)
			break;
		
		*(position + num) = kmp_match(src + offset ,slen - offset, parten, plen, next);
		if(*(position + num) == -1)
			break;
		
		*(position + num) += offset;
		if(*(position + num) > slen)
			break;
		
		hLog("find position = [%d]", *(position + num));
		
		offset = *(position + num) + 1;
		num ++;
	}
	
	free(next);
	
	return num;
}

//暴力搜索，按字节逐一匹配
int search_all_match_violent(BYTE * src, int slen, BYTE * parten, int plen, int * position, int ptnum)
{
	if(src ==NULL || parten == NULL || position == NULL)
		return 0;

	if(plen <= 0 || slen <= 0 ||plen > slen)
		return 0;
	
	int offset = 0;
	int num = 0;
	
	while(TRUE)
	{
		if(offset > (slen-plen) || num > ptnum)
			break;

		if(memcmp(src + offset, parten, plen) == 0)
		{
			*(position + num) = offset;
			
			iLog("find position = [%d]\n", *(position + num));
			
			num ++;
		}
		
		offset ++;
	}
	
	return num;
}

int search_assign_index_match_position(BYTE * src, int slen, BYTE * parten, int plen, int index)
{
	if(src ==NULL || parten == NULL)
		return 0;

	if(plen <= 0 || slen <= 0 ||plen > slen)
		return 0;

	int position = -1;
	int next_len = plen;
	int * next = (int *) malloc(sizeof(int) * next_len);
	memset(next ,0x00, sizeof(int) * next_len);
	init_next_table(parten, next, next_len);
	
	int offset = 0;
	int num = 0;
	
	while(num < index)
	{
		if(offset > (slen -plen))
			break;
		
		position = kmp_match(src + offset ,slen - offset, parten, plen, next);
		if(position == -1 )
			break;

		position += offset;
		hLog("find position = [%d]", position);
		
		offset = position + 1;
		num ++;
	}
	
	free(next);
	
	return position;
}

// 正则提取子串
static char* get_substr_by_pattern(char *s, regmatch_t *pmatch, char * buf, int buflen)
{
	memset(buf, '\0', buflen);
	memcpy(buf, s + pmatch->rm_so, pmatch->rm_eo - pmatch->rm_so);
	return buf;
}

int get_substr_by_regex_match(char * srcStr, const char * pattern, char * substring, int substr_len)
{
	regmatch_t pmatch;
	regex_t reg;
	memset(substring, 0x00, substr_len);
	regcomp(&reg, pattern, REG_EXTENDED);	//编译正则表达式
	
	int status = regexec(&reg, srcStr, 1, &pmatch, 0);/* 匹配正则表达式,regexec()函数一次只能匹配一个,不能连续匹配 */
	if(status == REG_NOMATCH)
	{
		hLog("Not Matched");
		regfree(&reg);
		return -1;
	}
	
	if(pmatch.rm_so == -1)
	{
		regfree(&reg);
		return -1;
	}
	
	char strtmp[1024] = {'\0'};
	strncpy(substring, get_substr_by_pattern(srcStr, &pmatch, strtmp, sizeof(strtmp)), substr_len);
	//hLog("Matched [start=%d, end=%d]: %s", pmatch.rm_so + 1, pmatch.rm_eo, substring);
	
	regfree(&reg);
	return 0;
}

/*
功能:通过正则表达式匹配长串获取需要的子串
参数:
	pattern:正则表达式
	longstring:匹配的源字符串
	substring:匹配到的子串
	substr_len:子串的最大长度
	index:需要获取的子串的索引值(从1开始)
*/
int get_assign_substr_by_regex_match(char * srcStr, const char * pattern, char * substring, int substr_len, int index)
{
	regmatch_t pmatch;
	regex_t reg;
	int times = 0;
	int offset = 0;
	memset(substring, 0x00, substr_len);

	regcomp(&reg, pattern, REG_EXTENDED);	//编译正则表达式

	while(offset < strlen(srcStr))
	{
		int status = regexec(&reg, srcStr + offset, 1, &pmatch, 0);/* 匹配正则表达式,regexec()函数一次只能匹配一个,不能连续匹配 */

		if(status == REG_NOMATCH)
		{
			hLog("Not Matched");
			return -1;
		}
		else if(pmatch.rm_so != -1)
		{
			times ++;
			if(times >= index)
			{
				char strtmp[1024];
				strncpy(substring, get_substr_by_pattern(srcStr + offset, &pmatch, strtmp, sizeof(strtmp)), substr_len);
				hLog("Matched [start=%d, end=%d]: %s", offset + pmatch.rm_so + 1, offset + pmatch.rm_eo, substring);
				break;
			}
		}

		offset += pmatch.rm_eo;
	}

	regfree(&reg);

	return 0;
}

MATCH_RESULT * regex_match_all_substr(char * srcStr, const char * pattern)
{
	regmatch_t pmatch;
	regex_t reg;
	int offset = 0;
	
	MATCH_RESULT * match_result = (MATCH_RESULT *) malloc(sizeof( MATCH_RESULT));
	match_result->number = 0;
	match_result->matchList = (NODE_SUBSTR *) malloc(sizeof(NODE_SUBSTR));
	match_result->matchList->next = NULL;
	match_result->matchList->substr = NULL;
	NODE_SUBSTR * p_substr = match_result->matchList;
	NODE_SUBSTR * node = NULL;
	
	regcomp(&reg, pattern, REG_EXTENDED);	//编译正则表达式
	while(offset < strlen(srcStr))
	{
		int status = regexec(&reg, srcStr + offset, 1, &pmatch, 0);/* 匹配正则表达式,regexec()函数一次只能匹配一个,不能连续匹配 */

		if(status == REG_NOMATCH)
		{
			//wLog("Not Matched");
			break;
		}
		else if(pmatch.rm_so != -1)
		{
			char strtmp[1024];
			get_substr_by_pattern(srcStr + offset, &pmatch, strtmp, sizeof(strtmp));
			//iLog("Matched [start=%d, end=%d]: %s", offset + pmatch.rm_so + 1, offset + pmatch.rm_eo, strtmp);
			match_result->number ++;
			
			node = (NODE_SUBSTR *) malloc(sizeof(NODE_SUBSTR));
			node->next = NULL;
			node->offset = offset;
			node->len = strlen(strtmp);
			node->substr = (char *) malloc(sizeof(char) * (node->len + 1));
			strncpy(node->substr, strtmp, node->len + 1);

			p_substr->next = node;
			p_substr = node;
		}

		offset += pmatch.rm_eo;
	}

	regfree(&reg);
	return match_result;
}

void show_substr(MATCH_RESULT * match_result)
{
	hLog("match %d substrs:",match_result->number);
	
	NODE_SUBSTR * p = match_result->matchList->next;
	while(p)
	{
		iLog("%s",p->substr);
		p = p->next;
	}

	return ;
}

void destroy_substr(MATCH_RESULT ** match_result)
{
	NODE_SUBSTR * p = (*match_result)->matchList;
	NODE_SUBSTR * tmp = NULL;

	while(p)
	{
		tmp = p;
		p = p->next;
		
		safe_free(&tmp->substr);
		safe_free(&tmp);
	}
	
	safe_free(match_result);

	return ;
}

char * byte2string(BYTE * bString, int len)
{
	char * start = str_dup("", len + 1);
	int cpLen = MIN(len, strnlen(P_CHAR(bString), len));
	memcpy(start, bString, cpLen);
	
	return start;
}



//////////////////////////////////////////////////////////////////////////
void * memmove(void *dest, const void *src, size_t n)
{
	char *cdest;
	const char *csrc;
	int i;

	/* Do the buffers overlap in a bad way? */
	if (src < dest && src + n >= dest) {
		/* Copy from end to start */
		cdest = dest + n - 1;
		csrc = src + n - 1;
		for (i = 0; i < n; i++) {
			*cdest-- = *csrc--;
		}
	}
	else {
		/* Normal copy is possible */
		cdest = dest;
		csrc = src;
		for (i = 0; i < n; i++) {
			*cdest++ = *csrc++;
		}
	}

	return dest;
}

