/*
 *	cli_entry.h
 *
 *	Copyright (c) 2002-2003, Jiann-Ching Liu
 */

#ifndef __CLIENT_ENTRY_H__
#define __CLIENT_ENTRY_H__

#ifndef MAX_NUM_OF_STREAM
#define MAX_NUM_OF_STREAM	4
#endif

struct client_entry_t {
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
	off_t			filesize[MAX_NUM_OF_STREAM];
	int			fd[MAX_NUM_OF_STREAM];
	short			filestatus[MAX_NUM_OF_STREAM];
	unsigned int		filecount[MAX_NUM_OF_STREAM];
	unsigned int		blockcnt[MAX_NUM_OF_STREAM];
	int			err[MAX_NUM_OF_STREAM];
	int			request_block[MAX_NUM_OF_STREAM];
	int			current_block[MAX_NUM_OF_STREAM];
	int			rdkio_block[MAX_NUM_OF_STREAM];
	int			cdkio_block[MAX_NUM_OF_STREAM];
	int			netio_block[MAX_NUM_OF_STREAM];
	char			map[MAX_NUM_OF_STREAM][23];
	int			mapidx[MAX_NUM_OF_STREAM];
	int			mapi[MAX_NUM_OF_STREAM];
	int			stbid;
	char			*buffer[MAX_NUM_OF_STREAM];
	int			buflen[MAX_NUM_OF_STREAM];
};

struct netio_buffer_t {
	char	b[23][1425];	// 23 x 1425 = 32775 > 32768
};

#endif
