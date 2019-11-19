/****************************************************************************************************************************
*
*   文 件 名 ： netopt.c 
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
#include <error.h>
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
#include <net/route.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>

#include "netopt.h"

#define NETCARD_WORK_MODE_HALF 0X01
#define NETCARD_WORK_MODE_FULL 0X02

int get_netcard_ethtool_info(char * netdev, int * speedSpt, int * duplexSpt, int * speed, int * duplex, int * autoneg)
{
	if(netdev == NULL || strlen(netdev) == 0)
		return -1;

	if(speedSpt == NULL ||duplexSpt == NULL || autoneg == NULL)
		return -1;
	
	*speedSpt = 0;
	*duplexSpt = 0;
	*autoneg = 0;
	
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);//通过socket访问网卡的 文件描述符号fd
	if(sockfd < 0)
	{
		assert(0);
		eLog("Cannot get control socket!");
		return -1;
	}

	struct ifreq ifr;  // 接口请求结构
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, netdev, sizeof(ifr.ifr_name));
	
	struct ethtool_cmd ep;
	ep.cmd = ETHTOOL_GSET; // ethtool-copy.h:380:#define ETHTOOL_GSET  0x00000001 /* Get settings. */
	ifr.ifr_data = (caddr_t)&ep; // caddr_t 是void类型，而这句话是什么意思
	if(ioctl(sockfd, SIOCETHTOOL, &ifr) != 0)  // 如果出错退出;
	{
		assert(0);
		eLog("ioctl SIOCETHTOOL is error!");
		close(sockfd);
		return -1;
	}

	close(sockfd);

	*speed = ep.speed;
	*duplex = ep.duplex;
	*autoneg = ep.autoneg;
	
	if((ep.supported & SUPPORTED_1000baseT_Full) > 0)
	{
		*speedSpt = 1000;
		*duplexSpt = NETCARD_WORK_MODE_FULL;
		return 0;
	}
	else if((ep.supported & SUPPORTED_1000baseT_Half) > 0)
	{
		*speedSpt = 1000;
		*duplexSpt = NETCARD_WORK_MODE_HALF;
		return 0;
	}
	else if((ep.supported & SUPPORTED_100baseT_Full) > 0)
	{
		*speedSpt = 100;
		*duplexSpt = NETCARD_WORK_MODE_FULL;
		return 0;
	}
	else if((ep.supported & SUPPORTED_100baseT_Half) > 0)
	{
		*speedSpt = 100;
		*duplexSpt = NETCARD_WORK_MODE_HALF;
		return 0;
	}
	else if((ep.supported & SUPPORTED_10baseT_Full) > 0)
	{
		*speedSpt = 10;
		*duplexSpt = NETCARD_WORK_MODE_FULL;
		return 0;
	}
	else if((ep.supported & SUPPORTED_10baseT_Half) > 0)
	{
		*speedSpt = 10;
		*duplexSpt = NETCARD_WORK_MODE_HALF;
		return 0;
	}
	else
		return -1;
}

int get_netcard_link_status( char * netdev)
{
	/* Open a socket. */
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0)
	{
		assert(0);
		eLog("socket error!");
		return -1;
	}
	
	struct ethtool_value edata;
	edata.cmd = ETHTOOL_GLINK;
	
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, netdev, sizeof(ifr.ifr_name));
	ifr.ifr_data = (char *)&edata;

	if(ioctl(sockfd, SIOCETHTOOL, &ifr) == -1)
	{
		assert(0);
		eLog("ioctl SIOCETHTOOL failed!");
		close(sockfd);
		return -1;
	}
	
	close(sockfd);
	return edata.data;
}



