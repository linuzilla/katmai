/*
 *	stb_table.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __STB_TABLE_H__
#define __STB_TABLE_H__

#include <netinet/in.h>

/*
extern pthread_mutex_t		stb_mutex;
extern unsigned int		stb_regist_cnt;
extern unsigned int		stb_opend_file;
extern unsigned int		stb_opend_file_all;
extern int			stb_qos_threshold;
extern int			best_diskio_buffer_size;
extern int			netio_block_per_diskio;
extern short			stop_services;
*/

// #define MPEG_BASEDIR		"/MPEG"

struct stiov3_fetch_data_t;
struct streamming_io_v3_server_t;

struct stbtable_t {
	int		(*add)(in_addr_t addr);
	int		(*regist)(const in_addr_t addr, const int u);
	off_t		(*setfile)(const int entry, const int channel,
				const char *file,
				const int cnt, const int stbid,
				char **rfname, int *err);
	int		(*setrequest)(const int i, const int channel,
						const int block,
						const char *map, const int qos);
	int		(*is_file_matched)(const int i, const int channel,
						const int fc);
	int		(*push)(const int i, const int channel, const int cmd);
	int		(*pop)(const int channel);
	int		(*init)(const int i, const int channel, const int sn);
	int		(*start)(const int i, const int sn);
	int		(*stop)(const int i, const int channel);
	int		(*read)(const int i, const int channel,
					struct stiov3_fetch_data_t *ptr);
	int		(*fill)(const int n, const int channel,
						const int mapflag,
						char *buffer, int *bid);
	int		(*udp)(const int n);
	int		(*streamming_lock)(const int i, const int channel);
	int		(*streamming_unlock)(const int i, const int channel);
	int		(*close_all)(void);
};

struct stbtable_t	* new_stbtable_v3 (
				struct streamming_io_v3_server_t *stio,
				const int num, const int enable_qos);

#endif
