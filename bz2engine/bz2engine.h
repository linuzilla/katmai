/*
 *	bz2engine.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __BZ2_ENGINE_H__
#define __BZ2_ENGINE_H__

struct bz2_engine_pd_t {
};

struct bz2_engine_t {
	int	(*init)(struct bz2_engine_t *, const int bs, const wf);
};

struct bz2_engine_t *new_bz2_engine (void);

#endif