/*******************************************************************************************************************
 * 函 数 名  ：  get_local_mac_addr
 * 负 责 人  ：  Zhangsheng
 * 创建日期  ：  2018年8月31日
 * 函数功能  ：  通过inctl获取本机MAC地址，以u_int8_t输出
 * 参数列表   ： 空
 * 返 回 值  ：  获取的u_int8_t值的指针
 * 调用关系  ：  
 * 其  它  ：  
 *
*******************************************************************************************************************/
int get_local_mac_addr(char * netdev, BYTE * macAddr)
{
	if(netdev == NULL || strlen(netdev) == 0)
		return -1;

	if(macAddr == NULL)
		return -1;
	
	struct ifreq ifr;  // 接口请求结构
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, netdev, sizeof(ifr.ifr_name));
	
	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if(sockfd == -1)
	{
		assert(0);
		eLog("Cannot get control socket!");
		return -1;
	}
	
	if(ioctl(sockfd, SIOCGIFHWADDR, &ifr) != 0)
	{
		assert(0);
		eLog("ioctl SIOCGIFHWADDR is error!");
		close(sockfd);
		return -1;
	}

	switch(ifr.ifr_hwaddr.sa_family)
	{
		case ARPHRD_ETHER:
			memcpy(macAddr, &ifr.ifr_addr.sa_data, 6);
			break;
		
		default:
			memset(macAddr, 0x00, 6);
			break;
	}
	
	close(sockfd);
	return 0;
}

char * get_local_mac_addr_as_string(char * netdev, BYTE loru, char spacer)
{
	if(netdev == NULL || strlen(netdev) == 0)
		return NULL;
	
	BYTE * macAddr = (BYTE *) malloc(6);
	if(macAddr == NULL)
		return NULL;

	if(get_local_mac_addr(netdev, macAddr) == -1)
	{
		safe_free(&macAddr);
		return NULL;
	}
	
	char * fmt = NULL;
	if(loru == FMT_UPPER)
		fmt = str_dup("%02X%c%02X%c%02X%c%02X%c%02X%c%02X", 128);
	else if(loru == FMT_LOWER)
		fmt = str_dup("%02x%c%02x%c%02x%c%02x%c%02x%c%02x", 128);
	else
		;
	
	if(fmt == NULL)
	{
		safe_free(&macAddr);
		return NULL;
	}
	
	char * macStr = str_dup_printf(64, fmt, \
					macAddr[0], spacer,\
					macAddr[1], spacer,\
					macAddr[2], spacer,\
					macAddr[3], spacer,\
					macAddr[4], spacer,\
					macAddr[5]);
	
	str_free(&fmt);
	safe_free(&macAddr);
	
	if(macStr == NULL)
		return NULL;
	else
		return macStr;
}


int get_local_ip_addr( char * netdev, struct in_addr * ipAddr)
{
	if(netdev == NULL || strlen(netdev) == 0)
		return -1;

	if(ipAddr == NULL)
		return -1;

	ipAddr->s_addr = 0;
	
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd == -1)
	{
		assert(0);
		eLog("socket error!");
		return -1;
	}

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, netdev, sizeof(ifr.ifr_name));
	if(ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) //SIOCGIFADDR 获取interface address
	{
		assert(0);
		eLog("ioctl SIOCGIFADDR is error!");
		close(sockfd);
		return -1;
	}

	struct sockaddr_in sin;
	memcpy(&sin, &ifr.ifr_addr, sizeof(ifr.ifr_addr));
	memcpy(&ipAddr->s_addr, &sin.sin_addr, sizeof(DWORD));
	
	close(sockfd);
	return 0;
}

char * get_local_ip_addr_as_string(char * netdev)
{
	struct in_addr addr;
	if(get_local_ip_addr(netdev, &addr) == -1)
		return NULL;
	
	return str_dup_printf(20, "%s", inet_ntoa(addr));
}

int set_local_ip_addr_as_string(char * netdev, char * ipaddr)
{
	ifconfig(netdev, "0.0.0.0");
	return ifconfig(netdev, ipaddr);
}

int get_local_netmask( char * netdev, struct in_addr * netmask)
{
	if(netdev == NULL || strlen(netdev) == 0)
		return -1;

	if(netmask == NULL)
		return -1;

	netmask->s_addr = 0;
		
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd == -1)
	{
		assert(0);
		eLog("socket error!");
		return -1;
	}

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, netdev, sizeof(ifr.ifr_name));
	if(ioctl(sockfd, SIOCGIFNETMASK, &ifr) < 0) 
	{
		assert(0);
		eLog("ioctl SIOCGIFNETMASK is error!");
		close(sockfd);
		return -1;	
	}

	struct sockaddr_in sin;
	memcpy(&sin, &ifr.ifr_addr, sizeof(ifr.ifr_addr));
	memcpy(&netmask->s_addr, &sin.sin_addr, sizeof(DWORD));
	
	close(sockfd);
	return 0;
}

