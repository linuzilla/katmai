/*
 *	vod_table.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __VODSVR_TABLE_H__
#define __VODSVR_TABLE_H__

struct vodsvr_t {
	int		(*add)(in_addr_t addr);
	int		(*regist_code)(const int n);
	int		(*unset_registcode)(const int n);
	int		(*udp_entry)(const int n);
	int		(*set_registcode)(in_addr_t addr, const int code,
					const int udp_entry);
};

struct vodsvr_t	* new_vodsvr (const int num);

#endif
