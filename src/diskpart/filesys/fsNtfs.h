/****************************************************************************************************************************
*
*   文 件 名 ： fsNtfs.h 
*   文件描述 ：  
*   创建日期 ：2019年8月30日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __FSNTFS_H__
#define __FSNTFS_H__

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

//////////////////////////////////////////////////
#define MFT_FILE_NUM  	16 //16个元文件

#define FILE_MFT      				0 	//MFT本身
#define FILE_MFTMIRR  				1	//MFT镜像
#define FILE_LOGFILE  				2 	//日志文件
#define FILE_VOLUME   				3 	//卷文件
#define FILE_ATTRDEF  				4 	//属性定义表
#define FILE_ROOT     				5 	//根目录 （实际名称为 “.”）
#define FILE_BITMAP   				6 	//位图文件
#define FILE_BOOT     				7 	//引导文件
#define FILE_BADCLUS  				8 	//坏簇文件
#define FILE_SECURE   				9 	//安全文件
#define FILE_UPCASE   				10 	//大写文件
#define FILE_EXTEND_METADATA_DIR    11 	//扩展元数据文件
#define FILE_EXTEND_REPARSE			12 	//重解析点文件
#define FILE_EXTEND_USNJRNL    		13 	//变更日志文件
#define FILE_EXTEND_QUOTA    		14 	//配额管理文件
#define FILE_EXTEND_OBJID    		15 	//对象ID文件

#define MFT_END_FLAG 	0XFFFF

//////////////////////////////////////////////////
#define ATTR_TYPE_NUM_MAX 17 //每个源文件的常用属性最大数量

#define ATTR_STANDARD_INFORMATION 	0x10 //标准属性，包含文件的基本属性（只读，创建时间和最后访问时间等）
#define ATTR_ATTRIBUTE_LIST   		0x20 //属性列表
#define ATTR_FILENAME     			0x30 //文件名属性（UNICODE编码）
#define ATTR_OBJECT_ID        		0x40 //对象ID属性，文件或目录的16字节唯一标识
#define ATTR_SECURITY_DESCRIPTOR  	0x50 //安全描述符属性， 文件的访问控制安全
#define ATTR_VOLUME_NAME      		0x60 //卷名属性
#define ATTR_VOLUME_INFORMATION   	0x70 //卷信息属性
#define ATTR_DATA         			0x80 //文件的数据属性
#define ATTR_INDEX_ROOT       		0x90 //索引根属性
#define ATTR_INDEX_ALLOCATION 		0xA0 //索引分配属性
#define ATTR_BITMAP       			0xB0 //位图属性
#define ATTR_REPARSE_POINT      	0xC0 //重解析点属性
#define ATTR_EA_INFORMATION   		0xD0 //扩展信息属性
#define ATTR_EA           			0xE0 //扩展属性
#define ATTR_LOGGED_UTILITY_STREAM	0x100 //EFS加密属性

///////////////////////文件属性值类型///////////////////////////
#define FILE_ATTR_READ_ONLY      	0x1
#define FILE_ATTR_HIDDEN     	 	0x2
#define FILE_ATTR_SYSTEM     	 	0x4
#define FILE_ATTR_ARCHIVE        	0x20
#define FILE_ATTR_DEVICE     	 	0x40
#define FILE_ATTR_NORMAL     	 	0x80
#define FILE_ATTR_TEMPORARY      	0x100
#define FILE_ATTR_SPARSE     	 	0x200
#define FILE_ATTR_REPARSE        	0x400
#define FILE_ATTR_COMPRESSED     	0x800
#define FILE_ATTR_OFFLINE        	0x1000
#define FILE_ATTR_NOT_INDEXED    	0x2000
#define FILE_ATTR_ENCRYPTED      	0x4000
#define FILE_ATTR_DIRECTORY      	0x10000000
#define FILE_ATTR_INDEX_VIEW     	0x20000000

#define FLAG_COMPRESSED     	0x1
#define FLAG_ENCRYPTED      	0x4000
#define FLAG_SPARSE     		0x8000

#define BLK_SHR     9

#define MAX_MFT     (1024 >> BLK_SHR)
#define MAX_IDX     (16384 >> BLK_SHR)

#define INDEX_EXT_OFFSET 0X18

#define valueat(buf, ofs, type)   *((type*)(((char*)buf)+ofs))

#define AF_ALST     1
#define AF_GPOS     2

#define RF_COMP     1
#define RF_CBLK     2
#define RF_BLNK     4

#define LOWBYTE( a )	((BYTE)(a) & 0xF)
#define HIGBYTE( a )	(((BYTE)(a) & 0xF0) >> 4)

#define MAX_PATH	260
#define SECTORSIZE	512
#define DATA_RUNS_MAX	512

#define RST_SUCCEED		      	  0x00     //操作成功
#define RST_UNKNOWN_ERROR         0x01     //操作失败,原因不明
#define RST_BUFFER_ALLOC_FAIL     0x02     //内存分配失败    
#define RST_READDISK_ERROR        0x03     //读磁盘失败  
#define RST_WRITEDISK_ERROR       0x04     //写磁盘失败
#define RST_NOENOUGH_FREE         0x05	   //没有足够的连续空间	  
#define RST_PROC_FALG_ERROR       0x06     //进度标志错误 JDBJ
#define RST_LIST_FALG_ERROR       0x07     //进度列表标志错误  JDLB
#define RST_NOW_UNSUPPORT         0x08     //暂不支持此操作 
#define RST_PART_TOO_BIG          0x09     //分区太大了

#define UNICODE_NAME 	0x01
#define MAX_RUNS 		0x200
#define NON_DIR			0x08
#define EMPTY_DIR 		0x10


#define RESIDENT_DELETED_FILE 		0x00
#define RESIDENT_NORMAL_FILE 		0x01
#define RESIDENT_DELETED_DIR 		0x02
#define RESIDENT_NORMAL_DIR 		0x03

#pragma pack(1)

typedef struct _MFT_DATE_
{
    WORD    day:5;    	//1-31
    WORD    month :4;  	//1-12
    WORD    year:7;    	//0-127,相对1980年的年数
}MFT_DATE, *pMFT_DATE;

typedef struct _MFT_TIME_
{
    WORD    twoSec:5; 	//0-29,每2秒加1，表示0-58秒
    WORD    min:6; 		//0-59
    WORD    hour:5; 	//0-23
}MFT_TIME, *pMFT_TIME;

typedef struct _FILE_TIME_ 
{
	//UINT32 LowDateTime;
	UINT64 DateTime;
} FILE_TIME,*pFILE_TIME;

typedef struct _NTFS_BOOT_INFO_
{
    UINT8 	Jumpto[3];   			
    UINT8 	OemID[8];          //NTFS
    UINT16 	SecInByte;         //扇区字节数
    UINT8 	SecPerClr;         //每簇扇区数
    UINT16 	ResSct;            //保留扇区数。
    UINT8 	NtRevers0[5];      //NTFS中不使用
    UINT8 	FormatID;          //磁盘格式代号
    UINT16 	SecPerFat;         //每FAT扇区数
    UINT16 	SecPerTrk;         //每磁道扇区数
    UINT16 	Sides;             //磁盘面数
    UINT32 	HideSecNum;         //隐藏扇区数 user
    UINT32 	BigTotalSec;       //NT UNUSED
    UINT32 	BigSecPerFat;      //NT UNUSED
    UINT64 	TotalSecOfPart; 	//分区中扇区总数
    UINT64 	MFTAddrClr;        //MFT起始簇
    UINT64 	MFTMirrAddrClr;    //MFT镜像所在簇
    UINT32	ClustPerRec;       //40h每个文件记录块所占的簇数,一般固定是1KB,而不管簇的本身是多大。
                                 //所以，对于簇小于1KB的分区，就是实际占用的值，对于簇大于1KB的簇分区，一律是246(0xF6)
    UINT32 	ClustPerIndex;     //每个索引块占的簇数
    UINT32 	NtfsSerialNoL;       //分区序列号
    UINT32 	NtfsSerialNoH;
    UINT16 	NtfsCheckSum;        //校验和    
}NTFS_BOOT_INFO,*pNTFS_BOOT_INFO;

// 文件记录头
typedef struct _MFT_HEADER_
{
	/*+0x00*/  UINT8 	HeadID[4];            // 固定值'FILE'
	/*+0x04*/  UINT16 	UsnOffset;        // 更新序列号偏移, 与操作系统有关
	/*+0x06*/  UINT16 	UsnCount;         // 固定列表大小Size in words of Update Sequence Number & Array (S)
	/*+0x08*/  UINT64 	LogSn;               // 日志文件序列号(LSN)
	/*+0x10*/  UINT16  	SequenceNum;   	// 序列号(用于记录文件被反复使用的次数)
	/*+0x12*/  UINT16  	LinkCount;        // 硬连接数
	/*+0x14*/  UINT16  	AttributeOffset;  // 第一个属性偏移
	/*+0x16*/  UINT16  	Resident;            // flags, 00表示删除文件,01表示正常文件,02表示删除目录,03表示正常目录
	/*+0x18*/  UINT32  	MFTRealSize;       // 文件记录实时大小(字节) 当前MFT表项长度,到FFFFFF的长度+4
	/*+0x1C*/  UINT32  	MFTAllocSize;   // 文件记录分配大小(字节)
	/*+0x20*/  UINT64  	BaseFileRecord;   // = 0 基础文件记录 File reference to the base FILE record
	/*+0x28*/  UINT16  	NextAttributeId; // 下一个自由ID号
	/*+0x2A*/  UINT16  	Pading;           // 边界
	/*+0x2C*/  UINT32  	MFTRecordNum;  // windows xp中使用,本MFT记录号
	/*+0x30*/  UINT16  	Usn;      		// 更新序列号
	/*+0x32*/  UINT8  	UpdateArray[0];      // 更新数组
} MFT_HEADER, *pMFT_HEADER;

