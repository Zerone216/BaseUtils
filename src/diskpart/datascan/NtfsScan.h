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
*   文 件 名 ： NtfsScan.h 
*   文件描述 ：  
*   创建日期 ：2019年8月15日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __NTFSSCAN_H__
#define __NTFSSCAN_H__


#include <common/baselib/baselib.h>
#include "../diskopt/diskrw.h"
#include "scanbase.h"


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

#define LOB( a )		( (BYTE)(a) & 0xF )
#define HIB( a )		( ( (BYTE)(a) & 0xF0 ) >> 4 )

#define MAX_PATH 260

#define NTFS_SECTORS_PER_CLUSTER 8
#define NTFS_CLUSTER_SIZE (SECTOR_SIZE * NTFS_SECTORS_PER_CLUSTER)

#define $DATA			0x80
#define $NAME			0x30
#define $ATTR_LIST		0x20
#define $INDEX_ALLOC	0xA0
#define $INDEX_ROOT		0x90
#define $BITMAP			0xb0

#define NOERROR                   0x00
#define RET_NO_ERROR		      0x00     //操作成功
#define RET_UNKNOWN_ERROR         0x01     //操作失败,原因不明
#define RET_BUFFER_ALLOC_FAIL     0x02     //内存分配失败    
#define RET_READDISK_ERROR        0x03     //读磁盘失败  
#define RET_WRITEDISK_ERROR       0x04     //写磁盘失败
#define RET_NOENOUGH_FREE         0x05	   //没有足够的连续空间	  
#define RET_PROC_FALG_ERROR       0x06     //进度标志错误 JDBJ
#define RET_LIST_FALG_ERROR       0x07     //进度列表标志错误  JDLB
#define RET_NOW_UNSUPPORT         0x08     //暂不支持此操作 
#define RET_PART_TOO_BIG          0x09     //分区太大了

#define UNICODE_NAME 0x01
#define MAX_RUNS 0x200
#define NON_DIR	0x08
#define EMPTY_DIR 0x10

#pragma pack(1)

typedef struct _NTFS_BOOT_SECTOR_
{
    BYTE      Jumpto[3];   
    BYTE      OemID[8];          //NTFS
    WORD      SecInByte;         //扇区字节数
    BYTE      SecPerClr;         //每簇扇区数
    WORD      ResSct;            //保留扇区数。
    BYTE      NtRevers0[5];      //NTFS中不使用
    BYTE      FormatID;          //磁盘格式代号
    WORD      SecPerFat;         //每FAT扇区数
    WORD      SecPerTrk;         //每磁道扇区数
    WORD      Sides;             //磁盘面数
    DWORD     Hsector;           //隐藏扇区数 user
    DWORD     BigTotalSec;       //NT UNUSED
    DWORD     BigSecPerFat;      //NT UNUSED
    ULONGLONG ullSectorsOfParti; //分区中扇区总数
    ULONGLONG ullMFTAddr;        //MFT起始簇
    ULONGLONG ullMFTMirrAddr;    //MFT镜像所在簇
    DWORD     ClustPerRec;       //40h每个文件记录块所占的簇数,一般固定是1KB,而不管簇的本身是多大。
                                 //所以，对于簇小于1KB的分区，就是实际占用的值，对于簇大于1KB的簇分区，一律是246(0xF6)
    DWORD     ClustPerIndex;     //每个索引块占的簇数
    DWORD     NtSerialNoL;       //分区序列号
    DWORD     NtSerialNoH;
    WORD      NtCheckSum;        //校验和    
}NTFS_BOOT_SECTOR,*pNTFS_BOOT_SECTOR;

typedef struct _NTFS_BITMAP_INFO_
{
	DWORD m_BitmapPos;
	DWORD m_BitmapSize;
}NTFS_BITMAP_INFO, *pNTFS_BITMAP_INFO;

