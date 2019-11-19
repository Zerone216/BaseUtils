/****************************************************************************************************************************
*
*   文 件 名 ： libtcp.c 
*   文件描述 ：  
*   创建日期 ：2019年4月10日
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
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>

#include "libtcp.h"

int init_tcp_socket(int blockMode, int sendbufLen, int recvbufLen, int addrReuse, int portReuse)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockfd == -1)
	{
		assert(0);
		return -1;
	}
	
	//用于任意类型、任意状态套接口的设置选项值
	if(setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sendbufLen, sizeof(sendbufLen)) < 0) 
	{
		assert(0);
		return -1;
	}

	//用于任意类型、任意状态套接口的设置选项值
	if(setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &recvbufLen, sizeof(recvbufLen)) < 0) 
	{
		assert(0);
		return -1;
	}
	
	//设置套接字选项避免地址使用错误
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &addrReuse, sizeof(addrReuse)) < 0)
	{
		assert(0);
		return -1;
	}
	
	/*
	//设置套接字选项避免地址使用错误
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &portReuse, sizeof(portReuse)) < 0)
	{
		assert(0);
		return -1;
	}
	*/

	if(blockMode == TCP_SOCK_NONBLOCK)//设置为非阻塞模式
		fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
	
	return sockfd;
}

void close_tcp_socket(int sockfd)
{
	shutdown(sockfd, SHUT_RDWR); //关闭读写通道，对所有共享该套接字进程有效，防止死锁
	close(sockfd); //关闭描述符，释放资源， 重置引用计数
	return;
}

int init_tcp_server(char * serverip, int serverport, double timeout, int blockMode)
{
	int socketfd = init_tcp_socket(blockMode, SNDBUF_LEN, RCVBUF_LEN, TRUE, TRUE);
	if(socketfd == -1)
		return -1;
	
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(struct sockaddr_in));
	serveraddr.sin_family = AF_INET;
	//serveraddr.sin_addr.s_addr = (Isempty(serverip) ? htonl(INADDR_ANY) : inet_addr(serverip));//IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。
	serveraddr.sin_addr.s_addr = inet_addr(serverip);//IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。
	serveraddr.sin_port = htons(serverport);//设置端口

	///bind，成功返回0，出错返回-1
	if(bind(socketfd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr_in)) == -1)
	{
		assert(0);
		eLog("bind %s:%d error!", serverip, serverport);
		close_tcp_socket(socketfd);
		return -1;
	}

	///listen，成功返回0，出错返回-1
	if(listen(socketfd, 32) == -1)
	{
		assert(0);
		eLog("listen %s:%d error!", serverip, serverport);
		close_tcp_socket(socketfd);
		return -1;
	}

	struct timeval tv = time_parse(timeout);  //超时时间设置
	fd_set server_fd_set;
	
	while(1)
	{
		FD_ZERO(&server_fd_set);
		FD_SET(socketfd, &server_fd_set);
		
		int ret = select(socketfd + 1, &server_fd_set, NULL, NULL, &tv);
		if(ret < 0)
		{
			assert(0);
			eLog("select error!");
			continue;
		}
		else if(ret == 0)
		{
			eLog("select timeout!");
			close_tcp_socket(socketfd);
			return -1;
		}
		else
		{
			if(FD_ISSET(socketfd, &server_fd_set))
			{
				struct sockaddr_in client_addr;
				socklen_t length = sizeof(client_addr);
				iLog("accepting...");

				int connect_fd = accept(socketfd, (struct sockaddr*)&client_addr, &length);
				if(connect_fd < 0)
				{
					assert(0);
					eLog("accept error!");
					close_tcp_socket(socketfd);
					return -1;
				}
				
				iLog("new client[%s:%d] connect succeed: connect_fd[%d]!", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), connect_fd);

				//if(strcmp(serveripstr, inet_ntoa(client_addr.sin_addr)) == 0)
				//	continue;
				
				close_tcp_socket(socketfd);
				if(blockMode == TCP_SOCK_NONBLOCK)
					fcntl(connect_fd, F_SETFL, fcntl(connect_fd, F_GETFL, 0) | O_NONBLOCK);
				
				return connect_fd;
			}
		}
	}
}

