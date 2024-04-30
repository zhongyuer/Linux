/********************************************************************************
 *      Copyright:  (C) 2024 LiYi<1751425323@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  database.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(19/04/24)
 *         Author:  LiYi <1751425323@qq.com>
 *      ChangeLog:  1, Release initial version on "19/04/24 19:39:50"
 *                 
 ********************************************************************************/

#ifndef DATABASE_H
#define DATABASE_H

sqlite3* sqlite_init_db();
int sqlite_insert_data(char *id,float temp,char *time);
int sqlite_check_data(sqlite3*db);
int sqlite_get_data(sqlite3* db,char *send_buf);
int sqlite_delete_data(sqlite3* db);


#endif