typedef struct _NTFS_DATA_SCAN_INFO_
{
	int diskfd;
	DDWORD partBeginSec;
	DDWORD partTotalSec;
	NTFS_BOOT_SECTOR m_ntfsBoot;
	LPBYTE m_pptr;             //临时缓冲指针，使用时需要分配内存
	ULONGLONG m_ullRootAddr;      //根目录起始扇区
	UINT m_uMFTSize;                   //MFT记录的大小,从MFTHEAD中获得
	int ntfsBmNum;
	NTFS_BITMAP_INFO ntfsBitmapInfo[512];
	BYTE sectorBuf[SECTOR_SIZE];                     //读BITMAP表时用的缓冲
	DWORD m_SecInBuf;                   //当前在缓冲中的扇区,
	DWORD m_clusternum;                 //总簇数,不包括2个保留簇
	WORD m_secpercluster;              //每簇的扇区数
	DWORD m_datastart;                  //每个分区中，相对boot扇区的相对扇区数
	WORD m_fat1start;                       //FAT1起始扇区
	DWORD datasec;                           //每个分区的扇区总数
}NTFS_DATA_SCAN_INFO, *pNTFS_DATA_SCAN_INFO;

//下面是NTFS部份的代码
/*********************************************************************************************/
typedef struct tag_MFTHEAD                  //MFT文件记录头结构
{
	BYTE			bHeadID[4];             //FILE
	USHORT			usFixupOffset;
	USHORT			usFixupNum;             //sizeof fixup-list +1
	BYTE			bReserve1[8];
	WORD			wUnknownSeqNum; // ???
	USHORT			usLinkNum;
	USHORT			usAttrOffset;            //0x14 第一个属性偏移
	WORD			wResident;
	ULONG			ulMFTSize;               //文件记录实际大小
	ULONG			ulMFTAllocSize;          //文件记录分配大小
	ULONGLONG		ullMainMFT;
	WORD			wNextFreeID;
	WORD			wFixup[0x10];

} MFTHEAD, *LPMFTHEAD;

typedef struct tag_$FILE_NAME          //文件名属性结构
{
	DWORD		dwMFTIndex;
	WORD		wReserve1;
	WORD		wReserve2;
	ULONGLONG	ullTime1;
	ULONGLONG	ullTime2;
	ULONGLONG	ullTime3;
	ULONGLONG	ullTime4;
	ULONGLONG	ullAllocSize;
	ULONGLONG	ullFileSize;
	ULONGLONG	ullFileAttr;
	BYTE		bNameLen;
	BYTE		bNameType;
	wchar_t		pwChar[MAX_PATH];

} $FILE_NAME, *LP$FILENAME;

typedef struct tag_RESIDATTR
{

	ULONG	ulDataSize;
	USHORT	usRDataOffset;
	WORD	wUnknownAttrIndexID; // attribute is indexed, ???

} RESIDATTR, *LPRESIDATTR;

typedef struct tag_NONRESIDATTR
{

	ULONGLONG	ullVCNStart;
	ULONGLONG	ullVCNEnd;
	USHORT		usNrDataOffset;
	WORD		wComprEngine; //???
	DWORD		deReserve2;
	ULONGLONG	ullAllocSize;
	ULONGLONG	ullDataSize;
	ULONGLONG	ullInitSize;
	ULONGLONG	ullComprSize; //  is this runs if non-compressed?

} NONRESIDATTR, *LPNONRESIDATTR;

typedef struct tag_MFTATTR
{

	DWORD	dwAttrType;
	USHORT	usAttrSize;
	WORD	wReserve1;
	BYTE	bISResident;
	BYTE	bLenName;
	USHORT	usDataOffset; 	// offset to name or resident data;
	WORD	wISCompr;		// 1 compressed, 0 non-compressed;
	WORD	wAttrID;

	union unAttrib
	{
		RESIDATTR		ResidAttr;
		NONRESIDATTR	NonResidAttr;
	} unAttrib;

} MFTATTR, *LPMFTATTR;

//索引记录由索引头和一组索引项组成
typedef struct tag_INDX           //标准索引头结构
{
	BYTE		bDirID[4];        //INDX
	WORD		wFixupOffset;
	WORD		wFixupNum;
	BYTE		wReserve1[8];
	BYTE		bReserve2[8];
	WORD		wHeadSize;        //0x18 结构头的大小，不包括当前位置前0x18个字节
	WORD		wReserve3;
	DWORD		dwUseSize;        //0x1c 实际使用大小
	DWORD		dwAllocSize;      //0x20 索引记录大小，不包括前面0x18个字节, 4096-0x18
	DWORD		dwReserve3;
	BYTE		bFixup[0x0A];

} INDX, *LPINDX;

