/*
 *	mcast.h
 *
 *	Copyright (c) 2002, written by Jiann-Ching Liu
 */

#ifndef __MULTICAST_H__
#define __MULTICAST_H__

#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>

#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
#endif

#if (GCC_VERSION < 2096)
// typedef u_int32_t	in_addr_t;
#endif

#define DEFAULT_FS_MCAST_ADDR		"225.1.1.1"
#define DEFAULT_FS_MCAST_PORT		6150

struct mcast_lib_pd_t {
	int			fd;
	struct sockaddr_in      addr;
	struct sockaddr_in	mcaddr;
	struct sockaddr_in	rmaddr;
	struct sockaddr_in	bcaddr;
	u_int32_t		mcast_group;
	u_int32_t		our_addr;

	struct sockaddr_in	*addrlist;
	short			*addruse;
	int			listcnt;
	int			ioflag;
	int			sendflag;
	int			recvflag;
	short			nonblocking_flag;
};

struct mcast_lib_t {
	struct mcast_lib_pd_t	pd;
	int			(*open)(struct mcast_lib_t *, const char *,
						const char *interface,
						const int is_loopback);
	void			(*close)(struct mcast_lib_t *);
	int			(*bind)(struct mcast_lib_t *,
					const in_addr_t addr, const int port);
	int			(*receive)(struct mcast_lib_t *,
					void *buf, size_t len);
	int			(*recvfrom)(struct mcast_lib_t *,
					void *buf, size_t len,
					struct sockaddr *from,
					socklen_t *fromlen);
	int			(*recv)(struct mcast_lib_t *,
					void *buf, size_t len, const int sec);
	int			(*recvms)(struct mcast_lib_t *,
					void *buf, size_t len, const int msec);
	int			(*send)(struct mcast_lib_t *,
					void *buf, size_t len);
	int			(*bsend)(struct mcast_lib_t *, const int port,
					void *buf, size_t len);
	int			(*sendback)(struct mcast_lib_t *,
					void *buf, size_t len);
	int			(*sendto)(struct mcast_lib_t *, const int n,
					void *buf, size_t len);
	void			(*unregist)(struct mcast_lib_t *self,
					const int n);
	char * 			(*remote_addr)(struct mcast_lib_t *self);
	int			(*remote_port)(struct mcast_lib_t *self);
	int			(*regist_rmaddr)(struct mcast_lib_t *self);
	int			(*regist_addr)(struct mcast_lib_t *self,
						const char *ip, const int port);

	int			(*nonblocking)(struct mcast_lib_t *self,
						const int n);

	int			(*num_of_registed)(struct mcast_lib_t *self);

	int			(*set_sendflag)(struct mcast_lib_t *self,
						const int f);
	int			(*set_recvflag)(struct mcast_lib_t *self,
						const int f);
	
	void			(*dispose)(struct mcast_lib_t *);
};

struct mcast_lib_t * new_multicast (const int n);

#endif
