/*********************************************************************************
 *      Copyright:  (C) 2024 LiYi<1751425323@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  packet.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(19/04/24)
 *         Author:  LiYi <1751425323@qq.com>
 *      ChangeLog:  1, Release initial version on "19/04/24 21:53:49"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include <string.h>
#include <time.h>



#include "packet.h"
#include "ds18b20.h"

int get_device(char *devid, int size)
{
	int		sn = 1;

	if( !devid || size< DEVID_LEN )
	{
		printf("Invalid input arugment\n");
		return -1;
	}

	memset(devid,0,size);
	snprintf(devid, size, "RIP:%04d", sn);

	return 0;
}

int get_time(char *sample_time, int size)
{
	time_t		tim;
	struct tm	*p;

	time(&tim);
	p = localtime(&tim);

	memset(sample_time,0,size);
	snprintf(sample_time, size, "%04d-%02d-%02d %02d:%02d:%02d", 
			1900+p->tm_year, 1+p->tm_mon, p->tm_mday,
			p->tm_hour, p->tm_min, p->tm_sec);

	return 0;
}

int pack_data(pack_info_t *pack_info, char *pack_buf, int size)
{
	char	*buf = pack_buf;

	memset(buf,0,size);
	snprintf(buf,size,"%s;%s;%.3f",pack_info->devid, pack_info->sample_time, pack_info->temperature);

	return strlen(buf);
}

