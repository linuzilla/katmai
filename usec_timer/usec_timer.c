/*
 *	usec_timer.c
 *
 *	Copyright (c) 2002-3, Jiann-Ching Liu
 */

#include <sys/types.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/time.h>
#include "usec_timer.h"

#ifndef USE_PROCESSOR_TIME
static void tvsub (struct timeval *tdiff,
		const struct timeval *t1, const struct timeval *t0) {
	tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
	tdiff->tv_usec = t1->tv_usec - t0->tv_usec;

	if (tdiff->tv_usec < 0)
		tdiff->tv_sec--, tdiff->tv_usec += 1000000;
}
#endif

static void timer_reset (struct usec_timer_t *self) { self->pd.timer_idx = 0; }

static int timer_started (struct usec_timer_t *self) {
	int	rc = -1;

	if (self->pd.timer_idx < NUMBER_OF_USEC_TIMER) {
		rc = self->pd.timer_idx;
#ifndef USE_PROCESSOR_TIME
		gettimeofday (&self->pd.t_begin[self->pd.timer_idx++],
						(struct timezone *) 0);
#else
		self->pd.t_begin[self->pd.timer_idx++] = clock ();
#endif

	}

	return rc;
}

static double timer_ended (struct usec_timer_t *self) {
	// elapsed time

	if (self->pd.timer_idx > 0) {
#ifndef USE_PROCESSOR_TIME
		struct timeval	td;
		struct timeval	t_end;
		gettimeofday (&t_end, (struct timezone *) 0);
		tvsub (&td, &t_end, &self->pd.t_begin[--self->pd.timer_idx]);

	        return td.tv_sec + (td.tv_usec / 1000000.);
#else
		clock_t		t_end;

		t_end = clock ();

		return (double) (t_end -
				self->pd.t_begin[--self->pd.timer_idx]) /
				CLOCKS_PER_SEC;
#endif
	}

	return 0.0;
}

static double timer_elapsed (struct usec_timer_t *self) {
	// elapsed time

	if (self->pd.timer_idx > 0) {
#ifndef USE_PROCESSOR_TIME
		struct timeval	td;
		struct timeval	t_end;
		gettimeofday (&t_end, (struct timezone *) 0);
		tvsub (&td, &t_end, &self->pd.t_begin[self->pd.timer_idx - 1]);

	        return td.tv_sec + (td.tv_usec / 1000000.);
#else
		clock_t		t_end;

		t_end = clock ();

		return (double) (t_end -
				self->pd.t_begin[self->pd.timer_idx - 1]) /
				CLOCKS_PER_SEC;
#endif
	}

	return 0.0;
}

static double timer_snap (struct usec_timer_t *self, const int n) {
	// elapsed time

	if ((n >= 0) && (n < self->pd.timer_idx)) {
#ifndef USE_PROCESSOR_TIME
		struct timeval	td;
		struct timeval	t_end;

		gettimeofday (&t_end, (struct timezone *) 0);
		tvsub (&td, &t_end, &self->pd.t_begin[n]);

	        return td.tv_sec + (td.tv_usec / 1000000.);
#else
		clock_t		t_end;

		t_end = clock ();

		return (double) (t_end - self->pd.t_begin[n]) /
			CLOCKS_PER_SEC;
#endif
	}

	return 0.0;
}

static void timer_dispose (struct usec_timer_t *self) {
	free (self);
}

struct usec_timer_t * new_usec_timer (void) {
	struct usec_timer_t	*self;

	if ((self = malloc (sizeof (struct usec_timer_t))) != NULL) {
		self->reset	= timer_reset;
		self->start	= timer_started;
		self->ended	= timer_ended;
		self->elapsed	= timer_elapsed;
		self->snap	= timer_snap;
		self->dispose	= timer_dispose;

		self->reset (self);
	}

	return self;
}
