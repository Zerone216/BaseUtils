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
*   文 件 名 ： libudp.h 
*   文件描述 ：  
*   创建日期 ：2019年4月11日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __LIBUDP_H__
#define __LIBUDP_H__

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

#define BROAD_CAST_ADDR "255.255.255.255"

#define TOTAL_DATA_LEN 1430

#pragma pack(1)

typedef struct UDP_PACKET
{
	int type;		/*数据包类型*/
	int pack_size;		/*包的有效数据*/
	BYTE mac[6];
}UDP_PACKET;

#pragma pack()
 int close_broadcast_socket(int sockfd);
 int close_multicast_socket(int sockfd, BYTE joinFlag);
 void close_udp_socket(int sockfd);
 int close_unicast_socket(int sockfd);
 struct sockaddr_in generate_new_addr(char * ip, int port, BYTE logFlag);
 struct ip_mreq generate_new_mreq(char * hostIp, char * groupIp);
 struct sockaddr_in * get_send_addr();
 struct sockaddr_in * get_recv_addr();
 struct ip_mreq get_multi_mreq_info();
 int init_broadcast_socket(char * broadip,int broadport, int sendbufLen, int recvbufLen, int addrReuse);
 int init_multicast_socket(char * localip, int localport, char * groupIp, int groupPort, int sendbufLen, int recvbufLen, int addrReuse, int joinFlag, int loop, int ttl);
 int init_uniqcast_socket(char * localip, int localport, char * dst_addr, int dst_port, int sendbufLen, int recvbufLen, int addrReuse, BYTE ttl);
 int recv_broadcast_pkt(int sockfd, BYTE * buf, int bufLen, BYTE * mac, int * pktType, double timeout);
 int recv_multicast_pkt(int sockfd, BYTE * buf, int bufLen, BYTE * mac, int * pktType, double timeout);
 int recv_udp_pkt(int sockfd, BYTE * buf, int bufLen, BYTE * mac, int * pktType, double timeout);
 int recv_uniqcast_pkt(int sockfd, BYTE * buf, int bufLen, BYTE * mac, int * pktType, double timeout);
 int send_broadcast_pkt(int sockfd, BYTE * mac, BYTE * buf, int bufLen,  int pktType);
 int send_multicast_pkt(int sockfd, BYTE * mac, BYTE * buf, int bufLen,  int pktType);
 int send_udp_pkt(int sockfd, BYTE * mac, BYTE * buf, int bufLen,  int pktType);
 int send_uniqcast_pkt(int sockfd, BYTE * mac, BYTE * buf, int bufLen,  int pktType);
 int set_send_addr_info(struct sockaddr_in sendAddr);
 int set_recv_addr_info(struct sockaddr_in recvAddr);
 int set_multi_mreq_info(struct ip_mreq multiMreq);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __LIBUDP_H__ */