char * get_local_netmask_as_string(char * netdev)
{
	struct in_addr addr;
	if(get_local_netmask(netdev, &addr) == -1)
		return NULL;
	
	return str_dup_printf(20, "%s", inet_ntoa(addr));
}

int set_local_netmask_as_string(char * netdev, char * netmask)
{
	int ret = -1;
	char * pSetting = str_dup_printf(64, "netmask %s", netmask);
	if(pSetting)
	{
		ret = ifconfig(netdev, pSetting);
		str_free(&pSetting);
	}
	
	return ret;
}

int get_local_broadcast(char * netdev, struct in_addr * broadcast)
{
	if(netdev == NULL || strlen(netdev) == 0)
		return -1;

	if(broadcast)
	{
		int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if(sockfd == -1)
		{
			assert(0);
			eLog("socket error!");
			return -1;
		}

		struct ifreq ifr;
		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, netdev, sizeof(ifr.ifr_name));
		
		struct sockaddr_in * sin = (struct sockaddr_in *)&ifr.ifr_addr;
		
		if(ioctl(sockfd, SIOCGIFBRDADDR, &ifr) < 0)	 
		{
			assert(0);
			eLog("ioctl SIOCGIFBRDADDR error!");
			close(sockfd);
			return -1;
		}

		memcpy(broadcast, &sin->sin_addr, sizeof(DWORD));
		
		close(sockfd);
	}
	
	return 0;
}

char * get_local_broadcast_as_string(char * netdev)
{
	struct in_addr addr;
	if(get_local_broadcast(netdev, &addr) == -1)
		return NULL;
	
	return str_dup_printf(20, "%s", inet_ntoa(addr));
}

int set_local_broadcast_as_string(char * netdev, char * broadcast)
{
	int ret = -1;
	char * pSetting = str_dup_printf(64, "broadcast %s", broadcast);
	if(pSetting)
	{
		ret = ifconfig(netdev, pSetting);
		str_free(&pSetting);
	}
	
	return ret;
}

static int readNlSock(int sockFd, char * bufPtr, int bufLen, int seqNum, int pId)
{
	struct nlmsghdr *nlHdr;
	int readLen = 0, msgLen = 0;
	
	do
	{
		/* Recieve response from the kernel */
		if((readLen = recv(sockFd, bufPtr, bufLen - msgLen, 0)) < 0)
		{
			assert(0);
			eLog("SOCK READ error!");
			return -1;
		}
		
		nlHdr = (struct nlmsghdr *)bufPtr;

		/* Check if the header is valid */
		if((NLMSG_OK(nlHdr, readLen) == 0) || (nlHdr->nlmsg_type == NLMSG_ERROR))
		{
			assert(0);
			eLog("Error in recieved packet");
			return -1;
		}

		/* Check if the its the last message */
		if(nlHdr->nlmsg_type == NLMSG_DONE)
			break;
		else
		{
			/* Else move the pointer to buffer appropriately */
			bufPtr += readLen;
			msgLen += readLen;
		}

		/* Check if its a multi part message */
		if((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0)/* return if its not */
			break;
	}while((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != pId));
	
	return msgLen;
}

/* For parsing the route info returned */
static int parseRoutes(char * netdev, struct nlmsghdr *nlHdr, struct ROUTE_INFO *rtInfo,struct in_addr * gateway)
{
	struct rtmsg *rtMsg;
	struct rtattr *rtAttr;
	int rtLen;
	char *tempBuf = NULL;

	tempBuf = (char *)malloc(100);
	rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);

	/* If the route is not for AF_INET or does not belong to main routing tablethen return. */
	if((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN))
		return -1;

	/* get the rtattr field */
	rtAttr = (struct rtattr *)RTM_RTA(rtMsg);
	rtLen = RTM_PAYLOAD(nlHdr);
	for(;RTA_OK(rtAttr,rtLen);rtAttr = RTA_NEXT(rtAttr,rtLen))
	{
		switch(rtAttr->rta_type)
		{
			case RTA_OIF:
				if_indextoname(*(int *)RTA_DATA(rtAttr), rtInfo->ifName);
				break;
			case RTA_GATEWAY:
				rtInfo->gateWay = *(u_int *)RTA_DATA(rtAttr);
				break;
			case RTA_PREFSRC:
				rtInfo->srcAddr = *(u_int *)RTA_DATA(rtAttr);
				break;
			case RTA_DST:
				rtInfo->dstAddr = *(u_int *)RTA_DATA(rtAttr);
				break;
		}
	}

	free(tempBuf);

	struct in_addr dstAddr;
	dstAddr.s_addr = rtInfo->dstAddr;
	if (strncmp(rtInfo->ifName, netdev, strlen(netdev)) == 0 && \
		strstr((char *)inet_ntoa(dstAddr), "0.0.0.0"))
	{
		gateway->s_addr = rtInfo->gateWay;
		return 0;
	}
	else
		return -1;
}

