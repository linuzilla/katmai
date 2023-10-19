/*
 *	buffer_cache.h
 *
 *	Copyright (c) 2002, written by Jiann-Ching Liu
 */

#ifndef __BUFFER_CACHE_H__
#define __BUFFER_CACHE_H__

#include <pthread.h>

#ifndef FMP_BUFFER_FAVIOR_SIZE
#define FMP_BUFFER_FAVIOR_SIZE	32768
#endif

#ifndef ALLOCATE_MEMORY_ON_DEMEND
#define ALLOCATE_MEMORY_ON_DEMEND	1
#endif

struct disk_buffer_t {
	char	buffer[FMP_BUFFER_FAVIOR_SIZE];
	int	rc;	// return code (length)
};

struct buffer_cache_t {
#if ALLOCATE_MEMORY_ON_DEMEND == 1
	int			have_memory;
	int			(*allocate_memory)(struct buffer_cache_t *self);
	int			(*free_memory)(struct buffer_cache_t *self);
#endif
	int			num;
	int			allocated;
	int			buf_free;
	int			buf_ready;
	int			front;
	int			rear;
	// int			current;
	struct disk_buffer_t	*dkbuf;
	pthread_mutex_t		mutex;
	pthread_cond_t		available_condi;
	pthread_cond_t		ready_condi;
	struct disk_buffer_t *	(*p_request)(struct buffer_cache_t *self);
	int			(*p_ready)(struct buffer_cache_t *self);
	struct disk_buffer_t *	(*c_request)(struct buffer_cache_t *self);
	int			(*c_release)(struct buffer_cache_t *self);
	struct disk_buffer_t *	(*c_tryreq)(struct buffer_cache_t *self);
	int			(*cleanup)(struct buffer_cache_t *self);
	int			(*flush)(struct buffer_cache_t *self);
	int			(*setbuf)(struct buffer_cache_t *self,
							const int n);
};

struct buffer_cache_t * new_buffer_cache (const int n);

#endif
