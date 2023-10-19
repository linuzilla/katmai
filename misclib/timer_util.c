/*
 *	timer_utils.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
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

#define NUMBER_OF_TIMER	10

#ifndef USE_PROCESSOR_TIME
static struct timeval	t_begin[NUMBER_OF_TIMER];
#else
static clock_t		t_begin[NUMBER_OF_TIMER];
#endif
static int		timer_idx = 0;

#ifndef USE_PROCESSOR_TIME
static void tvsub (struct timeval *tdiff,
		const struct timeval *t1, const struct timeval *t0) {
	tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
	tdiff->tv_usec = t1->tv_usec - t0->tv_usec;

	if (tdiff->tv_usec < 0)
		tdiff->tv_sec--, tdiff->tv_usec += 1000000;
}
#endif

void timer_clear (void) { timer_idx = 0; }

int timer_started (void) {
	int	rc = -1;

	if (timer_idx < NUMBER_OF_TIMER) {
		rc = timer_idx;
#ifndef USE_PROCESSOR_TIME
		gettimeofday (&t_begin[timer_idx++], (struct timezone *) 0);
#else
		t_begin[timer_idx++] = clock ();
#endif

	}

	return rc;
}

double timer_ended (void) {
	// elapsed time

	if (timer_idx > 0) {
#ifndef USE_PROCESSOR_TIME
		struct timeval	td;
		struct timeval	t_end;
		gettimeofday (&t_end, (struct timezone *) 0);
		tvsub (&td, &t_end, &t_begin[--timer_idx]);

	        return td.tv_sec + (td.tv_usec / 1000000.);
#else
		clock_t		t_end;

		t_end = clock ();

		return (double) (t_end - t_begin[--timer_idx]) /
			CLOCKS_PER_SEC;
#endif
	}

	return 0.0;
}

double timer_snap (const int n) {
	// elapsed time

	if ((n >= 0) && (n < timer_idx)) {
#ifndef USE_PROCESSOR_TIME
		struct timeval	td;
		struct timeval	t_end;
		gettimeofday (&t_end, (struct timezone *) 0);
		tvsub (&td, &t_end, &t_begin[n]);

	        return td.tv_sec + (td.tv_usec / 1000000.);
#else
		clock_t		t_end;

		t_end = clock ();

		return (double) (t_end - t_begin[n]) /
			CLOCKS_PER_SEC;
#endif
	}

	return 0.0;
}