//常驻属性和非常驻属性的公用部分
typedef struct _AttributeHeader {
	UINT32 		ATTR_Type; //属性类型
	UINT32 		ATTR_Size; //属性头和属性体的总长度
	UINT8 		ATTR_ResFlag; //是否是常驻属性（0常驻 1非常驻）
	UINT8 		ATTR_NameSize; //属性名的长度
	UINT16 		ATTR_NameOffset; //属性名的偏移 相对于属性头
	UINT16 		ATTR_Flags; //标志（0x0001压缩 0x4000加密 0x8000稀疏）
	UINT16 		ATTR_Id; //属性唯一ID
}AttributeHeader,*pAttributeHeader;

//常驻属性
typedef struct _ResidentAttributeBody {
	UINT32 		ATTR_DataSize; //属性数据的长度
	UINT16 		ATTR_DataOffset; //属性数据相对于属性头的偏移
	UINT8 		ATTR_Index; //索引
	UINT8 		ATTR_Resvd; //保留
	UINT8 		ATTR_AttrName[0];//属性名，Unicode，结尾无0
}ResidentAttributeBody, *pResidentAttributeBody;

//非常驻属性
typedef struct _NonResidentAttributeBody {
	UINT64 		ATTR_StartVCN; //本属性中数据流起始虚拟簇号 
	UINT64 		ATTR_EndVCN; //本属性中数据流终止虚拟簇号
	UINT16 		ATTR_DataOff; //簇流列表相对于属性头的偏移
	UINT16 		ATTR_CmpressZ; //压缩单位 2的N次方
	UINT32 		ATTR_Resvd;
	UINT64 		ATTR_AllocSize; //属性分配的大小
	UINT64 		ATTR_ValidSize; //属性的实际大小
	UINT64 		ATTR_InitedSize; //属性的初始大小
	UINT8 		ATTR_AttrName[0];
}NonResidentAttributeBody, *pNonResidentAttributeBody;

