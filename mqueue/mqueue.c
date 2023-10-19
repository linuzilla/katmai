/*
 *	mqueue.c	(multiple queue)
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

/*
 *	本程式碼假定 input data 不可能有問題, 不做 range check
 *	以增加速度 ...
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mqueue.h"

static int mq_enqueue (struct multiple_queue_t *self,
					const int n, const int q) {
	int		i;
	int		rc = 1;
	int		sig = 0;

	pthread_mutex_lock   (&self->mutex);

	i = self->location[n].q;

	if ((i < 0) || (i > q)) {
		self->location[n].q = q;

		if (self->use_cnt[q] < self->count - 1) {
			if (i > q) {
				fprintf (stderr, "Jumping from Q%d to Q%d\n",
						i, q);
				self->queue[i][self->location[n].i] = -1;
			}

			self->location[n].q = q;
			self->location[n].i = self->rear[q];

			self->queue[q][self->rear[q]] = n;
			self->rear[q] = (self->rear[q] + 1) % self->count;
			self->use_cnt[q]++;
			self->inqueue_cnt++;
			sig = 1;
		} else {
			fprintf (stderr, "ENQUEUE: full on queue %d\n", q);
		}
	}

	if (sig) pthread_cond_signal (&self->condi);

	pthread_mutex_unlock (&self->mutex);

	return rc;
}

static int mq_dequeue (struct multiple_queue_t *self, const int wait) {
	int	rc = -1;
	int	i;

	do {
		pthread_mutex_lock   (&self->mutex);

		while (wait && (self->inqueue_cnt == 0)) {
			pthread_cond_wait (&self->condi, &self->mutex);
		}

		if (self->inqueue_cnt == 0) {
			fprintf (stderr, "DEQUEUE: MQ Underflow\n");
		} else {
			for (i = 0; i < self->number_of_queue; i++) {
				if (self->use_cnt[i] > 0) {
					rc = self->queue[i][self->front[i]];
					self->front[i] = (self->front[i] + 1)
							% self->count;
					self->use_cnt[i]--;
					self->inqueue_cnt--;

					if (rc >= 0) {
						self->location[rc].q = -1;
					}

					break;
				}
			}
		}

		pthread_mutex_unlock (&self->mutex);
	} while (rc < 0);

	return rc;
}

static int mq_is_empty (struct multiple_queue_t *self) {
	int	rc = 0;

	pthread_mutex_lock   (&self->mutex);
	rc = (self->inqueue_cnt == 0) ? 1 : 0;
	pthread_mutex_unlock (&self->mutex);

	return rc;
}

struct multiple_queue_t * new_multiple_queue (const int n,
						const int nq, const int maxv) {
	struct multiple_queue_t	*self = NULL;
	int			i;

	if ((nq > MAX_NUM_OF_MQUEUE) || (nq < 1)) {
		fprintf (stderr, "Illegle setting\n");
		return NULL;
	}

	if ((self = malloc (sizeof (struct multiple_queue_t))) != NULL) {
		self->count		= n + 1;
		self->max_value		= maxv;
		self->number_of_queue	= nq;
		self->inqueue_cnt	= 0;
		self->location   	= calloc (maxv + 1,
						sizeof (struct mq_location_t));

		for (i = 0; i < nq; i++) {
			self->queue[i]   = calloc (n + 1, sizeof (int));
			self->front[i]   = 0;
			self->rear[i]    = 0;
			self->use_cnt[i] = 0;
			// pthread_mutex_init (&self->qmutex, NULL);
		}

		for (i = 0; i <= maxv; i++) {
			self->location[i].q = -1;
			self->location[i].i = -1;
		}

		self->enqueue	= mq_enqueue;
		self->dequeue	= mq_dequeue;
		self->is_empty	= mq_is_empty;

		pthread_mutex_init (&self->mutex, NULL);
		pthread_cond_init  (&self->condi, NULL);
	}

	return self;
}
