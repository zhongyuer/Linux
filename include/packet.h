/********************************************************************************
 *      Copyright:  (C) 2024 LiYi<1751425323@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  packet.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(19/04/24)
 *         Author:  LiYi <1751425323@qq.com>
 *      ChangeLog:  1, Release initial version on "19/04/24 21:21:04"
 *                 
 ********************************************************************************/
#ifndef PACKET_H
#define PACKET_H


#define DEVID_LEN	8
#define TIME_LEN	32

typedef struct pack_info_s
{
	char		devid[DEVID_LEN+1];
	char		sample_time[TIME_LEN];
	float		temperature;
}pack_info_t;


extern int get_device(char *devid, int size);

extern int get_time(char *sample_time, int size);

int pack_data(pack_info_t *pack_info, char *pack_buf, int size);

#endif
