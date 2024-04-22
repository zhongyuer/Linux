/********************************************************************************
 *      Copyright:  (C) 2024 LiYi<1751425323@qq.com>
 *                  All rights reserved.
 *
 *       Filename:  socket.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(19/04/24)
 *         Author:  LiYi <1751425323@qq.com>
 *      ChangeLog:  1, Release initial version on "19/04/24 21:11:02"
 *                 
 ********************************************************************************/

#ifndef SOCKET_H
#define SOCKET_H

int socket_init(socket_t *sock, char *hostname, int port);
int socket_connect(socket_t *sock);
int socket_diag(socket_t *sock);
int socket_write(socket_t *sock, char *data, int bytes);
int socket_close(socket_t *sock);

#endif
