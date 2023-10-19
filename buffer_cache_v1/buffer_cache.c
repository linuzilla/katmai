/*
 *	buffer_cache.c
 *
 *	Copyright (c) 2002, written by Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "buffer_cache.h"


#if ALLOCATE_MEMORY_ON_DEMEND == 1
static int bc_allocate_memory (struct buffer_cache_t *self) {
	if (self->dkbuf != NULL) free (self->dkbuf);

	if ((self->dkbuf = calloc (self->allocated,
				sizeof (struct disk_buffer_t))) == NULL) {
		perror ("malloc");
		return 0;
	} else {
		self->have_memory = 0;
	}
	return 1;
}

static int bc_free_memory (struct buffer_cache_t *self) {
	self->flush (self);
	self->cleanup (self);

	if (self->dkbuf != NULL) free (self->dkbuf);
	self->have_memory = 0;
	self->dkbuf = NULL;

	return 1;
}
#endif

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

static int bc_setbuffer (struct buffer_cache_t *self, const int n) {
	// A little bit danger here !!!!!
	if ((self->num != n) && (n > 3) && (n <= self->allocated)) {
		self->flush (self);
		self->num = n;
		self->cleanup (self);
	} else {
		self->flush (self);
	}

	return self->num;
}

struct buffer_cache_t * new_buffer_cache (const int n) {
	struct buffer_cache_t	*ptr;
	int			allocate;
	int			total_allocate;

	allocate = sizeof (struct buffer_cache_t);

	if ((ptr = malloc (allocate)) == NULL) {
		perror ("malloc");
		return NULL;
	}

	total_allocate = allocate;

	allocate = sizeof (struct disk_buffer_t);

	if ((ptr->dkbuf = calloc (n, allocate)) == NULL) {
		perror ("malloc");
		return NULL;
	}

	total_allocate = allocate * n;

	fprintf (stderr, "[*] Pre-Cache buffer: %.2f MBytes Allocated\n",
		(double) total_allocate / 1024. / 1024.);

	ptr->num = ptr->allocated = n;


#if ALLOCATE_MEMORY_ON_DEMEND == 1
	ptr->have_memory = 0;
	free (ptr->dkbuf);
	ptr->dkbuf = NULL;
#endif

	// --------------------------------------------------------------

	pthread_cond_init  (&ptr->available_condi, NULL);
	pthread_cond_init  (&ptr->ready_condi,     NULL);
	pthread_mutex_init (&ptr->mutex, NULL);

#if ALLOCATE_MEMORY_ON_DEMEND == 1
	ptr->allocate_memory = bc_allocate_memory;
	ptr->free_memory     = bc_free_memory;
#endif

	ptr->p_request = bc_producer_request;	// producer request buffer
	ptr->p_ready   = bc_producer_ready;	// producer buffer ready

	ptr->c_request = bc_consumer_request;	// consumer request
	ptr->c_release = bc_consumer_release;	// consumer release
	ptr->c_tryreq  = bc_consumer_try_request; // consumer try request

	ptr->cleanup   = bc_cleanup;
	ptr->flush     = bc_flush;
	ptr->setbuf    = bc_setbuffer;

	// --------------------------------------------------------------

	ptr->cleanup (ptr);

	return ptr;
}
