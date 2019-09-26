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
*   文 件 名 ： scanbase.h 
*   文件描述 ：  
*   创建日期 ：2019年8月15日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __SCANBASE_H__
#define __SCANBASE_H__

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

#define SECTORS_PER_BLOCK 64  //每个数据块的扇区数
#define BLOCK_SIZE (SECTOR_SIZE * SECTORS_PER_BLOCK) //数据块的大小（字节）

#define BITMAP_BIT_SECTOR 64 //位表中的一位代表的扇区数量
#define BITMAP_BYTE_SECTOR (BITMAP_BIT_SECTOR * 8) //位表中的一字节代表的扇区数量

#define SECTORS_PER_CLUSTER 128
#define CLUSTER_SIZE (SECTOR_SIZE * SECTORS_PER_CLUSTER)

#define SECTOR_END_FLAG 0XAA55

#pragma pack(1)

//记录位表中的某字节的信息，
typedef struct _SCAN_BITMAP_INFO_
{
	//BYTE * pBitmap;
	DWORD PosByte; //指出这个字节在bitmap中的偏移从0开始
	BYTE PosBit; //指出这个位是在这个字节中的偏移(0~7),>7表示此位无效
	BYTE tmpByte; //存放bitmap中的某个临时字节
}SCAN_BITMAP_INFO, *pSCAN_BITMAP_INFO;

#pragma pack()

 DWORD calc_valid_data_size(BYTE * pBitmap, DWORD bitmapSize);
 BOOL check_bit_on_bitmap_marked(BYTE * pBitmap, DWORD byte, BYTE bit);
 BOOL check_byte_on_bitmap_marked(BYTE * pBitmap, DWORD byte);
 BOOL check_sector_on_bitmap_marked(BYTE * pBitmap, DDWORD logSec);
 DDWORD get_begin_sector_from_bitmap(DWORD byte, BYTE bit);
 BYTE get_bit_pos_in_bitmap(DDWORD logSec);
 DWORD get_byte_pos_in_bitmap(DDWORD logSec);
 SCAN_BITMAP_INFO * init_bitmap_info(BYTE * pBitmap, DDWORD logSec);
 int mark_bitmap_by_sector_info(BYTE * pBitmap, DDWORD beginSec, DDWORD secNum);
 void mark_bit_on_bitmap(BYTE * pBitmap, DWORD byte, BYTE bit);
 void mark_byte_on_bitmap(BYTE * pBitmap, DWORD byte);
 void release_bitmap_info(SCAN_BITMAP_INFO ** pScanBitmapInfo);
 
 BYTE * unknown_fs_generate_part_bitmap(int diskfd, DDWORD partBegiSsec, DDWORD partTotalSec, int bitmapSize);
 int unknown_fs_mark_part_on_disk_bitmap(BYTE * pBitmap, int diskfd, DDWORD partBegiSsec, DDWORD partTotalSec, int bitmapSize);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SCANBASE_H__ */
