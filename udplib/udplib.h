/*
 *	udplib.h
 *
 *	Copyright (c) 2002, written by Jiann-Ching Liu
 */

#ifndef __UDP_LIBRARY_H__
#define __UDP_LIBRARY_H__

#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>

#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
#endif

#if (GCC_VERSION < 2096)
typedef u_int32_t	in_addr_t;
#error Ooop!!
#endif

struct udplib_t {
	int			fd;
	struct sockaddr_in	myaddr;
	struct sockaddr_in	bcaddr;
	struct sockaddr_in	rmaddr;
	struct sockaddr_in	*addrlist;
	short			*addruse;
	int			myport;
	int			listcnt;
	int			ioflag;
	int			sendflag;
	int			recvflag;
	short			nonblocking_flag;
	int			(*getfd)(struct udplib_t *);
	int			(*open)(struct udplib_t *, const int port,
					const char *intf, const int isbind);
	void			(*close)(struct udplib_t *);
	void			(*dispose)(struct udplib_t *);
	int			(*num_of_registed)(struct udplib_t *);
	int			(*recvfrom)(struct udplib_t *,
					void *buf, size_t len,
					struct sockaddr *from,
					socklen_t *fromlen);
	int			(*receive)(struct udplib_t *,
					void *buf, size_t len);
	int			(*recv)(struct udplib_t *,
					void *buf, size_t len, const int);
	int			(*recvms)(struct udplib_t *,
					void *buf, size_t len, const int);
	int			(*bsend)(struct udplib_t *,
					const int port,
					void *buf, size_t len);
	int			(*send)(struct udplib_t *,
					void *buf, size_t len);
	int			(*sendto)(struct udplib_t *, const int n,
					void *buf, size_t len);
	char*			(*remote_addr)(struct udplib_t *);
	int			(*remote_port)(struct udplib_t *self);
	char*			(*my_ip)(struct udplib_t *);
	char*			(*ip_addr)(struct udplib_t *, const int n);
	int			(*udp_port)(struct udplib_t *, const int n);
	int			(*is_local)(struct udplib_t *);
	int			(*ip_cmp)(struct udplib_t *);
	in_addr_t		(*get_remote)(struct udplib_t *self);
	in_addr_t		(*get_myself)(struct udplib_t *);
	int			(*regist_rmaddr)(struct udplib_t *self);
	int			(*regist_ipaddr)(struct udplib_t *self);
	int			(*regist_addr)(struct udplib_t *self,
						const char *ip, const int port);
	int			(*regist_sockaddr)(struct udplib_t *self,
						struct sockaddr_in *addr);
	void			(*unregist)(struct udplib_t *self,
						const int n);
	int			(*nonblocking)(struct udplib_t *self,
						const int n);
	int			(*set_sendflag)(struct udplib_t *self,
						const int f);
	int			(*set_recvflag)(struct udplib_t *self,
						const int f);
	int			(*set_port)(struct udplib_t *self,
						const int n,
						const int port);
	int			(*local_port)(struct udplib_t *self);
	void			(*external_call)(struct udplib_t *self,
					void *caller,
					void (*cbk)(void *caller, const int i));
};


struct udplib_t * new_udplib (const int n);

#endif
