/*
 *	streamio_v3.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __STREAMMING_IO_V3__H_
#define __STREAMMING_IO_V3__H_

#ifndef MAX_STIOV3_CHANNEL
#define MAX_STIOV3_CHANNEL	5
#endif

#ifndef ENABLE_MD5SUM_FOR_STREAMMING_FILE
#define ENABLE_MD5SUM_FOR_STREAMMING_FILE	0
#endif

#ifndef ENABLE_PREFER_VOD_SERVER
#define ENABLE_PREFER_VOD_SERVER	1
#endif

#define DEBUG_STIOV3	0

#if ENABLE_MD5SUM_FOR_STREAMMING_FILE == 1
#include "md5.h"
#endif

struct thread_svc_t;
struct udplib_t;
struct stbtable_t;
struct vodsvr_t;
struct stio_engine_t;
struct streamming_io_v3_client_t;

struct stio_v3_svc_param_t {
	struct streamming_io_v3_server_t	*stiov3svr;
	int					channel;
};

// ------------------------------------------------------------------------

struct stio_v3_client_local_t {
	struct streamming_io_v3_client_t	*parent;
	int					channel;
	struct stio_engine_t                    *save_stio;
};

// ------------------------------------------------------------------------

struct streamming_io_v3_server_pd_t {
	int				my_id;
	volatile short			terminate;
	short				already_started;
	short				stop_services;
	int				stb_qos_threshold;
	int				version_major;
	int				version_minor;
	struct thread_svc_t		*thr;
	struct udplib_t			*udp;
	struct stbtable_t		*stb;
	pthread_t			svcthr[100];
	// int				(*fopen)(const int, const char *);
	char *				(*file_pathname)(const int,
							const char *);
	char *				(*real_filename)(const int);
};

struct streamming_io_v3_server_t {
	struct streamming_io_v3_server_pd_t	pd;

	void		(*dispose)(struct streamming_io_v3_server_t *self);
	int		(*start)(struct streamming_io_v3_server_t *self);
	int		(*stop)(struct streamming_io_v3_server_t *self);

	int		(*pause)(struct streamming_io_v3_server_t *self,
							const int);

	int		(*start_svc)(struct streamming_io_v3_server_t *self);
	void		(*close_all)(struct streamming_io_v3_server_t *self);

	int		(*cksum)(const void *ptr, const int len);
	int		(*set_parameter)(struct streamming_io_v3_server_t *,
					const int channel,
					const int diskbuf, const int netbuf);

	void		(*set_fopen)(struct streamming_io_v3_server_t *,
					int (*fopen)(const int, const char *));
	void		(*set_file_pathname)(struct streamming_io_v3_server_t *,
					char *(*f)(const int, const char *));
	void		(*set_real_filename)(struct streamming_io_v3_server_t *,
					char *(*f)(const int));
};

// ------------------------------------------------------------------------

struct streamming_io_v3_client_pd_t {
	int				stbid;
	int				port;
	int				current_VODserver;
	int				have_VODserver;
	int				qos_threshold;
	int				file_server;
	pthread_mutex_t			vod_mutex;
	pthread_cond_t			vod_condi;
	volatile short			terminate;
	volatile short			cut;
	struct udplib_t			*udp;
	struct thread_svc_t		*thr;
	struct vodsvr_t			*vodsvr;
	char				server_ip[16];
	int				server_port;
	int				vodsvr_response[MAX_STIOV3_CHANNEL];
	int				need_filesize[MAX_STIOV3_CHANNEL];
	volatile off_t			filesize[MAX_STIOV3_CHANNEL];
	int				need_reopen[MAX_STIOV3_CHANNEL];
	int				open_errno[MAX_STIOV3_CHANNEL];
	pthread_mutex_t			mutex[MAX_STIOV3_CHANNEL];
	pthread_cond_t			condi[MAX_STIOV3_CHANNEL];
	unsigned int			blockwant[MAX_STIOV3_CHANNEL];
	unsigned int			blockcnt[MAX_STIOV3_CHANNEL];
	unsigned int			filecnt[MAX_STIOV3_CHANNEL];
	int				bufok[MAX_STIOV3_CHANNEL][23];
	unsigned char			bbuf[MAX_STIOV3_CHANNEL][23][1425];
	int				brcvcnt[MAX_STIOV3_CHANNEL];
	char				stm_buffer[MAX_STIOV3_CHANNEL][2000];
	struct stio_v3_pdu_t		*stmpdu[MAX_STIOV3_CHANNEL];
	struct stiov3_fileio_t		*stmptr[MAX_STIOV3_CHANNEL];
	struct stiov3_fetch_request_t	*stmreq[MAX_STIOV3_CHANNEL];
	char				readbuffer[MAX_STIOV3_CHANNEL][32768];
	int				rdbufidx[MAX_STIOV3_CHANNEL];
	int				rdbuflen[MAX_STIOV3_CHANNEL];
	short				may_need_fix_filename[
							MAX_STIOV3_CHANNEL];

	volatile int			*compare_variable[MAX_STIOV3_CHANNEL];
	int				compare_operator[MAX_STIOV3_CHANNEL];
	int				compare_expect[MAX_STIOV3_CHANNEL];
	struct stio_v3_client_local_t	stio[MAX_STIOV3_CHANNEL];
#if ENABLE_MD5SUM_FOR_STREAMMING_FILE == 1
	struct md5_ctx			md5ctx[MAX_STIOV3_CHANNEL];
	short				have_md5sum[MAX_STIOV3_CHANNEL];
#endif
	struct stio_v3_pdu_t		*reopen_pdu[MAX_STIOV3_CHANNEL];
	int				reopen_pdu_len[MAX_STIOV3_CHANNEL];
	int				timeout_openfile;
	int				timeout_response;
	int				streamming_delay;
	int				retry_openfile;
	int				retry_response;
#if ENABLE_PREFER_VOD_SERVER == 1
	int				curr_svr_id;
	int				prefsvr_id;
	int				prefsvr_divider;
#endif
};

struct streamming_io_v3_client_t {
	struct streamming_io_v3_client_pd_t	pd;

	int		(*find_server)(struct streamming_io_v3_client_t *);
	void		(*dispose)(struct streamming_io_v3_client_t *self);
	int		(*start)(struct streamming_io_v3_client_t *self);
	int		(*stop)(struct streamming_io_v3_client_t *self);

	int		(*openfile)(struct streamming_io_v3_client_t *,
					const int channel,
					const char *fname);
	int		(*reopen)(struct streamming_io_v3_client_t *,
					const int channel);
	int		(*seek)(struct streamming_io_v3_client_t *,
					const int channel, const off_t pos);
	int		(*closefile)(struct streamming_io_v3_client_t *,
					const int channel);
	off_t		(*filesize)(struct streamming_io_v3_client_t *,
					const int channel);

	int		(*read)(struct streamming_io_v3_client_t *,
					const int channel,
					char *buffer, const int len);

	void		(*abort)(struct streamming_io_v3_client_t *);
	int		(*get_server)(struct streamming_io_v3_client_t *,
						char **server, int **port);
	int		(*get_server_id)(struct streamming_io_v3_client_t *);
	int		(*regist_stio)(struct streamming_io_v3_client_t *,
					const int channel,
					struct stio_engine_t *);

	int		(*cksum)(const void *ptr, const int len);
	int		(*set_delay)(struct streamming_io_v3_client_t *,
				const int which, const int usec);
#if ENABLE_PREFER_VOD_SERVER == 1
	int		(*set_prefer_server)(struct streamming_io_v3_client_t *,
				const int id, const int divider);
#endif
};


// ------------------------------------------------------------------------

struct streamming_io_v3_server_t *
		new_streamming_io_v3_server (const int maxclient,
						const int port, const int qos);
struct streamming_io_v3_client_t *
		new_streamming_io_v3_client (const int port, const int cliport);

#endif
