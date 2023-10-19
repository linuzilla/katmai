/*
 *	pqueue.h	(priority queue)
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __PRIORITY_QUEUE_H__
#define __PRIORITY_QUEUE_H__

#include <pthread.h>

struct priority_queue_t {
	int		*queue;
	int		count;	// Queue 中最大可存的數目
	int		front;
	int		rear;
	int		use_cnt;
	pthread_mutex_t	mutex;
	pthread_cond_t	condi;

	int		(*priority_callback)(const int *i, const int *j);

	int		(*dequeue)(struct priority_queue_t *self, const int f);
	int		(*enqueue)(struct priority_queue_t *self, const int i);
	int		(*is_empty)(struct priority_queue_t *self);
};

struct priority_queue_t	* new_priority_queue (const int n,
				int (*func)(const int *i, const int *j));

#endif
