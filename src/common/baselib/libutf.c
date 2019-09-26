
#include "libutf.h"

static boolean isLegalUTF8(const UTF8 *source, int length)
{
    UTF8 a;
    const UTF8 *srcptr = NULL;
    
    if (NULL == source){
        eLog("ERR, isLegalUTF8: source=[%p]", source);
        return FALSE;
    }
    srcptr = source+length;
 
    switch (length) {
		default:
			eLog("ERR, isLegalUTF8 1: length=%d\n", length);
			return FALSE;
		/* Everything else falls through when "TRUE"... */
		case 4:
			if ((a = (*--srcptr)) < 0x80 || a > 0xBF){
				eLog("ERR, isLegalUTF8 2: length=%d, a=%x\n", length, a);
				return FALSE;
			}
		case 3:
			if ((a = (*--srcptr)) < 0x80 || a > 0xBF){
				eLog("ERR, isLegalUTF8 3: length=%d, a=%x\n", length, a);
				return FALSE;
			}
		case 2: 
			if ((a = (*--srcptr)) > 0xBF){
				eLog("ERR, isLegalUTF8 4: length=%d, a=%x\n", length, a);
				return FALSE;
			}
			switch (*source)
			{
				/* no fall-through in this inner switch */
				case 0xE0: 
					if (a < 0xA0){
						eLog("ERR, isLegalUTF8 1: source=%x, a=%x\n", *source, a);
						return FALSE; 
					}
					break;
				case 0xED:
					if (a > 0x9F){
						eLog("ERR, isLegalUTF8 2: source=%x, a=%x\n", *source, a);
						return FALSE; 
					}
					break;
				case 0xF0:
					if (a < 0x90){
						eLog("ERR, isLegalUTF8 3: source=%x, a=%x\n", *source, a);
						return FALSE; 
					}
					break;
				case 0xF4:
					if (a > 0x8F){
						eLog("ERR, isLegalUTF8 4: source=%x, a=%x\n", *source, a);
						return FALSE; 
					}
					break;
				default:
					if (a < 0x80){
						eLog("ERR, isLegalUTF8 5: source=%x, a=%x\n", *source, a);
						return FALSE; 
					}
			}
		case 1: 
			if (*source >= 0x80 && *source < 0xC2){
				eLog("ERR, isLegalUTF8: source=%x\n", *source);
				return FALSE;
			}
    }
	
    if (*source > 0xF4)
		return FALSE;
	
    return TRUE;
}

