/****************************************************************************************************************************
*
*   文 件 名 ： netopt.h 
*   文件描述 ：  
*   创建日期 ：2019年4月11日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __NETOPT_H__
#define __NETOPT_H__

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
#include <sys/un.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>

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

#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR    (-1)

#define FMT_LOWER 0X00
#define FMT_UPPER 0X01

#define LOACL_NETCARD_SUPPORT_MAX 8

#pragma pack(1)

struct ROUTE_INFO
{
	char ifName[256];
	DWORD dstAddr;
	DWORD srcAddr;
	DWORD gateWay;
};

typedef struct _NETCARD_INFO_
{
	char devName[64];
	BYTE macAddr[6];
	char ipAddr[16];
	char gateWay[16];
	char netMask[16];
	char broadCast[16];
	int mTu;
	int meTric;
	int linkStatus;
	int wakeOnLan;
	int autoNeg;
	int duplex;
	int speed;
}NETCARD_INFO, *pNETCARD_INFO;

typedef struct _LOCAL_NETCARD_INFO_
{
	int netCardNum;
	NETCARD_INFO netCardInfo[LOACL_NETCARD_SUPPORT_MAX];
}LOCAL_NETCARD_INFO, *pLOCAL_NETCARD_INFO;

#pragma pack()

int get_loacl_netcard_info(NETCARD_INFO * pNetCardInfo, int index);
int get_loacl_netcard_num();
int get_local_broadcast(char * netdev, struct in_addr * broadcast);
char * get_local_broadcast_as_string(char * netdev);
int get_local_gateway(char * netdev, struct in_addr * gateway);
char * get_local_gateway_as_string(char * netdev);
int get_local_ip_addr( char * netdev, struct in_addr * ipAddr);
char * get_local_ip_addr_as_string(char * netdev);
int get_local_mac_addr(char * netdev, BYTE * macAddr);
char * get_local_mac_addr_as_string(char * netdev, BYTE loru, char spacer);
int get_local_netmask( char * netdev, struct in_addr * netmask);
char * get_local_netmask_as_string(char * netdev);
int get_local_network_info( char * netdev, char * ipaddr, char * netmask, char * gateway, char * broadcast);
int get_netcard_device_name(int index, char * netDevNmae, int bufLen);
int get_netcard_ethtool_info(char * netdev, int * speedSpt, int * duplexSpt, int * speed, int * duplex, int * autoneg);
int get_netcard_link_status( char * netdev);

char * get_subnet_prestr(char * netdev);
char * get_subnet_prestr_from_ip(char * ipaddr);
BYTE get_subnet_sufstr(char * netdev);
BYTE get_subnet_sufstr_from_ip(char * ipaddr);

int set_local_broadcast(char * netdev, struct in_addr * broadcast);
int set_local_broadcast_as_string(char * netdev, char * broadcast);
int set_local_gateway( char * netdev, struct in_addr * gateway);
int set_local_gateway_as_string(char * netdev, char * gateway);
int set_local_ip_addr( char * netdev, struct in_addr * ipAddr);
int set_local_ip_addr_as_string(char * netdev, char * ipaddr);
int set_local_netmask( char * netdev, struct in_addr * netmask);
int set_local_netmask_as_string(char * netdev, char * netmask);
int set_local_network_info( char * netdev, char * ipaddr, char * netmask, char * gateway, char * broadcast);

LOCAL_NETCARD_INFO * local_netcard_info_init();
int local_netcard_info_release(LOCAL_NETCARD_INFO ** pLocalNetcardInfo);
int local_netcard_info_display(LOCAL_NETCARD_INFO * pLocalNetcardInfo);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __NETWORK_H__ */
