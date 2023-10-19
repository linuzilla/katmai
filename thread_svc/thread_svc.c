/*
 *	thread_svc.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "thread_svc.h"

static int dummy_null_loop (void *ptr) {
	sleep (1);
	return 1;
}

static void * thread_svc_main (void *selfptr) {
	struct thread_svc_t	*self = selfptr;
	struct thread_svc_pd_t	*pd = &self->pd;
	int			(*handler)(void *) = pd->handler;
	void			*param = pd->param;

	pd->have_thr = pd->need_join = 1;

	// fprintf (stderr, "THREAD SVC main (%p)\n", self);

	if (pd->in_wait) {
		pthread_mutex_lock (&pd->mutex);
		while (pd->in_wait) pthread_cond_wait (&pd->condi, &pd->mutex);
		pthread_mutex_unlock (&pd->mutex);
	}

	while (! pd->terminate) {
		if (handler (param) != 0) break;
	}

	// fprintf (stderr, "THREAD SVC Terminated (%p)\n", self);

	pd->have_thr = 0;
	pthread_exit (NULL);
}

static int thrsvc_start (struct thread_svc_t *self) {
	struct thread_svc_pd_t	*pd = &self->pd;

	if (pd->have_thr) return 0;

	if (pd->need_join) {
                pd->need_join = 0;
                pthread_join (self->pd.thr, NULL);
	}

	pd->terminate = 0;
	pd->have_thr  = 1;
	if (pthread_create (&pd->thr, NULL, thread_svc_main, self) != 0) {
		pd->have_thr = 0;
	}

	return 1;
}

static int thrsvc_stop (struct thread_svc_t *self) {
	struct thread_svc_pd_t	*pd = &self->pd;

	// fprintf (stderr, "ThrSvc Stop\n");
	if (pd->have_thr) {
		pd->terminate = 1;
		self->conti (self);
	}

	if (pd->need_join) {
		pd->need_join = 0;
		// fprintf (stderr, "Waiting for join\n");
		pthread_join (self->pd.thr, NULL);

		return 1;
	}

	return 0;
}

static int thrsvc_kill (struct thread_svc_t *self, const int signo) {
	struct thread_svc_pd_t	*pd = &self->pd;

	if (pd->have_thr) {
		pthread_kill (self->pd.thr, signo);
		return 1;
	}
	return 0;
}

static void thrsvc_pause (struct thread_svc_t *self) { self->pd.in_wait = 1; }

static void thrsvc_conti (struct thread_svc_t *self) {
	struct thread_svc_pd_t	*pd = &self->pd;

	pd->in_wait = 0;

	if (pd->have_thr) pthread_cond_signal (&pd->condi);
}

static int thrsvc_is_running (struct thread_svc_t *self) {
	// fprintf (stderr, "[Thread for %p (%d)]\r", self, self->pd.have_thr);
	return self->pd.have_thr ? 1 : 0;
}

static void * thrsvc_set_handler (
			struct thread_svc_t *self, int (*ptr)(void*)) {
	struct thread_svc_pd_t	*pd = &self->pd;
	int (*rc)(void*) = pd->handler;

	if (ptr == NULL) {
		pd->handler	= dummy_null_loop;
	} else {
		pd->handler	= ptr;
	}

	return rc;
}

static void thrsvc_dispose (struct thread_svc_t *self) {
	self->stop (self);
	free (self);
}

struct thread_svc_t * new_thread_svc (int (*ptr)(void*), void *param) {
	struct thread_svc_t	*self;
	struct thread_svc_pd_t	*pd;

	if ((self = malloc (sizeof (struct thread_svc_t))) != NULL) {
		pd = &self->pd;

		pd->have_thr	= 0;
		pd->need_join	= 0;
		pd->terminate	= 0;
		pd->in_wait	= 0;
		pd->param	= param;
		pd->handler	= dummy_null_loop;

		pthread_mutex_init (&pd->mutex, NULL);
		pthread_cond_init  (&pd->condi, NULL);

		self->start		= thrsvc_start;
		self->stop		= thrsvc_stop;
		self->pause		= thrsvc_pause;
		self->conti		= thrsvc_conti;
		self->is_running	= thrsvc_is_running;
		self->set_handler	= thrsvc_set_handler;
		self->dispose		= thrsvc_dispose;
		self->kill		= thrsvc_kill;

		self->set_handler (self, ptr);
	}

	return self;
}
