/*
 *	client_entry.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __CLIENT_ENTRY__H__
#define __CLIENT_ENTRY__H__

// #include <sys/types.h>
// #include <netdb.h>
#include <netinet/in.h>
#include <time.h>

struct udplib_t;
struct thread_svc_t;
struct stio_v2_server_local_t;
struct streamming_io_v2_server_t;

struct client_entry_pd_t {
	struct streamming_io_v2_server_t	*parent;

	struct udplib_t		*udp;
	struct thread_svc_t	*thr;
	int			port;
	int			sn;
	short			inuse;
	time_t			lastuse;
	in_addr_t		addr;
	volatile short		terminate;
	volatile short		in_services;
	// int			(*fopen)(const int, const char *fname);
	struct stio_v2_server_local_t	*sl;
};

struct client_entry_t {
	struct client_entry_pd_t	pd;

	void		(*dispose)(struct client_entry_t *);
	int		(*start)(struct client_entry_t *);
	int		(*stop)(struct client_entry_t *);
	int		(*is_inuse)(struct client_entry_t *);
	int		(*unuse)(struct client_entry_t *);
	int		(*regist)(struct client_entry_t *,
				const in_addr_t, const int);

	/*
	void		(*set_fopen)(struct client_entry_t *,
				int (*)(const int, const char *));
				*/

	int		(*cksum)(const void *ptr, const int len);
};

struct client_entry_t	* new_client_entry (struct streamming_io_v2_server_t *);

#endif