typedef struct _AttributeUnion {
	AttributeHeader 	AttrHeader;
	union AttrBody
	{
		ResidentAttributeBody 	RsdAttrBody;
		NonResidentAttributeBody 	NonRsdAttrBody;
	} AttrBody;
}AttributeUnion, *pAttributeUnion;

//------- 下面这些结构只是属性数据内容部分，不包括属性头 ----
//------- 由属性头的属性值偏移量确定属性值的起始位置 ---
typedef struct _STANDARD_INFORMATION_ATTR_  //10H
{
    FILE_TIME 	CreateTime;    		// 文件创建时间 
    FILE_TIME 	ModifyTime;    		// 文件修改时间 
    FILE_TIME 	MftChangeTime;     	// MFT修改时间 
    FILE_TIME 	LatVisitedTime;    	// 文件最后访问时间 
    UINT32 		TradAtrribute;     	// 文件传统属性
    UINT8 		OtherInfo[28];      // 版本，所有者，配额，安全等等信息(详细略)
    UINT8 		UpdataSn[8];       // 文件更新序列号 
}STANDARD_INFORMATION_ATTR, *pSTANDARD_INFORMATION_ATTR;

typedef struct _ATTRIBUTE_LIST_ATTR_  //20H
{
    UINT32 		Type;           // 类型
    UINT16 		RecordType;     // 记录类型
    UINT16 		NameLen;        // 属性名长度(长度取决于 nameLen 的值)
    UINT8 		NameOffset;     // 属性名偏移
    UINT64 		StartVCN;       // 起始 VCN
    UINT64 		BaseRecordNum;  // 基本文件记录索引号
    UINT16 		AttributeId;    // 属性 id
}ATTRIBUTE_LIST_ATTR, *pATTRIBUTE_LIST_ATTR;

