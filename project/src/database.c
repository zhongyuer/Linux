/*********************************************************************************
 *      Copyright:  (C) 2024 LiYi<1751425323@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  database.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(18/04/24)
 *         Author:  LiYi <1751425323@qq.com>
 *      ChangeLog:  1, Release initial version on "18/04/24 21:20:16"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <errno.h>
#include "database.h"

sqlite3	*db;


/*创建数据库、创建表*/
sqlite3* sqlite_init_db()
{
	char	sql[128];
	char	*errmsg = NULL;
	int		ret = -1;

	ret = sqlite3_open("cliDB.db",&db);
	if(ret != SQLITE_OK)
	{
		printf("create database failure: %s\n",sqlite3_errmsg(db));
		sqlite3_free(errmsg);

		return 0;
	}
	else
	{
		memset(sql,0,sizeof(sql));
		sprintf(sql,"CREATE TABLE IF NOT EXISTS cliTABLE(ID INTEGER PRIMARY KEY AUTOINCREMNT,SN TEXT,TEMP REAL TIME TEXT)");
		ret = sqlite3_exec(db,sql,0,0,&errmsg);
		if(ret != SQLITE_OK)
		{
			printf("create table failure: %s\n",strerror(errno));
			sqlite3_free(errmsg);

			return 0;
		}

		return db;
	}
}

/*插入数据到数据库的表中*/
int sqlite_insert_data(char *id,float temp,char *ch)
{
	char	sql[128];
	char	*errmsg = NULL;
	int		ret = -1;

	sprintf(sql,"INSERT INTO cliTABLE (SN,TEMP,TIME) VALUES('%s','%f','%s')",id,temp,ch);
	ret = sqlite3_exec(db,sql,0,0,&errmsg);
	if(ret != SQLITE_OK)
	{
		printf("insert into table failure: %s",sqlite3_errmsg(db));
		sqlite3_free(errmsg);

		return -3;
	}

	return 0;
}

/* 检查数据库中的表是否有数据 */
int sqlite_check_data(sqlite3* db)
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
	else if(row > 0)
	{
		sqlite3_free_table(result);
		return row;
	}

	return 0;
}

/* 从数据库的表中获取数据 */
int sqlite_get_data(sqlite3* db,char *send_buf)
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
	if(ret != SQLITE_OK)
	{
		printf("get table data failure: %s\n",sqlite3_errmsg(db));
		sqlite3_free(errmsg);

		return -5;
	}	
	sprintf(send_buf,"%s;%s;%s",result[1*column+1],result[1*column+2],result[1*column+3]);
	printf("send_buf: %s\n",send_buf);

	return 0;
}

/* 删除数据库中表的信息 */
int sqlite_delete_data(sqlite3* db)
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

/*关闭数据库*/
int sqlite_close_db(sqlite3* db)
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