static int get_nlmsg_info(struct nlmsghdr * nlMsg, int bufLen)
{
	if(nlMsg == NULL)
		return -1;
	
	int msgSeq = 0;
	
	nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)); // Length of message.
	nlMsg->nlmsg_type = RTM_GETROUTE; // Get the routes from kernel routing table .
	nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; // The message is a request for dump.
	nlMsg->nlmsg_seq = msgSeq ++; // Sequence of the message packet.
	nlMsg->nlmsg_pid = getpid(); // PID of process sending the request.
	
	/* Create Socket */
	int sock, len;
	if((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)
		return -1;
	
	/* Send the request */
	if(send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0)
	{
		assert(0);
		eLog("Write To Socket Failed...\n");
		close(sock);
		return -1;
	}
	
	/* Read the response */
	if((len = readNlSock(sock, (char *)nlMsg, bufLen, msgSeq, getpid())) < 0) 
	{
		eLog("Read From Socket Failed...\n");
		close(sock);
		return -1;
	}
	
	close(sock);
	return len;
}

int get_local_gateway(char * netdev, struct in_addr * gateway)
{
	if(netdev == NULL || strlen(netdev) == 0)
		return -1;

	if(gateway == NULL)
		return -1;

	int bufLen = 8192;
	char * msgBuf = (char *) z_malloc(bufLen);
	if(msgBuf == NULL)
		return -1;
 	
	/* point the header and the msg structure pointers into the buffer */
	struct nlmsghdr * nlMsg = (struct nlmsghdr *)msgBuf;
	int len = get_nlmsg_info(nlMsg, bufLen);
	if( len < 0)
	{
		z_free(&msgBuf);
		return -1;
	}
	
	struct ROUTE_INFO * rtInfo = (struct ROUTE_INFO *)z_malloc(sizeof(struct ROUTE_INFO));
	if(rtInfo == NULL)
	{
		z_free(&msgBuf);
		return -1;
	}
	
	for(;NLMSG_OK(nlMsg,len);nlMsg = NLMSG_NEXT(nlMsg,len))
	{
		memset(rtInfo, 0, sizeof(struct ROUTE_INFO));
		if(parseRoutes(netdev, nlMsg, rtInfo, gateway) == 0)
			break;
	}
	
	z_free(&rtInfo);
	z_free(&msgBuf);
	
	return 0;
}

char * get_local_gateway_as_string(char * netdev)
{
	struct in_addr addr;
	if(get_local_gateway(netdev, &addr) == -1)
		return NULL;
	
	return str_dup_printf(20, "%s", inet_ntoa(addr));
}

int set_local_gateway_as_string(char * netdev, char * gateway)
{
	int ret = -1;
	char * pSetting = str_dup_printf(64, "add default gw %s", gateway);
	if(pSetting)
	{
		ret = route(netdev, pSetting);
		str_free(&pSetting);
	}
	
	return ret;
}

