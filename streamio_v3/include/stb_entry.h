/*
 *	stb_entry.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __STB_ENTRY_H__
#define __STB_ENTRY_H__

#include "streamio_v3.h"

struct stb_entry_t {
	int			n;
	int			stb_sn;
	int			udp;
	int			qos[MAX_STIOV3_CHANNEL];
	// pthread_t		thread;
	pthread_mutex_t		mutex[MAX_STIOV3_CHANNEL];
	pthread_cond_t		condi[MAX_STIOV3_CHANNEL];
	// short		running;
	short			ready;
	short			inuse;
	in_addr_t		addr;
	time_t			lastuse;
	off_t			filesize[MAX_STIOV3_CHANNEL];
	int			fd[MAX_STIOV3_CHANNEL];
	short			filestatus[MAX_STIOV3_CHANNEL];
	unsigned int		filecount[MAX_STIOV3_CHANNEL];
	unsigned int		blockcnt[MAX_STIOV3_CHANNEL];
	int			err[MAX_STIOV3_CHANNEL];
	int			request_block[MAX_STIOV3_CHANNEL];
	int			current_block[MAX_STIOV3_CHANNEL];
	int			rdkio_block[MAX_STIOV3_CHANNEL];
	int			cdkio_block[MAX_STIOV3_CHANNEL];
	int			netio_block[MAX_STIOV3_CHANNEL];
	char			map[MAX_STIOV3_CHANNEL][23];
	int			mapidx[MAX_STIOV3_CHANNEL];
	int			mapi[MAX_STIOV3_CHANNEL];
	int			stbid[MAX_STIOV3_CHANNEL];
	char			*buffer[MAX_STIOV3_CHANNEL];
	int			buflen[MAX_STIOV3_CHANNEL];
};

struct netio_buffer_t {
	char	b[23][1425];	// 23 x 1425 = 32775 > 32768
};

#endif
