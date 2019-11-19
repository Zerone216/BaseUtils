/****************************************************************************************************************************
*
*   文 件 名 ： libudp.c 
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
#include <sys/un.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>

#include "libudp.h"

static struct ip_mreq mreq;

static struct sockaddr_in sendaddr; //要发送数据的地址
static struct sockaddr_in recvaddr;//要接收数据的地址

struct sockaddr_in generate_new_addr(char * ip, int port, BYTE logFlag)
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));

	if(ip == NULL)
		return addr;
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(port);
	
	if(logFlag)
		iLog("generate new addr: [%s:%d]", ip, port);
	
	return addr;
}

struct ip_mreq generate_new_mreq(char * hostIp, char * groupIp)
{
	struct ip_mreq gmreq;
	memset(&gmreq, 0, sizeof(struct ip_mreq));
	gmreq.imr_multiaddr.s_addr = inet_addr(groupIp);// 设置组地址
	gmreq.imr_interface.s_addr = inet_addr(hostIp); //设置接收组播消息的主机的地址信息
	
	iLog("generate new mreq:  join [%s] --> [%s]", hostIp, groupIp);
	
	return gmreq;
}

struct ip_mreq get_multi_mreq_info()
{
	return mreq;
}

int set_multi_mreq_info(struct ip_mreq multiMreq)
{
	mreq = multiMreq;
	return 0;
}

struct sockaddr_in * get_send_addr()
{
	return &sendaddr;
}

int set_send_addr_info(struct sockaddr_in sendAddr)
{
	sendaddr = sendAddr;
	return 0;
}

struct sockaddr_in * get_recv_addr()
{
	return &recvaddr;
}

int set_recv_addr_info(struct sockaddr_in recvAddr)
{
	recvaddr = recvAddr;
	return 0;
}


////////////////////////////////////////////////////////////////////////
int send_udp_pkt(int sockfd, BYTE * mac, BYTE * buf, int bufLen,  int pktType)
{
	BYTE sendBuf[1500];
	bzero(sendBuf, sizeof(sendBuf));
	struct UDP_PACKET *head = (struct UDP_PACKET *)sendBuf;

	int udppktLen = sizeof(struct UDP_PACKET);
	if(bufLen > TOTAL_DATA_LEN)
	{
		eLog("The data size is too long!");
		return -1;
	}
	
	/*设置packet结构*/
	head->type = pktType;
	head->pack_size = bufLen;/*有效数据长度,不包括任何包头*/
	memcpy(head->mac, mac, 6);
	
	/*添加数据*/
	if(buf != NULL)
		memcpy(sendBuf + sizeof(struct UDP_PACKET), buf, bufLen);
	else
		bufLen = 0;
	
	int datalen = udppktLen + bufLen;
	if(sendto(sockfd, sendBuf, datalen, 0, (struct sockaddr *)get_send_addr(), sizeof(struct sockaddr)) == -1)
	{
		assert(0);
		eLog("sendto pkt[0x%x] error!", pktType);
		return -1;
	}
	
	return 0;
}

int recv_udp_pkt(int sockfd, BYTE * buf, int bufLen, BYTE * mac, int * pktType, double timeout)
{
	char recvBuf[1500];
	struct UDP_PACKET *head =(struct UDP_PACKET *)recvBuf;
	if(bufLen < TOTAL_DATA_LEN)
	{
		eLog("The buff size is too small!");
		return -1;
	}
	
	socklen_t adrlen = sizeof(struct sockaddr);
	struct timeval tv = time_parse(timeout);  //超时时间设置
	fd_set r_set;
	
	while(TRUE)
	{
		FD_ZERO(&r_set);
		FD_SET(sockfd, &r_set);

		int ret = select(sockfd + 1, &r_set, NULL, NULL, &tv);
		if(ret < 0)
		{
			assert(0);
			eLog("select error");
			return -1;
		}
		else if(ret == 0)
		{
			if(timeout > 0.0)
			{
				assert(0);
				eLog("select timeout !");
			}
			
			return 0;
		}
		else
		{
			if(FD_ISSET(sockfd, &r_set))
			{
				int recvLen = recvfrom(sockfd, recvBuf, sizeof(recvBuf), 0, (struct sockaddr *)get_recv_addr(), &adrlen);
				if(recvLen < 0)
				{
					assert(0);
					eLog("recvfrom error!");
					return -1;
				}
				
				if(head->pack_size > 1500 || head->pack_size < 0)
					return -1;
				
				*pktType = head->type;
				memcpy(mac, head->mac, 6);
				memcpy(buf, recvBuf + sizeof(struct UDP_PACKET), head->pack_size);
				
				break;
			}
			else  //没有包可以读了
				return 0;
		}
	}
	
	return head->pack_size;
}

void close_udp_socket(int sockfd)
{
	close(sockfd); //shutdown(sockfd, SHUT_RDWR); 
	return;
}

