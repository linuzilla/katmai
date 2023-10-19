/*
 *	xlist.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __XLIST_H__
#define __XLIST_H__

#include <pthread.h>

struct xlist_list_t;

struct xlist_pd_t {
	int			count;
	int			ssize;
	struct xlist_list_t	*list;
	struct xlist_list_t	*head;
	struct xlist_list_t	*tail;
	struct xlist_list_t	*freelist;
	int			usecnt;
	int			front;
	int			rear;
	pthread_mutex_t		mutex;
};

/*
 * 	push	    [======] <--
 * 	pop	    [======] -->
 * 	shift	<-- [======]
 * 	unshift	--> [======]
 */

struct xlist_t {
	struct xlist_pd_t	pd;

	void		(*dispose)(struct xlist_t *);
	void		(*cleanup)(struct xlist_t *);
	int		(*count)(struct xlist_t *);
	int		(*freebuf)(struct xlist_t *);
	int		(*usedbuf)(struct xlist_t *);
	int		(*push)(struct xlist_t *, void *);
	int		(*unshift)(struct xlist_t *, void *);
	int		(*pop)(struct xlist_t *, void *);
	int		(*shift)(struct xlist_t *, void *);
	int		(*front)(struct xlist_t *, void *);
	int		(*rear)(struct xlist_t *, void *);
};

struct xlist_t * new_xlist (const int n, const int ssize);

#endif
