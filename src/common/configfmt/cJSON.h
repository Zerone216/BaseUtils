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
*   文 件 名 ： cJSON.h 
*   文件描述 ：  
*   创建日期 ：2019年8月27日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __CJSON_H__
#define __CJSON_H__


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

/* cJSON Types: */
#define cJSON_False  	(1 << 0)
#define cJSON_True   	(1 << 1)
#define cJSON_NULL   	(1 << 2)
#define cJSON_Number 	(1 << 3)
#define cJSON_String 	(1 << 4)
#define cJSON_Array  	(1 << 5)
#define cJSON_Object 	(1 << 6)
#define cJSON_IsReference	256
#define cJSON_StringIsConst	512

#pragma pack(1)

/* The cJSON structure: */
typedef struct cJSON {
	struct cJSON *next,*prev;	/* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
	struct cJSON *child;		/* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
	int type;					/* The type of the item, as above. */
	char *valuestring;			/* The item's string, if type==cJSON_String */
	int valueint;				/* The item's number, if type==cJSON_Number */
	double valuedouble;			/* The item's number, if type==cJSON_Number */
	char *string;				/* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
} cJSON;

typedef struct cJSON_Hooks {
      void *(*malloc_fn)(size_t sz);
      void (*free_fn)(void *ptr);
}cJSON_Hooks;

typedef struct _printbuffer_{
	char *buffer; 
	int length; 
	int offset; 
} printbuffer;

#pragma pack()

 void cJSON_AddItemReferenceToArray(cJSON *array, cJSON *item);
 void cJSON_AddItemReferenceToObject(cJSON *object,const char *string,cJSON *item);
 void cJSON_AddItemToArray(cJSON *array, cJSON *item);
 void cJSON_AddItemToObject(cJSON *object,const char *string,cJSON *item);
 void cJSON_AddItemToObjectCS(cJSON *object,const char *string,cJSON *item);
 cJSON *cJSON_CreateArray(void);
 cJSON *cJSON_CreateBool(int b);
 cJSON *cJSON_CreateDoubleArray(const double *numbers,int count);
 cJSON *cJSON_CreateFalse(void);
 cJSON *cJSON_CreateFloatArray(const float *numbers,int count);
 cJSON *cJSON_CreateIntArray(const int *numbers,int count);
 cJSON *cJSON_CreateNull(void);
 cJSON *cJSON_CreateNumber(double num);
 cJSON *cJSON_CreateObject(void);
 cJSON *cJSON_CreateString(const char *string);
 cJSON *cJSON_CreateStringArray(const char **strings,int count);
 cJSON *cJSON_CreateTrue(void);
 void cJSON_Delete(cJSON *c);
 void cJSON_DeleteItemFromArray(cJSON *array,int which);
 void cJSON_DeleteItemFromObject(cJSON *object,const char *string);
 cJSON *cJSON_DetachItemFromArray(cJSON *array,int which);
 cJSON *cJSON_DetachItemFromObject(cJSON *object,const char *string);
 cJSON *cJSON_Duplicate(cJSON *item,int recurse);
 cJSON *cJSON_GetArrayItem(cJSON *array,int item);
 int cJSON_GetArraySize(cJSON *array);
 const char *cJSON_GetErrorPtr(void);
 cJSON *cJSON_GetObjectItem(cJSON *object,const char *string);
 int cJSON_HasObjectItem(cJSON *object,const char *string);
 void cJSON_InsertItemInArray(cJSON *array,int which,cJSON *newitem);
 void cJSON_Minify(char *json);
 cJSON *cJSON_Parse(const char *value);
 cJSON *cJSON_ParseWithOpts(const char *value,const char **return_parse_end,int require_null_terminated);
 char *cJSON_Print(cJSON *item);
 char *cJSON_PrintBuffered(cJSON *item,int prebuffer,int fmt);
 char *cJSON_PrintUnformatted(cJSON *item);
 void cJSON_ReplaceItemInArray(cJSON *array,int which,cJSON *newitem);
 void cJSON_ReplaceItemInObject(cJSON *object,const char *string,cJSON *newitem);
 cJSON * cJSON_ReadFromFile(const char * filename); 
 int cJSON_SaveToFile(cJSON *cjson, const char * filename);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CJSON_H__ */
