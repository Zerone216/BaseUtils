/****************************************************************************************************************************
*
*   文 件 名 ： fsFat32.h 
*   文件描述 ：  
*   创建日期 ：2019年9月4日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __FSFAT32_H__
#define __FSFAT32_H__

#include <common/baselib/baselib.h>

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

#define SECTORSIZE	512
#define FAT_TABLE_NUM_MAX 2

#define FAT_ENTRY_DELETED(DirId)  		(((DirId) == 0xe5 || (DirId) == 0x05) ? 1 : 0)//如果目录项开头是0xe5，表明是删除了的文件，数据恢复之类的用得到
#define FAT_ENTRY_END(DirId)      		(((DirId) == 0x00) ? 1 : 0)		//空文件
#define FAT_ENTRY_LONG(Attribute)		(((Attribute) == 0x0f) ? 1 : 0)	//长文件名项的标志

#define FAT_ENTRY_RDONLY(Attribute) 	(((Attribute) &  0x01) ? 1 : 0) //只读标志
#define FAT_ENTRY_HID(Attribute) 		(((Attribute) & ((BYTE)1 << 1)) ? 1 : 0) //隐藏标志
#define FAT_ENTRY_SYS(Attribute) 		(((Attribute) & ((BYTE)1 << 2)) ? 1 : 0) //系统标志

#define FAT_ENTRY_VOLUME(Attribute) 	((((Attribute) & 0x1f) == 0x08) ? 1 : 0) //卷标志
#define FAT_ENTRY_DIR(Attribute) 		(((Attribute) & ((BYTE)1 << 4)) ? 1 : 0) //目录标志
#define FAT_ENTRY_FILE(Attribute) 		(((Attribute) & ((BYTE)1 << 5)) ? 1 : 0) //文件标志


#pragma pack(1)


typedef struct _FAT32_BOOT_INFO_
{
    BYTE     Jumpto[3];     	//跳转指令 offset: 0
    BYTE     OemID[8];          //原始制造商 offset: 3
    WORD     BytesPerSector;    //每扇区字节数 offset:11
    BYTE     SecPerClr;     	//每簇扇区数
    WORD     ReservedSectors; 	//保留扇区数。到第一个FAT的扇区数（包含启动扇区）。该字段的值一般总为32。
    BYTE     NumOfFat;      	//此卷中FAT表数 offset:16
    WORD     Reserved;      	//根目录项数（只对FAT12/FAT16有效）。对于FAT32，该字段必须为0
    WORD     Reserved1;     	//扇区总数（Small Sectors）。只对FAT12/FAT16有效。对于FAT32分区，该字段必须为0
    BYTE     MediaID;      		//磁介质描述符。提供所使用的磁介质信息。0xF8代表硬盘，0xF0代表高密度3.5英寸软盘。
    WORD     SecPerFat;     	//每个FAT的扇区数，只对FAT12/FAT16 有效，对于FAT32分区，该字段必须为0。
    WORD     SecPerTrk;     	//每磁道扇区数。INT 13H中用到的磁盘结构中的"每磁道扇区数"。此时分区被分成磁道和磁头、柱面的乘积
    WORD     Sides;         	//磁头数目。INT 13H中用到的磁盘结构中的"磁头数目"。例如，一个1.44-MB，3.5英寸的软盘有2个磁头。
    DWORD    HidedSectors;  	//隐藏扇区数。对应分区在启动扇区前的扇区数目。该值用于启动时确定根目录和数据区的绝对偏移量。该字段只在使用INT 13H访问磁盘是有效。如果一个磁盘没有分区，该字段必须为0。
    DWORD    BigTotalSec;   	//该卷总扇区数（Large Sectors）。FAT32 分区中扇区总数  //36
    DWORD    BigSecPerFat;  	//每个FAT的扇区数（只对FAT32有效）。分区的每个FAT表所占的扇区数目。计算机使用该字段和FAT表数目和隐藏扇区数目来确定根目录的起始位置。计算机同样可以结合使用根目录项数（512）来确定用户数据的起始位置。
    WORD     BPB_ExtFlags;		//FAT32特有 offset:40
    WORD     BPB_FSVer;			//文件系统版本，FAT32特有 offset:42
    DWORD    BPB_RootClus;      //根目录开始的簇
    WORD     BPB_FSInfo;		//保留扇区FSINFO扇区数 offset:48
    WORD     BPB_BkBootSec;		//通常为6 offset:50
    char     BPB_Reserved[12];	//扩展用 offset:52
    BYTE     Phydrvnum;     	//物理驱动器号。和BIOS的物理驱动器号相关。无论物理磁盘驱动器的数目是多少，软盘驱动器为0x00，硬盘驱动器为0x80。一般情况下，该值在BIOS INT13H功能调用时预设，以确定要访问的磁盘。只有当设备为启动设备时，该字段才有用。   
    BYTE     Free1;         	//
    BYTE     Signature;     	//扩展启动标识。为了能被Windows 2000所识别，该字段必须为0x28或者0x29。  
    DWORD    Serialnum;     	// 分区序列号。在格式化磁盘时被创建的随机序列号，用于在不同的磁盘中来互相区别。
    BYTE     DISKLabel[11]; 	//卷标。该字段曾被用来存放卷标，现在卷标存放在根目录下的一个特殊的文件里。
    BYTE     FileSystem[8]; 	//文件系统类型。值为FAT32的文本。
	BYTE	 guidCode[420];
	WORD	 endFlag;
}FAT32_BOOT_INFO,*pFAT32_BOOT_INFO;

typedef struct _FILESYS_SECTOR_INFO_
{
	BYTE 	extGuidTag[4]; 	//扩展引导标签52 52 61 41“RRaA”
	BYTE 	unUsed[480]; 	//未用
	BYTE 	fileSysSign[4];	//文件系统签名72 72 41 61“rrAa”
	DWORD 	emptyClusterNum; //空闲簇数
	DWORD 	nextEmptyVCN;	//下一个空闲簇号
	BYTE 	reserved[14]; 	//保留
	WORD	endFlag; 		//结束标志“55 AA”
} FILESYS_SECTOR_INFO, *pFILESYS_SECTOR_INFO;

typedef struct _FAT_TABLE_INFO_
{
	DDWORD StartAddrSec;
	DDWORD TotalSecNum;
}FAT_TABLE_INFO, *pFAT_TABLE_INFO;

typedef struct _FDT_DATE_
{
    WORD    day:5;    	//1-31
    WORD    month :4;  	//1-12
    WORD    year:7;    	//0-127,相对1980年的年数
}FDT_DATE, *pFDT_DATE;

typedef struct _FDT_TIME_
{
    WORD    twoSec:5; 	//0-29,每2秒加1，表示0-58秒
    WORD    min:6; 		//0-59
    WORD    hour:5; 	//0-23
}FDT_TIME, *pFDT_TIME;

//fat32短目录项结构
typedef struct _SHORT_FDT32_
{
    BYTE        fileName[8];    //文件名，不足用空格（0x20）填充
    BYTE        extName[3];    	//扩展名(后缀名)，不足用空格（0x20）填充
    BYTE        attribute;    	//属性:0:读写；1只读；10隐藏；100系统；1000卷标；10000子目录；100000归档,0x0f表示是长目录结构;(除0xf外其他为二进制值)
    BYTE        reserved;    	//系统保留
    BYTE        milliTime;    	//创建时间的10毫秒位
    FDT_TIME    createTime;    	//文件创建时间
    FDT_DATE    createDate;    	//文件创建日期
    FDT_DATE    lastVisitDate;  //文件最后访问日期
    WORD        high16VCN;    	//文件起始簇号的高16位
    FDT_TIME    modifyTime;    	//文件的最近修改时间
    FDT_DATE    modifyDate;    	//文件的最近修改日期
    WORD        low16VCN;    	//文件起始簇号的低16位
    DWORD       fileSzie;    	//文件的长度
}SHORT_FDT32, *pSHORT_FDT32;

//fat32长目录项结构，文件名不足时用0xff填充
typedef struct _LONG_FDT32_
{
    BYTE    dirId;        		//目录项编号，从1开始
    WORD    longFileName1[5]; 	//文件名的第1-5个字符
    BYTE    attribute;    		//必须为0x0F
    BYTE    reserved;    		//系统保留,为0
    BYTE    checkValue;    		//校验值(根据短文件名计算得出)
    WORD    longFileName2[6];   //文件名的第6-11个字符
    WORD    startVCN;    		//文件起始簇号，目前常置0
    WORD    longFileName3[2];   //文件名的第12-13个字符
}LONG_FDT32, *pLONG_FDT32;

typedef struct _FDT_ITEM_
{
	union
	{
		LONG_FDT32 longFdt;
		SHORT_FDT32 shortFdt;		
	} ;
} FDT_ITEM , *pFDT_ITEM;


typedef struct _FAT32_DATA_ZONE_INFO_
{
	DDWORD DataStartSec;				//
	DWORD 	ClusterNum;                 //总簇数,不包括2个保留簇
	DWORD 	DataSecNum;                 //分区的数据区扇区总数
}FAT32_DATA_ZONE_INFO, *pFAT32_DATA_ZONE_INFO;

typedef struct _FAT32_PART_INFO_
{
	int 	devfd;
	UINT64 	partStartSec;
	UINT64 	partTotalSec;
	
	BYTE bootSec[SECTORSIZE * 2]; //引导扇区
	FAT32_BOOT_INFO * pFat32BootInfo; //指向bootSec的0x00偏移
	FILESYS_SECTOR_INFO * pFilesysSectorInfo; //指向bootSec的0x200偏移
	
	FAT_TABLE_INFO fatTableInfo[FAT_TABLE_NUM_MAX]; //FAT1和FAT2
	FAT32_DATA_ZONE_INFO fatDataZoneInfo;   //包括FDT在内的数据区
	
	BYTE errCode;
}FAT32_PART_INFO, *pFAT32_PART_INFO;

typedef struct _FAT32_DATA_INFO_
{
	int 		devfd;
	UINT64 		partStartSec;
	WORD     	BytesPerSec;
	WORD     	SecPerClr; 
	DWORD 		ClusterSize;
	UINT64 		DataZoneAddr; //FAT32数据区的偏移
	DWORD 		fileDataVSN; //文件的数据的相对于分区起始扇区的偏移
	DWORD 		fileAllocClrNum; //文件占用的簇数
	DWORD 		fileRealSize; //文件真实大小
}FAT32_FILE_DATA_INFO, *pFAT32_FILE_DATA_INFO;

#pragma pack()

int Fat32_read_data_block(int diskfd, DDWORD partStartSec, DDWORD StartSec, int secNum, BYTE * buff);
int Fat32_write_data_block(int diskfd, DDWORD partStartSec, DDWORD StartSec, int secNum, BYTE * buff);

FAT32_PART_INFO * Fat32_get_part_info(const char * devName, UINT64 partStartSec, UINT64 partTotalSec);
void Fat32_release_part_info(FAT32_PART_INFO ** pFat32PartInfo);
int Fat32_scan_file_in_direction(char * filePath, FAT32_PART_INFO * pFat32PartInfo, int limitDeepth);

int Fat32_read_file_from_part(char * devName, UINT64 partStartSec, UINT64 partTotalSec, char * filePath, char * newFile);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __FSFAT32_H__ */