typedef struct tag_INDXATTR              //索引项结构
{
	DWORD		dwMFTIndx;              //文件MFT参考号
	WORD		wReserve1;
	WORD		wReserve2;
	WORD		wcbSize;                 //索引项大小
	WORD		wNameAttrLen;
	WORD		wISSubNode;
	BYTE		bReserve3[2];
	DWORD		dwParentMFTIndx;
	DWORD		dwReserve4;
	ULONGLONG	ullCreateTime;
	ULONGLONG	ullLastModTime;
	ULONGLONG	ullModRcdTime;
	ULONGLONG	ullLastAccTime;
	ULONGLONG	ullAllocSize;
	ULONGLONG	ullFileSize;
	ULONGLONG	ullFileFlags;
	BYTE		bFileNameLen;              //文件名长度
	BYTE		bFileNSpace;               //文件名命名空间
	wchar_t		wzFileName[MAX_PATH];

} INDXATTR, *LPINDXATTR;

#pragma pack()

 int my_wcstombs(char *dest, const wchar_t *wcsrc, size_t n);
 DWORD  ntfs_check_mft_fixup(LPBYTE pMft, ULONG lcbMft);
 
 void ntfs_free_ptr_mem(BYTE ** m_pptr);
 int ntfs_get_clusterval(NTFS_DATA_SCAN_INFO * pDataScanInfo, DWORD number);
 DWORD ntfs_get_direction_runs(LPBYTE pMft, LPBYTE pRuns);
 DWORD ntfs_get_file_runs(LPBYTE pMft, LPBYTE pRuns);
 DWORD  ntfs_get_mft_file_runs(NTFS_DATA_SCAN_INFO * pDataScanInfo, MFTHEAD * mftHead, LPBYTE pRuns);
 DWORD  ntfs_get_mft_head (NTFS_DATA_SCAN_INFO * pDataScanInfo, LPMFTHEAD  pmftHead, ULONGLONG ullAddr);
 DWORD ntfs_get_mft_record (NTFS_DATA_SCAN_INFO * pDataScanInfo, LPBYTE pMft, DWORD mftindex ,char* szName );
 DWORD ntfs_get_name_from_mft_attr(LPBYTE pMft, LPSTR pszName, UINT uNameSize);
 WORD  ntfs_get_root_file_pos(char* filename, NTFS_DATA_SCAN_INFO * pDataScanInfo, DWORD* startsec);
 ULONGLONG  ntfs_get_runs_all_size(NTFS_DATA_SCAN_INFO * pDataScanInfo, LPBYTE pRuns, UINT ucbRuns);
 DWORD ntfs_list_root_directory(NTFS_DATA_SCAN_INFO * pDataScanInfo);
 DWORD ntfs_make_member_form_runs(NTFS_DATA_SCAN_INFO * pDataScanInfo, LPBYTE pRuns, LONGLONG *llStAddr, ULONGLONG *ulSize);
 DWORD ntfs_read_direction_runs_to_mem(NTFS_DATA_SCAN_INFO * pDataScanInfo, LPBYTE pRuns);
 DWORD ntfs_search_file_from_root(TCHAR* findname, NTFS_DATA_SCAN_INFO * pDataScanInfo, LPBYTE  pFileRuns);
 DWORD  ntfs_search_root_directory(NTFS_DATA_SCAN_INFO * pDataScanInfo);

 int ntfs_datascan_initial(NTFS_DATA_SCAN_INFO * pDataScanInfo);
 NTFS_DATA_SCAN_INFO * ntfs_datascan_init(int diskfd, DDWORD partBeginSec, DDWORD partTotalSec);
 void ntfs_datascan_end(NTFS_DATA_SCAN_INFO ** pDataScanInfo);

 BYTE * ntfs_generate_part_bitmap(int diskfd, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize);
 int ntfs_part_mark_on_disk_bitmap(BYTE * pBitmap, int diskfd, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __NTFSSCAN_H__ */