//FILE_NAME 0X30属性体
typedef struct _FILENAME_ATTR_  //30H 
{
	UINT64 		ParentFR; //父目录的MFT记录的记录索引。注意：该值的低6字节是MFT记录号，高2字节是该MFT记录的序列号
	FILE_TIME 	CreatTime;
	FILE_TIME 	AlterTime;
	FILE_TIME 	MFTChg;
	FILE_TIME 	ReadTime;
	UINT64 		AllocSz;
	UINT64 		ValidSz;//文件的真实尺寸
	UINT32 		DOSAttr;//DOS文件属性
	UINT32 		EaReparse;//扩展属性与链接
	BYTE 		NameSz;//文件名的字符数
	BYTE 		NamSpace;	/*命名空间，该值可为以下值中的任意一个
							0：POSIX　可以使用除NULL和分隔符“/”之外的所有UNICODE字符，最大可以使用255个字符。注意：“：”是合法字符，但Windows不允许使用。
							1：Win32　Win32是POSIX的一个子集，不区分大小写，可以使用除““”、“＊”、“?”、“：”、“/”、“<”、“>”、“/”、“|”之外的任意UNICODE字符，但名字不能以“.”或空格结尾。
							2：DOS　DOS命名空间是Win32的子集，只支持ASCII码大于空格的8BIT大写字符并且不支持以下字符““”、“＊”、“?”、“：”、“/”、“<”、“>”、“/”、“|”、“+”、“,”、“;”、“=”；同时名字必须按以下格式命名：1~8个字符，然后是“.”，然后再是1~3个字符。
							3：Win32&DOS　这个命名空间意味着Win32和DOS文件名都存放在同一个文件名属性中。
							*/
	BYTE 		FileName[0];
}FILENAME_ATTR,*pFILENAME_ATTR;