int get_local_network_info( char * netdev, char * ipaddr, char * netmask, char * gateway, char * broadcast)
{
	if(netdev == NULL || strlen(netdev) == 0)
		return -1;

	char * addr = NULL;
	
	////////////////////IP地址///////////////////
	if(ipaddr)
	{
		addr = get_local_ip_addr_as_string( netdev);
		if(addr)
		{
			strcpy(ipaddr, addr);
			str_free(&addr);
		}
	}
	
	////////////////////子网掩码///////////////////
	if(netmask)
	{
		addr = get_local_netmask_as_string( netdev);
		if(addr)
		{
			strcpy(netmask, addr);
			str_free(&addr);
		}
	}
	
	////////////////////网关///////////////////
	if(gateway)
	{
		addr = get_local_gateway_as_string( netdev);
		if(addr)
		{
			strcpy(gateway, addr);
			str_free(&addr);
		}
	}

	////////////////////广播///////////////////
	if(broadcast)
	{
		addr = get_local_broadcast_as_string( netdev);
		if(addr)
		{
			strcpy(broadcast, addr);
			str_free(&addr);
		}
	}
	
	return 0;
}

int set_local_network_info( char * netdev, char * ipaddr, char * netmask, char * gateway, char * broadcast)
{
	if(netdev == NULL || strlen(netdev) == 0)
		return -1;
	
	////////////////////IP地址///////////////////
	if(ipaddr)
		set_local_ip_addr_as_string( netdev, ipaddr);
	
	////////////////////子网掩码///////////////////
	if(netmask)
		set_local_netmask_as_string(netdev, netmask);
	
	////////////////////网关///////////////////
	if(gateway)
		set_local_gateway_as_string(netdev, gateway);
	
	////////////////////广播///////////////////
	if(broadcast)
		set_local_broadcast_as_string(netdev, broadcast);
	
	return 0;
}

char * get_subnet_prestr_from_ip(char * ipaddr)
{
	char * ip_pre =str_dup("",16);
	if(ip_pre == NULL)
		return NULL;
	
	BYTE ipArr[4];
	memset(ipArr,0x00,sizeof(ipArr));
	DWORD ipv = (DWORD)inet_addr(ipaddr);
	memcpy(ipArr,&ipv, sizeof(ipArr));
	
	sprintf(ip_pre,"%d.%d.%d",ipArr[0], ipArr[1], ipArr[2]);
	
	return ip_pre;
}

BYTE get_subnet_sufstr_from_ip(char * ipaddr)
{
	BYTE ipArr[4];
	memset(ipArr,0x00,sizeof(ipArr));
	DWORD ipv = (DWORD)inet_addr(ipaddr);
	memcpy(ipArr,&ipv, sizeof(ipArr));
	
	return ipArr[3];
}

char * get_subnet_prestr(char * netdev)
{
	char * iparr = get_local_ip_addr_as_string(netdev);
	if(iparr == NULL)
		return NULL;
	
	char * ip_pre = get_subnet_prestr_from_ip(iparr);
	str_free(&iparr);
	
	return ip_pre;
}

BYTE get_subnet_sufstr(char * netdev)
{
	char * iparr = get_local_ip_addr_as_string(netdev);
	if(iparr == NULL)
		return 0;

	BYTE ipSuf = get_subnet_sufstr_from_ip(iparr);
	str_free(&iparr);
	
	return ipSuf;
}

int get_loacl_netcard_num()
{
	int netCardNum = 0;
	char * result = get_cmd_result("ls /sys/class/net/ | grep -v lo | xargs -n1 | wc -l");
	if(result == NULL)
		return netCardNum;
	
	netCardNum = atoi(result);
	free_cmd_result(&result);
	
	return netCardNum;
}

int get_netcard_device_name(int index, char * netDevNmae, int bufLen)
{
	int netCardNum = 0;
	char * result = get_cmd_result("ls /sys/class/net/ | grep -v lo | xargs -n1 | sort | sed -n '%d'p", index);
	if(result == NULL)
		return -1;
	
	strncpy(netDevNmae, result, bufLen);
	free_cmd_result(&result);
	
	return 0;
}


