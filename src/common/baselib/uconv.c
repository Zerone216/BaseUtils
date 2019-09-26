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
*   文 件 名 ： uconv.c 
*   文件描述 ：  
*   创建日期 ：2019年9月6日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <regex.h>
#include <stdarg.h>	/* ANSI C header file */
#include <syslog.h> 	/* for syslog() */
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <getopt.h>
#include <asm/types.h>
#include <linux/kernel.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/io.h>

#include "uconv.h"
#include "gbkuni30.h"

int sconv_gbk_to_unicode(const char *gbkstr, int slen, wchar *outbuf, int osize)
{
    int ret = -1;
    int cnt = 0, i = 0, cb;
    const unsigned char *p = (const unsigned char *)gbkstr;
    wchar *op = outbuf;
    unsigned char c1;
    wchar c, chr;

    do {
        if (gbkstr == 0) {
            break;
        }
        if (slen < 0) {
            slen = 0;
            while (*p++) {
                ++slen;
            }
            p = (const unsigned char *)gbkstr;
        }
        while (i < slen) {
            c1 = *p;
            if (c1 < 0x80) {
                chr = c1;
                cb = 1;
            } else {
                if (i + 1 >= slen) {
                    break;
                }
                c = c1 << 8 | *(p + 1);
                chr = gbk_2_uni_tab[c];
                if (0 == chr) {
                    chr = '?';
                }
                cb = 2;
            }

            i += cb;
            p += cb;

            if (op) {
                if (cnt + 2 <= osize) {
                    *op++ = chr;
                } else {
                    break;
                }
            }
            cnt += 2;
        }
        ret = cnt;
    } while (0);

    return ret;
}

int sconv_unicode_to_gbk(const wchar *wstr, int wlen, char *outbuf, int osize)
{
    int ret = -1;
    int cnt = 0, i, cb;
    const wchar *p = wstr;
    char *op = outbuf;
    wchar c, chr;

    do {
        if (0 == wstr) {
            break;
        }
        if (wlen < 0) {
            wlen = 0;
            while (*p++) {
                ++wlen;
            }
            p = wstr;
        }
        for (i = 0; i < wlen; ++i, ++p) {
            c = *p;
            if (c < 0x80) {
                cb = 1;
                chr = c;
            } else {
                chr = uni_2_gbk_tab[c];
                if (0 == chr) {
                    chr = '?';
                }
                cb = 2;
            }
            if (op) {
                if (cnt + cb <= osize) {
                    if (cb == 1) {
                        *op++ = (char)chr;
                    } else {
                        // 						if (0x81 && chr >= 0x100) {
                        // 							*op++ = (char)(chr >> 8);
                        // 						}
                        *op++ = (char)(chr >> 8);
                        *op++ = (char)chr;
                    }
                } else {
                    break;
                }
            }
            cnt += cb;
        }
        ret = cnt;
    } while (0);
    return ret;
}

/*
* UTF-8
*/

/* Specification: RFC 3629 */

static int utf8_mbtowc(ucs4_t *pwc, const unsigned char *s, int n)
{
    unsigned char c = s[0];

    if (c < 0x80) {
        *pwc = c;
        return 1;
    } else if (c < 0xc2) {
        return RET_ILSEQ;
    } else if (c < 0xe0) {
        if (n < 2) {
            return RET_TOOFEW(0);
        }
        if (!((s[1] ^ 0x80) < 0x40)) {
            return RET_ILSEQ;
        }
        *pwc = ((ucs4_t)(c & 0x1f) << 6)
               | (ucs4_t)(s[1] ^ 0x80);
        return 2;
    } else if (c < 0xf0) {
        if (n < 3) {
            return RET_TOOFEW(0);
        }
        if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
                && (c >= 0xe1 || s[1] >= 0xa0))) {
            return RET_ILSEQ;
        }
        *pwc = ((ucs4_t)(c & 0x0f) << 12)
               | ((ucs4_t)(s[1] ^ 0x80) << 6)
               | (ucs4_t)(s[2] ^ 0x80);
        return 3;
    } else if (c < 0xf8 && sizeof(ucs4_t) * 8 >= 32) {
        if (n < 4) {
            return RET_TOOFEW(0);
        }
        if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
                && (s[3] ^ 0x80) < 0x40
                && (c >= 0xf1 || s[1] >= 0x90))) {
            return RET_ILSEQ;
        }
        *pwc = ((ucs4_t)(c & 0x07) << 18)
               | ((ucs4_t)(s[1] ^ 0x80) << 12)
               | ((ucs4_t)(s[2] ^ 0x80) << 6)
               | (ucs4_t)(s[3] ^ 0x80);
        return 4;
    } else if (c < 0xfc && sizeof(ucs4_t) * 8 >= 32) {
        if (n < 5) {
            return RET_TOOFEW(0);
        }
        if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
                && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40
                && (c >= 0xf9 || s[1] >= 0x88))) {
            return RET_ILSEQ;
        }
        *pwc = ((ucs4_t)(c & 0x03) << 24)
               | ((ucs4_t)(s[1] ^ 0x80) << 18)
               | ((ucs4_t)(s[2] ^ 0x80) << 12)
               | ((ucs4_t)(s[3] ^ 0x80) << 6)
               | (ucs4_t)(s[4] ^ 0x80);
        return 5;
    } else if (c < 0xfe && sizeof(ucs4_t) * 8 >= 32) {
        if (n < 6) {
            return RET_TOOFEW(0);
        }
        if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
                && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40
                && (s[5] ^ 0x80) < 0x40
                && (c >= 0xfd || s[1] >= 0x84))) {
            return RET_ILSEQ;
        }
        *pwc = ((ucs4_t)(c & 0x01) << 30)
               | ((ucs4_t)(s[1] ^ 0x80) << 24)
               | ((ucs4_t)(s[2] ^ 0x80) << 18)
               | ((ucs4_t)(s[3] ^ 0x80) << 12)
               | ((ucs4_t)(s[4] ^ 0x80) << 6)
               | (ucs4_t)(s[5] ^ 0x80);
        return 6;
    } else {
        return RET_ILSEQ;
    }
}

