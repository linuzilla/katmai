/*
 *	pdp_s6.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __PDP_S6_H__
#define __PDP_S6_H__

// #include "rs232.h"

struct rs232_t;

struct pdp_s6_private_data_t {
	int		fd;
	struct rs232_t	*rs232;
};

struct pdp_s6_t {
	struct pdp_s6_private_data_t	pd;
	int	(*open)(struct pdp_s6_t *, const char *str);
	void	(*close)(struct pdp_s6_t *);
};

struct pdp_s6_t	* new_PDP_S6 (void);

#endif
