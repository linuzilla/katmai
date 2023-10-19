/*
 *	usec_timer.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __USEC_TIMER_H__
#define __USEC_TIMER_H__

#include <sys/types.h>
#include <time.h>

#define NUMBER_OF_USEC_TIMER		10

struct usec_timer_pd_t {
#ifndef USE_PROCESSOR_TIME
	struct timeval		t_begin[NUMBER_OF_USEC_TIMER];
#else
	clock_t			t_begin[NUMBER_OF_USEC_TIMER];
#endif
	int			timer_idx;
};

struct usec_timer_t {
	struct usec_timer_pd_t	pd;

	void	(*reset)(struct usec_timer_t *self);
	int	(*start)(struct usec_timer_t *self);
	double	(*ended)(struct usec_timer_t *self);
	double	(*elapsed)(struct usec_timer_t *self);
	double	(*snap)(struct usec_timer_t *self, const int n);
	void	(*dispose)(struct usec_timer_t *self);
};

struct usec_timer_t * new_usec_timer (void);

#endif