static int utf8_wctomb(unsigned char *r, ucs4_t wc, int n) /* n == 0 is acceptable */
{
    int count;
    if (wc < 0x80) {
        count = 1;
    } else if (wc < 0x800) {
        count = 2;
    } else if (wc < 0x10000) {
        count = 3;
    } else if (wc < 0x200000) {
        count = 4;
    } else if (wc < 0x4000000) {
        count = 5;
    } else if (wc <= 0x7fffffff) {
        count = 6;
    } else {
        return RET_ILUNI;
    }
    if (n < count) {
        return RET_TOOSMALL;
    }
    switch (count) { /* note: code falls through cases! */
    case 6:
        r[5] = 0x80 | (wc & 0x3f);
        wc = wc >> 6;
        wc |= 0x4000000;
    case 5:
        r[4] = 0x80 | (wc & 0x3f);
        wc = wc >> 6;
        wc |= 0x200000;
    case 4:
        r[3] = 0x80 | (wc & 0x3f);
        wc = wc >> 6;
        wc |= 0x10000;
    case 3:
        r[2] = 0x80 | (wc & 0x3f);
        wc = wc >> 6;
        wc |= 0x800;
    case 2:
        r[1] = 0x80 | (wc & 0x3f);
        wc = wc >> 6;
        wc |= 0xc0;
    case 1:
        r[0] = (unsigned char)wc;
    }
    return count;
}
// iconv end.


int sconv_utf8_to_unicode(const char *utf8str, int slen, wchar *outbuf, int osize)
{
    int ret = -1;
    const unsigned char *p = (const unsigned char *)utf8str;
    int i = 0, cnt = 0, cb;
    wchar *op = outbuf;
    ucs4_t wc;

    do {
        if (0 == utf8str) {
            break;
        }
        if (slen < 0) {
            slen = 0;
            while (*p++) {
                ++slen;
            }
            p = (const unsigned char *)utf8str;
        }
        for (i = 0; i < slen;) {
            cb = utf8_mbtowc(&wc, p, slen - i);
            if (cb <= 0) {
                break;
            }
            i += cb;
            p += cb;

            if (op) {
                if (cnt + 2 <= osize) {
                    *op++ = (wchar)wc;
                } else {
                    break;
                }
            }

            cnt += 2;
        }
        ret = cnt;
    } while (0);

    return ret;
}


int sconv_unicode_to_utf8(const wchar *wstr, int wlen, char *outbuf, int osize)
{
    int ret = -1;
    const wchar *p = wstr;
    int cnt = 0, i = 0, cb;
    unsigned char *op = (unsigned char *)outbuf;
    ucs4_t wc;
    unsigned char tmp[4];

    do {
        if (wstr == 0) {
            break;
        }
        if (wlen < 0) {
            wlen = 0;
            while (*p++) {
                ++wlen;
            }
            p = wstr;
        }
        for (i = 0; i < wlen * 2; i += 2, ++p) {
            wc = *p;

            if (op) {
                cb = utf8_wctomb(op, wc, osize - cnt);
                if (cb <= 0) {
                    break;
                }
                op += cb;
            } else {
                cb = utf8_wctomb(tmp, wc, 4);
            }

            cnt += cb;
        }
        ret = cnt;
    } while (0);

    return ret;
}

