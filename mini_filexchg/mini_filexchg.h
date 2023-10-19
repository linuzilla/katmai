/*
 *	mini_filexchg.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __MINI_FILEXCHG_H__
#define __MINI_FILEXCHG_H__

struct tcplib_t;
struct timeval;
struct timezone;

struct mini_filexchg_pd_t {
	struct tcplib_t		*tcp;
	char			*exportdir;
	char			*uploaddir;
	char			*request;
	char			*response;
};

struct mini_filexchg_t {
	struct mini_filexchg_pd_t	pd;

	int	(*start)(struct mini_filexchg_t *, const int port);
	void	(*stop)(struct mini_filexchg_t *);
	void	(*exports)(struct mini_filexchg_t *, const char *path);
	void	(*uploads)(struct mini_filexchg_t *, const char *path);
	int	(*ftpget)(struct mini_filexchg_t *self,
				const char *ip, const int port,
				const char *rmfile, const char *lcfile);
	int	(*ftpput)(struct mini_filexchg_t *self,
				const char *ip, const int port,
				const char *rmfile, const char *lcfile);
	int	(*gettimeofday)(struct mini_filexchg_t *self,
				const char *ip, const int port,
				struct timeval *tv, struct timezone *tz);
	int	(*rdate)(struct mini_filexchg_t *self,
				const char *ip, const int port);
	void	(*set_request)(struct mini_filexchg_t *, const char *request);
	void	(*set_response)(struct mini_filexchg_t *, const char *response);
	void	(*dispose)(struct mini_filexchg_t *);
};

struct mini_filexchg_t	* new_mini_filexchg (void);

#endif
