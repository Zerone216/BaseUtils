/****************************************************************************************************************************
*
*   文 件 名 ： gptpt.h 
*   文件描述 ：  
*   创建日期 ：2019年5月14日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __GPTPT_H__
#define __GPTPT_H__

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
#define GPT_PART_NUM_MAX 128
#define GPT_PART_INFO_SIZE 128 
#define GPT_PART_TABLE_SIZE (GPT_PART_NUM_MAX * GPT_PART_INFO_SIZE)  //512*32

/////////////////////////////分区类型定义//////////////////////////////////
#define	GPT_BLANK_PART   	0x00 //空闲分区
#define	GPT_RECOVERY_PART   0x01 //win recovery分区
#define	GPT_ESP_PART    	0x02 //win esp分区
#define	GPT_MSR_PART    	0x03 //win msr分区
#define	GPT_OS_PART    		0x04 //系统分区
#define	GPT_DATA_PART    	0x50 //数据分区
#define	GPT_BASIC_PART    	(GPT_OS_PART | GPT_DATA_PART) 	//基础分区
#define	GPT_SRV_ESP_PART	0x12 //隐藏的服务分区
#define	GPT_UNKNOWN_PART	0xff //未知类型的分区

//////////////////////////////////////分区类型GUID////////////////////////////////////////////////
#define GPT_ENT_TYPE_INVALID_PART_GUID 	{ 0X00000000, 0X0000, 0X0000, { 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00 }} //无效的分区GUID
#define GPT_ENT_TYPE_MS_RECOVERY_GUID	{0XDE94BBA4,0X06D1,0X4D40,{0XA1,0X6A,0XBF,0XD5,0X01,0X79,0XD6,0XAC}} //WIN Recovery分区的TYPE GUID
#define GPT_ENT_TYPE_MS_ESP_GUID 		{0XC12A7328, 0XF81F, 0X11D2, { 0XBA, 0X4B, 0X00, 0XA0, 0XC9, 0X3E, 0XC9, 0X3B }} //WIN ESP分区的TYPE GUID
#define GPT_ENT_TYPE_MS_RESERVED_GUID 	{0XE3C9E316,0X0B5C,0X4DB8,{0X81,0X7D,0XF9,0X2D,0XF0,0X02,0X15,0XAE}} //WIN MSR分区的TYPE GUID
#define GPT_ENT_TYPE_MS_BASIC_PART_GUID	{0XEBD0A0A2, 0XB9E5, 0X4433, {0X87, 0XC0, 0X68, 0XB6, 0XB7, 0X26, 0X99, 0XC7}}   //基础分区（数据分区or系统分区）的TYPE GUID
#define GPT_ENT_TYPE_BLANK_PART_GUID 	{0X6A8D2AC7, 0X1DD2, 0X11B2, {0X99, 0XA6, 0X08, 0X00, 0X20, 0X73, 0X66, 0X31}} //GPT分区表中占据空间用的分区TYPE GUID
#define GPT_ENT_TYPE_SRV_ESP_GUID 		{0XBFBFAFE7, 0XA34F, 0X448A, {0X9A, 0X5B, 0X62, 0X13, 0XEB, 0X73, 0X6C, 0X22}} //隐藏的服务分区的TYPE GUID
#define GPT_ENT_TYPE_LINUX_SWAP_GUID	{0X0657FD6D,0XA4AB,0X43C4,{0X84,0XE5,0X09,0X33,0XC8,0X4B,0X4F,0X4F}} //SWAP分区的TYPE GUID

////////////////////////////////GPT分区表中分区的属性//////////////////////////////////////////////
#define	GPT_ATTRIBUTE_PLATFORM_REQUIRED					0x0000000000000001
#define	GPT_BASIC_DATA_ATTRIBUTE_NO_DRIVE_LETTER		0x8000000000000000
#define	GPT_BASIC_DATA_ATTRIBUTE_HIDDEN					0x4000000000000000
#define	GPT_BASIC_DATA_ATTRIBUTE_SHADOW_COPY			0x2000000000000000
#define	GPT_BASIC_DATA_ATTRIBUTE_READ_ONLY				0x1000000000000000

/////////////////////////////////////////////////////////////////////
#define DO_PARTTABLE_BACKUP		0x01 //分区表备份
#define DO_PARTTABLE_RECOVERY 	0x02 //分区表恢复


#pragma pack(1)

typedef struct GPT_PART
{
	BYTE ent_type[16];
	BYTE ent_uuid[16] ;
	ULONGLONG	ent_lba_start;
	ULONGLONG	ent_lba_end;
	ULONGLONG	ent_attr;
#define	GPT_ENT_ATTR_PLATFORM		(1ULL << 0)
	BYTE	ent_name[72];		/* UNICODE-16 */
} GPT_PART, *pGPT_PART;

typedef struct GPT_PART_INFO
{
	GPT_PART gtpPart;
	char devName[32]; //分区对应的设备名:如 /dev/sda1
	BYTE partType; //分区类型标记
}GPT_PART_INFO, *pGPT_PART_INFO;

typedef struct GPT_PART_TABLE
{
	PROTECT_MBR_SECTOR pMbrSec; //0扇区，MBR保护扇区
	GPT_HEADER gptHeader; //1扇区， GPT头信息
	int partNum;
	GPT_PART_INFO gPartInfo[GPT_PART_NUM_MAX]; //2~34扇区，分区信息（备份分区信息在末尾GPT头备份之前）
	GPT_HEADER gptHeaderBak; //最后一个扇区， GPT头备份信息
	int blankPartNum; //空闲分区的数量
}GPT_PART_TABLE, *pGPT_PART_TABLE;



#pragma pack()

int get_gpt_parttable(char * diskDevName, int diskfd, DDWORD diskCapability, GPT_PART_TABLE * gptTartTable);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __GPTPT_H__ */