typedef struct _OBJECT_ID_ATTR_ //40H
{
    UINT8 		ObjectId[16];            // 全局对象ID 给文件的唯一ID号 
    UINT8 		OriginalVolumeId[16];    // 全局原始卷ID 永不改变 
    UINT8 		OriginalObjectId[16];    // 全局原始对象ID 派给本MFT记录的第一个对象ID 
    UINT8 		Domain[16];              // 全局域ID (未使用)
} OBJECT_ID_ATTR, *pOBJECT_ID_ATTR;

typedef struct _VOLUME_NAME_ATTR_ //60H
{
    UINT8 		VolumeName[128];    //通用属性头，最大为 127 外加一个结束符'\0'
}VOLUME_NAME_ATTR, *pVOLUME_NAME_ATTR;


typedef struct _VOLUME_INFORMATION_ATTR_ //70H
{    //----- 通用属性头 --- 
    UINT8 		NoUsed[8];      // 00
    UINT8 		MainVersion;     // 主版本号 1--winNT, 3--Win2000/XP
    UINT8 		SecVersion;      // 次版本号 当主为3, 0--win2000, 1--WinXP/7
    UINT16 		Flag;           /* 标志 :
							  0x0001  坏区标志 下次重启时chkdsk/f进行修复
							  0x0002  调整日志文件大小 
							  0x0004  更新装载 
							  0x0008  装载到 NT4 
							  0x0010  删除进行中的USN 
							  0x0020  修复对象 Ids 
							  0x8000  用 chkdsk 修正卷 
							*/
    UINT8 		Filled[4];       // 00 
}VOLUME_INFORMATION_ATTR,*pVOLUME_INFORMATION_ATTR;

typedef struct _DATA_ATTR_  //80H，需要解析
{
	UINT8 DataRuns[0];
}DATA_ATTR, *pDATA_ATTR;

typedef struct _BITMAP_ATTR_  //B0H，需要解析
{
	UINT8 BitmapData[0];
}BITMAP_ATTR, *pBITMAP_ATTR;

typedef struct _INDEX_ATTR_HEADER_
{
	UINT32 		EntryOff;//第一个目录项的偏移
	UINT32 		TalSzOfEntries;//目录项的总尺寸(包括索引头和下面的索引项)
	UINT32 		AllocSize;//目录项分配的尺寸
	UINT8 		Flags;/*标志位，此值可能是以下和值之一：
				  		0x00       小目录(数据存放在根节点的数据区中)
				  		0x01       大目录(需要目录项存储区和索引项位图)*/
	UINT8 		Resvd[3];
}INDEX_ATTR_HEADER,*pINDEX_ATTR_HEADER;

typedef struct _INDEX_ROOT_ATTR_  // 0X90H 属性体
{
	UINT32 		AttrType;//属性的类型
	UINT32 		ColRule;//整理规则
	UINT32 		EntrySz;//目录项分配尺寸
	UINT8 		ClusPerRec;//每个目录项占用的簇数
	UINT8 		Resvd[3];
	INDEX_ATTR_HEADER IndexHeader; //索引头
	UINT8 		IndexEntry[0];//索引项  可能不存在
}INDEX_ROOT_ATTR,*pINDEX_ROOT_ATTR;

//标准索引头的结构
typedef struct _STD_INDEX_HEADER_ 
{
	UINT8 		IndexFlag[4];  		//固定值 "INDX"
	UINT16 		USNOffset;			//更新序列号偏移
	UINT16 		USNSize;			//更新序列号和更新数组大小
	UINT64 		Lsn;               	//日志文件序列号(LSN)
	UINT64 		IndexCacheVCN;		//本索引缓冲区在索引分配中的VCN
	UINT32 		IndexEntryOffset;	//索引项的偏移 相对于当前位置
	UINT32 		IndexEntrySize;		//索引项的大小
	UINT32 		IndexEntryAllocSize;  //索引项分配的大小
	UINT8 		HasLeafNode;		//置一 表示有子节点
	UINT8 		Fill[3];			//填充
	UINT16 		USN;				//更新序列号
	UINT8 		USNArray[0];		//更新序列数组
}STD_INDEX_HEADER,*pSTD_INDEX_HEADER;

