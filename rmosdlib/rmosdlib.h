/*
 *	rmosdlib.h
 *
 *	Copyright (c) 2002-2003, Jiann-Ching Liu
 */

#ifndef __RMOSDLIB_H__
#define __RMOSDLIB_H__

struct rmosdlib_private_data_t;

struct rmosdlib_t {
	struct rmosdlib_private_data_t	*pd;

	void		(*dispose)(struct rmosdlib_t *);
	int		(*open)(struct rmosdlib_t *);
	int		(*close)(struct rmosdlib_t *);
	void		(*osd_on)(struct rmosdlib_t *);
	void		(*osd_off)(struct rmosdlib_t *);
	void		(*displayBitmap)(struct rmosdlib_t *,
						void *, const int);
};

struct rmosdlib_t *	new_rmosdlib (void);

#endif