int get_loacl_netcard_info(NETCARD_INFO * pNetCardInfo, int index)
{
	get_netcard_device_name(index + 1, pNetCardInfo->devName, sizeof(pNetCardInfo->devName));

	get_local_mac_addr(pNetCardInfo->devName, pNetCardInfo->macAddr);
	get_local_network_info(pNetCardInfo->devName, pNetCardInfo->ipAddr, pNetCardInfo->netMask, pNetCardInfo->gateWay, pNetCardInfo->broadCast);
	pNetCardInfo->linkStatus = get_netcard_link_status(pNetCardInfo->devName);
	
	int speedSpt = 0;
	int duplexSpt = 0;
	get_netcard_ethtool_info(pNetCardInfo->devName, &speedSpt, &duplexSpt, &pNetCardInfo->speed, &pNetCardInfo->duplex, &pNetCardInfo->autoNeg);


	return 0;
}


LOCAL_NETCARD_INFO * local_netcard_info_init()
{
	LOCAL_NETCARD_INFO * pLocalNetcardInfo = (LOCAL_NETCARD_INFO *) z_malloc(sizeof(LOCAL_NETCARD_INFO));
	if(pLocalNetcardInfo == NULL)
		return NULL;

	pLocalNetcardInfo->netCardNum = get_loacl_netcard_num();
	
	int i = 0;
	for(i = 0; i < pLocalNetcardInfo->netCardNum; i ++)
		get_loacl_netcard_info(&pLocalNetcardInfo->netCardInfo[i], i);	

	return pLocalNetcardInfo;
}

int local_netcard_info_release(LOCAL_NETCARD_INFO ** pLocalNetcardInfo)
{
	if(*pLocalNetcardInfo)
	{
		free(*pLocalNetcardInfo);
		*pLocalNetcardInfo = NULL;
	}
	
	return 0;
}

int local_netcard_info_display(LOCAL_NETCARD_INFO * pLocalNetcardInfo)
{
	if(pLocalNetcardInfo == NULL)
		return -1;
	
	iLog("=========================  show local netcard info start =========================");

	iLog("netCardNum=[%d]", pLocalNetcardInfo->netCardNum);

	int i = 0;
	for(i = 0; i < pLocalNetcardInfo->netCardNum; i ++)
	{
		iLog("----------------------------netcard[%d]----------------------------", i);
		iLog("devName=[%s]", pLocalNetcardInfo->netCardInfo[i].devName);
		
		iLog("macAddr=["FMT_MAC_ADDR "]", SNIF_MAC_ADDR(pLocalNetcardInfo->netCardInfo[i].macAddr));
		iLog("ipAddr=[%s]", pLocalNetcardInfo->netCardInfo[i].ipAddr);
		iLog("gateWay=[%s]", pLocalNetcardInfo->netCardInfo[i].gateWay);
		iLog("netMask=[%s]", pLocalNetcardInfo->netCardInfo[i].netMask);
		iLog("broadCast=[%s]", pLocalNetcardInfo->netCardInfo[i].broadCast);
		
		iLog("mTu=[%d]", pLocalNetcardInfo->netCardInfo[i].mTu);
		iLog("meTric=[%d]", pLocalNetcardInfo->netCardInfo[i].meTric);
		iLog("linkStatus=[%d]", pLocalNetcardInfo->netCardInfo[i].linkStatus);
		iLog("wakeOnLan=[%d]", pLocalNetcardInfo->netCardInfo[i].wakeOnLan);
		iLog("autoNeg=[%d]", pLocalNetcardInfo->netCardInfo[i].autoNeg);
		iLog("duplex=[%d]", pLocalNetcardInfo->netCardInfo[i].duplex);
		iLog("speed=[%d]", pLocalNetcardInfo->netCardInfo[i].speed);
	
	}

	iLog("=========================  show local netcard info end =========================");
	
	return 0;
}
