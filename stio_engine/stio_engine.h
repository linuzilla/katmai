/*
 *	stio_engine.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __STREAM_IO_ENGINE_H__
#define __STREAM_IO_ENGINE_H__

#include <sys/types.h>

struct stio_engine_prefetch_t;
struct stio_engine_t;

struct stio_engine_pd_t {
	struct stio_engine_prefetch_t	*pch;
	off_t				fsize;
	off_t				fp_pos;
	int				fd;
	void				*myself;
	volatile short			abort_flag;
	volatile short			in_read;
	volatile short			end_of_prefetch;
	volatile short			in_pq_prefetch;
	void				*param_for_plugin;
	int				repeat_cnt;
	short				have_next_file;
	short				have_next_precache;
	char				current_file[128];
	char				next_file[128];
	char				*name;
	int				(*endof_prefetch_callback)(void *);
	void				*endof_prefetch_param;

	int	(*open)(struct stio_engine_t *, const char *fname);
	int	(*open_next)(struct stio_engine_t *, const char *fname);
	int	(*reset)(struct stio_engine_t *);
	int	(*seek)(struct stio_engine_t *, const off_t pos);
	ssize_t	(*read)(struct stio_engine_t *, void *buffer, size_t len);
	void	(*close)(struct stio_engine_t *);
	off_t	(*filesize)(struct stio_engine_t *);
	void	(*abort)(struct stio_engine_t *);
	void	(*dispose)(struct stio_engine_t *);
	int	(*read_plugin)(void *buffer, int len, void *param);
};

struct stio_engine_t {
	struct stio_engine_pd_t	pd;

	int	(*open)(struct stio_engine_t *, const char *fname);
	int	(*open_next)(struct stio_engine_t *, const char *fname);
	int	(*reset)(struct stio_engine_t *);
	int	(*seek)(struct stio_engine_t *, const off_t pos);
	ssize_t	(*read)(struct stio_engine_t *, void *buffer, size_t len);
	off_t	(*read_pos)(struct stio_engine_t *);
	void	(*close)(struct stio_engine_t *);
	off_t	(*filesize)(struct stio_engine_t *);
	int	(*end_of_prefetch)(struct stio_engine_t *);
	void	(*abort)(struct stio_engine_t *);
	void	(*dispose)(struct stio_engine_t *);
	int	(*set_repeat)(struct stio_engine_t *, const int);
	int	(*set_eop_callback)(
			struct stio_engine_t *self, int(*func)(void *parm),
			void *parm);

	int	(*prefetch)(struct stio_engine_t *, const int nbuf,
						const int blen);
	int	(*prefetch_is_running)(struct stio_engine_t *);
	int	(*freebuf)(struct stio_engine_t *);
	void *	(*set_read_plugin)(struct stio_engine_t *,
				int (*)(void *, int, void *), void *);
	int	(*set_buffer)(struct stio_engine_t *, const int);
	int	(*set_keep_buffer)(struct stio_engine_t *, const int);
	int	(*set_prefetch_buffer)(struct stio_engine_t *, const int);
	int	(*start_pq_prefetch)(struct stio_engine_t *self);
	int	(*stop_pq_prefetch)(struct stio_engine_t *self);
	void	(*set_name)(struct stio_engine_t *, char *name);
};

struct stio_engine_t	* new_stio_engine (void);

#endif
