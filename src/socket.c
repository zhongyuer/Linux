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

#include "socket.h"


//int socket_init(socket_ctx_t *sock, char *host, int port)

int socket_init(socket_ctx_t *sock, char *host, int port)
{
	if( !port <= 0 )
		return -1;
	memset(&sock, 0 , sizeof(sock));

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

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
 	if(sockfd<0)
	{ 
		printf("create socket failure: %s\n",strerror(errno));
		return -1;
	}	 
	printf("socket create fd[%d] seccessfully\n",sockfd);
       
	memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(sock->port);
	inet_aton(sock->host, &serv_addr.sin_addr);

	ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if(ret <0 )
	{
		printf("connect to server [%s:%d] failure: %s\n",sock->host, sock->port, strerror(errno));
		close(sockfd);
		return -1;
	}
	else
	{
		sock->fd = sockfd;
		printf("connect to server [%s:%d] on fd[%d] successfully\n",sock->host, sock->port, sockfd);
	}

	return 0;
}

int socket_write(socket_ctx_t *sock, char *data)
{
	int			rv = 0;

	if(write(sock->fd, data, sizeof(data)) < 0)
	{
		printf("write data to server [%s;%d] failure: %s\n",sock->host, sock->port,strerror(errno));
		goto CleanUp;
	}
	printf("write data to server: %s\n", data);
 	
	memset(data,0,sizeof(data));
 	rv = read(sock->fd,data,sizeof(data));
 	if(rv < 0)
	{
		printf("read data from server failure: %s\n",strerror(errno));
		goto CleanUp;
	}

	else if(0 == rv)
	{
		printf("client connect to server get disconnected\n");
		goto CleanUp;
	}
	printf("read %d bytes data from server: %s\n", rv, data);

CleanUp:
	close(sock->fd);
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