//标准索引项的结构
typedef struct _STD_INDEX_ENTRY_
{
	UINT64 		MFTReferNumber;//文件的MFT参考号
	UINT16 		IndexEntrySize;//索引项的大小
	UINT16 		FileNameAttriBodySize;//文件名属性体的大小
	UINT16 		IndexFlag;//索引标志
	UINT8 		Fill[2];//填充
	UINT64 		FatherDirMFTReferNumber;//父目录MFT文件参考号
	FILE_TIME 	CreatTime;//文件创建时间
	FILE_TIME 	AlterTime;//文件最后修改时间
	FILE_TIME 	MFTChgTime;//文件记录最后修改时间
	FILE_TIME 	ReadTime;//文件最后访问时间
	UINT64 		FileAllocSize;//文件分配大小
	UINT64 		FileRealSize;//文件实际大小
	UINT64 		FileFlag;//文件标志
	UINT8 		FileNameSize;//文件名长度
	UINT8 		FileNamespace;//文件命名空间
	UINT8 		FileNameAndFill[0];//文件名和填充
}STD_INDEX_ENTRY,*pSTD_INDEX_ENTRY;

//////////////////////////////////////////////////////
typedef struct _MftAttributeItem 
{
	///////////////////////////////////////////
	AttributeUnion * pAttrUnion;
	UINT8 * pAttrData;
	
	////////////////////////////////////////////
	STANDARD_INFORMATION_ATTR * pStdInfoAttr;
	ATTRIBUTE_LIST_ATTR * pAttrListAttr;
	FILENAME_ATTR * pFileNameAttr;
	OBJECT_ID_ATTR * pObjIdAttr;
	VOLUME_NAME_ATTR * pVolNameAttr;
	VOLUME_INFORMATION_ATTR * pVolInforAttr;
	DATA_ATTR * pDataAttr;
	INDEX_ROOT_ATTR * pIndexRootAttr;
	BITMAP_ATTR * pBitmapAttr;
	////////////////////////////////////////////
}MftAttributeItem, *p_MftAttributeItem;


/////////////////////////////////////////////////////

typedef struct _DATA_RUNS_
{
    UINT64 VSNStart;
    UINT64 VSNNum;
}DATA_RUNS, *pDATA_RUNS;

typedef struct _DATA_BITMAP_
{
	BYTE * pBitmap;
	int bitmapLen;
	int pDataBitmapType;
	int DataBitmapNum;
	DATA_RUNS pBitmapDataRun[DATA_RUNS_MAX];
}DATA_BITMAP, *pDATA_BITMAP;

typedef struct _MFT_DATA_RUNS_
{
	BYTE * pDataRuns;
	int pDataRunLen;
	int pDataRunType;
	int DataRunNum;
	DATA_RUNS pFileDataRun[DATA_RUNS_MAX];
}MFT_DATA_RUNS, *pMFT_DATA_RUNS;

typedef struct _MFT_SECTOR_INFO_
{
	UINT8 		SectorData[SECTORSIZE * 2]; //每个MFT信息占用2个扇区
	MFT_HEADER * pMftHeader; //直接指向SectorData的对应位置
	UINT8 		MftAttrNum; //属性数量
	MftAttributeItem MftAttrItem[ATTR_TYPE_NUM_MAX];
	MFT_DATA_RUNS mftDataRuns;
	DATA_BITMAP DataBitmap;
} MFT_FILE_INFO, *pMFT_FILE_INFO;