int connect_tcp_server(char * serverip, int serverport, double timeout, int blockMode)
{
	int sockfd = init_tcp_socket(blockMode, SNDBUF_LEN, RCVBUF_LEN, TRUE, TRUE);
	if(sockfd == -1)
		return -1;
	
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(serverport);
	server_addr.sin_addr.s_addr = inet_addr(serverip);
	iLog("start connect server %s:%d", serverip, serverport);
	
	if(blockMode == TCP_SOCK_NONBLOCK)
		ErrClean(); //非阻塞socket每次在调用connect之前将errno清零
	
	int connected = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in));
	if(connected == 0)
	{
		assert(0);
		eLog("connect error!");
		close_tcp_socket(sockfd);
		return -1;
	}

	int errFlag = 0;
	if(!ErrCmp(EINPROGRESS))
	{
		assert(0);
		iLog("connected = %d, ErrExp:%s", connected, ErrExp());
		errFlag = 1;
	}
	else
	{
		struct timeval tm = time_parse(timeout);
		
		fd_set wset, rset;
		FD_ZERO(&wset);
		FD_ZERO(&rset);

		FD_SET(sockfd, &wset);
		FD_SET(sockfd, &rset);

		int ret = select(sockfd + 1, &rset, &wset, NULL, &tm);
		if(ret < 0)
		{
			assert(0);
			eLog("select error!");
			errFlag = 1;
		}
		else if(ret == 0)
		{
			assert(0);
			eLog("select timeout!");
			errFlag = 1;
		}
		else if( ret == 1)
		{
			if(FD_ISSET(sockfd, &wset)) 
			{
				iLog("connect succeed!");
				if(blockMode == TCP_SOCK_NONBLOCK)
					fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
				errFlag = 0;
			}
			else
			{
				assert(0);
				eLog("select return=[%d]", ret);
				errFlag = 1;
			}
		}
		else
		{
			eLog("select result=[%d]", ret);
			eLog("sockfd is not writable or readable!");
			errFlag = 1;
		}
	}
	
	if(errFlag)
	{
		assert(0);
		eLog("select timeout!");
		close_tcp_socket(sockfd);
		return -1;
	}
	else
		return sockfd;
}

int send_tcp_pkt(int sockfd, char * buf, int bufLen, double timeout, int pktType)
{
	if(buf == NULL)
		bufLen = 0;
	
	int pktheadLen = sizeof(struct package_tcp_head);
	int leftLen = pktheadLen + bufLen; //剩余字节长度
	char *sendBuf = (char*)malloc(leftLen);
	if(sendBuf == NULL)
	{
		assert(0);
		eLog("malloc buf error!");
		return -1;
	}
	
	struct package_tcp_head * head = (struct package_tcp_head *)sendBuf;
	head->pkt_flag = PKT_FLAG;
	head->pkt_type = pktType;
	head->pkt_len = bufLen;	/*有效数据长度,不包括任何包头*/
	
	if(bufLen > 0 && buf != NULL)
		memcpy(sendBuf + pktheadLen, buf, bufLen);
	
	struct timeval tv = time_parse(timeout);
	int retvalue = 0;
	fd_set w_set;
	
	while(1)
	{
		FD_ZERO(&w_set);
		FD_SET(sockfd, &w_set);

		int ret = select(sockfd + 1, NULL, &w_set, NULL, &tv);
		if(ret == 0)
		{
			eLog("select timeout!");
			retvalue = -1;
			break;
		}
		else if(ret < 0)
		{
			eLog("select error!");
			retvalue = -1;
			break;
		}
		else
		{
			if(FD_ISSET(sockfd, &w_set)) //可写
			{
				int sendLen = send(sockfd, sendBuf, leftLen, 0);
				if(sendLen < 0)
				{
					assert(0);
					if(ErrCmp(EAGAIN) || ErrCmp(EINTR))
						continue;
					else
					{
						eLog("send pkt[%d] error!", pktType);
						retvalue = -1;
						break;
					}
				}
				else
				{
					if(sendLen < leftLen)
					{
						sendBuf += sendLen;
						leftLen -= sendLen;
					}
					else
					{
						retvalue = 0;
						break;
					}
				}
			}
			else
			{
				assert(0);
				eLog("sockfd is unwritable!");
				retvalue = -1;
				break;
			}
		}
	}

	if(sendBuf != NULL)
		free(sendBuf);

	return retvalue;
}

