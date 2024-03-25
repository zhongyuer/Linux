/*********************************************************************************
 *      Copyright:  (C) 2024 linuxer<linuxer@email.com>
 *                  All rights reserved.
 *
 *       Filename:  fork.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(29/02/24)
 *         Author:  linuxer <linuxer@email.com>
 *      ChangeLog:  1, Release initial version on "29/02/24 15:43:15"
 *                 
 **************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>


int main(int argc,char **argv)
{
	pid_t pid;


	printf("parent process PID[%d] star running...\n",getpid);

	pid = fork();
	if(pid<0)
	{	
		printf("fork()create child process failure: %s\n",strerror(errno));
		return -1;
	}	
	else if(pid ==0 )
	{
		printf("child process PID[%d] star running,my parent PID is [%d]\n",getpid(),getppid());
		return 0;
	}
	else if(pid > 0)
	{
		printf("parent process PID[%d] continu running, and child process PID is[%d]\n",getpid,pid);
		return 0;
	}
}
