/****************************************************************************************************************************
*
*   文 件 名 ： semaphore.h 
*   文件描述 ：  
*   创建日期 ：2019年4月11日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include "../baselib/baselib.h"

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


#define SYSEM_MODE 00666
#define SEM_MODE (IPC_CREAT | SYSEM_MODE)

#pragma pack(1)

union semun
{
	int val;
	struct semid_ds *buf;
	ushort *array;
};

typedef struct
{
	void * mdata;
	int mutex; //互斥信号量 
	int nempty, nstored;//同步信号量 
} SHARED_MEM;

#pragma pack()

 int sems_create(SHARED_MEM * p_map);
 int sems_destroy(const int id);
 int sems_get(SHARED_MEM * p_map);
 int sems_init(SHARED_MEM * p_map, int bufNum);
 int sem_post(const int id);
 int sem_wait(const int id);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __SEMAPHORE_H__ */
