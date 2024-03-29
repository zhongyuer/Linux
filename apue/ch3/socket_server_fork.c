/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  socket_server_fork.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(09/03/24)
 *         Author:  linuxer <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "09/03/24 11:23:02"
 *                 
 ********************************************************************************/
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>     
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>

void print_usage(char *progname)
{
	printf("%s usage: \n",progname);
	printf("-p(--port):sepcify server listen port.\n");
	printf("-h(--help):print this help information.\n");
	return ;
}

int main(int argc,char **argv)
{
	int        sockfd = -1;
	int        rv = -1;
	struct     sockaddr_in servaddr;
	struct     sockaddr_in cliaddr;
	socklen_t  len;
	int        port = 0;
	int        clifd;
	int        ch;
	int        on = 1;
	pid_t      pid;

	struct option  opts[] = {
		{"port",required_argument,NULL,'p'},
		{"help",no_argument,NULL,'h'},
		{NULL,0,NULL,0}
	};
	while((ch=getopt_long(argc,argv,"p:h",opts,NULL)) != -1)
	{
		switch(ch)
		{
			case'p':
				port=atoi(optarg);
				break;
			case'h':
				print_usage(argv[0]);
				return 0;
		}
	}

	if(!port)
	{
	print_usage(argv[0]);
	return 0;
	}

	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0)
	{
		printf("Create socket failure: %s\n",strerror(errno));
		return -1;
	}
	printf("Create sockfd[%d] successfully!\n",sockfd);

	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
	
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(port);
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);

	rv=bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	if(rv<0)
	{
		printf("Socket[%d] bind on port[%d] failure: %s\n",sockfd,strerror(errno));
		return -2;
	}

	listen(sockfd,13);
	printf("Start to listen on port [%d]\n",port);

	while(1)
	{
		printf("Start accept new client incoming...\n");

		clifd=accept(sockfd,(struct sockaddr *)&cliaddr,&len);
		if(clifd<0)
		{
			printf("Accept new client failure: %s",strerror(errno));
			continue;
		}

		printf("Accept new client[%s,%d] successfully\n",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));

printf("1\n");

		pid=fork();
		if(pid<0)
		{
			printf("fork() create child process failure:%s\n",strerror(errno));
			close(clifd);
			continue;
		}
		else if(pid>0)
		{
			close(clifd);
			continue;
		}
		else if(0==pid)
		{
			char buf[1024];
			int  i;
			
			printf("Child process start tocommunicate with socket client...\n");

			close(sockfd);

			while(1)
			{
				memset(buf,0,sizeof(buf));
				rv=read(clifd,buf,sizeof(buf));
				if(rv<0)
				{
					printf("Read data from client sockfd[%d] failure: %s\n",clifd,strerror(errno));
					close(clifd);
					exit(0);
				}
				else if(rv==0)
				{
					printf("Socket[%d] get disconnected\n",clifd);
					close(clifd);
					exit(0);
				}
				else if(rv>0)
				{
					printf("Read %d byte data from Server: %s\n",rv,buf);
				}
				for(i=0;i<rv;i++)
				{
					buf[i]=toupper(buf[i]);

				}

				rv=write(clifd,buf,rv);
				if(rv<0)
				{
					printf("Write to client by sockfd[%d] failure: %s\n",clifd,strerror(errno));
					close(clifd);
					exit(0);
				}

			}

		}

	}
	close(sockfd);

	return 0;

}