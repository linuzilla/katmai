/*
 *	ftpclient.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __FTP_CLIENT__H__
#define __FTP_CLIENT__H__

struct ftp_client_private_date_t {
	struct sockaddr_in	servaddr;
	struct sockaddr_in	myctladdr;
	struct sockaddr_in	data_addr;
	short			connected;
	short			have_peer;
	short			passive;
	short			verbose;
	short			debug;
	short			login_ok;
	short			mode;
	int			sockfd;
	int			datafd;
	int			code;
	FILE			*cin;
	FILE			*cout;
	char			pasv[64];
	char			reply_string[BUFSIZ];
	off_t			restart_point;
};

struct ftp_client_t {
	struct ftp_client_private_date_t	pd;

	int	(*setpeer)(struct ftp_client_t *,
				const char *host, const int port);
	int	(*login)(struct ftp_client_t *,
				const char *user, const char *pass);
	int	(*connect)(struct ftp_client_t *);
	int	(*close)(struct ftp_client_t *);
	int	(*passive)(struct ftp_client_t *, const int);
	int	(*verbose)(struct ftp_client_t *, const int);
	int	(*debug)(struct ftp_client_t *, const int);
	int	(*settype)(struct ftp_client_t *, const int);
	int	(*quit)(struct ftp_client_t *);
	int	(*dir)(struct ftp_client_t *, char *path);
	int	(*cwd)(struct ftp_client_t *, const char *path);
	int	(*cdup)(struct ftp_client_t *);
	int	(*get)(struct ftp_client_t *, char *rm, char *lc);
	int	(*reget)(struct ftp_client_t *, char *rm, char *lc);
	int	(*put)(struct ftp_client_t *, char *rm, char *lc);
	int	(*del)(struct ftp_client_t *, const char *path);
};

struct ftp_client_t	*new_ftpclient (void);

#endif