typedef struct _NTFS_FILE_DATA_INFO_
{
	int 	devfd;
	UINT64 	partStartSec;
	UINT16 	SecInByte;         //扇区字节数
    UINT8 	SecPerClr;         //每簇扇区数
	UINT16 	ClusterSize;
	UINT64 	MftAddrSec;
	DWORD 	MftId;
	DDWORD fileAllocSize;
	DDWORD fileRealSize;
	MFT_FILE_INFO * pMftFileInfo;
}NTFS_FILE_DATA_INFO, *pNTFS_FILE_DATA_INFO;

typedef struct _NTFS_PART_INFO_
{
	int 	devfd;
	UINT64 	partStartSec;
	UINT64 	partTotalSec;
	NTFS_BOOT_INFO * pNtfsBootInfo;
	UINT64 	MftAddrSec;
	MFT_FILE_INFO MftFileInfo[MFT_FILE_NUM];
	BYTE errCode;
}NTFS_PART_INFO, *pNTFS_PART_INFO;

#pragma pack()

 int Ntfs_analyse_mft_attr_info(MFT_FILE_INFO * pMftFileInfo);
 int Ntfs_anylase_file_path(char * filePath);

 NTFS_BOOT_INFO * Ntfs_get_boot_info(int devfd, DDWORD partStartSec);
 NTFS_PART_INFO * Ntfs_get_part_info(const char * devName, UINT64 partStartSec, UINT64 partTotalSec);
 void Ntfs_release_boot_info(NTFS_BOOT_INFO ** pNtfsBootInfo);
 void Ntfs_release_mft_file_info(MFT_FILE_INFO ** pMftFileInfo);
 void Ntfs_release_part_info(NTFS_PART_INFO ** pNtfsPartInfo);
 int Ntfs_StringFindFirstOf(char * baseStr, char assign);
 //DWORD Ntfs_search_file_in_direction(char * filePath, int deepIndex, NTFS_PART_INFO * pNtfsPartInfo, DWORD mftFileId, DDWORD * fileAllocSize, DDWORD * fileRealSize); 
 DWORD Ntfs_search_file_in_direction(char * filePath, int deepIndex, NTFS_PART_INFO * pNtfsPartInfo, DDWORD RunsVSN, DWORD mftFileId, DDWORD * fileAllocSize, DDWORD * fileRealSize);
MFT_FILE_INFO * Ntfs_get_mft_file_info (int devfd, UINT64 partStartSec, DWORD MftId, MFT_DATA_RUNS * pMftDataRuns);
 MFT_FILE_INFO * Ntfs_get_assign_mft_file_info (int devfd, UINT64 partStartSec, DWORD MftId, MFT_DATA_RUNS * pMftDataRuns);

 int Ntfs_get_file_path_deepth(char * filePath);
int Ntfs_scan_file_in_direction(char * PathName, NTFS_PART_INFO * pNtfsPartInfo, int limitDeepth);
NTFS_FILE_DATA_INFO * Ntfs_get_file_data_info(char * filePath, NTFS_PART_INFO * pNtfsPartInfo);
void Ntfs_release_file_data_info(NTFS_FILE_DATA_INFO ** pNtfsFileDataInfo);
int Ntfs_read_file_by_data_info(NTFS_FILE_DATA_INFO * pNtfsFileDataInfo, BYTE * databuf, int bufLen);
int Ntfs_read_file_by_data_info(NTFS_FILE_DATA_INFO * pNtfsFileDataInfo, BYTE * databuf, int bufLen);
int Ntfs_analyse_bitmap(BYTE * pBitmap, int pLen);
int Ntfs_judge_cluster_if_used(BYTE * pBitmap, int pLen, DWORD clusterNo);
int Ntfs_read_file_from_data_info(char * newFile, NTFS_FILE_DATA_INFO * pNtfsFileDataInfo);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __FSNTFS_H__ */
