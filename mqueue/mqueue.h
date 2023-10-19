/*
 *	mqueue.h	(Multiple Queue)
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __MULTIPLE_QUEUE_H__
#define __MULTIPLE_QUEUE_H__

#include <pthread.h>

#define MAX_NUM_OF_MQUEUE	4

struct mq_location_t {
	int		q;
	int		i;
};

struct multiple_queue_t {
	int			*queue[MAX_NUM_OF_MQUEUE];
	struct mq_location_t	*location;
	int			max_value;
	int			count;	// Queue 中最大可存的數目
	int			number_of_queue;
	int			front[MAX_NUM_OF_MQUEUE];
	int			rear[MAX_NUM_OF_MQUEUE];
	int			use_cnt[MAX_NUM_OF_MQUEUE];
	int			inqueue_cnt;
	//pthread_mutex_t	qmutex[MAX_NUM_OF_MQUEUE];
	pthread_mutex_t		mutex;
	pthread_cond_t		condi;

	int		(*dequeue)(struct multiple_queue_t *self, const int f);
	int		(*enqueue)(struct multiple_queue_t *self,
					const int i, const int q);
	int		(*is_empty)(struct multiple_queue_t *self);
};

struct multiple_queue_t	* new_multiple_queue (const int n,
					const int nq, const int maxv);

#endif