//接收数据内容
int recv_tcp_pkt_data(int sockfd, char * buf, int bufLen, double timeout)
{
	if(buf == NULL)
		return -1;
	
	int leftLen = bufLen;
	int retvalue = 0;
	fd_set r_set;
	struct timeval tv = time_parse(timeout);  //select 时限
	
	while(TRUE)
	{
		FD_ZERO(&r_set);
		FD_SET(sockfd, &r_set);

		int ret = select(sockfd + 1, &r_set, NULL, NULL, &tv);
		if(ret == 0)
		{
			eLog("select timeout!");
			retvalue = -1;
			break;
		}
		else if(ret < 0)
		{
			assert(0);
			eLog("select error!");
			retvalue = -1;
			break;
		}
		else
		{
			if(FD_ISSET(sockfd, &r_set))
			{
				ErrClean();
				int recvLen = recv(sockfd, buf, leftLen, 0);
				if(recvLen == -1)
				{
					assert(0);
					if(ErrCmp(EAGAIN) || ErrCmp(EINTR) || ErrCmp(EWOULDBLOCK))
						continue;
					else
					{
						eLog("recv data error!");
						retvalue = -1;
						break;
					}
				}
				else if(recvLen == 0)
				{
					assert(0);
					
					// 这里表示对端的socket已正常关闭.
					wLog("The system detects that the connection between the machine and the server has been disconnected.");
					retvalue = -2;
					break;
				}
				else
				{
					if(recvLen < leftLen)     // 需要再次读取
					{
						buf += recvLen;
						leftLen -= recvLen;
					}
					else
					{
						retvalue = 0;
						break;
					}
				}
			}
			else
			{
				assert(0);
				eLog("sockfd is unreadable!");
				retvalue = -1;
				break;
			}
		}
	}

	return retvalue;
}

//接收包头数据
int recv_tcp_pkt_head(int sockfd, int pktType, int timeout)
{
	int pktheadLen = sizeof(struct package_tcp_head);
	struct package_tcp_head * pkthead = (struct package_tcp_head *)malloc(pktheadLen);
	if(pkthead == NULL)
		return -1;
	
	while(TRUE)
	{
		int ret = recv_tcp_pkt_data(sockfd, (char *)pkthead, pktheadLen, timeout);
		if(ret < 0)
		{
			eLog("recv pkt_head error!");
			free(pkthead);
			return ret;
		}
		else
		{
			if(pkthead->pkt_flag == PKT_FLAG)
			{
				if(pktType != pkthead->pkt_type)
				{
					char *tmpbuf = (char *)malloc(pkthead->pkt_len);
					if(tmpbuf == NULL)
					{
						assert(0);
						eLog("malloc  tmpbuf error!");
						free(pkthead);
						return -1;
					}

					//取出该包的数据丢弃继续读下一个包
					ret = recv_tcp_pkt_data(sockfd, tmpbuf, pkthead->pkt_len, timeout);
					if(ret < 0)
					{
						eLog("recv pkt_data error!");
						free(tmpbuf);
						free(pkthead);
						return ret;
					}

					free(tmpbuf);
					continue;
				}
				else
					break;
			}
		}
	}

	int pktLen = pkthead->pkt_len;
	free(pkthead);
	
	return pktLen;
}

int recv_tcp_pkt(int sockfd, char * buf, int bufLen, double timeout, int pktType)
{
	int pktLen = recv_tcp_pkt_head(sockfd, pktType, timeout);
	if(pktLen < 0)
	{
		Log(("Recv tcp header failed!"));
		return -1;
	}

	if(bufLen < pktLen)
	{
		Log(("The buffer len is too small!"));
		return -1;
	}
	
	return recv_tcp_pkt_data(sockfd, buf, pktLen, timeout);
}

