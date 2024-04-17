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
#include <sqlite3.h>

#define LENTH 64

static inline void print_usage(char *progname);
int get_temperature(float *temp);
int get_device(char *id);
int get_time(char *ch);

int sqlite_init_db();
int sqlite_insert_data(char *id,float temp,char *time);
int sqlite_check_data();
int sqlite_get_data();
int sqlite_delete_data();
int sqlite_close_db();

sqlite3*	db;


int main(int argc,char **argv)
{
	int                   daemon_run=0;
	char                  *progname=NULL;
	int                   opt;

	struct sockaddr_in    serv_addr;
	int                   connfd = -1;
	int                   rv = -1;
	char                  buf[1024];
	char                  *serv_ip;
	int                   serv_port = 0;
	float                 temp;
	int                   on;
	int                   time;
	char				  id[32];
	char				  ch[64];
	char				  buf_send[1024];

	struct option  long_options[]=
	{
		{"daemon",no_argument,NULL,'d'},
		{"ip",required_argument,NULL,'i'},
		{"port",required_argument,NULL,'p'},
		{"help",no_argument,NULL,'h'},
		{"time",required_argument,NULL,'t'},
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
				time = atoi(optarg);
				break;

			defalt:
				break;
		}
	}
	if(!serv_port||!serv_ip||!time)
	{
		print_usage(progname);
		return -1;
	}

 while(1)
 {
	sleep(time);
	connfd = socket(AF_INET,SOCK_STREAM,0);
	if(connfd<0)
	{
		printf("create socket failure: %s\n",strerror(errno));
		return -1;
	}
	printf("socket create fd[%d] seccessfully\n",connfd);

	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(serv_port);
	inet_aton(serv_ip,&serv_addr.sin_addr);

	if(connect(connfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
	{
		printf("connect to server [%s:%d] failure: %s\n",serv_ip,serv_port,strerror(errno));
		return 0;
	}

	if((on=get_temperature(&temp))<0)
	{
		printf("get temperature failure: %s\n",strerror(errno));
	    return -1;
	}

	if(get_device(id)<0)
	{
		printf("get device failure: %s\n",strerror(errno));
		return -1;
	}

	if(get_time(ch)<0)
	{
		printf("get time failure: %s\n",strerror(errno));
		return -1;
	}

	snprintf(buf,128,"%s;%f;%s\n",id,temp,ch);

	sqlite_init_db();
	sqlite_insert_data(id,temp,ch);
//	sqlite_check_data();
//	sqlite_get_data(buf_send);
//	sqlite_delete_data();
	int sqlite_close_db();


	if(write(connfd,buf,strlen(buf))<0)
	{
		printf("write data to server [%s:%d] failure: %s\n",serv_ip,serv_port,strerror(errno));
		goto cleanup;
	}
	printf("write data to server: %s\n",buf);

	memset(buf,0,sizeof(buf));
	rv = read(connfd,buf,sizeof(buf));
	if(rv<0)
	{
		printf("read data from server failure: %s\n",strerror(errno));
		goto cleanup;
	}

	else if(0 ==rv)
	{
		printf("client connect to server get disconnceted\n");
		goto cleanup;
	}
	printf("read %d bytes data from server: %s\n",rv,buf);

 }

cleanup:
	close(connfd);
	return 0;

}

/* 命令行参数解析 */
static inline void print_usage(char *progname)
{
	printf("usage: %s [OPTION]...\n",progname);
	printf("\nMandatory argument to long options are mandatory for short too:\n");

	printf("-b[--deamon] set program set running on background\n");
	printf("-p[--port] socket server port address\n");
	printf("-i[--ip] socket server ip address\n");
	printf("-h[--help] display this help imformation\n");
	printf("-t[--time] the time between the reported data");

	printf("\nExample: %s  -i 192.168.0.1 -p 8889 -t 5\n",progname);
	return ;
}

/*从ds18b20中获取温度*/
int get_temperature(float *temp)	
{	
	int              fd=-1;
	char             buf[128];
	char             *ptr=NULL;
	DIR              *dirp=NULL;
	char             W1_path[64]="/sys/bus/w1/devices/";
	struct dirent    *direntp=NULL;
	char             chip_sn[32];
	char             ds18b20_path[64];
	int              found=0;

	dirp=opendir(W1_path);
	if(!dirp)
	{
		printf("open folder %s failure: %s\n",W1_path,strerror(errno));
		return -1;
	}
	
	while(NULL != (direntp=readdir(dirp)))
	{
		if(strstr(direntp->d_name,"28-"))
		{
			strncpy(chip_sn,direntp->d_name,sizeof(chip_sn));
			found=1;
		}
	}	
	
	closedir(dirp);

	if(!found)
	{
		printf("can not find ds18b20 chipset\n");
		return -2;
	}

	snprintf(ds18b20_path,sizeof(ds18b20_path),"%s%s/w1_slave",W1_path,chip_sn);

	fd=open(ds18b20_path,O_RDONLY);
	if(fd<0)
	{
		printf("open file failure: %s\n",strerror(errno));
		return -1;
	}

	memset(buf,0,sizeof(buf));
	if(read(fd,buf,sizeof(buf))<0)
	{
		printf("read data from fd=%d failure: %s\n",fd,strerror(errno));
		return -2;
	}	

	ptr=strstr(buf,"t=");
	if(!ptr)
	{
		printf("can not find t=strstr\n");
		return -1;
	}
	
	ptr+=2;

	*temp=atof(ptr)/1000;
	printf("temperature: %f\n",*temp); 

	close(fd);

	return 0;
 
}

/* 获取产品系列号 */
int get_device(char *id)
{
	int		sn = 1;

	snprintf(id,LENTH,"RPI%04d",sn);

	return 0;
}

/* 获取时间 */
int get_time(char *ch)
{
	time_t		tim;
	struct tm	*p;
	
	time(&tim);
	p = localtime(&tim);

	snprintf(ch,LENTH,"%d-%d-%d %d:%d:%d",1900+p->tm_year,1+p->tm_mon,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);

	return 0;
}

/*创建并打开数据库和表*/
int sqlite_init_db()
{
	char	sql[128];
	char	*errmsg = NULL;
	int		ret = -1;

	ret = sqlite3_open("cliDB.db",&db);
	if(ret != SQLITE_OK)
	{
		printf("create database failure: %s\n",sqlite3_errmsg(db));
		sqlite3_free(errmsg);

		return -1;
	}
	else
	{
		memset(sql,0,sizeof(sql));
		sprintf(sql,"CREATE TABLE IF NOT EXISTS cliTABLE(ID INTEGER PRIMARY KEY AUTOINCREMENT,SN TEXT,TEMP REAL,TIME TEXT)");
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

/*插入数据到数据库中的表*/
int sqlite_insert_data(char *id,float temp,char *ch)
{
	char	sql[128];
	char	*errmsg = NULL;
	int		ret = -1;

	sprintf(sql,"INSERT INTO cliTABLE (SN,TEMP,TIME) VALUES('%s','%f','%s')",id,temp,ch);
	ret = sqlite3_exec(db,sql,0,0,&errmsg);
	if(ret != SQLITE_OK)
	{
		printf("insert into table failure: %s\n",sqlite3_errmsg(db));
		sqlite3_free(errmsg);

		return -3;
	}

	return 0;
}

/*检查数据库中的表的数据*/
int sqlite_check_data()
{
	char	sql[128];
	int		ret = -1;
	int		row = 0;
	int		column = 0;
	char	**result;
	char	*errmsg = NULL;

	memset(sql,0,sizeof(sql));
	sprintf(sql,"SELECT * FROM cliTABLE");
	ret = sqlite3_get_table(db,sql,&result,&row,&column,&errmsg);
	if(ret != SQLITE_OK)
	{
		printf("check table data failure: %s\n",sqlite3_errmsg(db));
		sqlite3_free(errmsg);
	
		return -4;
	}
	else if(row>0)
	{
		return row;
	}
	return 0;
}

/* 从数据库的表中获取数据 */
int	sqlite_get_data(char *send_buf)
{
	char	sql[128];
	int		ret = -1;
	int		row = 0;
	int		column = 0;
	char	**result;
	char	*errmsg = NULL;

	memset(sql,0,sizeof(sql));
	sprintf(sql,"SELECT * FROM cliTABLE LIMIT 1");
	
	ret = sqlite3_get_table(db,sql,&result,&row,&column,&errmsg);
	if(ret != SQLITE_OK);
	{
		printf("get table data failure: %s\n",sqlite3_errmsg(db));
		sqlite3_free(errmsg);

		return -5;
	}

	sprintf(send_buf,"%s;%s;%s",result[1*column+1],result[1*column+2],result[1*column+3]);
	printf("send_buf: %s\n",send_buf);

	return 0;
}

/* 删除数据库中表的数据 */
int sqlite_delete_data()
{
	char	sql[128];
	int		ret = -1;
	int		row = 0;
	int		column = 0;
	char	**result;
	char	*errmsg = NULL;

	memset(sql,0,sizeof(sql));
	sprintf(sql,"DELETE FROM cliTABLE WHERE ROEID IN(SELECT ROWID FROM cliTABLE LIMIT 1);");
	ret = sqlite3_exec(db,sql,0,0,&errmsg);
	if(ret != SQLITE_OK)
	{
		printf("delete table data failure: %s\n",sqlite3_errmsg(db));
		sqlite3_free(errmsg);

		return -6;
	}

	return 0;
}

/* 关闭数据库 */
int	sqlite_close_db()
{
	char	*errmsg = NULL;


	if(db == NULL)
	{
		printf("close sqlite db failure: %s\n",sqlite3_errmsg(db));
		sqlite3_free(errmsg);
		
		return -7;	
	}

	return 0;
}
