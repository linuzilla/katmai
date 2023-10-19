/*
 *	cronjob.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __CRONJOB_H__
#define __CRONJOB_H__

#include <sys/types.h>
#include <time.h>

struct cronjob_table_data_t {
	u_int64_t			min;
	u_int32_t			hour;
	u_int32_t			day;
	u_int32_t			mon;
	u_int32_t			wday;
	char				*cmd;
	struct cronjob_table_data_t	*next;
};

struct cronjob_private_data_t {
	struct cronjob_table_data_t 	*tbl;
	struct cronjob_table_data_t	cktbl;
	struct cronjob_table_data_t	*findptr;
};

struct cronjob_t {
	struct cronjob_private_data_t	pd;

	int	(*add)(struct cronjob_t *, const char *str);
	void	(*list)(struct cronjob_t *);
	char*	(*check_first)(struct cronjob_t *, struct tm *);
	char*	(*check_next)(struct cronjob_t *);
};

struct cronjob_t * new_cronjob (void);

#endif
