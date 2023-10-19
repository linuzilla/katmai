/*
 *	timeutil.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __TIMEUTIL_H__
#define __TIMEUTIL_H__

struct timeutil_t {
	int	(*str2hm)(const char *str, int *hour, int *min);
	int	(*str2hms)(const char *str, int *hour, int *min, int *sec);
};

struct timeutil_t * new_timeutil_t (void);

#endif
