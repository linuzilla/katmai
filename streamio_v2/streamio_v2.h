/*
 *	streamio_v2.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __STREAMMING_IO_V2__H_
#define __STREAMMING_IO_V2__H_

#include <pthread.h>
#include <netinet/in.h>

#ifndef MAXIMUM_STIOV2_SERVICES
#define MAXIMUM_STIOV2_SERVICES		4
#endif

#define STIOV2_PROTOCOL_VERSION		2


struct udplib_t;
// struct streamming_io_v2_pdu_t;
struct thread_svc_t;
struct client_entry_t;
struct stio_v2_client_local_t;
struct stio_engine_t;

struct streamming_io_v2_server_pd_t {
	struct client_entry_t	**cli;
	struct thread_svc_t	*thr;
	volatile short		terminate;
	struct udplib_t		*udp;
	int			maxclient;
	int			port;
	volatile short		stop_services;
	short			already_started;
	int			disk_bufsize[MAXIMUM_STIOV2_SERVICES];
	int			net_bufsize[MAXIMUM_STIOV2_SERVICES];
	int			disknet_fact[MAXIMUM_STIOV2_SERVICES];
	int			packet_map[MAXIMUM_STIOV2_SERVICES];
	int			(*fopen)(const int, const char *);
};

struct streamming_io_v2_server_t {
	struct streamming_io_v2_server_pd_t	pd;

	int	(*start)(struct streamming_io_v2_server_t *self);
	int	(*stop)(struct streamming_io_v2_server_t *self);
	int	(*pause)(struct streamming_io_v2_server_t *self, const int);
	void	(*dispose)(struct streamming_io_v2_server_t *self);
	int	(*regist)(struct streamming_io_v2_server_t *self,
			const in_addr_t, const int ent, const int sn);
	int	(*cksum)(const void *ptr, const int len);
	int	(*set_parameter)(struct streamming_io_v2_server_t *,
					const int channel,
					const int diskbuf, const int netbuf);
	void	(*set_fopen)(struct streamming_io_v2_server_t *,
				int (*fopen)(const int, const char *));
};

// ---------------------------------------------------------------

struct streamming_io_v2_client_pd_t {
	struct thread_svc_t		*thr;
	struct udplib_t			*sudp;
	struct udplib_t			*rudp;
	int				port;
	int				svcport;
	volatile short			terminate;
	volatile short			have_server;
	// pthread_mutex_t		mutex;
	// pthread_cond_t		condi;
	pthread_mutex_t			svr_mutex;
	pthread_cond_t			svr_condi;
	int				sn;
	struct stio_v2_client_local_t	*cl;
	volatile short			abort;
	char				server_ip[16];
	int				server_port;
	int				server_id;
};

struct streamming_io_v2_client_t {
	struct streamming_io_v2_client_pd_t	pd;

	int	(*find_server)(struct streamming_io_v2_client_t *);
	void	(*dispose)(struct streamming_io_v2_client_t *);
	int	(*cksum)(const void *ptr, const int len);
	int	(*openfile)(struct streamming_io_v2_client_t *,
			const int channel,
			const char *fname);
	int	(*reopen)(struct streamming_io_v2_client_t *,
			const int channel);
	int	(*closefile)(struct streamming_io_v2_client_t *,
			const int channel);
	off_t	(*filesize)(struct streamming_io_v2_client_t *,
			const int channel);
	int	(*read)(struct streamming_io_v2_client_t *,
			const int channel,
			char *buffer, const int len);
	int	(*get_server)(struct streamming_io_v2_client_t *,
				char **server, int **port);
	int	(*get_server_id)(struct streamming_io_v2_client_t *);
	void	(*abort)(struct streamming_io_v2_client_t *);
	int	(*regist_stio)(struct streamming_io_v2_client_t *,
			const int channel,
			struct stio_engine_t *);
};


struct streamming_io_v2_server_t	*
		new_streamming_io_v2_server (const int maxclient,
						const int port);

struct streamming_io_v2_client_t	*
		new_streamming_io_v2_client (const int port);

#endif
