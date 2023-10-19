/*
 *	stream_mpeg.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __STREAM_MPEG_H__
#define __STREAM_MPEG_H__

struct stio_engine_t;
struct thread_svc_t;
struct fmplib_t;
struct usec_timer_t;
struct rmimage256_t;
struct rmosdlib_t;

struct stream_mpeg_pd_t {
	struct stio_engine_t	*stio;
	struct thread_svc_t	*thr;
	struct fmplib_t		*fmp;
	struct usec_timer_t	*timer;
	int			volume_level;
	int			nbuf;
	int			rbuf;
	int			start_play_buffer;
	int			repeat_cnt;
	char			current_audio_mode;
	int			current_audio_channel;
	short			have_load_driver;
	short			on_play;
	short			dummy_play;
	short			verbose;
	volatile short		terminate;
	volatile short		abort;
	short			bitrate_problem;
	double			danger_bitrate;
	short			under_run;
	volatile short		playing;
	volatile short		playing_error;
	volatile short		reset_mpeg;
	volatile short		into_replay_state;
	volatile short		in_replay_state;
	pthread_mutex_t		mutex;
	pthread_cond_t		condi;
	int			(*user_streamtype)(void *);
	void			*param;
	struct rmimage256_t	*osdimg;
	struct rmosdlib_t	*osdptr;
	int			osdimg_x;
	int			osdimg_y;
	volatile short		change_plattle;
	/*
	double			bitrate = 0.0;
	double			gbitrate = 0.0;
	double			minbps   = 0.0;
	*/

	struct thread_svc_t	*pub_thr;
	short			have_public;
	volatile short		terminate_public;
	volatile short		enable_public;
	volatile int		public_state;
	volatile int		public_stop_playing;
	struct stio_engine_t	*pub_stio;
	pthread_mutex_t		pub_mutex;
	pthread_cond_t		pub_condi;

	void			(*set_volume_callback)(void *);
	void			*volcbk_parm;

	void			(*set_mixer_callback)(void *);
	void			*mixcbk_parm;
};

struct stream_mpeg_t {
	struct stream_mpeg_pd_t	pd;

	int		(*open)(struct stream_mpeg_t *, const char *fname);
	int		(*start)(struct stream_mpeg_t *);
	int		(*reset)(struct stream_mpeg_t *);
	int		(*dummy_start)(struct stream_mpeg_t *);
	void		(*stop)(struct stream_mpeg_t *);
	void		(*close)(struct stream_mpeg_t *);
	int		(*is_playing)(struct stream_mpeg_t *);
	int		(*verbose)(struct stream_mpeg_t *, const int);
	void		(*abort)(struct stream_mpeg_t *);
	void		(*dispose)(struct stream_mpeg_t *);
	int		(*set_minplay)(struct stream_mpeg_t *, const int);
	int		(*set_repeat)(struct stream_mpeg_t *, const int);
	int		(*set_volume)(struct stream_mpeg_t *, const int);
	int		(*set_buffer)(struct stream_mpeg_t *, const int);
	void		(*set_user_streamtype)(struct stream_mpeg_t *,
				int (*st)(void *), void *param);
	struct stio_engine_t * (*stio_engine)(struct stream_mpeg_t *self);
	struct stio_engine_t * (*pub_stio_engine)(struct stream_mpeg_t *self);
	double		(*set_danger_bitrate)(struct stream_mpeg_t *self,
						const double dbr);
	int		(*bitrate_problem)(struct stream_mpeg_t *self);
	int		(*clear_bitrate_problem)(struct stream_mpeg_t *self);
	int		(*clear_under_run)(struct stream_mpeg_t *self);
	int		(*is_under_run)(struct stream_mpeg_t *self);
	int		(*wait_for_playing)(struct stream_mpeg_t *self);
	int		(*in_fmp)(struct stream_mpeg_t *);
	int		(*chip_id)(struct stream_mpeg_t *);
	int		(*audio_mode)(struct stream_mpeg_t *, const char);
	int		(*audio_channel)(struct stream_mpeg_t *, const int);
	int		(*pq_prefetch)(struct stream_mpeg_t *, const char *);
	void		(*preset_audio_mode)(struct stream_mpeg_t *,
							const char);
	void		(*preset_audio_channel)(struct stream_mpeg_t *,
							const int);

	void		(*change_palette)(struct stream_mpeg_t *self,
				const int sy, const int su, const int sv,
				const int yy, const int uu, const int vv);

	int		(*open_public)(struct stream_mpeg_t *,
						const char *fname);
	int		(*set_public)(struct stream_mpeg_t *,
						const int onoff);

	void		(*set_vol_cbk)(struct stream_mpeg_t *,
				void (*fcn)(void *), void *);

	void		(*set_mix_cbk)(struct stream_mpeg_t *,
				void (*fcn)(void *), void *);
};

struct stream_mpeg_t * new_stream_mpeg (const int nbuf);

#endif
