/****************************************************************************************************************************
*
*   文 件 名 ： FatScan.h 
*   文件描述 ：  
*   创建日期 ：2019年8月15日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __FATSCAN_H__
#define __FATSCAN_H__


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


#pragma pack(1)

typedef struct _FAT16_BOOT_SECTOR_
{
    BYTE  Jumpto[3];        //  {0EBH, 03CH, 090H};
    BYTE  OemID[8];         //  "MSDOS5.0"
    WORD  SecInByte;        //  dw 200H         ; 扇区字节数
    BYTE  SecPerClr;        //  db 00H           ; 每簇扇区数
    WORD  ResSectors;       //  dw 0001H         ; 保留扇区数
    BYTE  NumOfFat;         //  db 02H           ; FAT数
    WORD  RootEntry;        //  dw 512           ; 根目录数
    WORD  TotalSec;         //  dw 0000H         ; 总扇区数           ; unused
    BYTE  FormatID;         //  db 0F8H          ; 磁盘格式代号
    WORD  SecPerFat;        //  dw 0000H         ; 每FAT扇区数
    WORD  SecPerTrk;        //  dw 003fH         ; 每磁道扇区数
    WORD  Sides;            //  dw 0000H         ; 磁盘面数
    DWORD HideSector;       //  dd 003fH         ; 隐藏扇区数
    DWORD BigTotalSec;      //  dd  00000000H   ; Big total number of sectors
    WORD  Phydrvnum;        //  dw  0080H       ; physical drive number
    BYTE  Signature;        //  db  029H        ; Extended Boot Record Signature
    DWORD Serialnum ;       //  dd  015470ff4h  ; Volume Serial Number
    BYTE  DISKLabel[11];    //  BYTE  "DISK1_VOL1 "
    BYTE  FileSystem[8];    //  BYTE  "FAT16   "
}FAT16_BOOT_SECTOR, *pFAT16_BOOT_SECTOR;

typedef struct _FAT16_DATA_SCAN_INFO_
{
	int diskfd;
	DDWORD partBeginSec;
	DDWORD partTotalSec;
	BYTE sectorBuf[SECTOR_SIZE];                     //读BITMAP表时用的缓冲
	FAT16_BOOT_SECTOR FAT16boot;
	DWORD m_SecInBuf;                   //当前在缓冲中的扇区,
	DWORD m_clusternum;                 //总簇数,不包括2个保留簇
	WORD m_secpercluster;              //每簇的扇区数
	DWORD m_datastart;                  //每个分区中，相对boot扇区的相对扇区数
	WORD m_fat1start;                       //FAT1起始扇区
	DWORD datasec;                           //每个分区的扇区总数
}FAT16_DATA_SCAN_INFO, *pFAT16_DATA_SCAN_INFO;

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _FAT32_BOOT_SECTOR_
{
    BYTE     Jumpto[3];     
    BYTE     OemID[8];              
    WORD     BytesPerSector;        //扇区字节数
    BYTE     SecPerClr;     //每簇扇区数
    WORD     ReservedSectors;       //保留扇区数。到第一个FAT的扇区数（包含启动扇区）。该字段的值一般总为32。
    BYTE     NumOfFat;      //FAT数  17
    WORD     Reserved;      //根目录项数（只对FAT12/FAT16有效）。对于FAT32，该字段必须为0
    WORD     Reserved1;     //扇区总数（Small Sectors）。只对FAT12/FAT16有效。对于FAT32分区，该字段必须为0
    BYTE     FormatID;      //磁介质描述符。提供所使用的磁介质信息。0xF8代表硬盘，0xF0代表高密度3.5英寸软盘。
    WORD     SecPerFat;     //每个FAT的扇区数，只对FAT12/FAT16 有效，对于FAT32分区，该字段必须为0。
    WORD     SecPerTrk;     //每磁道扇区数。INT 13H中用到的磁盘结构中的"每磁道扇区数"。此时分区被分成磁道和磁头、柱面的乘积
    WORD     Sides;         //磁头数目。INT 13H中用到的磁盘结构中的"磁头数目"。例如，一个1.44-MB，3.5英寸的软盘有2个磁头。
    DWORD    HidedSectors;  //隐藏扇区数。对应分区在启动扇区前的扇区数目。该值用于启动时确定根目录和数据区的绝对偏移量。该字段只在使用INT 13H访问磁盘是有效。如果一个磁盘没有分区，该字段必须为0。
    DWORD    BigTotalSec;   //扇区总数（Large Sectors）。FAT32 分区中扇区总数  //36
    DWORD    BigSecPerFat;  //每个FAT的扇区数（只对FAT32有效）。分区的每个FAT表所占的扇区数目。计算机使用该字段和FAT表数目和隐藏扇区数目来确定根目录的起始位置。计算机同样可以结合使用根目录项数（512）来确定用户数据的起始位置。
    WORD     BPB_ExtFlags;
    WORD     BPB_FSVer;
    DWORD    BPB_RootClus;          //根目录开始的簇
    WORD     BPB_FSInfo;
    WORD     BPB_BkBootSec;
    char     BPB_Reserved[12];
    BYTE     Phydrvnum;     //物理驱动器号。和BIOS的物理驱动器号相关。无论物理磁盘驱动器的数目是多少，软盘驱动器为0x00，硬盘驱动器为0x80。一般情况下，该值在BIOS INT13H功能调用时预设，以确定要访问的磁盘。只有当设备为启动设备时，该字段才有用。   
    BYTE     Free1;         //
    BYTE     Signature;     //扩展启动标识。为了能被Windows 2000所识别，该字段必须为0x28或者0x29。  
    DWORD    Serialnum;     // 分区序列号。在格式化磁盘时被创建的随机序列号，用于在不同的磁盘中来互相区别。
    BYTE     DISKLabel[11]; //卷标。该字段曾被用来存放卷标，现在卷标存放在根目录下的一个特殊的文件里。
    BYTE     FileSystem[8]; //文件系统类型。值为FAT32的文本。  
}FAT32_BOOT_SECTOR,*pFAT32_BOOT_SECTOR;

typedef struct _FAT32_DATA_SCAN_INFO_
{
	int diskfd;
	DDWORD partBeginSec;
	DDWORD partTotalSec;
	BYTE sectorBuf[SECTOR_SIZE];                     //读BITMAP表时用的缓冲
	FAT32_BOOT_SECTOR FAT32boot;
	DWORD m_SecInBuf;                   //当前在缓冲中的扇区,
	DWORD m_clusternum;                 //总簇数,不包括2个保留簇
	WORD m_secpercluster;              //每簇的扇区数
	DWORD m_datastart;                  //每个分区中，相对boot扇区的相对扇区数
	WORD m_fat1start;                       //FAT1起始扇区
	DWORD datasec;                           //每个分区的扇区总数
}FAT32_DATA_SCAN_INFO, *pFAT32_DATA_SCAN_INFO;

#pragma pack()

 void fat16_datascan_end(FAT16_DATA_SCAN_INFO ** pDataScanInfo);
 FAT16_DATA_SCAN_INFO * fat16_datascan_init(int diskfd, DDWORD partBeginSec, DDWORD partTotalSec);
 BOOL  fat16_datascan_initial(FAT16_DATA_SCAN_INFO * pDataScanInfo);
 WORD fat16_get_clusterval (FAT16_DATA_SCAN_INFO * pDataScanInfo, DWORD number);

 BYTE * fat16_generate_part_bitmap(int diskfd, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize);
 int fat16_part_mark_on_disk_bitmap(BYTE * pBitmap, int diskfd, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize);

 ///////////////////////////////////////////////////////////////////////////////////////////////////

 void fat32_datascan_end(FAT32_DATA_SCAN_INFO ** pDataScanInfo);
 BOOL fat32_datascan_initial(FAT32_DATA_SCAN_INFO * pDataScanInfo);
 DWORD fat32_get_clusterval (FAT32_DATA_SCAN_INFO * pDataScanInfo, DDWORD number);
 
 BYTE * fat32_generate_part_bitmap(int diskfd, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize); 
 int fat32_part_mark_on_disk_bitmap(BYTE * pBitmap, int diskfd, DDWORD partBeginSec, DDWORD partTotalSec, int bitmapSize);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __FATSCAN_H__ */