///////////////////////////////UDP单播///////////////////////////////////
//初始化单播socket
int init_uniqcast_socket(char * localip, int localport, char * dst_addr, int dst_port, int sendbufLen, int recvbufLen, int addrReuse, BYTE ttl)
{
	/* 创建 socket 用于UDP通讯 */
	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd < 0)
	{
		assert(0);
		eLog("socket creating err in udptalk");
		return -1;
	}
	
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &addrReuse, sizeof(addrReuse)) < 0)
	{
		assert(0);
		eLog("Setting SO_REUSEADDR error");
		close_udp_socket(sockfd);
		return -1;
	}
	
	if(setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sendbufLen, sizeof(sendbufLen)) < 0)
	{
		assert(0);
		eLog("Setting SO_SNDBUF error");
		close_udp_socket(sockfd);
		return -1;
	}

	if(setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &recvbufLen, sizeof(recvbufLen)) < 0)
	{
		assert(0);
		eLog("Setting SO_SNDBUF error");
		close_udp_socket(sockfd);
		return -1;
	}

	/*
	int donot = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_DONTROUTE, &donot, sizeof(donot)) < 0)
	{
		assert(0);
		eLog("Setting SO_DONTROUTE error");
		close_udp_socket(sockfd);
		return -1;
	}
	*/
	
	if(setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
	{
		assert(0);
		eLog("setsockopt IP_TTL failed!");
		close_udp_socket(sockfd);
		return -1;
	}
		
	struct sockaddr_in recvAddr = generate_new_addr(localip, localport, TRUE);// htonl(INADDR_ANY)
	struct sockaddr_in sendAddr = generate_new_addr(dst_addr, dst_port, TRUE);
	set_send_addr_info(sendAddr);
	set_recv_addr_info(recvAddr);
	
	if(bind(sockfd, (struct sockaddr *)get_recv_addr(), sizeof(struct sockaddr_in)) == -1)/* 绑定端口和IP信息到socket上 */
	{
		assert(0);
		eLog("Bind error!");
		close_udp_socket(sockfd);
		return -1;
	}
	
	iLog("Bind localaddr succeed!");
	return sockfd;
}

int send_uniqcast_pkt(int sockfd, BYTE * mac, BYTE * buf, int bufLen,  int pktType)
{
	return send_udp_pkt(sockfd, mac, buf, bufLen, pktType);
}

int recv_uniqcast_pkt(int sockfd, BYTE * buf, int bufLen, BYTE * mac, int * pktType, double timeout)
{
	return recv_udp_pkt(sockfd, buf, bufLen, mac, pktType, timeout);
}

int close_unicast_socket(int sockfd)
{
	close_udp_socket(sockfd);
	return 0;
}

////////////////////////////////////UDP组播///////////////////////////////////////
//初始化组播socket
int init_multicast_socket(char * localip, int localport, char * groupIp, int groupPort, int sendbufLen, int recvbufLen, int addrReuse, int joinFlag, int loop, int ttl)
{
	/* 创建 socket 用于UDP通讯 */
	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd < 0)
	{
		assert(0);
		eLog("socket creating err in udptalk");
		return -1;
	}
	
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&addrReuse, sizeof(addrReuse)) < 0)
	{
		assert(0);
		eLog("Setting SO_REUSEADDR error");
		close_udp_socket(sockfd);
		return -1;
	}

	if(setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sendbufLen, sizeof(sendbufLen)) < 0)
	{
		assert(0);
		eLog("Setting SO_SNDBUF error");
		close_udp_socket(sockfd);
		return -1;
	}

	if(setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &recvbufLen, sizeof(recvbufLen)) < 0)
	{
		assert(0);
		eLog("Setting SO_RCVBUF error");
		close_udp_socket(sockfd);
		return -1;
	}

	/*
	int donot = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_DONTROUTE, (char*)&donot, sizeof(donot)) < 0)
	{
		assert(0);
		eLog("Setting SO_DONTROUTE error");
		close_udp_socket(sockfd);
		return -1;
	}
	*/

	struct sockaddr_in localAddr = generate_new_addr(localip, localport, TRUE);// htonl(INADDR_ANY)
	struct sockaddr_in multiAddr = generate_new_addr(groupIp, groupPort, TRUE);
	
	if(joinFlag == TRUE) //客户端需要添加本机IP到组播地址，从组播地址接收数据需要绑定组播地址，使用单播发送数据给服务端地址
	{
		if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0)
		{
			assert(0);
			eLog("setsockopt IP_MULTICAST_LOOP failed!");
			close_udp_socket(sockfd);
			return -1;
		}
		
		if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
		{
			assert(0);
			eLog("setsockopt IP_MULTICAST_TTL failed!");
			close_udp_socket(sockfd);
			return -1;
		}

		/*
		struct in_addr addr;
		addr.s_addr = inet_addr(inet_ntoa(localip));
		if(setsockopt(sockfd,IPPROTO_IP,IP_MULTICAST_IF,&addr,sizeof(addr)) < 0)
		{
			assert(0);
			eLog("setsockopt IP_MULTICAST_IF failed!");
			close_udp_socket(sockfd);
			return -1;
		}
		*/

		// 设置要加入组播的地址
		set_multi_mreq_info(generate_new_mreq(localip, groupIp));// htonl(INADDR_ANY);
		struct ip_mreq imreq = get_multi_mreq_info();
		if(setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imreq, sizeof(struct ip_mreq)) < 0) //把本机加入组播地址，即本机网卡作为组播成员，只有加入组才能收到组播消息
		{
			assert(0);
			eLog("setsockopt IP_ADD_MEMBERSHIP failed!");
			close_udp_socket(sockfd);
			return -1;
		}

		set_recv_addr_info(multiAddr);//从组播地址读取数据
		
		/* 绑定端口和IP信息到socket上 */
		if(bind(sockfd, (struct sockaddr *)get_recv_addr(), sizeof(struct sockaddr_in)) < 0)
		{
			assert(0);
			eLog("Bind error!");
			close_udp_socket(sockfd);
			return -1;
		}
		
		iLog("Bind multiaddr succeed!");
		
	}
	else //服务端不需要添加本机IP到组播IP，只需要往组播地址发数据即可，但是接受数据要绑定本机地址
	{
		set_send_addr_info(multiAddr);//往组播地址发送数据
		set_recv_addr_info(localAddr);
		if(bind(sockfd, (struct sockaddr *)get_recv_addr(), sizeof(struct sockaddr_in)) == -1)/* 绑定端口和IP信息到socket上 */
		{
			assert(0);
			eLog("Bind error!");
			close_udp_socket(sockfd);
			return -1;
		}

		iLog("Bind localaddr succeed!");
	}
	
	return sockfd;
}

