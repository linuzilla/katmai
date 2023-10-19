/*
 *	rbmcast.h	(Realiable Broadcast/Multicast Library)
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __RBMCAST_LIB_H
#define __RBMCAST_LIB_H

#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>


struct thread_svc_t;
struct rbm_win_buffer_t;

struct rbmcast_lib_pd_t {
	int32_t			magic_id;
	u_int32_t		mcast_group;
	u_int32_t		our_addr;
	u_int32_t		seq;
	u_int16_t		server_id;
	int16_t			client_id;
	int			port;
	int			fd;
	int			sendflag;
	int			recvflag;
	int			errcode;
	volatile short		stage;
	short			ttl;
	short			loopback;
	short			use_broadcast;
	short			is_server;
	short			addrcnt;
	short			addruse_cnt;
	short			*addruse;
	struct sockaddr_in	*addrlist;
	struct sockaddr_in	addr;
	struct sockaddr_in	rmaddr;
	struct sockaddr_in	bcaddr;
	struct sockaddr_in	mcaddr;
	struct thread_svc_t	*thr;
	struct thread_svc_t	*sender;
	volatile int		terminate;

	pthread_mutex_t		mutex;
	pthread_cond_t		condi;

	u_int32_t		expect_seq;
	u_int32_t		max_expect_seq;
	u_int32_t		max_recv_seq;

	struct rbm_win_buffer_t	*wbuf;
	int			wb_rear;
	int			wb_front;
	int			wb_ptr;
	int			wb_cnt;
	pthread_mutex_t		wb_mutex;
	pthread_cond_t		wb_condi;
	pthread_mutex_t		wb_ready_mutex;
	pthread_cond_t		wb_ready_condi;

	int			winsize;
};

struct rbmcast_lib_t {
	struct rbmcast_lib_pd_t		pd;
	// ---------------------------------------------------------------
	int		(*open)(struct rbmcast_lib_t *, const char *mgrp,
					const char *intf);
	void		(*close)(struct rbmcast_lib_t *);
	int		(*bind)(struct rbmcast_lib_t *,
					const int port, const int magic,
					const int is_server);
	int		(*listen)(struct rbmcast_lib_t *, const int noc);
	int		(*connect)(struct rbmcast_lib_t *, const int timeout);
	void		(*stop)(struct rbmcast_lib_t *);
	// ---------------------------------------------------------------
	int		(*ttl)(struct rbmcast_lib_t *, const int ttl);
	int		(*loopback)(struct rbmcast_lib_t *, const int lp);
	// ---------------------------------------------------------------
	int		(*set_sendflag)(struct rbmcast_lib_t *, const int);
	int		(*set_recvflag)(struct rbmcast_lib_t *, const int);
	// ---------------------------------------------------------------
	int		(*num_of_client)(struct rbmcast_lib_t *);
	int		(*mtu)(struct rbmcast_lib_t *);
	// ---------------------------------------------------------------
	int		(*send)(struct rbmcast_lib_t *,
					const void *buf, const int len);
	// ---------------------------------------------------------------
	void		(*dispose)(struct rbmcast_lib_t *);
};

struct rbmcast_lib_t	*new_rbmcast (void);

#endif
