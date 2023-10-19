/*
 *	tcplib.h
 *
 *	Copyright (c) 2002, Jainn-Ching Liu
 */

#ifndef __TCP_LIBRARY_H__
#define __TCP_LIBRARY_H__

#include <netinet/in.h>
#include <pthread.h>

struct tcplib_private_data_t {
	struct sockaddr_in	myaddr;
	struct sockaddr_in	rmaddr;
	int			fd;
	volatile short		terminate;
	pthread_t		thr;
	short			have_thr;
	short			multiconn;
	void			(*callback)(const int fd, void *ptr);
	void			*param;
};

struct tcplib_t {
	struct tcplib_private_data_t	pd;
	int	(*listen)(struct tcplib_t *,
			void (*cbk)(const int, void *ptr), const int port,
			void *param);
	void	(*stop)(struct tcplib_t *);
	int	(*connect)(struct tcplib_t *,
			const char *ip, const int port);
	void	(*close)(struct tcplib_t *);
	void	(*dispose)(struct tcplib_t *);
	int	(*multiconn)(struct tcplib_t *self, const int n);
};

struct tcplib_t	* new_tcplib (void);

#endif
