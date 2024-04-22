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

int socket_init(socket_t *sock, char *hostname, int port)
{
	connfd = socket(AF_INET, SOCK_STREAM, 0);
	if(connfd<0)
	{
		printf("create socket failure: %s\n",strerror(errno));
		return -1;
	}
	printf("socket create fd[%d] seccessfully\n",connfd);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(serv_port);
	inet_aton(serv_ip, &serv_addr.sin_addr);
}

int socket_connect(socket_t *sock)
{
	ret = connect(connfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if(ret <0 )
	{
		printf("connect to server [%s:%d] failure: %s\n",serv_ip,serv_port,strerror(errno));
	}
}

int socket diag(socket_t *sock)
{




}

int socket write(socket_t *sock, char *data, int bytes)
{
	if(write(connfd, buf, sizeof(buf))<0)
	{
		printf("write data to server [%s;%d] failure: %s\n",serv_ip,serv_port,strerror(errno));
		goto CleanUp;
	}
	printf("write data to server: %s\n", buf);

	else if(0 == rv)
	{
		printf("client connect to server get disconnected\n");
		goto CleanUp;
	}
	printf("read %d bytes data from server: %s\n", rv, buf);

}
