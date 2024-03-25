/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  server.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(27/02/24)
 *         Author:  linuxer <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "27/02/24 14:46:34"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <string.h>
 #include <sys/types.h>        
       #include <sys/socket.h>
#include <unistd.h>
       #include <arpa/inet.h>





#define MSG_STR "hello zhongyue" 

int main(int argc,char **argv)
{
	int                socketfd = -1;
	int                rv = -1;
	struct sockaddr_in serveraddr;
    char               *serverip="192.168.0.8";
	int                port = 12345;
	int                buf[1024];



	socketfd=socket(AF_INET,SOCK_STREAM,0);
	if(socketfd < 0)
	{
		printf("create socket failure:%s\n",strerror(errno));
		return -1;
	}
    printf("create socket[%d] successfully!\n,socketfd");
    
	memset(&serveraddr,0,sizeof(serveraddr));
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port = htons(port);
	inet_aton(serverip,&serveraddr.sin_addr);
	rv=connect(socketfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr));
	if(rv < 0)
	{
		printf("create to server[%s,%d] failure: %s\n",serverip,port,strerror(errno));
		return -2;
	}
	printf("create to server[%s,%d] successfully!\n",serverip,port);



while(1)
{
	rv = write(socketfd,MSG_STR,strlen(MSG_STR));
	if(rv < 0)
	{
		printf("write to server by socketfd[%d] failure: %s\n",socketfd,strerror(errno));
		return -3;
	}
	
	memset(buf,0,sizeof(buf));
	rv = read(socketfd,buf,sizeof(buf));
	if(rv < 0)
	{
		printf("read data from server by socketfd[%d] faillure:%s\n",socketfd,strerror(errno));
		return -4;
	}
	else if(rv ==0)
	{
		printf("socket[%d] get disconnected\n",socketfd);
		return -5;
	}
	else if(rv > 0)
	{
		printf("read %d bytes data from server:%s\n",rv,buf);

	}
}

close(socketfd);
}
