/*
 *	playlist.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __PLAYLIST_H__
#define __PLAYLIST_H__

#include <time.h>

struct strlist_t;

struct playlist_private_data_t {
	time_t			start;
	time_t			stop;
	int			cnt;
	struct strlist_t *	list;
	short			loop_mode;
	struct strlist_t *	pointer;
	char			*plfile;
	char			*plrecfile;
};

struct playlist_t {
	struct playlist_private_data_t	pd;
	int	(*set_starttime)(struct playlist_t *, const char *);
	int	(*set_stoptime)(struct playlist_t *, const char *);
	int	(*load_playlist)(struct playlist_t *, const char *file);
	int	(*auto_playlist)(struct playlist_t *, const int);
	char *  (*playlist_file)(struct playlist_t *, const int);
	char *  (*playrecord_file)(struct playlist_t *, const int);
	time_t	(*start_time)(struct playlist_t *);
	time_t	(*stop_time)(struct playlist_t *);
	void	(*clean)(struct playlist_t *);
	int	(*add)(struct playlist_t *, const char *fname);
	int	(*looping)(struct playlist_t *, const int v);
	void	(*first)(struct playlist_t *);
	char*	(*next)(struct playlist_t *);
	char*	(*current)(struct playlist_t *);
	void	(*dispose)(struct playlist_t *);
};

struct playlist_t *	new_playlist (void);

#endif
