/*
 *	pqueue.c	(priority queue)
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "pqueue.h"

static int pq_enqueue (struct priority_queue_t *self, const int n) {
	int	i, found;
	int	rc = 1;
	int	sig = 0;

	pthread_mutex_lock   (&self->mutex);

	if (self->priority_callback == NULL) {
		if (self->front == self->rear) {
			self->queue[self->rear] = n;
			self->rear = (self->rear + 1) % self->count;
			self->use_cnt++;
			sig = 1;
		} else {
			i     = self->front;
			found = 0;

			do {
				if (self->queue[i] == n) {
					found = 1;
					break;
				}
				i = (i + 1) % self->count;
			} while (i != self->rear);

			if (! found) {
				if (self->use_cnt < self->count - 1) {
					self->queue[self->rear] = n;
					self->rear = (self->rear + 1) %
								self->count;
					self->use_cnt++;
					sig = 1;
				} else {
					fprintf (stderr, "ENQUEUE: full\n");
					rc = 0;
				}
			} else {
				fprintf (stderr,
					"ENQUEUE: already exists on %d!!\n", n);
			}
		}
	} else {
		if (self->use_cnt == 0) {
			self->queue[0] = n;
			self->use_cnt++;
			sig = 1;
		} else if (bsearch (&n, self->queue, self->use_cnt,
					sizeof (int),
					(int (*)(const void *, const void *))
					self->priority_callback) != NULL) {
			fprintf (stderr, "ENQUEUE: (p) duplicate !!\n");
		} else if (self->use_cnt < self->count - 1) {
			self->queue[++self->use_cnt] = n;
			qsort (self->queue, self->use_cnt, sizeof (int),
					(int (*)(const void *, const void *))
					self->priority_callback);
			sig = 1;
		} else {
			fprintf (stderr, "ENQUEUE: (p) full\n");
			rc = 0;
		}
	}

	if (sig) pthread_cond_signal (&self->condi);

	pthread_mutex_unlock (&self->mutex);

	return rc;
}

static int pq_dequeue (struct priority_queue_t *self, const int wait) {
	int	rc = -1;

	pthread_mutex_lock   (&self->mutex);

	while (wait && (self->use_cnt == 0)) {
		pthread_cond_wait (&self->condi, &self->mutex);
	}

	if (self->use_cnt == 0) {
		fprintf (stderr, "DEQUEUE: PQ Underflow\n");
	} else if (self->priority_callback == NULL) {
		rc = self->queue[self->front];
		self->front = (self->front + 1) % self->count;
		self->use_cnt--;
	} else {
		rc = self->queue[--self->use_cnt];
	}

	pthread_mutex_unlock (&self->mutex);

	return rc;
}

static int pq_is_empty (struct priority_queue_t *self) {
	int	rc = 0;

	pthread_mutex_lock   (&self->mutex);
	rc = (self->use_cnt == 0) ? 1 : 0;
	pthread_mutex_unlock (&self->mutex);

	return rc;
}

struct priority_queue_t * new_priority_queue (
			const int n, int (*func)(const int *i, const int *j)) {
	struct priority_queue_t	*self = NULL;

	if ((self = malloc (sizeof (struct priority_queue_t))) != NULL) {
		self->count   = n + 1;
		self->queue   = calloc (n + 1, sizeof (int));
		self->front   = 0;
		self->rear    = 0;
		self->use_cnt = 0;
		self->priority_callback	= func;

		self->enqueue	= pq_enqueue;
		self->dequeue	= pq_dequeue;
		self->is_empty	= pq_is_empty;

		pthread_mutex_init (&self->mutex, NULL);
		pthread_cond_init  (&self->condi, NULL);
	}

	return self;
}
