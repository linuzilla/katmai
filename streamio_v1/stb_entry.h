/*
 *	stb_entry.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __STB_ENTRY_H__
#define __STB_ENTRY_H__

struct stb_entry_t {
	int			n;
	int			stb_sn;
	int			udp;
	int			qos;
	// pthread_t		thread;
	pthread_mutex_t		mutex;
	pthread_cond_t		condi;
	// short		running;
	short			ready;
	short			inuse;
	in_addr_t		addr;
	time_t			lastuse;
	off_t			filesize;
	int			fd;
	short			filestatus;
	unsigned int		filecount;
	unsigned int		blockcnt;
	int			err;
	int			request_block;
	int			current_block;
	int			rdkio_block;
	int			cdkio_block;
	int			netio_block;
	char			map[23];
	int			mapidx;
	int			mapi;
	int			stbid;
	char			*buffer;
	int			buflen;
};

struct netio_buffer_t {
	char	b[23][1425];	// 23 x 1425 = 32775 > 32768
};

#endif
