/****************************************************************************************************************************
*
*   文 件 名 ： stdtypes.h 
*   文件描述 ：标准类型声明 
*   创建日期 ：2019年1月9日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __STDTYPES_H__
#define __STDTYPES_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef char CHAR;
typedef short SHORT;
typedef void VOID;

typedef u_int8_t BYTE;
typedef BYTE u8;
typedef BYTE CHAR8;
typedef BYTE UINT8;
typedef BYTE BOOL;

typedef u_int16_t WORD;
typedef WORD u16;
typedef WORD CHAR16;
typedef WORD UINT16;

typedef u_int32_t DWORD;
typedef DWORD u32;
typedef DWORD UINT32;

typedef u_int64_t DDWORD;
typedef DDWORD u64;
typedef DDWORD UINT64;

typedef long LONG;
typedef unsigned long ULONG;

typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;

typedef WORD USHORT;
typedef DWORD UINT;

typedef BYTE * LPBYTE;
typedef BYTE* LPSTR;
typedef WORD* LPWORD;
typedef WORD* LPWCHAR;

typedef DWORD* LPDWORD;
typedef DDWORD* LPDDWORD;

typedef unsigned char TCHAR;
typedef long long __int64;

typedef void (*sighandler_t)(int);

#define P_VOID(p) (VOID *)(p)
#define DP_VOID(p) (VOID **)(p)

#define G_POINTER(p) P_VOID(p)

#define P_BYTE(p) (BYTE *)(p)
#define P_WORD(p) (WORD *)(p)
#define P_WCHAR(p) (LPWCHAR)(p)

#define P_DWORD(p) (DWORD *)(p)
#define P_DDWORD(p) (DDWORD *)(p)
#define P_INT(p) (BYTE *)(p)
#define P_CHAR(p) (CHAR *)(p)
#define P_ASCII(p) (CHAR *)(p)

#define P_CHAR16(p) (CHAR16 *)(p)
#define P_UNICODE(p) (CHAR16 *)(p)

//typedef unsigned char bool;
//typedef enum {  false = 0, true} bool;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __STDTYPES_H__ */
