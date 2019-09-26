/****************************************************************************************************************************
*
*	Copyright (c) 1998-2018  XI'AN SAMING TECHNOLOGY Company 
*	西安三茗科技股份有限公司  版权所有 1998-2018 
*
*   PROPRIETARY RIGHTS of XI'AN SAMING TECHNOLOGY Company are involved in  the subject matter of this material.       
*   All manufacturing, reproduction,use, and sales rights pertaining to this subject matter are governed bythe license            
*   agreement. The recipient of this software implicitly accepts the terms of the license.	
*
*   本软件文档资料是西安三茗科技股份有限责任公司的资产,任何人士阅读和使用本资料必须获得相应的书面
*   授权,承担保密责任和接受相应的法律约束. 								     
*
*----------------------------------------------------------------------------------------------------------------------------
*
*   文 件 名 ：uuidgen.h 
*   文件描述 ：VOI生成uuid的功能接口头文件
*   创建日期 ：2018年9月17日
*   版本号 ： 9.7.2.0.1 
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __UUIDGEN_H__
#define __UUIDGEN_H__

#include <uuid/uuid.h>

//#include "uuidlib.h"
#include "../../baselib/baselib.h"

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

#define MODE_UUID_GEN_RANDOM 0Xa1
#define MODE_UUID_GEN_TIME 0Xa2
#define MODE_UUID_GEN_TIME_SAFE 0Xa3

#define CASE_UUID_PARSE_UPPER 0Xe1
#define CASE_UUID_PARSE_LOWWER 0Xe2

#pragma pack(1)




#pragma pack()

int compara_uuid(uuid_t uuid1,uuid_t uuid2);
void generate_uuid(uuid_t uuid, int mode);
int parse_uuid(const char * strin, uuid_t uuid);
char * unparse_uuid(const uuid_t uuid, int icase);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __UUIDGEN_H__ */
