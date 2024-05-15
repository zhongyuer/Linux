/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  socket_client.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(25/03/24)
 *         Author:  linuxer <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "25/03/24 22:28:23"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <libgen.h>
#include <time.h>
#include <netinet/tcp.h>
#include <sqlite3.h>

#include "socket.h"
#include "ds18b20.h"
#include "packet.h"
#include "database.h"
#include "logger.h"

//#define LENTH 64

static inline void print_usage(char *progname);


int main(int argc,char **argv)
{
	int                   daemon_run=0;
	char                  *progname=NULL;
	int                   opt;
	int					  interval = 0;

	char				  *logfile="sock_client.log";
	int				  	  loglevel=LOG_LEVEL_INFO;
	int				      logsize= 10;

	int					  sample_flag = 0;
	time_t				  last_time = 0;
	time_t				  now_time;

	int                   rv = -1;
	char                  pack_buf[1024];
	pack_info_t			  pack_info;
	socket_ctx_t		  sock;
	char                  *serv_ip = NULL;
	int                   serv_port;

	sqlite3*			  db;
//	float                 temp;

	char                  send_buf[1024];
	int                   row = 0;


	struct option  long_options[]=
	{
		{"daemon",no_argument,NULL,'d'},
		{"ip",required_argument,NULL,'i'},
		{"port",required_argument,NULL,'p'},
		{"help",no_argument,NULL,'h'},
		{"invertal",required_argument,NULL,'t'},
		{NULL,0,NULL,0}
	};

	progname=basename(argv[0]);

	while((opt=getopt_long(argc,argv,"di:p:ht:",long_options,NULL)) != -1)
	{
		switch(opt)
		{
			case'b':
				daemon_run = 1;
				break;

			case'i':
				serv_ip = optarg; 
				break;

			case'p':
				serv_port = atoi(optarg);
				break;

			case'h':
				print_usage(progname);
				return EXIT_SUCCESS;

			case't':
				interval = atoi(optarg);
				break;

			defalt:
				break;
		}
	}

	if(!serv_port || !serv_ip || !interval)
	{
		print_usage(progname);
		return -1;
	}

	if( log_open(logfile, loglevel, logsize, THREAD_LOCK_NONE) <0 )
	{
		printf("initial log system failed\n");
		return -1;
	}

	socket_init(&sock, serv_ip, serv_port);
	socket_connect(&sock);

	db=sqlite_init_db();
	if (db == NULL)
	{
		log_error("Failed to initialize the database!\n");
		return -1;
	}
	log_debug("sqlite initiallize OK!\n");
	//log_debug("interval: %d\n",interval);

	while(1)
	{
		/* 采样时间到了 */
		time(&now_time);
		if( difftime(now_time, last_time) > interval )
		{	
			/* 获取温度 */
			if(ds18b20_get_temperature(&pack_info.temperature) < 0)
			{
				log_error("get temperature failure: %s\n",strerror(errno));
				return -1;
			}
			log_debug("temperature: %f\n",pack_info.temperature);
		
			/* 获取产品序列号 */
			if(get_device(pack_info.devid, sizeof(pack_info.devid)) < 0) 
			{
				log_error("get device failure: %s\n",strerror(errno));
				return -1;
			}
			log_debug("devid: %s\n",pack_info.devid);

			/* 获取时间 */
			if(get_time(pack_info.sample_time, sizeof(pack_info.sample_time)) < 0) 
			{	
				log_error("get time failure: %s\n",strerror(errno));
				return -1;
			}
			log_debug("time: %s\n",pack_info.sample_time);

			last_time = now_time;
			sample_flag = 1;
		}
	
		/* 判断socket是否连接 */
		if( socket_net_status(&sock) == -1 ) 
		{
			socket_connect(&sock); //重新连接
		}

		/* 判断socket是否断开 */
		if( socket_net_status(&sock) == -1)
		{
			if( sample_flag )
			{	
				sqlite_insert_data(db, &pack_info);//把数据写到数据库
				log_info("network failure but insert data ok\n");
				sample_flag = 0;
			}

			continue;
		}

	//代码走这里就是 socket 连接成功
		
		if( sample_flag )
		{
			/*  把数据库打包成结构体 */
			pack_data(&pack_info, pack_buf, sizeof(pack_buf));
			log_debug("pack_buf: %s\n", pack_buf);
			
			/* 发送采样数据 */
			rv = socket_write(&sock, pack_buf, strlen(pack_buf));

			if( rv < 0 ) //发送失败
			{
				log_error("socket write sample data failure: %s\n",strerror(errno));
				sqlite_insert_data(db, &pack_info); //数据写入数据库
				log_warn("write failure but insert data OK\n");
				log_debug("insert data: %s\n",pack_buf);
			}

			sample_flag = 0;
		}

		row = sqlite_check_data(db);
		log_debug("row: %d\n", row);

		if( row >0 )
		{
			/* 发送数据库数据 */
			log_debug("row: %d\n",row);
			sqlite_get_data(db, send_buf); //提取一条数据
			rv = socket_write(&sock, send_buf, strlen(send_buf)); //发送数据
			if( rv == 0 )
			{
				log_info("send sqlite data successfully\n");
				sqlite_delete_data(db); //删除数据库中的数据
			}
		}
		
	}

	return 0;
}

/* 命令行参数解析 */
static inline void print_usage(char *progname)
{
    log_info("Usage: %s [OPTION]...\n",progname);

    log_info("\nMandatory arguments to long options are mandatory for short too:\n");

    log_info("-b[--deamon] set program set running on background\n");
    log_info("-p[--port] socket server port address\n");
    log_info("-i[--ip] socket server ip address\n");
    log_info("-h[--help] display this help imformation\n");
    log_info("-t[--time] the time between the reported data");

    log_info("\nExample: %s  -i 192.168.0.1 -p 8889 -t 5\n",progname);
    return ;
}
