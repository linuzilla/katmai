/*
 *	holktv_stb.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __HOLKTV_STB_H__
#define __HOLKTV_STB_H__

#ifndef USING_STREAMIO_VERSION
#define USING_STREAMIO_VERSION	3
#endif

#include <pthread.h>

#include "rc4encrypt.h"
#include "md5.h"


#define VOLUME_LEVEL_SOURCE     7

#define HOLKTV_VOL_ID_CDIN	4
#define HOLKTV_VOL_ID_PCM	5

#define HKTV_CHANNEL_C0		0
#define HKTV_CHANNEL_C1		1
#define HKTV_CHANNEL_C2		2
#define HKTV_CHANNEL_C3		3
#define HKTV_CHANNEL_C4		4
#define HKTV_CHANNEL_C5		5
#define HKTV_CHANNEL_C6		6
#define HKTV_CHANNEL_C7		7
#define HKTV_CHANNEL_C8		8
#define HKTV_CHANNEL_C9		9
#define HKTV_CHANNEL_LEFT	10
#define HKTV_CHANNEL_RIGHT	11
#define HKTV_CHANNEL_MIX	12
#define HKTV_CHANNEL_NONE	(-1)
#define HKTV_CHANNEL_MP3	(-2)


struct streamming_io_v2_client_t;
struct streamming_io_v3_client_t;
struct stream_mp3_t;
struct lyrics_osd_t;
struct stream_mpeg_t;
struct thread_svc_t;
struct holktv_playtype_t;
struct aumixer_t;

struct volume_level_t {
	union {
		int             src[VOLUME_LEVEL_SOURCE];

		struct {
			int     fmp;
			int     main;
			int     linein;
			int     lineout;
			int     cdin;
			int     pcm;
			int     mic;    // microphone;
		};
	};
};

struct holktv_regcmd_parm_t {
	struct udplib_t			*udp;
	struct holktv_cmd_pdu		*pdu;
	struct holktv_cmd_reply_pdu	*rpdu;
};

// extern const int volume_mapping[VOLUME_LEVEL_SOURCE];

struct holktv_stb_pd_t {
	RC4_KEY		rc4key[5];
	unsigned char	md5rc4key[MD5_DIGEST_LENGTH];
	short		have_key;
	short		new_file[5];
	short		can_cut;
	short		allow_cut;
	short		can_replay;
	char		hol_header[5][25];
	int		hol_idx[5];
	int		fmt[5];
	int		in_playing;
	int		minplay;
	int		nbuf;
	int		prefer_vocal;
	int		mtv_channel[2];
	char		mtv_mode[2];
	volatile short	terminate;
	volatile short	replaying;
	volatile short	close_fader;
	volatile short	in_fade;
	volatile short	endof_prefetch;
	volatile short	already_prefetch;

	struct holktv_playtype_t		*pltype;

#if USING_STREAMIO_VERSION == 2
	struct streamming_io_v2_client_t	*client;
#elif USING_STREAMIO_VERSION == 3
	struct streamming_io_v3_client_t	*client;
#endif
	struct stream_mp3_t			*mp3;
	struct lyrics_osd_t			*lyr;
	struct stream_mpeg_t			*mpeg;
	struct thread_svc_t			*thr;
	struct thread_svc_t			*fader;
	struct aumixer_t			*mixer;

	pthread_mutex_t				mutex;
	pthread_cond_t				condi;

	pthread_mutex_t				mutex_fader;
	pthread_cond_t				condi_fader;

	// struct volume_level_t		volume_running;
	struct volume_level_t			volume_setting;

	int					pcm_volume;
	int					mpeg_volume;
};

struct holktv_stb_t {
	struct holktv_stb_pd_t	pd;

	void		(*setkey)(struct holktv_stb_t *,
				const void *keystr, size_t keylen);
	void		(*clearkey)(struct holktv_stb_t *);

	int		(*bind_channel)(struct holktv_stb_t *,
				const int channel);

	void		(*set_mp3_prefetch)(struct holktv_stb_t *,
				const int nbuf, const int bsize);

	void		(*set_keep_buffer)(struct holktv_stb_t *,
				const int nbuf_mpeg, const int nbuf_mp3);

	void		(*set_pq_buffer)(struct holktv_stb_t *,
				const int nbuf);

	int		(*set_mpeg_minplay)(struct holktv_stb_t *,
						const int mpbuf);

	int		(*get_mpeg_status)(struct holktv_stb_t *,
					const int, int *, int *);

	int		(*set_mpeg_decrypt)(struct holktv_stb_t *, const int);
	int		(*set_mp3_decrypt)(struct holktv_stb_t *, const int);
	int		(*set_pmpeg_decrypt)(
					struct holktv_stb_t *, const int);
	int		(*prepare)(struct holktv_stb_t *, const int type,
					const char *mpegfile,
					const char *mp3file,
					const int music_channel,
					const int vocal_channel);
	int		(*pq_prefetch)(struct holktv_stb_t *,
						const char *mpegf);
	int		(*play)(struct holktv_stb_t *);
	int		(*replay)(struct holktv_stb_t *);
	int		(*cut)(struct holktv_stb_t *);
	int		(*wait_condition)(struct holktv_stb_t *);
	char *		(*server_ip)(struct holktv_stb_t *);
	int		(*server_port)(struct holktv_stb_t *);
	int		(*server_id)(struct holktv_stb_t *);
	int		(*is_playing)(struct holktv_stb_t *);
	int		(*in_fmp)(struct holktv_stb_t *);
	void		(*set_system_volume)(struct holktv_stb_t *);
	int		(*set_volume_level)(struct holktv_stb_t *,
				const char *volume_str);
	int		(*get_volume_level)(struct holktv_stb_t *, char *);
	void		(*volume_switch)(struct holktv_stb_t *, const int,
					const int);
	int		(*load_volume)(struct holktv_stb_t *, const char *);
	int		(*save_volume)(struct holktv_stb_t *, const char *);
	void		(*cdpcm)(struct holktv_stb_t *, const int, const int);
	int		(*prefer_vocal)(struct holktv_stb_t *, const int);

	void		(*streamming_delay)(struct holktv_stb_t *,
					int *dof, int *dor, int *dos);

	int		(*mpeg_decrypt)(void *buffer, int len, void *param);
	int		(*pmpeg_decrypt)(void *buffer, int len, void *param);
	int		(*mp3_decrypt[2])(void *buffer, int len, void *param);

	void		(*lyr_rgb)(struct holktv_stb_t *, const int isdefault);
	void		(*lyr_palette)(struct holktv_stb_t *,
					const int singer, const int channel,
					const int yy, const int uu,
					const int vv);

	int		(*enable_public_mpeg)(struct holktv_stb_t *,
					const int onoff);

	int		(*set_public_mpeg)(struct holktv_stb_t *,
					const char *file);

	int		(*cmd_regist)(const char,
					int (*)(struct holktv_regcmd_parm_t *));
	int		(*command_processing)(const int port);



	void		(*dispose)(struct holktv_stb_t *);

	struct stream_mpeg_t *	(*get_mpeg)(struct holktv_stb_t *);
};

struct holktv_stb_t * new_holktv_stb (const int port, const int mpegbuf);

#endif