ConversionResult Utf8_To_Utf16 (const UTF8* sourceStart, UTF16* targetStart, size_t outLen , ConversionFlags flags)
{
    ConversionResult result = conversionOK;
    const UTF8* source = sourceStart;
    UTF16* target      = targetStart;
    UTF16* targetEnd   = targetStart + outLen/2;
    const UTF8*  sourceEnd = NULL;
 
    if ((NULL == source) || (NULL == targetStart)){
        eLog("ERR, Utf8_To_Utf16: source=%p, targetStart=%p\n", source, targetStart);
        return conversionFailed;
    }
    sourceEnd   = strlen((const char*)sourceStart) + sourceStart;
 
    while (*source){
        UTF32 ch = 0;
        unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
        if (source + extraBytesToRead >= sourceEnd){
            eLog("ERR, Utf8_To_Utf16----sourceExhausted: source=%p, extraBytesToRead=%d, sourceEnd=%p\n", source, extraBytesToRead, sourceEnd);
            result = sourceExhausted;
			break;
        }
        /* Do this check whether lenient or strict */
        if (! isLegalUTF8(source, extraBytesToRead+1)){
            eLog("ERR, Utf8_To_Utf16----isLegalUTF8 return FALSE: source=%p, extraBytesToRead=%d\n", source, extraBytesToRead);
            result = sourceIllegal;
            break;
        }
        /*
        * The cases all fall through. See "Note A" below.
        */
        switch (extraBytesToRead) {
			case 5: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
			case 4: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
			case 3: ch += *source++; ch <<= 6;
			case 2: ch += *source++; ch <<= 6;
			case 1: ch += *source++; ch <<= 6;
			case 0: ch += *source++;
        }
        ch -= offsetsFromUTF8[extraBytesToRead];
 
        if (target >= targetEnd) {
            source -= (extraBytesToRead+1); /* Back up source pointer! */
            eLog("ERR, Utf8_To_Utf16----target >= targetEnd: source=%p, extraBytesToRead=%d\n", source, extraBytesToRead);
            result = targetExhausted;
			break;
        }
        if (ch <= UNI_MAX_BMP){
			/* Target is a character <= 0xFFFF */
            /* UTF-16 surrogate values are illegal in UTF-32 */
            if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END){
                if (flags == strictConversion){
                    source -= (extraBytesToRead+1); /* return to the illegal value itself */
                    eLog("ERR, Utf8_To_Utf16----ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END: source=%p, extraBytesToRead=%d\n", source, extraBytesToRead);
                    result = sourceIllegal;
                    break;
                } else {
                    *target++ = UNI_REPLACEMENT_CHAR;
                }
            } else{
                *target++ = (UTF16)ch; /* normal case */
            }
        }else if (ch > UNI_MAX_UTF16){
            if (flags == strictConversion) {
                result = sourceIllegal;
                source -= (extraBytesToRead+1); /* return to the start */
                eLog("ERR, Utf8_To_Utf16----ch > UNI_MAX_UTF16: source=%p, extraBytesToRead=%d\n", source, extraBytesToRead);
                break; /* Bail out; shouldn't continue */
            } else {
                *target++ = UNI_REPLACEMENT_CHAR;
            }
        } else {
            /* target is a character in range 0xFFFF - 0x10FFFF. */
            if (target + 1 >= targetEnd) {
                source -= (extraBytesToRead+1); /* Back up source pointer! */
                eLog("ERR, Utf8_To_Utf16----target + 1 >= targetEnd: source=%p, extraBytesToRead=%d\n", source, extraBytesToRead);
                result = targetExhausted; break;
            }
            ch -= halfBase;
            *target++ = (UTF16)((ch >> halfShift) + UNI_SUR_HIGH_START);
            *target++ = (UTF16)((ch & halfMask) + UNI_SUR_LOW_START);
        }
    }
    return result;
}
 
int Utf16_To_Utf8 (const UTF16* sourceStart, UTF8* targetStart, size_t outLen ,  ConversionFlags flags)
{
    int result = 0;
    const UTF16* source = sourceStart;
    UTF8* target = targetStart;
    UTF8* targetEnd = targetStart + outLen;
    
    if ((NULL == source) || (NULL == targetStart))
	{
        eLog("ERR, Utf16_To_Utf8: source=%p, targetStart=%p\n", source, targetStart);
        return conversionFailed;
    }
	
    while(*source)
	{
        UTF32 ch;
        unsigned short bytesToWrite = 0;
        const UTF32 byteMask = 0xBF;
        const UTF32 byteMark = 0x80; 
        const UTF16* oldSource = source; /* In case we have to back up because of target overflow. */
        ch = *source++;
        /* If we have a surrogate pair, convert to UTF32 first. */
        if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
            /* If the 16 bits following the high surrogate are in the source buffer... */
            if ( *source ){
                UTF32 ch2 = *source;
                /* If it's a low surrogate, convert to UTF32. */
                if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
                    ch = ((ch - UNI_SUR_HIGH_START) << halfShift) + (ch2 - UNI_SUR_LOW_START) + halfBase;
                    ++source;
                }else if (flags == strictConversion) { /* it's an unpaired high surrogate */
                    --source; /* return to the illegal value itself */
                    result = sourceIllegal;
                    break;
                }
            } else { /* We don't have the 16 bits following the high surrogate. */
                --source; /* return to the high surrogate */
                result = sourceExhausted;
                break;
            }
        } else if (flags == strictConversion) {
            /* UTF-16 surrogate values are illegal in UTF-32 */
            if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END){
                --source; /* return to the illegal value itself */
                result = sourceIllegal;
                break;
            }
        }
        /* Figure out how many bytes the result will require */
        if(ch < (UTF32)0x80){	     
			bytesToWrite = 1;
        } else if (ch < (UTF32)0x800) {     
            bytesToWrite = 2;
        } else if (ch < (UTF32)0x10000) {  
            bytesToWrite = 3;
        } else if (ch < (UTF32)0x110000){ 
            bytesToWrite = 4;
        } else {	
            bytesToWrite = 3;
            ch = UNI_REPLACEMENT_CHAR;
        }
		
        target += bytesToWrite;
        if (target > targetEnd) {
            source = oldSource; /* Back up source pointer! */
            target -= bytesToWrite; result = targetExhausted; break;
        }
        switch (bytesToWrite) { /* note: everything falls through. */
			case 4: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
			case 3: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
			case 2: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
			case 1: *--target = (UTF8)(ch | firstByteMark[bytesToWrite]);
        }
        target += bytesToWrite;
    }
    return result;
}

