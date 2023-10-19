/*
 *	simconf.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __SIMCONF_H__
#define __SIMCONF_H__

struct simconf_data_t;

struct simconf_private_data_t {
	struct simconf_data_t	*data;
};

struct simconf_t {
	struct simconf_private_data_t	pd;

	int	(*load)(struct simconf_t *, const char *file);
	int	(*add)(struct simconf_t *, const char *var, const char *val);
	void	(*dispose)(struct simconf_t *);
	struct simconf_data_t *
		(*search_var)(struct simconf_t *, const char *var);
	int	(*getint)(struct simconf_t *, const char *var);
	char *	(*getstr)(struct simconf_t *, const char *var);
	char *	(*getip)(struct simconf_t *, const char *var);
	void	(*clean)(struct simconf_t *);
	int	(*variable_type)(struct simconf_t *, const char *var);
};

struct simconf_t * new_simconf (void);

#endif
