/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  server_epoll.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(17/03/24)
 *         Author:  linuxer <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "17/03/24 10:22:13"
 *                 
********************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/resource.h>
#include <sys/epoll.h>
#include <libgen.h>
#include <stdlib.h>
#include <sqlite3.h>

#define MAX_EVENTS     512
#define ARRAY_SIZE(x)  (sizeof(x)/size(x[0]))

static inline void print_usage(char *progname);
int socket_server_init(char *listen_ip,int listen_port);
void set_socket_rlimit(void);

int sqlite_init_db();
int sqlite_insert_data(char *id,char *temp,char *time);
int sqlite_close_db();

sqlite3*	db;

int main(int argc,char **argv)
{
	int                   listenfd,connfd;
	int                   serv_port = 0;
	int                   daemon_run = 0;
	char                  *progname = NULL;
	int                   opt;
	int                   rv;
	int                   i,j;
	int					  on = -1;
	int                   found;
	char                  buf[1024];

	int                   epollfd;
	struct epoll_event    event;
	struct epoll_event    event_array[MAX_EVENTS];
	int                   events;

	char				  s[16] = ";";
	char				  *id;
	char				  *temp;
	char			      *time;


	struct option     long_options[] = 
	{
		{"daemon",no_argument,NULL,'d'},
		{"port",required_argument,NULL,'p'},
		{"help",no_argument,NULL,'h'},
		{NULL,0,NULL,0}
	};

	progname=basename(argv[0]);

	while((opt=getopt_long(argc,argv,"dp:h",long_options,NULL)) != -1)
	{
		switch(opt)
		{
			case'd':
				daemon_run = 1;
				break;

			case'p':
				serv_port = atoi(optarg);
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

	set_socket_rlimit();
	
	if((listenfd=socket_server_init(NULL,serv_port))<0)
	{
		printf("ERROR: %s server listen on port %d failure",argv[0],serv_port);
		return -2;
	}
	printf("%s server start to listen on port %d\n",argv[0],serv_port);

	if(daemon_run)
	{
		daemon(0,0);
	}

	if((epollfd=epoll_create(MAX_EVENTS))<0)
	{
		printf("epoll_create() failure； %s\n",strerror(errno));

		return -3;
	}
	
	event.events=EPOLLIN;
	event.data.fd=listenfd;

	if(epoll_ctl(epollfd,EPOLL_CTL_ADD,listenfd,&event)<0)
	{
		printf("epoll add listen socket failure: %s\n",strerror(errno));
		return -4;
	}


	for(; ;)
	{
		events=epoll_wait(epollfd,event_array,MAX_EVENTS,-1);
		if(events<0)
		{
			printf("epoll failure: %s\n",strerror(errno));
			break;
		}
		else if(events == 0)
		{
			printf("epoll get timeout\n");
			continue;
		}

		for(i=0;i<events;i++)
		{
			if((event_array[i].events&EPOLLERR)||(event_array[i].events&EPOLLHUP))
			{
				printf("epoll_wait get error on fd[%d]: %s\n",event_array[i].data.fd,strerror(errno));
				epoll_ctl(epollfd,EPOLL_CTL_DEL,event_array[i].data.fd,NULL);
				close(event_array[i].data.fd);
			}
			
			if(event_array[i].data.fd == listenfd)
			{
				if((connfd=accept(listenfd,(struct sockaddr *)NULL,NULL))<0)
				{
					printf("accept new client failure: %s\n",strerror(errno));
					continue;
				}
				event.data.fd = connfd;
				event.events  = EPOLLIN;
				
				if(epoll_ctl(epollfd,EPOLL_CTL_ADD,connfd,&event) < 0 )
				{
					printf("epoll add client socket failure: %s\n",strerror(errno));
					close(event_array[i].data.fd);
					continue;
				}
				printf("epoll add new client socket[%d] ok.\n",connfd);
			}
			
			else
			{
				if((rv=read(event_array[i].data.fd,buf,sizeof(buf)))<=0)
				{
					printf("socket[%d] read failure or get disconnect and will be removed.\n",event_array[i].data.fd);
					epoll_ctl(epollfd,EPOLL_CTL_DEL,event_array[i].data.fd,NULL);
					close(event_array[i].data.fd);
					continue;

				}
				else
				{
					printf("socket[%d] read get %d bytes data\n",event_array[i].data.fd,rv);
					printf("buf: %s\n",buf);
					for(j=0;j<rv;j++)
						buf[j]=toupper(buf[j]);

					if(write(event_array[i].data.fd,buf,rv)<0)
					{
						printf("socket[%d] write failure: %s\n",event_array[i].data.fd,strerror(errno));
						epoll_ctl(epollfd,EPOLL_CTL_DEL,event_array[i].data.fd,NULL);
						close(event_array[i].data.fd);
					}

					id = strtok(buf,s);
					temp = strtok(NULL,s);
					time = strtok(NULL,s);
				//	printf("id: %s,temp: %s,time: %s\n",id,temp,time);

					sqlite_init_db();
					sqlite_insert_data(id,temp,time);
					sqlite_close_db();

				}
			}	
		}
	}

CleanUp:
	close(listenfd);
	return 0;

}


static inline void print_usage(char *progname)
{
	printf("usage: %s [OPTION]...\n",progname);

	printf(" %s is a socket server program, which used to verify client and echo back string from it\n",progname);
	printf("\nMandatory arguments to long options are mandatory for short too:\n");

	printf("-d[--daemon] set program set riunning on background\n");
	printf("-p[--port] socket server address\n");
	printf("-h[--help] display this help imformation\n");

	printf("\nExample: %s -b -p 8889\n",progname);
	return ;
}

int socket_server_init(char *listen_ip,int listen_port)
{
	struct sockaddr_in     servaddr;
	int                    rv=0;
	int                    on=1;
	int                    listenfd;

	if((listenfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		printf("use socket() to create a TCP socket failure: %s\n",strerror(errno));
				return -1;
			
	}

setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

memset(&servaddr,0,sizeof(servaddr));
servaddr.sin_family=AF_INET;
servaddr.sin_port=htons(listen_port);

if(!listen_ip)
{
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
}
else
{
	if(inet_pton(AF_INET,listen_ip,&servaddr.sin_addr)<=0)
	{
		printf("inet_pton() set listen IP address failure.\n");
		rv=-2;
		goto CleanUp;
	}
}
if(bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr))<0)
{
	printf("use bind() to bind the TCP socket failure: %s\n",strerror(errno));
	rv=-3;
	goto CleanUp;
}

	if(listen(listenfd,64)<0)
	{
		printf("use bind() to bind the TCP socket failure: %s\n",strerror(errno));
		rv=-4;
		goto CleanUp;

	}

CleanUp:
	if(rv<0)
		close(listenfd);
	else
		rv=listenfd;

	return rv;
}

void set_socket_rlimit(void)
{
	struct rlimit limit = {0};

	getrlimit(RLIMIT_NOFILE,&limit);
	limit.rlim_cur = limit.rlim_max;
	setrlimit(RLIMIT_NOFILE,&limit);

	printf("set socket open fd max count to %d\n",limit.rlim_max);
}

/* 创建并打开数据库和表*/
int sqlite_init_db()
{
	char    sql[128];
	char    *errmsg = NULL;
	int     ret = -1;

	ret = sqlite3_open("serDB.db",&db);
	if(ret != SQLITE_OK)
	{
		printf("create database failure: %s\n",sqlite3_errmsg(db));
		sqlite3_free(errmsg);

		return -1;
	}
	else
	{
		memset(sql,0,sizeof(sql));
		sprintf(sql,"CREATE TABLE IF NOT EXISTS serTABLE(ID INTEGER PRIMARY KEY AUTOINCREMENT,SN TEXT,TEMP REAL,TIME TEXT)");
		ret = sqlite3_exec(db,sql,0,0,&errmsg);
		if(ret != SQLITE_OK)	
		{
			printf("create table failure: %s\n",strerror(errno));				
			sqlite3_free(errmsg);
		
			return -2;
		}

		return 0;
	}
}

/* 插入数据到数据库中的表*/
int sqlite_insert_data(char *id,char *temp,char *time)
{
	char    sql[128];
	char    *errmsg = NULL;
	int     ret = -1;

	sprintf(sql,"INSERT INTO serTABLE (SN,TEMP,TIME) VALUES('%s','%s','%s')",id,temp, time);
	ret = sqlite3_exec(db,sql,0,0,&errmsg);
	if(ret != SQLITE_OK)
	{
		printf("insert into table failure: %s\n",sqlite3_errmsg(db));
		sqlite3_free(errmsg);

		return -3;
	}

	return 0;
}

/*  关闭数据库 */
int sqlite_close_db()
{
	char    *errmsg = NULL;

	if(db == NULL)
	{
		printf("close sqlite db failure: %s\n",sqlite3_errmsg(db));
		sqlite3_free(errmsg);

		return -7;
	}

	return 0;
}


