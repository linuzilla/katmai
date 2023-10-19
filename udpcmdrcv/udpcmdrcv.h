/*
 *	udpcmdrcv.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __UDPCMDRCV_H__
#define __UDPCMDRCV_H__

#include <sys/types.h>
#include <pthread.h>

struct udplib_t;
struct genpdu_t;
struct gp_cmd_handler_t;
struct generic_pdu_callback_data_t;

struct udpcmdrcv_sendfile_pd_t {
	int		cmd;
	int		fd;
	int		id;
	int		udpslot;
	int		rmid;
	volatile int	tcpport;
	char		*file;
	short		inftp;
	short		transfer_completed;
	struct genpdu_t	*pdu;
	struct udplib_t	*udp;
	pthread_t	thr;
	pthread_mutex_t	mutex;
	off_t		filesize;
};

struct udpcmdrcv_sendfile_t {
	struct udpcmdrcv_sendfile_pd_t	pd;
	int	(*handler)(struct generic_pdu_callback_data_t *);
	int	(*send)(struct udpcmdrcv_sendfile_t *,
				const int n, const char *file);
	int	(*recv)(struct udpcmdrcv_sendfile_t *,
				const int n, const char *file);
};

struct udpcmdrcv_private_data_t {
	struct udplib_t			*udp;
	struct genpdu_t			*pdu;
	struct udpcmdrcv_sendfile_t	*ftp;
	volatile short			terminate;
	pthread_t			thr;
	short				have_udp;
	short				services;
	void				(*callback)(const int, const int);
};

struct udpcmdrcv_t {
	struct udpcmdrcv_private_data_t	pd;

	struct udplib_t *	(*udp)(struct udpcmdrcv_t *);
	struct genpdu_t *	(*pdu)(struct udpcmdrcv_t *);
	int			(*listen)(struct udpcmdrcv_t *,
					const int port,
					const char *intf,
					const int isbind);
	int			(*stop)(struct udpcmdrcv_t *);
	int			(*addcmd)(struct udpcmdrcv_t *,
					struct gp_cmd_handler_t *);
	int			(*bsend)(struct udpcmdrcv_t *,
					const int port,
					const int cmd);
	int			(*bcast)(struct udpcmdrcv_t *,
					const int port,
					const int cmd,
					void *data,
					int len);
	int			(*reply)(struct udpcmdrcv_t *,
					const int cmd,
					void *data,
					int len);
	int			(*regist_ftp)(struct udpcmdrcv_t *,
					const int cmd);
	int			(*sendfile)(struct udpcmdrcv_t *,
					const int n,
					const char *file);
	int			(*recvfile)(struct udpcmdrcv_t *,
					const int n,
					const char *file);
	int			(*regist_addr)(struct udpcmdrcv_t *,
					const char *ip,
					const int port);
	void			(*set_callback)(struct udpcmdrcv_t *,
					void (*cbk)(const int, const int));
	void			(*set_flag_callback)(struct udpcmdrcv_t *,
					unsigned int (*cbk)(struct genpdu_t *));
};

struct udpcmdrcv_t * new_udpcmdrcv (const int nudp, const int ncmd);

struct udpcmdrcv_sendfile_t * new_udpcmdrcv_sendfile (
			struct udpcmdrcv_t *self,
			const int cmd);

#endif
