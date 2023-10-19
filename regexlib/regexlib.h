/*
 *	regexlib.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __REGEXLIB_H__
#define __REGEXLIB_H__

#include <regex.h>

struct regexlib_private_data {
	char		*regex;
	int		have_preg;
	regex_t		preg;
	regmatch_t      *pmatch;
	char		**mstr;
	int		mstrlen;
};

struct regexlib_t {
	struct regexlib_private_data	pd;

	int	(*regex)(struct regexlib_t *self, const char *str);
	int	(*exec)(struct regexlib_t *self, const char *str);
	char*	(*var)(struct regexlib_t *self, const int n);
	void	(*cleanup)(struct regexlib_t *);
	void	(*dispose)(struct regexlib_t *);
};

struct regexlib_t * new_regex_lib (void);

#endif
