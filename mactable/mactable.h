/*
 *	mactable.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __MAC_TABLE_H__
#define __MAC_TABLE_H__

struct ipmac_table_t;

struct mac_table_private_data_t {
	int			num;
	int			usecnt;
	short			have_changed;
	struct ipmac_table_t	*ipmac;
};

struct mac_table_t {
	struct mac_table_private_data_t	pd;

	int	(*load_table)(struct mac_table_t *, const char *file);
	int	(*save_table)(struct mac_table_t *, const char *file);
	void	(*clear_all)(struct mac_table_t *);
	int	(*is_verify)(struct mac_table_t *, const int n);
	int	(*verify)(struct mac_table_t *, const int n);
	int	(*wakeup_unverify)(struct mac_table_t *);
	void	(*check_mac)(struct mac_table_t *, const int n);
	int	(*unset_verify_all)(struct mac_table_t *);
	int	(*verify_all)(struct mac_table_t *);
	int	(*verified_cnt)(struct mac_table_t *);
	int	(*add)(struct mac_table_t *, const char *ip,
				const unsigned char *mac);
	int	(*wakeup)(struct mac_table_t *, const int n);
	int	(*wakeup_all)(struct mac_table_t *);
	int	(*is_changed)(struct mac_table_t *);
	void	(*clear_change)(struct mac_table_t *);
	void	(*external_call)(struct mac_table_t *, void *caller,
			void (*cbk)(void *caller, const int n, const char *ip,
					const char *mac, int *idx));
};

struct mac_table_t *	new_mac_table (const int num);

#endif
