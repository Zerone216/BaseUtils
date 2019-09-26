
#ifndef __LIBUTF_H__
#define __LIBUTF_H__
 
 #include <locale.h>
#include <wchar.h>
#include "libbase.h"
#include "log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef unsigned char   boolean;
typedef unsigned int CharType ;
typedef unsigned char UTF8;
typedef unsigned short UTF16;
typedef unsigned int UTF32;

typedef unsigned short UCS2;
typedef unsigned int UCS4;



#define halfShift	10
#define UNI_SUR_HIGH_START  (UTF32)0xD800
#define UNI_SUR_HIGH_END    (UTF32)0xDBFF
#define UNI_SUR_LOW_START   (UTF32)0xDC00
#define UNI_SUR_LOW_END     (UTF32)0xDFFF

/* Some fundamental constants */
#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD
#define UNI_MAX_BMP (UTF32)0x0000FFFF
#define UNI_MAX_UTF16 (UTF32)0x0010FFFF
#define UNI_MAX_UTF32 (UTF32)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (UTF32)0x0010FFFF
  
static const UTF32 halfMask = 0x3FFUL;
static const UTF32 halfBase = 0x0010000UL;

static const UTF8 firstByteMark[7] = 
{
	0x00,
	0x00,
	0xC0,
	0xE0,
	0xF0,
	0xF8,
	0xFC
};

static const UTF32 offsetsFromUTF8[6] = 
{ 
	0x00000000UL, 
	0x00003080UL, 
	0x000E2080UL, 
	0x03C82080UL, 
	0xFA082080UL, 
	0x82082080UL
};

static const char trailingBytesForUTF8[256] =
{
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};
	
typedef enum 
{
	strictConversion = 0,
	lenientConversion
} ConversionFlags;

typedef enum 
{
	conversionOK, 		/* conversion successful */
	sourceExhausted,	/* partial character in source, but hit end */
	targetExhausted,	/* insuff. room in target for conversion */
	sourceIllegal,		/* source sequence is illegal/malformed */
	conversionFailed
} ConversionResult;

#pragma pack(1)

#pragma pack()

ConversionResult Utf8_To_Utf16 (const UTF8* sourceStart, UTF16* targetStart, size_t outLen , ConversionFlags flags);
int Utf16_To_Utf8 (const UTF16* sourceStart, UTF8* targetStart, size_t outLen ,  ConversionFlags flags);
int StrUtf16ToUcs4(const UTF16 * utf16, int utf16Len, UCS4 * ucs4, int ucs4Len);
int StrUcs4ToUtf16(const UCS4 * ucs4, int ucs4Len, UTF16 * utf16, int utf16Len);
int StrUtf16ToUtf8 (const UTF16 * utf16, int utf16Len, UTF8 * utf8, int utf8Len);

#endif
