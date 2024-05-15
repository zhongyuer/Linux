/*********************************************************************************
 *      Copyright:  (C) 2024 ioter<ioter@gmail.com>
 *                  All rights reserved.
 *
 *       Filename:  signal.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(15/05/24)
 *         Author:  ioter <ioter@gmail.com>
 *      ChangeLog:  1, Release initial version on "15/05/24 14:07:04"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include <signal.h>
#include "mysignal.h"

int		g_signal = 0;

void install_signal()
{
	struct sigaction	sigact;

	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigact.sa_handler = signal_handler;

	sigaction(SIGTERM, &sigact, 0);
	sigaction(SIGINT, &sigact, 0);
	sigaction(SIGPIPE, &sigact, 0);

	return ;
}

void signal_handler(int signal)
{
	switch(signal)
	{
		case SIGINT:
			printf("SIGINT---stopping\n");
			g_signal = 1;
			break;
		case SIGTERM:
			printf("SIGTERM---stopping\n");
			g_signal = 1;
			break;
		case SIGPIPE:
			printf("SIGPIPE---warnning\n");
			g_signal = 1;
			break;
		
		default:
			break;
	}
}
