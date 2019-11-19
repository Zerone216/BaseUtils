/****************************************************************************************************************************
*
*   文 件 名 ： diskopt.h 
*   文件描述 ：  
*   创建日期 ：2019年5月14日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __DISKOPT_H__
#define __DISKOPT_H__

#include <common/baselib/baselib.h>
#include <diskpart/diskopt/diskopt.h>
#include <diskpart/parttable/parttable.h>

#include "diskrw.h"

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

#define LOCAL_DISK_NUM_MAX 8  //本机最多支持8块硬盘
#define DISK_PART_NUM_MAX 128 //每块硬盘最多支持128个分区

#pragma pack(1)

typedef struct PART_BASE_INFO
{
	DDWORD startSec;
    DDWORD totalSec;
	DDWORD endSec;
}PART_BASE_INFO,*pPART_BASE_INFO;

typedef struct FREE_PART_INFO
{
	int freePartNum;
	PART_BASE_INFO freePartInfo[DISK_PART_NUM_MAX];
}FREE_PART_INFO,*pFREE_PART_INFO;

//兼容GPT和MBR 的分区信息
typedef struct DEF_PART_INFO
{
	char partName[64]; //分区名称
	char partLabel[32]; //分区卷标
	char devName[32];  //分区设备名： /dev/sda1 or /dev/nvme0n1p1
	DEV_NODE_INFO devNode; //分区设备节点信息
	BYTE partType; //分区类型：系统分 or 数据分区 or 交换分区
	BYTE fileSys; //分区文件系统 （注：MBR分区文件系统和类型一样）
	DDWORD beginSec; //分区起始扇区
	DDWORD sectorNum; //分区容量
	DDWORD endSec; //分区结束扇区
	DDWORD partAttr; //分区属性（GPT 隐藏，只读等）	
	
	BYTE partGuid[16]; //分区GUID
	BYTE typeGuid[64]; //分区类型GUID
	
	BYTE partMode; // 分区模式：主分区 or 扩展分区 or 逻辑分区
	DDWORD partTabSec; //该分区信息所在分区表的扇区位置
	BYTE itemIndex; //在该扇区分区表项数组中的索引：0~3
	
	//以下是MBR专用
	BYTE	bootId;         /* 0 or 0x80 */         
	BYTE	beginHead;
	WORD    beginCylnAndSec;
	BYTE	endHead;
	WORD    endCylnAndSec;
	
	//////////////////////
	BYTE * pBitmap; //分区有效数据的位表指针
	DDWORD extBitSec; //分区位表对应的额外非本分区扇区（位表以64扇区为一个数据块映射，当分区大小非64整数倍时，最后一个数据块会有多余扇区越界）
	DDWORD validSec; //分区有效数据扇区数量
}DEF_PART_INFO, *pDEF_PART_INFO;

typedef struct DEF_DISK_INFO
{
	char devName[32]; //硬盘设备名 ： /dev/sda or /dev/nvme0n1
	char serialNum[32]; //硬盘序列号 ：   ST500DM002-9YN14C-W1D1JHGX
	BYTE interfaceType; // 接口类型： ata, usb, m.2(Nvme)
	BYTE removable; //设备是否可移动
	DEV_NODE_INFO devNode; //硬盘设备节点信息
	
	int diskfd; //硬盘操作描述符
	//////////////////////////////////////////
	DISK_CHS diskChs; //硬盘C.H.S参数
	DDWORD diskCapability; //硬盘容量
	//////////////////////////////////////////
	BYTE ptMode; //分区表类型：GPT or MBR or other
	BYTE diskGuid[16]; //硬盘GUID
	int partNum; //当前分区数量
	DEF_PART_INFO partInfo[DISK_PART_NUM_MAX]; //分区信息
	//////////////////////////////////////////
	
	BYTE * pBitmap; //磁盘有效数据的位表指针
	FREE_PART_INFO freeSpace; //磁盘空闲分区信息
}DEF_DISK_INFO, *pDEF_DISK_INFO;


typedef struct LOCAL_DISK_INFO
{
	int diskNum;
	DEF_DISK_INFO diskInfo[LOCAL_DISK_NUM_MAX];
}LOCAL_DISK_INFO, *pLOCAL_DISK_INFO;

#pragma pack()

LOCAL_DISK_INFO * local_disk_info_init();
void local_disk_info_release(LOCAL_DISK_INFO ** plocalDisk);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __DISKOPT_H__ */
