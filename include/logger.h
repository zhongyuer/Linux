/********************************************************************************
 *      Copyright:  (C) 2024 LiYi<1751425323@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  logger.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(20/04/24)
 *         Author:  LiYi <1751425323@qq.com>
 *      ChangeLog:  1, Release initial version on "20/04/24 14:22:51"
 *                 
 ********************************************************************************/

#ifndef	_LOGGER_H_
#define	_LOGGER_H_

#include <stdio.h>
#include <stdarg.h>

#define LOG_VERSION "v0.1"

enum {
	LOG_LEVEL_ERROR,
	LOG_LEVEL_WARN,
	LOG_LEVEL_INFO,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_TRACE,
	LOG_LEVEL_MAX
};

enum {
	LOG_LOCK_DISAABLE,
	LOG_LOCK_ENABLE,
};

#define ROLLBACK_NONE	0

#define THREAD_LOCK_NONE	0
#define THREAD_LOCK_EN		1
int log_open(char *fname, int level, int size, int lock);

void log_close(void);

void _log_write(int level, const char *file, int line, const char *fmt, ...);

void log_dump(int level, const char *promt, char *buf, size_t len);

#define log_trace(...) _log_write(LOG_LEVEL_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) _log_write(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  _log_write(LOG_LEVEL_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  _log_write(LOG_LEVEL_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) _log_write(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#endif
