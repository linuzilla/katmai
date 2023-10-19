/*
 *	cron.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __CRON_H__
#define __CRON_H__

#include <pthread.h>
#include "cronjob.h"

struct cron_private_data_t {
	struct cronjob_t	*cj;
	short			on_the_run;
	int			timeslot;
	volatile int		terminate;
	pthread_t		cron_thread;
	void			(*callback)(const char *cmd);
};

struct cron_t {
	struct cron_private_data_t	pd;
	int	(*load_crontable)(struct cron_t *self, const char *file);
	int	(*set_timeslot)(struct cron_t *self, const int min);
	void	(*list)(struct cron_t *self);
	int	(*run)(struct cron_t *self);
	int	(*stop)(struct cron_t *self);
	int	(*add)(struct cron_t *self, const char *str);
	void *	(*set_callback)(struct cron_t *self, void (*f)(const char *));
};

struct cron_t *	new_cron (void);

#endif
