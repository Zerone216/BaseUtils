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
*   文 件 名 ： dpbase.h 
*   文件描述 ：  
*   创建日期 ：2019年8月16日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __DPBASE_H__
#define __DPBASE_H__

#include <common/baselib/baselib.h>
#include "../diskopt/diskrw.h"

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

////////////////////////////////////////////////////////
#define DISK_INTERFACE_UNKNOWN 0x00  //
#define DISK_INTERFACE_ATA 0x01   //SATA接口
#define DISK_INTERFACE_M2 0x02   //M.2（Nvme）接口
#define DISK_INTERFACE_SCSI 0x03  //
#define DISK_INTERFACE_USB 0x04
#define DISK_INTERFACE_VIRT 0x05

////////////////////////////////////////////////////////
#define DISK_PART_TABLE_UNKNOWN 0x00
#define DISK_PART_TABLE_MBR 0x01
#define DISK_PART_TABLE_GPT 0x02

////////////////////////////////////////////////////////
#define DISK_PART_MODE_PRIMARY 0x00 //主分区 （GPT全部为主分区， MBR最多只能有4个）
#define DISK_PART_MODE_EXTEND 0x01 //扩展分区--MBR
#define DISK_PART_MODE_LOGIC 0x02 //逻辑分区--MBR

 //////////////////分区文件系统//////////////////////
#define PT_FS_UNFORMAT		0X00
#define PT_FS_NTFS			0X07
#define PT_FS_FAT16 		0X06
#define PT_FS_FAT32 		0X0B
#define PT_FS_DM6_AUX1		0X51
#define PT_FS_DM6_AUX3		0X53
#define PT_FS_DM6			0X54
#define PT_FS_EZD			0X55
#define PT_FS_LINUX_SWAP   	0X82	//linux SWAP
#define PT_FS_LINUX_EXT2 	0X83 	//linux EXT2 
#define PT_FS_LINUX_EXT3 	0X84 	//linux EXT3
#define PT_FS_LINUX_EXT4 	0X85 	//linux EXT4
#define PT_FS_BSD			0XA5
#define PT_FS_NETBSD		0XA9
#define PT_FS_UNKNOWN		0XFF

/*
 * For large disks g.cylinders is truncated, so we use BLKGETSIZE.
 */
#define BLKGETSIZE64 _IOR(0x12,114,size_t)	/* return device size in bytes (u64 *arg) */
#define BLKGETSIZE _IO(0x12,96)    /* return device size */

#pragma pack(1)

typedef struct DEV_NODE_INFO
{
	WORD major;
	WORD minor;
}DEV_NODE_INFO, *pDEV_NODE_INFO;

typedef struct DISK_CHS
{
	DWORD cylinderSize;
	DWORD start;
	DDWORD size;
	
	WORD cylinders;  //柱面数
    WORD heads;   //磁道数
    WORD sectors;  //扇区数
}DISK_CHS, *pDISK_CHS;


typedef struct DEF_PART
{
	BYTE	bootId;         /* 0 or 0x80 */         
	BYTE	beginHead;           
	WORD    beginCylnAndSec;
	BYTE	pType;
	BYTE	endHead;
	WORD    endCylnAndSec;
	DWORD 	beginSec;
	DWORD 	totalSec;
}DEF_PART, *pDEF_PART;

/////////////////////////////////////gpt partition/////////////////////////////////////
//0扇区
typedef struct PROTECT_MBR_SECTOR
{
	BYTE ebrData[440];
	DWORD diskUuid;
	WORD uFlag;
	DEF_PART mbrDp[4];
	WORD EndFlag;
}PROTECT_MBR_SECTOR, *pPROTECT_MBR_SECTOR;

//1扇区
typedef struct GPT_HEADER
{
	char		hdr_sig[8];                     /* 0x00 */
#define	GPT_HDR_SIG		"EFI PART"
	DWORD	hdr_revision;                   	/* 0x08 */
#define	GPT_HDR_REVISION	0x00010000
	DWORD	hdr_size;                       	/* 0x0c */
	DWORD	hdr_crc_self;                   	/* 0x10 */
	DWORD	reserved;                     	/* 0x14 */
	ULONGLONG	hdr_lba_self;                   /* 0x18 */
	ULONGLONG	hdr_lba_alt;                    /* 0x20 */
	ULONGLONG	hdr_lba_start;                  /* 0x28 */
	ULONGLONG	hdr_lba_end;                    /* 0x30 */
	BYTE hdr_guid[16];                      /* 0x38 disk GUID */
	ULONGLONG	hdr_lba_table;                  /* 0x48 */
	DWORD	hdr_entries;                    	/* 0x50 */
	DWORD	hdr_entsz;                      	/* 0x54 */
	DWORD	hdr_crc_table;                  	/* 0x58 */
	BYTE 	padding[420];                   	/* 0x5c */
} GPT_HEADER, *pGPT_HEADER;

/////////////////////////////////////gpt partition/////////////////////////////////////


#pragma pack()

 void close_disk(int * diskfd);
 int get_dev_node_info(char * devName, DEV_NODE_INFO * devNode);
 int get_disk_devname_by_index(char * devName, int index);
 DISK_CHS get_disk_geometry(char *devName, int diskfd);
 int get_disk_guid(BYTE ptMode, int diskfd, BYTE * diskGuid);
 BYTE get_disk_interface_type(char * devName);
 BYTE get_disk_parttable_mode(char * devName);
 BYTE get_disk_removable(char * devName);
 int get_disk_serial_num(char * devName, char * serialNum, int bufLen);
 int get_local_disk_num();
 
 int get_mbr_part_guid(char * partDevName, BYTE * partGuid, int bufLen);
 int get_mbr_part_type_guid(char * partDevName, BYTE * partType, int bufLen);
 int get_part_dev_name(DDWORD beginSec, char * diskDevName, char * partDevName, int bufLen);
 BYTE get_part_filesys(char * partDevName);
 int get_part_label_name(char * partDevName, char * partLabel, int bufLen);
 int open_disk(char * devname, int rwMode);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __DPBASE_H__ */
