/*
 *	buffer_cache.c
 *
 *	Copyright (c) 2002, written by Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "buffer_cache.h"

static int bc_num_buf_free (struct buffer_cache_t *self) {
	return self->buf_free;
}

static struct disk_buffer_t *
			bc_producer_request (struct buffer_cache_t *self) {
	struct disk_buffer_t	*ptr;
	// request buffer
	pthread_mutex_lock   (&self->mutex);

	while (self->buf_free == 0) {
		pthread_cond_wait (&self->available_condi, &self->mutex);
	}

	--self->buf_free;

	// self->current = self->front;
	// self->dkbuf[self->front].ready = 0;
	ptr = &self->dkbuf[self->front];

	self->front = (self->front + 1) % self->num;
	pthread_mutex_unlock (&self->mutex);

	return ptr;
}

static struct disk_buffer_t *
			bc_producer_try_request (struct buffer_cache_t *self) {
	struct disk_buffer_t	*ptr;
	// request buffer
	pthread_mutex_lock   (&self->mutex);

	if (self->buf_free == 0) {
		ptr = NULL;
	} else {
		--self->buf_free;

		// self->current = self->front;
		// self->dkbuf[self->front].ready = 0;
		ptr = &self->dkbuf[self->front];

		self->front = (self->front + 1) % self->num;
	}

	pthread_mutex_unlock (&self->mutex);

	return ptr;
}

static int bc_producer_ready (struct buffer_cache_t *self) {
	pthread_mutex_lock (&self->mutex);
	// self->dkbuf[self->current].ready = 1;
	self->buf_ready++;
	pthread_cond_signal (&self->ready_condi);
	pthread_mutex_unlock (&self->mutex);
	return 0;
}

static struct disk_buffer_t *
			bc_consumer_request (struct buffer_cache_t *self) {
	struct disk_buffer_t	*ptr;

	pthread_mutex_lock (&self->mutex);

	while (self->buf_ready == 0) {
		pthread_cond_wait (&self->ready_condi, &self->mutex);
	}

	ptr = &self->dkbuf[self->rear];
	self->rear = (self->rear + 1) % self->num;

	--self->buf_ready;

	pthread_mutex_unlock (&self->mutex);
	return ptr;
}

struct disk_buffer_t *  bc_consumer_try_request (struct buffer_cache_t *self) {
	struct disk_buffer_t	*ptr;

	pthread_mutex_lock (&self->mutex);

	if (self->buf_ready != 0) {
		ptr = &self->dkbuf[self->rear];
		self->rear = (self->rear + 1) % self->num;
		--self->buf_ready;
	} else {
		ptr = NULL;
	}

	pthread_mutex_unlock (&self->mutex);

	return ptr;
}

static int bc_consumer_release (struct buffer_cache_t *self) {
	pthread_mutex_lock (&self->mutex);

	++self->buf_free;
	pthread_cond_signal (&self->available_condi);

	pthread_mutex_unlock (&self->mutex);
	return 0;
}

static int bc_flush (struct buffer_cache_t *self) {
	while (self->c_tryreq (self) != NULL) self->c_release (self);

	return 1;
}

static int bc_cleanup (struct buffer_cache_t *self) {
	pthread_mutex_lock (&self->mutex);
	self->buf_free  = self->num;
	self->buf_ready = 0;
	self->front     = self->rear = 0;

	pthread_mutex_unlock (&self->mutex);

	return 1;
}

static int bc_setbuffer (struct buffer_cache_t *self, const int nbuf) {
	int	n = nbuf;

	if ((n <= 0) || (n > self->allocated)) n = self->allocated;

	// A little bit danger here !!!!!
	if ((self->num != n) && (n > 3) && (n <= self->allocated)) {
		self->flush (self);
		self->num = n;
		self->cleanup (self);
		// fprintf (stderr, "Set buffer cache to %d buffer\n", n);
	} else {
		self->flush (self);
	}

	return self->num;
}

static void bc_dispose (struct buffer_cache_t *self) {
	int		i;

	if (self->dkbuf != NULL) {
		for (i = 0; i < self->num; i++) {
			if (self->dkbuf[i].buffer != NULL) {
				free (self->dkbuf[i].buffer);
			}
		}

		free (self->dkbuf);
	}
	free (self);
}

static int bc_bufsize (struct buffer_cache_t *self) {
	return self->bsize;
}

struct disk_buffer_t * bc_top_try_request (struct buffer_cache_t *self) {
	struct disk_buffer_t	*ptr;

	pthread_mutex_lock (&self->mutex);

	if (self->top_used < self->buf_ready) {
		ptr = &self->dkbuf[self->top];
		self->top = (self->top + 1) % self->num;
		++self->top_used;
	} else {
		ptr = NULL;
	}

	pthread_mutex_unlock (&self->mutex);

	return ptr;
}

static void bc_top_reset (struct buffer_cache_t *self) {
	self->top      = self->rear;
	self->top_used = 0;
}

struct buffer_cache_t * new_buffer_cache (const int n, const int bsize) {
	struct buffer_cache_t	*ptr;
	int			allocate;
	int			total_allocate;
	int			i;


	allocate = sizeof (struct buffer_cache_t);

	if ((ptr = malloc (allocate)) == NULL) {
		perror ("malloc");
		return NULL;
	}

	// -----------------------------------------------------------

	ptr->p_request = bc_producer_request;	// producer request buffer
	ptr->p_ready   = bc_producer_ready;	// producer buffer ready
	ptr->p_tryreq  = bc_producer_try_request;

	ptr->c_request = bc_consumer_request;	// consumer request
	ptr->c_release = bc_consumer_release;	// consumer release
	ptr->c_tryreq  = bc_consumer_try_request; // consumer try request

	ptr->t_tryreq  = bc_top_try_request;
	ptr->t_reset   = bc_top_reset;

	ptr->bufsize   = bc_bufsize;
	ptr->cleanup   = bc_cleanup;
	ptr->flush     = bc_flush;
	ptr->setbuf    = bc_setbuffer;
	ptr->dispose   = bc_dispose;

	ptr->num_buf_free  = bc_num_buf_free;

	// -----------------------------------------------------------

	total_allocate = allocate;

	allocate = sizeof (struct disk_buffer_t);

	ptr->num = 0;

	if ((ptr->dkbuf = calloc (n, allocate)) == NULL) {
		perror ("malloc");
		ptr->dispose (ptr);
		return NULL;
	}

	ptr->num = ptr->allocated = n;
	total_allocate = allocate * n;

	for (i = 0; i < n; i++) ptr->dkbuf[i].buffer = NULL;

	for (i = 0; i < n; i++) {
		if ((ptr->dkbuf[i].buffer = malloc (bsize)) == NULL) {
			ptr->dispose (ptr);
			return NULL;
		}
		total_allocate += bsize;
	}

	fprintf (stderr, "[*] Pre-Cache buffer: %.2f MBytes Allocated [%d]\n",
		(double) total_allocate / 1024. / 1024.,
		total_allocate);

	// --------------------------------------------------------------

	pthread_cond_init  (&ptr->available_condi, NULL);
	pthread_cond_init  (&ptr->ready_condi,     NULL);
	pthread_mutex_init (&ptr->mutex, NULL);


	// --------------------------------------------------------------

	ptr->cleanup (ptr);

	return ptr;
}
