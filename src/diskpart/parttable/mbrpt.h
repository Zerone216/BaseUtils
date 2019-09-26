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
*   文 件 名 ： mbrpt.h 
*   文件描述 ：  
*   创建日期 ：2019年5月14日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __MBRPT_H__
#define __MBRPT_H__

#include <common/baselib/baselib.h>
#include "dpbase.h"

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

#define MBR_PART_NUM_MAX 64 //MBR主分区最大数（包含扩展分区）
#define MBR_PRI_PART_NUM_MAX 4 //MBR主分区最大数（包含扩展分区）
#define LOGIC_PART_NUM_MAX 16  //MBR逻辑分区最大数

/////////////////////MBR分区类型 ////////////////////
#define MPT_INVAILD_PART			0x00
#define MPT_NTFS_PART				0x07
#define MPT_FAT32_1_PART			0x0B
#define MPT_FAT32_2_PART			0x0C
#define MPT_FAT16_1_PART			0x06
#define MPT_FAT16_2_PART			0x0E
#define MPT_SRV_PART				0x12	//服务分区在MBR分区表中的标志为0x12
#define MPT_EXTEND_PART				0x05 	//扩展分区
#define MPT_WIN_EXTEND_PART			0x0f 	//扩展分区
#define MPT_DM6_AUX1_PART			0x51
#define MPT_DM6_AUX3_PART			0x53
#define MPT_DM6_PART				0x54
#define MPT_EZD_PART				0x55
#define MPT_LINUX_SWAP_PART      	0x82	//linux swap
#define MPT_LINUX_EXT_PART			0x83 	//linux EXT2 /EXT3 /EXT4 
#define MPT_LINUX_EXTEND_PART		0x85
#define MPT_BSD_PART				0xa5
#define MPT_NETBSD_PART				0xa9
#define MPT_UNKNOWN_PART			0xff

#pragma pack(1)

typedef struct MBR_PART
{
	BYTE	bootId;         /* 0 or 0x80 */         
	BYTE	beginHead;           
	WORD    beginCylnAndSec;
	BYTE	pType;
	BYTE	endHead;
	WORD    endCylnAndSec;
	DWORD 	beginSec;
	DWORD 	totalSec;
}MBR_PART, *pMBR_PART;

//0扇区
typedef struct MBR_SECTOR
{
	BYTE ebrData[440];
	DWORD diskUuid;
	WORD uFlag;
	MBR_PART mbrDp[MBR_PRI_PART_NUM_MAX];
	WORD EndFlag;
}MBR_SECTOR, *pMBR_SECTOR;

typedef struct MBR_PART_INFO
{
	DWORD		partTabSec;
	BYTE 		itemIndex; //在该扇区分区表项数组中的索引：0~3
	MBR_PART 	mbrPart;
	char 		devName[32];
}MBR_PART_INFO, EXTEND_PART_INFO, LOGIC_PART_INFO, *pMBR_PART_INFO;

typedef struct MBR_PART_TABLE
{
	int partNum;
	MBR_PART_INFO mbrPartInfo[MBR_PART_NUM_MAX];
}MBR_PART_TABLE, *pMBR_PART_TABLE;


typedef struct MBR_PARTTABLE
{
	DWORD partTabSec;
	MBR_SECTOR mbrSector;
	struct MBR_PARTTABLE * next;
}MBR_PARTTABLE, *pMBR_PARTTABLE;

#pragma pack()

int get_mbr_parttable(int diskfd, MBR_PART_TABLE * mbrPartTable);
MBR_PARTTABLE * get_mbr_parttable_data_from_disk(int diskfd);
int write_mbr_parttable_data_to_disk(int diskfd, MBR_PARTTABLE * pMbrPt);
void release_mbr_parttable_data(MBR_PARTTABLE * pMbrPt);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __MBRPT_H__ */
