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

#define	HOSTNAME_LEN	64

typedef struct socket_ctx_s
{
	char	host[HOSTNAME_LEN];
	int		port;
	int		fd;
	int		connected;
}socket_ctx_t;

extern int socket_init(socket_ctx_t *sock, char *host, int port);
extern int socket_connect(socket_ctx_t *sock);
extern int socket_write(socket_ctx_t *sock, char *data, int size);
extern int socket_close(socket_ctx_t *sock);
extern int socket_net_status(socket_ctx_t *sock);
extern void set_socket_rlimit(void);

#endif
