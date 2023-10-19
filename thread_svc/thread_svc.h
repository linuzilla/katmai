/*
 *	thread_svc.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __THREAD_SVC__H__
#define __THREAD_SVC__H__

#include <pthread.h>

struct thread_svc_pd_t {
	short		have_thr;
	short		need_join;
	volatile short	terminate;
	volatile short	in_wait;
	pthread_mutex_t	mutex;
	pthread_cond_t	condi;
	pthread_t	thr;
	void		*param;
	int		(*handler)(void *);
};

struct thread_svc_t {
	struct thread_svc_pd_t	pd;

	int	(*start)(struct thread_svc_t *self);
	int	(*stop)(struct thread_svc_t *self);
	int	(*kill)(struct thread_svc_t *self, const int signo);
	void	(*conti)(struct thread_svc_t *self);
	void	(*pause)(struct thread_svc_t *self);
	void	(*dispose)(struct thread_svc_t *self);
	int	(*is_running)(struct thread_svc_t *self);
	void*	(* set_handler)(struct thread_svc_t *self, int (*ptr)(void*));
};

struct thread_svc_t * new_thread_svc (int (*ptr)(void*), void *param);

#endif
