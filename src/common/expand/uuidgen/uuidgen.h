/****************************************************************************************************************************
*
*   文 件 名 ：uuidgen.h 
*   文件描述 ：生成uuid的功能接口头文件
*   创建日期 ：2018年9月17日
*   版本号 ： 
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
