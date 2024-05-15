/********************************************************************************
 *      Copyright:  (C) 2024 ioter<ioter@gmail.com>
 *                  All rights reserved.
 *
 *       Filename:  mysignal.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(15/05/24)
 *         Author:  ioter <ioter@gmail.com>
 *      ChangeLog:  1, Release initial version on "15/05/24 14:18:25"
 *                 
 ********************************************************************************/
#ifndef _SIGNAL_H_

#define _SIGNAL_H_

extern void install_signal();

extern void signal_handler(int signal);

extern int g_signal;

#endif