int UTF16_To_UCS4(const UTF16 * utf16, UCS4 * ucs4)
{
	if(0xD800 <= *utf16 && *utf16 <= 0xDBFF)
	{
		*ucs4 = ((*utf16 & 0x3FF) << 10) + (*(utf16+1) & 0x3FF) + 0x10000;
		return 2;
	}
	else
	{
		*ucs4 = *utf16;
		return 1;
	}
}

int StrUtf16ToUcs4(const UTF16 * utf16, int utf16Len, UCS4 * ucs4, int ucs4Len)
{
	int utf16Bytes = 0;
	int transLen = 0;
	int i = 0, j = 0;
	for(i = 0, j = 0; i < utf16Len && j < ucs4Len; i += utf16Bytes, j ++)
	{
		utf16Bytes = UTF16_To_UCS4(utf16 + i, ucs4 + j);
		transLen += 1;
	}

	return transLen;
}

int UCS4_To_UTF16(UCS4 * ucs4, UTF16 * utf16)
{
    if (*ucs4 < 0x10000)
    {
        *utf16 = *ucs4;
        return 1;
    }
    else
    {
        *ucs4 -= 0x10000;
        *utf16 = 0xD800 | ((*ucs4 >> 10) & 0x3FF);
        *(utf16+1) = 0xDC00 | (*ucs4 & 0x3FF);
        return 2;
    }
}

int StrUcs4ToUtf16(const UCS4 * ucs4, int ucs4Len, UTF16 * utf16, int utf16Len)
{
	int ucs4Bytes = 0;
	int transLen = 0;
	int i = 0, j = 0;
	for(i = 0, j = 0; i < ucs4Len && j < utf16Len; i ++, j += ucs4Bytes)
	{
		ucs4Bytes = UCS4_To_UTF16((UCS4 *)ucs4 + i, utf16 + j);
		transLen += ucs4Bytes;
	}

	return transLen;
}

int StrUtf16ToUtf8 (const UTF16 * utf16, int utf16Len, UTF8 * utf8, int utf8Len)
{
	memset(utf8, '\0', utf8Len);
	
	int i = 0, j = 0;
	for(i = 0, j = 0; i < utf16Len && j < utf8Len;)
	{
		if(utf16[j] >= 0x0001 && utf16[j] <= 0x00FF)
		{
			utf8[i ++] = utf16[j ++];
		}
		else if(utf16[j] >= 0x0100 && utf16[j] < 0xFFFF)
		{
			utf8[i ++] = utf16[j ++];
			utf8[i ++] = ((utf16[j ++] >> 8) & 0X00FF);
		}
		else
			break;
	}

	return 0;
}