int send_multicast_pkt(int sockfd, BYTE * mac, BYTE * buf, int bufLen,  int pktType)
{
	return send_udp_pkt(sockfd, mac, buf, bufLen, pktType);
}

int recv_multicast_pkt(int sockfd, BYTE * buf, int bufLen, BYTE * mac, int * pktType, double timeout)
{
	return recv_udp_pkt(sockfd, buf, bufLen, mac, pktType, timeout);
}

int close_multicast_socket(int sockfd, BYTE joinFlag)
{
	if(joinFlag)
	{
		struct ip_mreq multiMreq = get_multi_mreq_info();
		if(setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &multiMreq, sizeof(struct ip_mreq)) < 0)//把本机脱离组播地址
		{
			assert(0);
			eLog("setsockopt IP_DROP_MEMBERSHIP failed!");
			return -1;
		}
	}
	
	close_udp_socket(sockfd);
	return 0;
}

////////////////////////////////////UDP广播///////////////////////////////////////
//初始化服务端广播socket
int init_broadcast_socket(char * broadip,int broadport, int sendbufLen, int recvbufLen, int addrReuse)
{
	/* 创建 socket 用于UDP通讯 */
	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd < 0)
	{
		assert(0);
		eLog("socket creating error in udptalk!");
		return -1;
	}
	
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &addrReuse, sizeof(addrReuse)) < 0)
	{
		assert(0);
		eLog("setsockopt SO_REUSEADDR error!");
		close_udp_socket(sockfd);
		return -1;
	}
	
	int opt = 1;//设置该套接字为广播类型
	if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) < 0)
	{
		assert(0);
		eLog("setsockopt SO_BROADCAST error!");
		close_udp_socket(sockfd);
		return -1;
	}

	if(setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sendbufLen, sizeof(sendbufLen)) < 0)
	{
		assert(0);
		eLog("setsockopt SO_SNDBUF error!");
		close_udp_socket(sockfd);
		return -1;
	}

	if(setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &recvbufLen, sizeof(recvbufLen)) < 0)
	{
		assert(0);
		eLog("setsockopt SO_RCVBUF error!");
		close_udp_socket(sockfd);
		return -1;
	}
	
	struct sockaddr_in broadAddr = generate_new_addr(broadip, broadport, TRUE);
	set_send_addr_info(broadAddr);//往广播地址发送数据
	set_recv_addr_info(broadAddr);//从广播地址接收数据
	
	if(bind(sockfd, (struct sockaddr *)get_recv_addr(), sizeof(struct sockaddr_in)) == -1)/* 绑定端口和IP信息到socket上 */
	{
		assert(0);
		eLog("Bind error!");
		close_udp_socket(sockfd);
		return -1;
	}
	
	iLog("Bind broadaddr succeed!");
	
	return sockfd;
}

int send_broadcast_pkt(int sockfd, BYTE * mac, BYTE * buf, int bufLen,  int pktType)
{
	return send_udp_pkt(sockfd, mac, buf, bufLen, pktType);
}

int recv_broadcast_pkt(int sockfd, BYTE * buf, int bufLen, BYTE * mac, int * pktType, double timeout)
{
	return recv_udp_pkt(sockfd, buf, bufLen, mac, pktType, timeout);
}

int close_broadcast_socket(int sockfd)
{
	close_udp_socket(sockfd);
	return 0;
}

