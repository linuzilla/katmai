/*
 *	stream_mp3.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __STREAM_MP3_H__
#define __STREAM_MP3_H__

#include <pthread.h>

struct mp3lib_t;
struct stio_engine_t;

struct stream_mp3_pd_t {
	struct mp3lib_t		*mp3[2];
	struct stio_engine_t	*stio[2];
	short			ready[2];
	short			need_join;
	pthread_mutex_t		mutex;
	pthread_cond_t		condi;
	volatile short		terminate;
	volatile short		thrwait;
	volatile short		channel_change;
	pthread_t		thr;
	short			have_thr;
	short			can_restart;
	int			selected;
	volatile short		reset_mp3;
};

struct stream_mp3_t {
	struct stream_mp3_pd_t	pd;

	void	(*dispose)(struct stream_mp3_t *);
	int	(*init)(struct stream_mp3_t *, const int, const char *fname);
	void	(*start)(struct stream_mp3_t *);
	void	(*reset)(struct stream_mp3_t *);
	int	(*stop)(struct stream_mp3_t *);
	int	(*is_running)(struct stream_mp3_t *);
	void	(*pause)(struct stream_mp3_t *);
	int	(*run)(struct stream_mp3_t *);
	void	(*abort)(struct stream_mp3_t *);
	int	(*select)(struct stream_mp3_t *, const int);
	struct stio_engine_t * (*stio_engine)(struct stream_mp3_t *self,
			const int channel);
};

struct stream_mp3_t * new_stream_mp3 (void);

#endif
