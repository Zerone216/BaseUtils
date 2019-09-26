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
*   文 件 名 ： uuidgen.c 
*   文件描述 ：VOI生成uuid的功能接口源文件 
*   创建日期 ：2018年9月18日
*   版本号 ： 9.7.2.0.1 
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#include "uuidgen.h"


/*******************************************************************************************************************
 * 函 数 名  ：  generate_uuid
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月19日
 * 函数功能  ：  生成一个uuid
 * 参数列表  ： 
        uuid_t uuid  要生成的uuid指针
        int mode     生成uuid的模式
 * 返 回 值  ：  暂无
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
void generate_uuid(uuid_t uuid, int mode)
{
	uuid_clear(uuid);
	switch(mode)
	{
		case MODE_UUID_GEN_RANDOM :
			uuid_generate_random(uuid);
			break;
		case MODE_UUID_GEN_TIME :
			uuid_generate_time(uuid);
			break;
		case MODE_UUID_GEN_TIME_SAFE :
			uuid_generate_time_safe(uuid);
			break;
		default:
			uuid_generate(uuid);
			break;
	}
	
	return;
}


/*******************************************************************************************************************
 * 函 数 名  ：  compara_uuid
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月19日
 * 函数功能  ：  比较两个uuid是否一样
 * 参数列表  ： 
        uuid_t uuid1  要比较的第一个uuid
        uuid_t uuid2  要比较的第二个uuid
 * 返 回 值  ：  比较的结果
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
int compara_uuid(uuid_t uuid1,uuid_t uuid2)
{
	return uuid_compare( uuid1, uuid2);
}


/*******************************************************************************************************************
 * 函 数 名  ：  parse_uuid
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月19日
 * 函数功能  ：  将一个字符串形式的uuid转换成uuid_t类型
 * 参数列表  ： 
        const char * strin  需要转换的uuid字符串
        uuid_t uuid         转换后的uuid
 * 返 回 值  ：  转换uuid的结果
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
int parse_uuid(const char * strin, uuid_t uuid)
{
	return uuid_parse(strin, uuid);
}


/*******************************************************************************************************************
 * 函 数 名  ：  unparse_uuid
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年9月19日
 * 函数功能  ：  讲一个uuit_t形式的uuid转换成字符串形式
 * 参数列表  ： 
        const uuid_t uuid  要转换的uuid
        int icase          转换后的字符串uuid的形式（全大写或者全小写）
 * 返 回 值  ：  转换后的字符串形式的uuid
 * 调用关系  ：  
 * 修改记录  ：  
 * 其 它  ：  
*******************************************************************************************************************/
char * unparse_uuid(const uuid_t uuid, int icase)
{
	char * uuidstr = str_dup("", 64);	
	switch(icase)
	{
		case CASE_UUID_PARSE_LOWWER :
			uuid_unparse_lower(uuid, uuidstr);
			break;
		case CASE_UUID_PARSE_UPPER :
			uuid_unparse_upper(uuid, uuidstr);
			break;
		default:
			uuid_unparse(uuid, uuidstr);
			break;
	}
	
	return uuidstr;
}

