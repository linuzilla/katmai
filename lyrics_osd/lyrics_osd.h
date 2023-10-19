/*
 *	lyrics_osd.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __LYRICS_OSD_H__
#define __LYRICS_OSD_H__

#include <pthread.h>

struct koklib_t;
struct rmosdlib_t;
struct rmimage256_t;
struct koklib_bitmap_t;
struct xtimer_t;
struct stio_engine_t;

struct lyrics_osd_pd_t {
	pthread_t		thr;
	volatile short		have_thr;
	volatile short		terminate;
	volatile short		thrwait;
	pthread_cond_t		condi;
	pthread_mutex_t		mutex;
	struct koklib_t		*kok;
	struct koklib_bitmap_t	*kbmp;
	struct rmosdlib_t	*osd;
	struct rmimage256_t	*img;
	struct rmimage256_t	*banner;
	struct xtimer_t		*xtm;
	int			nk;	// Number of kbmp
	int			np;	// Number of lyrics para
	short			can_restart;
	short			need_join;
	short			have_fmp;
	short			use_default_rgb;
};

struct lyrics_osd_t {
	struct lyrics_osd_pd_t	pd;

	void	(*dispose)(struct lyrics_osd_t *);
	int	(*init)(struct lyrics_osd_t *, const char *fname);
	void	(*start)(struct lyrics_osd_t *);
	int	(*stop)(struct lyrics_osd_t *);
	int	(*is_running)(struct lyrics_osd_t *);
	void	(*pause)(struct lyrics_osd_t *);
	int	(*reinit)(struct lyrics_osd_t *);
	int	(*restart)(struct lyrics_osd_t *);
	int	(*reset)(struct lyrics_osd_t *);
	int	(*rgb)(struct lyrics_osd_t *, const int from);
	void	(*cleanup)(struct lyrics_osd_t *);
	void	(*abort)(struct lyrics_osd_t *);
	void	(*set_palette)(const int singer, const int channel,
				const int yy, const int uu, const int vv);
	struct stio_engine_t * (*stio_engine)(struct lyrics_osd_t *);

	struct rmimage256_t *	(*image)(struct lyrics_osd_t *);
};

struct lyrics_osd_t * new_lyrics_osd (const int have_fmp);

#endif
