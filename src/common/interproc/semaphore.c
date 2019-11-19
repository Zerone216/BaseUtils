/****************************************************************************************************************************
*
*   文 件 名 ： semaphore.c 
*   文件描述 ：  
*   创建日期 ：2019年4月11日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <regex.h>
#include <stdarg.h>	/* ANSI C header file */
#include <syslog.h> 	/* for syslog() */
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/sem.h>

#include <getopt.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <asm/types.h>
#include <arpa/inet.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <linux/ethtool.h>
#include <linux/rtnetlink.h>
#include <linux/sockios.h>
#include <linux/if_ether.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/io.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>

#include "semaphore.h"

/*创建System_V信号灯*/
int sems_create(SHARED_MEM * p_map)
{
	/*allocate system V semaphore*/
	if((p_map->mutex = semget(IPC_PRIVATE, 1, SEM_MODE)) == -1)
		err_sys("semget mutex error");

	if((p_map->nempty = semget(IPC_PRIVATE, 1, SEM_MODE)) == -1)
		err_sys("semget nempty error");

	if((p_map->nstored = semget(IPC_PRIVATE, 1, SEM_MODE)) == -1)
		err_sys("semget nstored error");
	
	return 0;
}

/*为信号灯赋初值*/
int sems_init(SHARED_MEM * p_map, int bufNum)
{
	unsigned short *sptr;
	union semun arg;

	if((sptr = (unsigned short *)calloc(1, sizeof(unsigned short))) == NULL)
		err_sys("calloc error");
	
	arg.array = sptr;
	
	sptr[0] = 1;
	if(semctl(p_map->mutex, 0, SETALL, arg) == -1)
		err_sys("semctl mutex error");

	sptr[0] = bufNum;
	if(semctl(p_map->nempty, 0, SETALL, arg) == -1)
		err_sys("semctl nempty error");

	sptr[0] = 0;
	if(semctl(p_map->nstored, 0, SETALL, arg) == -1)
		err_sys("semctl nstored error");
	
	free(sptr);
	return 0;
}

/*显示创建的信号灯信息*/
int sems_get(SHARED_MEM * p_map)
{
	unsigned short *sptr;
	union semun arg;

	if((sptr = (unsigned short *)calloc(1, sizeof(unsigned short))) == NULL)
		err_sys("calloc error");

	arg.array = sptr;

	semctl(p_map->mutex, 0, GETALL, arg);
	iLog("mutex:id[%d] val[%d] ", p_map->mutex, *sptr);
	semctl(p_map->nempty, 0, GETALL, arg);
	iLog("nempty:id[%d] val[%d] ", p_map->nempty, *sptr);
	semctl(p_map->nstored, 0, GETALL, arg);
	iLog("nstored:id[%d] val[%d]\n", p_map->nstored, *sptr);
	
	free(sptr);
	
	return 0;
}

/*获取信号灯*/
int sem_wait(const int id)
{
	struct sembuf op;

	op.sem_num = 0;
	op.sem_op = -1;
	op.sem_flg = 0;

	if(semop(id, &op, 1) == -1)
		return (-1);

	return 0;
}

/*释放信号灯*/
int sem_post(const int id)
{
	struct sembuf op;

	op.sem_num = 0;
	op.sem_op = 1;
	op.sem_flg = 0;

	if(semop(id, &op, 1) == -1)
		return (-1);

	return 0;
}

/*销毁信号灯*/
int sems_destroy(const int id)
{
	if(semctl(id, 0, IPC_RMID) == -1)
		err_sys("sem:%d destroy error");

	return 0;
}

