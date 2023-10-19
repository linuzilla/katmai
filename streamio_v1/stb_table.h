/*
 *	stb_table.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __STB_TABLE_H__
#define __STB_TABLE_H__

#include <netinet/in.h>

struct stb_vod_fetch_data_t;

struct stbtable_t {
	int		(*add)(in_addr_t addr);
	int		(*regist)(const in_addr_t addr, const int u);
	off_t		(*setfile)(const int entry, const char *file,
				const int cnt, const int stbid, int *err);
	int		(*setrequest)(const int i, const int block,
						const char *map, const int qos);
	int		(*is_file_matched)(const int i, const int fc);
	int		(*push)(const int i, const int cmd);
	int		(*pop)(void);
	int		(*init)(const int i, const int sn);
	int		(*start)(const int i, const int sn);
	int		(*stop)(const int i);
	int		(*read)(const int i, struct stb_vod_fetch_data_t *ptr);
	int		(*fill)(const int n, const int mapflag,
						char *buffer, int *bid);
	int		(*udp)(const int n);
	int		(*streamming_lock)(const int i);
	int		(*streamming_unlock)(const int i);
	int		(*close_all)(void);
};

struct stbtable_t	* new_stbtable (const int num, const int enable_qos);

#endif
