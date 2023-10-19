/*
 *	fmplib.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __FMPLIB_H__
#define __FMPLIB_H__

struct timeval;

enum fmp_stream_t {
	FMP_SYSTEM_STREAM,
	FMP_PROGRAM_STREAM,
	FMP_TRANSPORT_STREAM,
	FMP_VIDEO_STREAM,
	FMP_UNKNOW_STREAM
};

enum fmp_audio_mode_t {
	FMP_AM_STEREO,
	FMP_AM_RIGHT,
	FMP_AM_LEFT,
	FMP_AM_MIX
};

struct fmplib_t {
	int		(*driver_entry)(void);
	void		(*driver_unload)(void);
	void		(*set_volume)(const int vol);
	void		(*set_audio_channel)(const int channel);
	void		(*set_audio_mode)(const enum fmp_audio_mode_t mode);
	int		(*open)(const enum fmp_stream_t stream);
	void		(*play)(void);
	void		(*close)(void);
	void		(*stop)(void);
	void		(*pause)(void);
	void		(*push)(void);
	void		(*disable)(void);
	int		(*get_buffer)(void);
	int		(*em84xx_id)(void);
	unsigned long	(*buffer_size)(void);
	void		(*set_data_size)(unsigned long ds);
	int		(*is_infmp)(struct timeval *tv);
	void *		(*buffer_ptr)(void);
};

struct fmplib_t *	new_fmplib (void);

#endif
