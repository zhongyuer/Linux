/*********************************************************************************
 *      Copyright:  (C) 2024 LiYi<1751425323@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  socket.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(19/04/24)
 *         Author:  LiYi <1751425323@qq.com>
 *      ChangeLog:  1, Release initial version on "19/04/24 21:28:57"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>   
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/resource.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <netdb.h>
#include "socket.h"


int socket_init(socket_ctx_t *sock, char *host, int port)
{	
	memset(sock, 0 , sizeof(*sock));
	
	if(host != NULL)
	{
		strncpy(sock->host, host, HOSTNAME_LEN);
		sock->host[HOSTNAME_LEN - 1]='\0';
	}

	sock->fd = -1;
	sock->port = port;

	return 0;
}

int socket_connect(socket_ctx_t *sock)
{
	int					ret = 0;
	int					sockfd = 0;
	struct sockaddr_in	serv_addr;
	struct addrinfo		*result;
	struct addrinfo		*rp;
	struct addrinfo		hinst;
	struct in_addr		inaddr;		
	int					ch = 0;;
	char				port_str[32];
	char				addr_str[64];

	memset(&hinst, 0, sizeof(hinst));
	hinst.ai_family = AF_INET;
	hinst.ai_socktype = SOCK_STREAM;
	//hinst.ai_flags = AI_CANONNAME;
	hinst.ai_protocol = 0;

	if(inet_aton(sock->host, &inaddr))
	{
		hinst.ai_flags = AI_NUMERICHOST;
		//printf("%s is a valid IP don't parser\n", sock->host);
	}
	
	snprintf(port_str, sizeof(port_str), "%d", sock->port);
	ch = getaddrinfo(sock->host, port_str, &hinst, &result);
	if( ch != 0)
	{
		printf("analyze [%s,%s] failure: %s\n",sock->host, port_str, gai_strerror(errno));
		return -1;
	}
	printf("analyze successfully\n");

	for(rp = result; rp != NULL; rp=rp->ai_next)
	{
		char				ipaddr[32];
		struct sockaddr_in	*sp = (struct sockaddr_in *) rp->ai_addr;

		memset(ipaddr,0, sizeof(ipaddr));
		if( inet_ntop(AF_INET, &sp->sin_addr, ipaddr, sizeof(ipaddr)))
		{
			printf("domain name resolution [%s->%s]\n", sock->host, ipaddr);

		}
		
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(sockfd <0 )
		{
			printf("create socket failure: %s\n",gai_strerror(errno));
			continue;
		}
	
		ret = connect(sockfd, rp->ai_addr, rp->ai_addrlen);
		if(ret <0 )
		{
			//printf("connect to server [%s:%d] failure: %s\n",sock->host, sock->port, strerror(errno));
			close(sockfd);
			continue;
		}
		else
		{
			sock->fd = sockfd;
			printf("connect to server [%s:%d] on fd[%d] successfully\n", sock->host, sock->port, sockfd);
			inet_ntop(rp->ai_family, rp->ai_addr, addr_str, sizeof(addr_str));
			printf("IP address: %s\n", addr_str);
			break;
		}
	}

	freeaddrinfo(result);
	return 0;
}


int socket_write(socket_ctx_t *sock, char *data)
{
	int			rv = 0;

	if(write(sock->fd, data, sizeof(*data)) < 0)
	{
		printf("write data to server [%s;%d] failure: %s\n",sock->host, sock->port,strerror(errno));
		close(sock->fd);
	}
	printf("write data to server: %s\n", data);
 	
	memset(data,0,sizeof(*data));
 	rv = read(sock->fd,data,sizeof(data));
 	if(rv < 0)
	{
		printf("read data from server failure: %s\n",strerror(errno));
		close(sock->fd);
	}

	else if(0 == rv)
	{
		printf("client connect to server get disconnected\n");
		close(sock->fd);
	}
	printf("read %d bytes data from server: %s\n", rv, data);

//CleanUp:
//	close(sock->fd);
}

int	socket_net_status(socket_ctx_t *sock)
{
	struct tcp_info		info;
	socklen_t			len = sizeof(info);

	getsockopt(sock->fd, IPPROTO_TCP, TCP_INFO, &info, &len);
	if(info.tcpi_state == TCP_ESTABLISHED)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void set_socket_rlimit(void)
{
	struct rlimit limit = {0};

	getrlimit(RLIMIT_NOFILE, &limit);
	limit.rlim_cur = limit.rlim_max;
	setrlimit(RLIMIT_NOFILE, &limit);

	printf("set socket open fd max count to %d\n", limit.rlim_max);
}
