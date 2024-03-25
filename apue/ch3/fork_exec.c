/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  fork_exec.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/03/24)
 *         Author:  linuxer <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "04/03/24 19:42:33"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#define TMP_FILE   "/tmp/.ifconfig.log"

int main(int argc,char **argv)
{
	pid_t pid;
	int fd;
	char buf[1024];
	int rv;
	char *ptr;
	FILE *fp;
	char *ip_start;
	char *ip_end;
	char ipaddr[16];


		if((fd=open(TMP_FILE,O_RDWR|O_CREAT|O_TRUNC,0664))<0)
		{
			printf("rdirect standard output to file failure: %s",strerror(errno));
			return -1;
		}
	pid=fork();
	if(pid<0)
	{
		printf("fork() create child process failure: %s",strerror(errno));
		return -2;
	}
	else if(0==pid)
	{
		printf("child process start excute ifconfig program\n");
		dup2(fd,STDOUT_FILENO);
		execl("/sbin/ifconfig","ifconfig","eth0",NULL);
		printf("child process excute another program,will not return here. return here means execl() error\n");
		return -3;
	}
	else
	{
		sleep(3);
	}

	memset(buf,0,sizeof(buf));
	rv=read(fd,buf,sizeof(buf));
	printf("read %d bytes data directly read after child process write\n",rv);

	lseek(fd,0,SEEK_SET);
	rv=read(fd,buf,sizeof(buf));
	printf("read %d bytes data after lseek:\n %s",rv,buf);

	fp = fdopen(fd,"r");
	fseek(fp,0,SEEK_SET);
	

	while(fgets(buf,sizeof(buf),fp))
	{
		if(strstr(buf,"netmask"))
		{
			ptr=strstr(buf,"inet");
			if(!ptr)
			{
				break;
			}
			ptr += strlen("inet");
			
			while(isblank(*ptr))
			ptr++;

			ip_start = ptr;

			while(!isblank(*ptr))
			ptr++;

			ip_end = ptr;

			memset(ipaddr,0,sizeof(ipaddr));
			memcpy(ipaddr,ip_start,ip_end-ip_start);

			break;
		}

	}
	printf("parser and get IP address: %s\n",ipaddr);

	fclose(fp);
	unlink(TMP_FILE);

	return 0;
}

