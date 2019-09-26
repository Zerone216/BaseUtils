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
*   文 件 名 ： libtcp.h 
*   文件描述 ：  
*   创建日期 ：2019年4月10日
*   版本号 ：   
*   负责人 ： Zhangsheng 
*   其 他 ：
*   修改日志 ：
*
****************************************************************************************************************************/

#ifndef __LIBTCP_H__
#define __LIBTCP_H__

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


#define TCP_SOCK_NONBLOCK 0  //非阻塞
#define TCP_SOCK_BLOCK 1 //阻塞

#define SNDBUF_LEN 204800
#define RCVBUF_LEN 204800

#define PKT_FLAG 0x55aa77ee

#pragma pack(1)

struct package_tcp_head
{
	DWORD pkt_flag;  //0x55aa77ee
	int pkt_type;		/*数据包类型*/
	DDWORD pkt_len;		/*包的有效数据*/
};

#pragma pack()

 void close_tcp_socket(int sockfd);
 int connect_tcp_server(char * serverip, int serverport, double timeout, int blockMode);
 int init_tcp_server(char * serverip, int serverport, double timeout, int blockMode);
 int init_tcp_socket(int blockMode, int sendbufLen, int recvbufLen, int addrReuse, int portReuse);
 int recv_tcp_pkt(int sockfd, char * buf, int bufLen, double timeout, int pktType);
 int recv_tcp_pkt_data(int sockfd, char * buf, int bufLen, double timeout);
 int recv_tcp_pkt_head(int sockfd, int pktType, int timeout);
 int send_tcp_pkt(int sockfd, char * buf, int bufLen, double timeout, int pktType);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __LIBTCP_H__ */
