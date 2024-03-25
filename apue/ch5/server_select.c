/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  server_select.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(14/03/24)
 *         Author:  linuxer <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "14/03/24 20:56:47"
 *                 
 ********************************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>       
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <libgen.h>






#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

static inline void msleep(unsigned long ms);
static inline void print_usage(char *progname);
int socket_server_init(char *listen_ip,int listen_port);

int main(int argc,char **argv)
{
	int                 listenfd,connfd;
	int                 serv_port = 0;
	int                 daemon_run = 0;
	char                *progname = NULL;
	int                 opt;
	fd_set              rdset;
	int                 rv;
	int                 i,j;
	int                 found;
	int                 maxfd = 0;
	char                buf[1024];
	int                 fds_array[1024];

	struct option        long_option[] = 
	{
		{"daemon",no_argument,NULL,'b'},
		{"port",required_argument,NULL,'p'},
		{"help",no_argument,NULL,'h'},
		{NULL,0,NULL,0}
	};

	progname = basename(argv[0]);

	while((opt = getopt_long(argc,argv,"bp:h",long_option,NULL)) != -1)
	{
		switch(opt)
		{
			case'b':
				daemon_run=1;
				break;

			case'p':
				serv_port=atoi(optarg);
				break;

			case'h':
				print_usage(progname);
				return EXIT_SUCCESS;

			default:
				break;
		}

	}
	if(!serv_port)
	{
		print_usage(progname);
		return -1;
	}

	if((listenfd=socket_server_init(NULL,serv_port))<0)
	{
		printf("ERROR, %s server listen on port %d failure\n",argv[0],serv_port);
		return -2;
	}
	printf("%s server start to listen on port %d\n",argv[0],serv_port);

	if(daemon_run)
	{
		daemon(0,0);

	}
	for(i=0;i<ARRAY_SIZE(fds_array);i++)
	{
		fds_array[i]=-1;

	}
	fds_array[0] = listenfd;

	for(; ;)
	{
		FD_ZERO(&rdset);
		for(i=0;i<ARRAY_SIZE(fds_array);i++)
		{
			if(fds_array[i]<0)
				continue;

			maxfd = fds_array[i]>maxfd ? fds_array[i] : maxfd;
			FD_SET(fds_array[i],&rdset);

		}

		rv=select(maxfd+1,&rdset,NULL,NULL,NULL);
		if(rv<0)
		{
			printf("select failure: %s\n",strerror(errno));
			break;

		}
		else if(rv==0)
		{
			printf("select get timeout\n");
			continue;

		}

		if(FD_ISSET(listenfd,&rdset))
		{
			if((connfd=accept(listenfd,(struct sockaddr *)NULL,NULL))<0)
			{
				printf("accept new client failure: %s\n",strerror(errno));
				continue;

			}

			found = 0;
			for(i=0;i<ARRAY_SIZE(fds_array);i++)
			{
				if(fds_array[i]<0)
				{
					printf("accept new client[%d] and add it into array\n",connfd);
					fds_array[i] = connfd;
					found = 1;
					break;

				}

			}

		if(!found)
		{
			printf("accept new client[%d] but full,so refuse it\n",connfd);
			close(connfd);

		}
	}
	else
	{
		for(i=0;i<ARRAY_SIZE(fds_array);i++)
		{
			if(fds_array[i]<0|| !FD_ISSET(fds_array[i],&rdset))
				continue;

			if((rv=read(fds_array[i],buf,sizeof(buf)))<0)
			{
				printf("socket[%d] read failure or disconnect.\n",fds_array[i]);
				close(fds_array[i]);
				fds_array[i]=-1;

			}
			else
			{
				printf("socket[%d] read get %d bytes data\n",fds_array[i],rv);

				for(j=0;j<rv;j++)
					buf[j]=toupper(buf[i]);

				if(write(fds_array[i],buf,rv)<0)
				{
					printf("socket[%d] write failure: %s\n",fds_array[i],strerror(errno));
					close(fds_array[i]);
					fds_array[i]=-1;

				}
			}
		}
	}
 }


CleanUp:
	close(listenfd);
	return 0;
}
static inline void msleep(unsigned long ms)
{
	struct timeval        tv;

	tv.tv_sec = ms/1000;
	tv.tv_usec = (ms%1000)*1000;

	select(0,NULL,NULL,NULL,&tv);
}

static inline void print_usage(char *progname)
{
	printf("usage； %s [OPTION]...\n",progname);

	printf(" %sis a socket server progname,which used to verify client and echo back string from it\n",progname);
	printf("\nmandatory arguments to long options are mandatory for short options too: \n");

	printf(" -b[daemon] set progname running on background\n");
	printf(" -p[port] socket server port address\n");
	printf(" -p[help] display this help information\n");

	printf("\nexample: %s -b -p 8889\n",progname);
	return ;

}

int socket_server_init(char *listen_ip,int listen_port)
{
	struct sockaddr_in     servaddr;
	int                    rv=0;
	int                    on=1;
	int                    listenfd;

	if((listenfd = socket(AF_INET,SOCK_STREAM,0))<0)
	{
		printf("use socket() to create a TCP socket failure: %s\n",strerror(errno));
			return -1;
	}

setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(listen_port);

	if(!listen_ip)
	{
		servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	}
	else
	{
		if(inet_pton(AF_INET,listen_ip,&servaddr.sin_addr) <=0)
		{
			printf("inet_port() set listen IP address failure.\n");
			rv = -2;
			goto CleanUp;

		}

	}

	if(bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr))<0)
	{
		printf("use bind() to bind the TCP socket failure: %s\n",strerror(errno));
		rv = -4;
		goto CleanUp;

	}

CleanUp:
	if(rv<0)
		close(listenfd);
	else
		rv=listenfd;

	return rv;
}
