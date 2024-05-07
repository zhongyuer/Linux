/*********************************************************************************
 *      Copyright:  (C) 2024 LiYi<1751425323@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  ds18b20.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(19/04/24)
 *         Author:  LiYi <1751425323@qq.com>
 *      ChangeLog:  1, Release initial version on "19/04/24 19:55:30"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include "ds18b20.h"
#include "logger.h"
int ds18b20_get_temperature(float *temp)
{
	int				fd = -1;
	char			buf[128];
	char			*ptr = NULL;
	DIR				*dirp = NULL;
	char			W1_path[64]="/sys/bus/w1/devices/";
	struct	dirent	*direntp = NULL;
	char			chip_sn[32];
	char			ds18b20_path[64];
	int				found = 0;

	dirp=opendir(W1_path);
	if(!dirp)
	{
		log_error("open folder %s failure: %s\n",W1_path,strerror(errno));
		return -1;
	}

	while(NULL != (direntp=readdir(dirp)))
	{
		if(strstr(direntp->d_name,"28-"))
		{
			strncpy(chip_sn,direntp->d_name,sizeof(chip_sn));
			found = 1;
		}
	}

	closedir(dirp);

	if(!found)
	{
		log_error("can not find ds18b20 chipset\n");
		return -2;
	}

	snprintf(ds18b20_path,sizeof(ds18b20_path),"%s%s/w1_slave",W1_path,chip_sn);

	fd=open(ds18b20_path,O_RDONLY);
	if(fd<0)
	{
		log_error("open file failure: %s\n",strerror(errno));
		return -1;
	}

	memset(buf,0,sizeof(buf));
	if(read(fd,buf,sizeof(buf))<0)
	{
		log_error("read data from fd[%d] failure: %s\n",fd,strerror(errno));
		return -2;
	}

	ptr=strstr(buf,"t=");
	if(!ptr)
	{
		log_error("can not find t=strstr\n");
		return -1;
	}

	ptr+=2;

	*temp=atof(ptr)/1000;

	close(fd);

	return 0;
}
