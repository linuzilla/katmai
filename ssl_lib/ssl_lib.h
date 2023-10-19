/*
 *	ssl_lib.h
 *
 *	Copyright (c) 2004, Jiann-Ching Liu
 */

#ifndef __SSL_LIB_H__
#define __SSL_LIB_H__

struct SSL_lib_pd_t;

struct SSL_lib_t {
	struct SSL_lib_pd_t	*pd;

	int		(*listen)(struct SSL_lib_t *, const int port);
	int		(*connect)(struct SSL_lib_t *,
					const char *host, const int port);
	int		(*accept)(struct SSL_lib_t *);
	int		(*disconnect)(struct SSL_lib_t *);
	int		(*read)(struct SSL_lib_t *, void *buf, size_t len);
	int		(*write)(struct SSL_lib_t *, void *buf, size_t len);
	int		(*client_read)(struct SSL_lib_t *,
					void *buf, size_t len);
	int		(*client_write)(struct SSL_lib_t *,
					void *buf, size_t len);
	int		(*server_read)(struct SSL_lib_t *,
					void *buf, size_t len);
	int		(*server_write)(struct SSL_lib_t *,
					void *buf, size_t len);
	int		(*set_CAdir)(struct SSL_lib_t *, const char *path);
	int		(*set_CAfile)(struct SSL_lib_t *, const char *path);
	int		(*set_CERTfile)(struct SSL_lib_t *, const char *path);
	int		(*server_fd)(struct SSL_lib_t *);
	int		(*client_fd)(struct SSL_lib_t *);
	void		(*dispose)(struct SSL_lib_t *);
	const char	*(*errstr)(struct SSL_lib_t *);
	int		(*clear_server)(struct SSL_lib_t *);
	int		(*clear_client)(struct SSL_lib_t *);
	int		(*shutdown_server)(struct SSL_lib_t *);
	int		(*shutdown_client)(struct SSL_lib_t *);
	void		(*free_server)(struct SSL_lib_t *);
	void		(*free_client)(struct SSL_lib_t *);
	int		(*verbose)(struct SSL_lib_t *, const int l);
};

struct SSL_lib_t	*new_SSL_lib (void);

#endif
