/*
 *	holktv_stb.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/reboot.h>
#include "holktv_stb.h"
#include "holktv_pdu.h"
#include "rc4encrypt.h"
#include "md5.h"
#include "stream_mpeg.h"
#include "stream_mp3.h"
#include "lyrics_osd.h"
#include "streamio_v2.h"
#include "streamio_v3.h"
#include "stio_engine.h"
#include "thread_svc.h"
#include "aumixer.h"
#include "udplib.h"
#include "conio.h"
#include "katmai.h"

#define ENABLE_VOCAL_FADER      1

#define ENABLE_PURE_MPEG	1

#define HOLKTV_FADE_STEP	8
#define HOLKTV_FADE_SLEEP	100000


const int volume_mapping[VOLUME_LEVEL_SOURCE] = {
//      fmp,main,linein,lineout,cdin,pcm,mic
	 -1,   0,     6,     13,   8,  4,  7
};

static const struct volume_level_t   default_volume_level = {
	{
		{
			60,     // FMP
			90,     // main
			80,     // linein
			80,     // lineout
			80,     // cdin
			90,     // pcm
			 0      // microphone
		}
	}
};

static struct volume_level_t   volume_mask = {
	{
		{
			1,     // FMP
			1,     // main
			1,     // linein
			1,     // lineout
			1,     // cdin
			1,      // pcm
			1       // microphone
		}
	}
};

#define HKTV_VOL_PCM	5
#define HKTV_VOL_MPEG	4

#include "stb/holktv_stb_cmd.c"
#include "stb/holktv_stb_vol.c"

static struct holktv_playtype_t {
	int	type;	// 0 ~ 4
	int	major;	// 0->Mpeg, 1->Music MP3
	int	mpeg;
	int	mp3;
	int	music;
	int	vocal;
	int	lyrics;
	int	have_vocal;
	char	*desc;
} playtype[] = {
/*         t  m  M  3  m  v  l  h */
/* 0 */	{  0, 0, 1, 0, 0, 0, 0, 0, "MPEG (Original)"			},
/* 1 */	{  1, 0, 1, 1, 0, 1, 0, 1, "MPEG (Original) + Vocal MP3"	},
/* 2 */	{  2, 1, 2, 1, 1, 0, 1, 0, "Music MP3 + Background MPEG"	},
/* 3 */	{  3, 1, 2, 1, 1, 1, 1, 1, "Music+Vocal MP3 + Background MPEG"	},
/* 4 */	{  4, 0, 1, 0, 0, 0, 0, 1, "MPEG (Multiple channel, L/R channel)" },
/* 5 */	{  5, },
/* 6 */	{  6, },
/* 7 */	{  7, },
/* 8 */	{  8, },
/* 9 */	{  9, 0, 0, 0, 0, 0, 0, 0, "OSD Testing only"			}
};

static const unsigned char	filehead[] = "1a622778&EnYaDD";

static void hktv_setkey (struct holktv_stb_t *self,
				const void *keystr, size_t keylen) {
	md5sum (keystr, keylen, self->pd.md5rc4key);
	self->pd.have_key	= 1;
}

static void hktv_clearkey (struct holktv_stb_t *self) {
	self->pd.have_key = 0;
}

static int hktv_generic_decrypt (void *buffer, int len,
					void *param, const int channel) {
	struct holktv_stb_t		*self = param;

	if (buffer == NULL) {   // initialize
		// fprintf (stderr, "Holiday Streamming Channel %d Decrypt "
		//		"initiailized!\n", channel);
		self->pd.new_file[channel] = 1;
		self->pd.hol_idx[channel]  = 0;
		return 0;
	} else if (! self->pd.new_file[channel]) {
		RC4 (&self->pd.rc4key[channel], len, buffer, buffer);
		return len;
	} else if (len + self->pd.hol_idx[channel] < 25) {
		memcpy (&self->pd.hol_header[channel][
					self->pd.hol_idx[channel]],
							buffer, len);
		self->pd.hol_idx[channel] += len;
		//fprintf (stderr, "[=] Short packet %d, %d\n",
		//			len, self->pd.hol_idx);
		return 0;
	} else {
		unsigned char		key;
		int			i;
		int			st = 0;
		unsigned char		rcbuf[9];
		int			fmt = 0;
		unsigned char		temp[260];
		unsigned char		*buf = self->pd.hol_header[channel];
		char			*stbuf = buffer;

		if (self->pd.hol_idx[channel] < 25) {
			st = 25 - self->pd.hol_idx[channel];
			//fprintf (stderr, "[=] Memory copy %d, %d\n",
			//		self->pd.hol_idx, st);

			memcpy (&buf[self->pd.hol_idx[channel]], buffer, st);
			len -= st;
			self->pd.hol_idx[channel] = 25;
		}
		self->pd.new_file[channel] = 0;

		key = buf[15];

		/*
		fprintf (stderr, "[");
		for (i = 0; i < sizeof filehead - 1; i++) {
			fprintf (stderr, "%02x", buf[i]);
		}
		fprintf (stderr, "]\n");
		*/

		for (i = 0; i < sizeof filehead - 1; i++) {
			if (buf[i] != (filehead[i] ^ ((unsigned char)
						(168 - (i + 1) * 16) ^ key))) {
				fprintf (stderr,
					"HEADER ERROR on channel %d!!\n",
					channel);
				return -2;
			}
		}

		RC4_set_key (&self->pd.rc4key[channel], MD5_DIGEST_LENGTH,
							self->pd.md5rc4key);
		RC4 (&self->pd.rc4key[channel], key + 3, temp, temp);
		RC4 (&self->pd.rc4key[channel], sizeof rcbuf, &buf[16], rcbuf);
		self->pd.fmt[channel] = fmt = rcbuf[0];

		for (i = 0; i < 8; i++) {
			if (rcbuf[i+1] != (filehead[i] ^ ((i+1) * 23) ^ fmt)) {
				fprintf (stderr,
					"Additional HEADER ERROR on "
					"channel %d!!\n", channel);
				return -3;
			}
		}

		if (len > 0) {
			RC4 (&self->pd.rc4key[channel],len, &stbuf[st], buffer);
			return len;
		} else {
			return 0;
		}
	}
}

static int hktv_mpeg_decrypt (void *buffer, int len, void *param) {
//	struct holktv_stb_t		*self = param;

	return hktv_generic_decrypt (buffer, len, param, 0);
}

static int hktv_mp3_1_decrypt (void *buffer, int len, void *param) {
	return hktv_generic_decrypt (buffer, len, param, 1);
}

static int hktv_mp3_2_decrypt (void *buffer, int len, void *param) {
	return hktv_generic_decrypt (buffer, len, param, 2);
}

static int hktv_pubmpeg_decrypt (void *buffer, int len, void *param) {
	return hktv_generic_decrypt (buffer, len, param, 4);
}

static int hktv_mpeg_filefmt (void *selfptr) {
	struct holktv_stb_t *self = selfptr;

	return self->pd.fmt[0];
}

static int hktv_bind_channel (struct holktv_stb_t *self, const int channel) {
	struct holktv_stb_pd_t	*pd = &self->pd;
	struct stio_engine_t	*stio;

	switch (channel) {
	case 0:	// MPEG
		stio = pd->mpeg->stio_engine (pd->mpeg);
		break;
	case 1: // MP3 (Music)
		stio = pd->mp3->stio_engine (pd->mp3, 0);
		break;
	case 2: // MP3 (Vocal)
		stio = pd->mp3->stio_engine (pd->mp3, 1);
		break;
	case 3: // Lyrics
		stio = pd->lyr->stio_engine (pd->lyr);
		break;
	case 4: // Public MPEG
		stio = pd->mpeg->pub_stio_engine (pd->mpeg);
		break;
	default:
		return 0;
	}

	pd->client->regist_stio (pd->client, channel, stio);

	return 1;
}


static void hktv_set_mp3_prefetch (struct holktv_stb_t *self,
					const int nbuf, const int bsize) {
	struct holktv_stb_pd_t	*pd = &self->pd;
	struct stio_engine_t	*stio;

	stio = pd->mp3->stio_engine (pd->mp3, 0);
	stio->prefetch (stio, nbuf, bsize);

	stio = pd->mp3->stio_engine (pd->mp3, 1);
	stio->prefetch (stio, nbuf, bsize);
}

static void hktv_set_keep_buffer (struct holktv_stb_t *self,
				const int nbuf_mpeg, const int nbuf_mp3) {
	struct holktv_stb_pd_t	*pd = &self->pd;
	struct stio_engine_t	*stio;

	stio = pd->mpeg->stio_engine (pd->mpeg);
	stio->set_keep_buffer (stio, nbuf_mpeg);

	stio = pd->mp3->stio_engine (pd->mp3, 0);
	stio->set_keep_buffer (stio, nbuf_mp3);

	stio = pd->mp3->stio_engine (pd->mp3, 1);
	stio->set_keep_buffer (stio, nbuf_mp3);
}

static void hktv_set_pq_buffer (struct holktv_stb_t *self, const int nbuf) {
	struct holktv_stb_pd_t	*pd = &self->pd;
	struct stio_engine_t	*stio;

	stio = pd->mpeg->stio_engine (pd->mpeg);
	stio->set_prefetch_buffer (stio, nbuf);
}

static int hktv_set_mpeg_minplay (struct holktv_stb_t *self, const int v) {
	struct stream_mpeg_t	*mpeg = self->pd.mpeg;
	int			val = v;

	if (v < 10) val = 10;
	
	return mpeg->set_minplay (mpeg, self->pd.minplay = val);
}

static int hktv_get_mpeg_status (struct holktv_stb_t *self,
				const int clear, int *bu, int *be) {
	struct stream_mpeg_t	*mpeg = self->pd.mpeg;

	if ((clear & 1) != 0) {
		*bu = mpeg->clear_under_run (mpeg);
	} else {
		*bu = mpeg->is_under_run (mpeg);
	}

	if ((clear & 2) != 0) {
		*be = mpeg->clear_bitrate_problem (mpeg);
	} else {
		*be = mpeg->bitrate_problem (mpeg);
	}

	return *bu + *be;
}

static int hktv_in_fmp (struct holktv_stb_t *self) {
	struct stream_mpeg_t	*mpeg = self->pd.mpeg;

	return mpeg->in_fmp (mpeg);
}

static int hktv_prefer_vocal (struct holktv_stb_t *self, const int v) {
	struct holktv_stb_pd_t		*pd = &self->pd;
	int				rc  = pd->prefer_vocal;
	int				val = v > 0 ? 1 : 0;
	struct holktv_playtype_t	*pt = pd->pltype;
#if ENABLE_VOCAL_FADER != 1
	struct stream_mp3_t		*mp3 = pd->mp3;
	struct stream_mpeg_t		*mpeg = pd->mpeg;
#endif

	if (v >= 0) {
		if (pd->prefer_vocal != val) {
			pd->prefer_vocal = val;

			if (self->is_playing (self) && pt->have_vocal != 0) {
				if (pt->type == 4) { // Multiple channel
					if (pd->mtv_channel[1] ==
							HKTV_CHANNEL_NONE) {
						// 根本沒導唱
						return 0;
					}
#if ENABLE_VOCAL_FADER == 1
					if (val == 0) {
						pd->in_fade = 5;
					} else {
						pd->in_fade = 6;
					}

					pthread_mutex_lock   (&pd->mutex_fader);
					pthread_cond_signal  (&pd->condi_fader);
					pthread_mutex_unlock (&pd->mutex_fader);
#else
					if (pd->mtv_channel[0] !=
							pd->mtv_channel[1]) {
						mpeg->audio_channel (mpeg,
							pd->mtv_channel[val]);
						}

					if (pd->mtv_mode[0] !=
							pd->mtv_mode[1]) {
						mpeg->audio_mode (
							mpeg,
							pd->mtv_mode[val]);
					}
#endif
				} else if (pt->mpeg == 1) {
#if ENABLE_VOCAL_FADER == 1
					if (val == 0) {
						pd->in_fade = 1;
						volume_mask.cdin = 1;
						volume_mask.pcm  = 0;
					} else {
						pd->in_fade = 2;
						volume_mask.cdin = 0;
						volume_mask.pcm  = 1;
					}

					pthread_mutex_lock   (&pd->mutex_fader);
					pthread_cond_signal  (&pd->condi_fader);
					pthread_mutex_unlock (&pd->mutex_fader);
#else
					if (val == 0) {
						volume_mask.cdin = 1;
						volume_mask.pcm  = 0;

						self->cdpcm (self, 1, 0);
					} else {
						volume_mask.cdin = 0;
						volume_mask.pcm  = 1;

						self->cdpcm (self, 0, 1);
					}
#endif
				} else {
#if ENABLE_VOCAL_FADER == 1
					if (val == 0) {
						pd->in_fade = 3;
					} else {
						pd->in_fade = 4;
					}

					pthread_mutex_lock   (&pd->mutex_fader);
					pthread_cond_signal  (&pd->condi_fader);
					pthread_mutex_unlock (&pd->mutex_fader);
#else
					if (val == 0) {
						mp3->select (mp3, 0);
					} else {
						mp3->select (mp3, 1);
					}
#endif
				}
			}
		}
	}

	return rc;
}

static int hktv_set_mpeg_decrypt (struct holktv_stb_t *self, const int v) {
	struct stream_mpeg_t	*mpeg = self->pd.mpeg;
	struct stio_engine_t	*stio;

	stio = mpeg->stio_engine (mpeg);

	if (v > 0) {
		stio->set_read_plugin (stio, self->mpeg_decrypt, self); 
		mpeg->set_user_streamtype (mpeg, hktv_mpeg_filefmt, self);
		return 1;
	} else if (v == 0) {
		stio->set_read_plugin (stio, NULL, self); 
		mpeg->set_user_streamtype (mpeg, NULL, NULL);
		return 1;
	}
	return 0;
}

static int hktv_set_pubmpeg_decrypt (struct holktv_stb_t *self, const int v) {
	struct stream_mpeg_t	*mpeg = self->pd.mpeg;
	struct stio_engine_t	*stio;

	stio = mpeg->pub_stio_engine (mpeg);

	if (v > 0) {
		stio->set_read_plugin (stio, self->pmpeg_decrypt, self); 
		// mpeg->set_user_streamtype (mpeg, hktv_mpeg_filefmt, self);
		return 1;
	} else if (v == 0) {
		stio->set_read_plugin (stio, NULL, self); 
		// mpeg->set_user_streamtype (mpeg, NULL, NULL);
		return 1;
	}
	return 0;
}

static int hktv_set_mp3_decrypt (struct holktv_stb_t *self, const int v) {
	struct stream_mp3_t	*mp3 = self->pd.mp3;
	struct stio_engine_t	*stio;
	int			i;
	int			rc = 0;

	for (i = 0; i < 2; i++) {
		stio = mp3->stio_engine (mp3, i);

		if (v > 0) {
			stio->set_read_plugin (stio,
					self->mp3_decrypt[i], self); 
			rc = 1;
		} else if (v == 0) {
			stio->set_read_plugin (stio, NULL, self); 
		}
	}
	return rc;
}

static void set_delay_mpeg_volume (void *parm) {
	struct holktv_stb_t	*self = parm;
	struct holktv_stb_pd_t	*pd   = &self->pd;
	struct aumixer_t	*mix  = pd->mixer;

	mix->close_lock (1);

	mix->setvolume (volume_mapping[HKTV_VOL_PCM],  pd->pcm_volume);
	mix->setvolume (volume_mapping[HKTV_VOL_MPEG], pd->mpeg_volume);

	mix->close_lock (0);
	mix->close ();
}

static void set_original_mpeg_volume (void *parm) {
	struct holktv_stb_t	*self = parm;
	struct holktv_stb_pd_t	*pd   = &self->pd;
	struct stream_mpeg_t	*mpeg = pd->mpeg;
	int			mpeg_volume = pd->volume_setting.cdin;
	struct aumixer_t	*mix  = pd->mixer;

	fprintf (stderr, "Set Volume back to %d\n", mpeg_volume);
	mix->close_lock (1);

	mix->setvolume (volume_mapping[HKTV_VOL_PCM],  0);
	mix->setvolume (volume_mapping[HKTV_VOL_MPEG], mpeg_volume);

	mix->close_lock (0);
	mix->close ();

	mpeg->set_volume (mpeg, pd->volume_setting.fmp);
}

static int hktv_pq_prefetch (struct holktv_stb_t *self, const char *mpegfile) {
	struct stream_mpeg_t		*mpeg = self->pd.mpeg;
	// struct stio_engine_t		*stio = mpeg->stio_engine (mpeg);

	// mpeg->pq_prefetch (mpeg, mpegfile);
	// fprintf (stderr, "Prefetch [%s]\n", mpegfile);

	self->pd.already_prefetch = 1;

	mpeg->pq_prefetch (mpeg, mpegfile);

	// if (stio->open_next (stio, mpegfile) >= 0) {
	//	stio->start_pq_prefetch (stio);
	// }

	return 1;
}

static int hktv_prepare (struct holktv_stb_t *self, const int type,
						const char *mpegfile,
						const char *mp3file,
						const int music_channel,
						const int vocal_channel) {
	struct holktv_stb_pd_t		*pd   = &self->pd;
	struct stream_mpeg_t		*mpeg = pd->mpeg;
	struct stream_mp3_t		*mp3  = pd->mp3;
	struct lyrics_osd_t		*lyr  = pd->lyr;
	//struct aumixer_t		*mix  = pd->mixer;
	struct holktv_playtype_t	*pt;
	char				mp3music[256];
	char				mp3vocal[256];
	char				mp3lyrics[256];
#ifdef ENABLE_PURE_MPEG
	char				purempeg[256];
#endif
	int				fmp_volume, mpeg_volume, pcm_volume;


	if (type < 0) {
		if (type == -2) {
			pd->pltype = &playtype[9];
			// return type;
			return 0;
		}

		return 1;
	}

	if (type > 4) return 1;	// Type out of range

	if (mp3file != NULL) {
		/*
		sprintf (mp3music,  "%s-M.mp3", mp3file);
		sprintf (mp3vocal,  "%s-V.mp3", mp3file);
		sprintf (mp3lyrics, "%s.kok"  , mp3file);
		*/
		sprintf (mp3music,  "%s", mp3file);
		sprintf (mp3vocal,  "%s", mp3file);
		sprintf (mp3lyrics, "%s", mp3file);
	}

#ifdef ENABLE_PURE_MPEG
	sprintf (purempeg, "%s.mpg", mpegfile);
#endif

	pd->pltype = pt = &playtype[type];

	self->cut (self);

	if (pt->mpeg >= 2) {
		mpeg->set_buffer (mpeg, pd->nbuf / 4);
	} else {
		mpeg->set_buffer (mpeg, 0);
	}

	self->set_keep_buffer (self, -1, -1);
	self->set_keep_buffer (self,  1,  1);

	self->set_mpeg_decrypt (self, 1);

#ifndef ENABLE_PURE_MPEG
	if (mpeg->open (mpeg, mpegfile) < 0) return 2;
#else
	if (mpeg->open (mpeg, mpegfile) < 0) {
		if (mpeg->open (mpeg, purempeg) < 0) {
			return 2; // Mpeg open failed
		} else {
			self->set_mpeg_decrypt (self, 0);
		}
	}
#endif

	if (pt->mpeg >= 2) {
		struct stio_engine_t	*stio;

		//	Auto repeat for mpeg type 2

		stio = mpeg->stio_engine (pd->mpeg);
		// fprintf (stderr, "Auto repeat on\n");
		stio->set_repeat (stio, -1);

		// disable keep buffer on MPEG
		self->set_keep_buffer (self,  0,  1);

		fmp_volume  = mpeg_volume = 0;
		pcm_volume  = pd->volume_setting.src[HKTV_VOL_PCM];

		volume_mask.cdin = 0;
		volume_mask.pcm  = 1;
	} else {
		fmp_volume  = pd->volume_setting.fmp;
		mpeg_volume = pd->volume_setting.cdin;
		pcm_volume  = 0;
		volume_mask.cdin = 1;
		volume_mask.pcm  = 0;
	}

	if (pt->music  > 0) if (mp3->init (mp3, 0, mp3music) == 0) {
		mpeg->stop (mpeg);
		return 3;
	}

	if (pt->vocal  > 0) if (mp3->init (mp3, 1, mp3vocal) == 0) {
		mpeg->stop (mpeg);
		mp3->stop (mp3);
		return 4;
	}

	if (pt->lyrics > 0) if (lyr->init (lyr,   mp3lyrics) <= 0) {
		mpeg->stop (mpeg);
		mp3->stop (mp3);
		return 5;
	}

	if (pt->music > 0) {
		if ((pt->vocal > 0) && (pd->prefer_vocal)) {
			mp3->select (mp3, 1);
		} else {
			mp3->select (mp3, 0);
		}
		mp3->run (mp3);
	} else if (pt->vocal > 0) {
		if (pd->prefer_vocal) {
			mpeg_volume = 0;
			pcm_volume  = pd->volume_setting.src[HKTV_VOL_PCM];
			volume_mask.cdin = 0;
			volume_mask.pcm  = 1;
		}
		mp3->select (mp3, 1);
		mp3->run (mp3);
	}

	// Delay mixer
#ifdef NO_DELAY_MIXER_ON_PUBLIC
	mix->close_lock (1);

	mpeg->set_volume (mpeg, fmp_volume);
	mix->setvolume (volume_mapping[HKTV_VOL_PCM],  pcm_volume);
	mix->setvolume (volume_mapping[HKTV_VOL_MPEG], mpeg_volume);

	mix->close_lock (0);
	mix->close ();
#else
	mpeg->set_volume (mpeg, fmp_volume);

	pd->pcm_volume  = pcm_volume;
	pd->mpeg_volume = mpeg_volume;
	// mpeg->set_mixer (volume_mapping[HKTV_VOL_PCM], pcm_volume,
	//		volume_mapping[HKTV_VOL_MPEG], mpeg_volume);
#endif

	pd->mtv_channel[0] = 0;
	pd->mtv_channel[1] = HKTV_CHANNEL_NONE;
	pd->mtv_mode[0]    = 'S';
	pd->mtv_mode[1]    = 'S';

	if (pt->type == 4) {	// Multiple Channel
		int	i;
		int	v[2] = { music_channel, vocal_channel };

		for (i = 0; i < 2; i++) {
			switch (v[i]) {
			case HKTV_CHANNEL_LEFT:
				pd->mtv_mode[i] = 'L';
				pd->mtv_channel[i] = 0;
				break;
			case HKTV_CHANNEL_RIGHT:
				pd->mtv_mode[i] = 'R';
				pd->mtv_channel[i] = 0;
				break;
			case HKTV_CHANNEL_MIX:
				pd->mtv_mode[i] = 'M';
				pd->mtv_channel[i] = 0;
				break;
			case HKTV_CHANNEL_NONE:
				pd->mtv_channel[i] = HKTV_CHANNEL_NONE;
				break;
			default:
				if ((v[i] >= 0) && (v[i] <= 9)) {
					pd->mtv_mode[i] = 'S';
					pd->mtv_channel[i] = v[i];
				}
				break;
			}
		}
	}

	mpeg->preset_audio_channel (mpeg, pd->mtv_channel[0]);
	mpeg->preset_audio_mode    (mpeg, pd->mtv_mode[0]);

	pd->replaying = 0;
	pd->can_replay = 0;
	pd->already_prefetch = 0;


	fprintf (stderr, "[+] Playing Mode: [FMP:%d/%d,PCM:%d] %s\n",
			fmp_volume, mpeg_volume, pcm_volume, pt->desc);

	return 0;
}

static int hktv_play (struct holktv_stb_t *self) {
	struct thread_svc_t	*thr = self->pd.thr;

	// reset prefer_vocal
	self->pd.prefer_vocal = 0;

	self->pd.can_cut    = 1;
	self->pd.allow_cut  = 0;
	self->pd.in_playing = 1;
	self->pd.terminate  = 0;
	self->pd.replaying  = 0;

	thr->start (thr);
	return 1;
}

static int hktv_replay (struct holktv_stb_t *self) {
	if (self->pd.can_replay) {
		// FIXME: I hope this work ....
		// if (self->pd.already_prefetch) return 0;

		self->pd.replaying = 1;
		return 1;
	}

	return 0;
}

static int hktv_is_playing (struct holktv_stb_t *self) {
	return self->pd.in_playing ? 1 : 0;
}

static int hktv_cut (struct holktv_stb_t *self) {
	struct holktv_stb_pd_t		*pd   = &self->pd;
	struct thread_svc_t		*thr  = pd->thr;
	struct stream_mpeg_t		*mpeg = pd->mpeg;
	struct stream_mp3_t		*mp3  = pd->mp3;
	struct lyrics_osd_t		*lyr  = pd->lyr;
	int				rc = 0;

	if (! pd->allow_cut) return 0;

	while (self->pd.in_playing) {
		pd->terminate = 1;

		while (pd->can_cut == 1) usleep (1); // play but not start play
		if (pd->can_cut == 0) break; // no play at all !
		// mpeg->abort (mpeg);

		while (pd->can_cut == 2) usleep (1); // after mpeg play
		// if (pd->can_cut == 0) break;
		mp3->abort  (mp3);

		while (pd->can_cut == 3) usleep (1); // after mp3 play
		// if (pd->can_cut == 0) break;
		lyr->abort  (lyr);
		while (lyr->is_running (lyr)) usleep (1);

		mpeg->abort (mpeg);
		thr->stop   (thr);
		rc = 1;
		break;
	}

	return rc;
}

static int hktv_wait (struct holktv_stb_t *self) {
	struct holktv_stb_pd_t	*pd = &self->pd;
	int			rc = 0;

	// fprintf (stderr, "Entering to hktv_wait\n");

	pthread_mutex_lock (&pd->mutex);

	if (pd->in_playing) {
		pthread_cond_wait (&pd->condi, &pd->mutex);
	}

	if (! pd->in_playing) {
		rc = 1;
	} else if (pd->endof_prefetch) {
		rc = 2;
	}

	pthread_mutex_unlock (&pd->mutex);

	// fprintf (stderr, "Return from hktv_wait\n");

	return rc;
}

static void holktv_streamming_delay (struct holktv_stb_t *self,
					int *dof, int *dor, int *dos) {
	struct holktv_stb_pd_t	*pd = &self->pd;
	int			*ptr[4] = { NULL, dof, dor, dos };
	int			i;

	for (i = 1; i <= 3; i++) {
		if (ptr[i] != NULL) {
			*ptr[i] = pd->client->set_delay (
						pd->client, i, *ptr[i]);
		}
	}	
}

#if ENABLE_VOCAL_FADER == 1

static void fade_in (struct holktv_stb_pd_t *pd, const int channel) {
	int	i, j, vol;

	// fprintf (stderr, "[*] Fade In (%d)[%d]\n", channel,
	//				pd->volume_setting.src[channel]);

	for (i = 0, j = 1; i < HOLKTV_FADE_STEP; i++, j <<= 1)
		;

	for (i = 0; i < HOLKTV_FADE_STEP; i++, j >>= 1) {
		if ((vol = pd->volume_setting.src[channel] - j) > 0) {
			pd->mixer->setvolume (volume_mapping[channel], vol);
			usleep (HOLKTV_FADE_SLEEP);
		}
	}

	pd->mixer->setvolume (volume_mapping[channel],
					pd->volume_setting.src[channel]);
}

static void fade_out (struct holktv_stb_pd_t *pd, const int channel) {
	int	i, j, vol;

	// fprintf (stderr, "[*] Fade Out (%d)[%d]\n", channel,
	//				pd->volume_setting.src[channel]);

	for (i = 0, j = 1; i < HOLKTV_FADE_STEP; i++, j <<= 1) {
		if ((vol = pd->volume_setting.src[channel] - j) <= 0) break;
		pd->mixer->setvolume (volume_mapping[channel], vol);
		usleep (HOLKTV_FADE_SLEEP);
	}

	pd->mixer->setvolume (volume_mapping[channel], 0);
}

static int hktv_fader (void *selfptr) {
	struct holktv_stb_t		*self = selfptr;
	struct holktv_stb_pd_t		*pd   = &self->pd;
	struct stream_mp3_t		*mp3  = pd->mp3;
	struct stream_mpeg_t		*mpeg = pd->mpeg;
	// struct lyrics_osd_t		*lyr  = pd->lyr;
	// struct holktv_playtype_t	*pt   = pd->pltype;

	fprintf (stderr, "[*] Vocal/Music Fade In/Out\n");

	while (! pd->close_fader) {
		pthread_mutex_lock (&pd->mutex_fader);
		pthread_cond_wait (&pd->condi_fader, &pd->mutex_fader);
		pthread_mutex_unlock (&pd->mutex_fader);

		switch (pd->in_fade) {
		case 1:
			pd->mixer->close_lock (1);

			fade_out (pd, HOLKTV_VOL_ID_PCM);
			fade_in  (pd, HOLKTV_VOL_ID_CDIN);

			pd->mixer->close_lock (0);
			pd->mixer->close ();
			break;
		case 2:
			pd->mixer->close_lock (1);

			fade_out (pd, HOLKTV_VOL_ID_CDIN);
			fade_in  (pd, HOLKTV_VOL_ID_PCM);

			pd->mixer->close_lock (0);
			pd->mixer->close ();
			break;
		case 3:
			pd->mixer->close_lock (1);

			mp3->select (mp3, 0);
			usleep (1);
			fade_out (pd, HOLKTV_VOL_ID_PCM);
			usleep (200000);
			fade_in  (pd, HOLKTV_VOL_ID_PCM);

			pd->mixer->close_lock (0);
			pd->mixer->close ();
			break;
		case 4:
			pd->mixer->close_lock (1);

			mp3->select (mp3, 1);
			usleep (1);
			fade_out (pd, HOLKTV_VOL_ID_PCM);
			usleep (200000);
			fade_in  (pd, HOLKTV_VOL_ID_PCM);

			pd->mixer->close_lock (0);
			pd->mixer->close ();
			break;
		case 5:
		case 6:
			pd->mixer->close_lock (1);

			fade_out (pd, HOLKTV_VOL_ID_CDIN);

			if (pd->mtv_channel[0] != pd->mtv_channel[1]) {
				mpeg->audio_channel (mpeg,
					pd->mtv_channel[pd->in_fade - 5]);
			}

			if (pd->mtv_mode[0] != pd->mtv_mode[1]) {
				mpeg->audio_mode (mpeg,
					pd->mtv_mode[pd->in_fade - 5]);
			}

			fade_in  (pd, HOLKTV_VOL_ID_CDIN);

			pd->mixer->close_lock (0);
			pd->mixer->close ();
			break;
		}

		pd->in_fade = 0;
	}

	return 1;
}
#endif

static int prefetch_ok (void *selfptr) {
	struct holktv_stb_t		*self = selfptr;
	struct holktv_stb_pd_t		*pd   = &self->pd;

	// fprintf (stderr, "PRE-fetch of MPEG seems ok\n");
	pthread_mutex_lock (&pd->mutex);
	pd->endof_prefetch = 1;
	pthread_cond_broadcast (&pd->condi);
	pthread_mutex_unlock (&pd->mutex);

	return 1;
}

static int hktv_play_main (void *selfptr) {
	struct holktv_stb_t		*self = selfptr;
	struct holktv_stb_pd_t		*pd   = &self->pd;
	struct stream_mpeg_t		*mpeg = pd->mpeg;
	struct stream_mp3_t		*mp3  = pd->mp3;
	struct lyrics_osd_t		*lyr  = pd->lyr;
	struct holktv_playtype_t	*pt   = pd->pltype;
	struct stio_engine_t		*stio;


	stio = mpeg->stio_engine (mpeg);
	pd->allow_cut = 0;
	pd->endof_prefetch = 0;

	fprintf (stderr, "Holiday KTV Player Main thread Started !!\n");

	if (pt->mpeg == 0) {
		mpeg->dummy_start (mpeg);
	} else {
		mpeg->start (mpeg);

		if (! pt->lyrics) {
			// 原聲原影 ....
			stio->set_eop_callback (stio, prefetch_ok, selfptr);
		}
	}

	pd->can_cut = 2;

	if (mpeg->wait_for_playing (mpeg)) {
		if (pt->mp3    != 0) mp3->start (mp3);
		pd->can_cut = 3;
		if (pt->lyrics != 0) lyr->start (lyr);
		pd->can_cut = 4;


		usleep (100);	// Let the music play

		pd->can_replay = 1;

		pd->allow_cut = 1;
		while (! pd->terminate) {
			if (pd->replaying) {
				// FIXME 如果預載下首歌時 .... 會有問題
				pd->replaying = 0;

				if (pt->lyrics) {
					// FIXME: 關於同步 ....
					lyr->reset (lyr);
					mp3->pause (mp3);
					mp3->reset (mp3);
					usleep (1);
					mp3->start (mp3);
					lyr->start (lyr);
					// Very easy to crash
				} else {
					if (pt->mp3) {
						// FIXME: 關於同步 ....
						mp3->pause (mp3);
						mp3->reset (mp3);
						usleep (1);
						mpeg->reset (mpeg);
						mp3->start (mp3);
					} else {
						mpeg->reset (mpeg);
					}
				}
			}

			if (pt->lyrics) {
				if (! mp3->is_running (mp3)) break;
			} else {
				if (! mpeg->is_playing (mpeg)) break;
			}
			usleep (200000);	// 0.2 sec
		}
		pd->can_replay = 0;
	}

	if (pt->lyrics != 0) lyr->stop (lyr);
	if (pt->mp3    != 0) mp3->stop (mp3);

	mpeg->stop  (mpeg);
	mpeg->close (mpeg);

	pd->can_cut   = 0;
	pd->allow_cut = 0;

	pthread_mutex_lock (&pd->mutex);
	pd->in_playing = 0;
	pthread_cond_broadcast (&pd->condi);
	pthread_mutex_unlock (&pd->mutex);

	stio->set_eop_callback (stio, NULL, NULL);
	fprintf (stderr, "Holiday KTV Player Main thread Terminated !!\n");
	return 1;
}

static char * hktv_server_ip (struct holktv_stb_t *self) {
#if USING_STREAMIO_VERSION == 2
	struct streamming_io_v2_client_t	*client = self->pd.client;
#elif USING_STREAMIO_VERSION == 3
	struct streamming_io_v3_client_t	*client = self->pd.client;
#endif
	char					*ip;
	int					*port;

	client->get_server (client, &ip, &port);

	return ip;
}

static int hktv_server_port (struct holktv_stb_t *self) {
#if USING_STREAMIO_VERSION == 2
	struct streamming_io_v2_client_t	*client = self->pd.client;
#elif USING_STREAMIO_VERSION == 3
	struct streamming_io_v3_client_t	*client = self->pd.client;
#endif
	char					*ip;
	int					*port;

	client->get_server (client, &ip, &port);

	return *port;
}

static int hktv_server_id (struct holktv_stb_t *self) {
#if USING_STREAMIO_VERSION == 2
	struct streamming_io_v2_client_t	*client = self->pd.client;
#elif USING_STREAMIO_VERSION == 3
	struct streamming_io_v3_client_t	*client = self->pd.client;
#endif

	return client->get_server_id (client);
}

static void hktv_lyr_rgb (struct holktv_stb_t *self, const int isdefault) {
	struct lyrics_osd_t		*lyr  = self->pd.lyr;

	lyr->rgb (lyr, isdefault);
}

static void hktv_lyr_palette (struct holktv_stb_t *self,
			const int singer, const int channel,
			const int yy, const int uu, const int vv) {
	struct lyrics_osd_t		*lyr  = self->pd.lyr;

	lyr->set_palette (singer, channel, yy, uu, vv);
}

static void hktv_dispose (struct holktv_stb_t *self) {
	struct holktv_stb_pd_t	*pd = &self->pd;

#if ENABLE_VOCAL_FADER == 1
	pd->in_fade = 0;
	pd->close_fader = 1;

	pthread_mutex_lock   (&pd->mutex_fader);
	pthread_cond_signal  (&pd->condi_fader);
	pthread_mutex_unlock (&pd->mutex_fader);

	if (pd->fader  != NULL) pd->fader->dispose  (pd->fader);
#endif
	if (pd->thr    != NULL) pd->thr->dispose    (pd->thr);
	if (pd->mpeg   != NULL) pd->mpeg->dispose   (pd->mpeg);
	if (pd->lyr    != NULL) pd->lyr->dispose    (pd->lyr);
	if (pd->mp3    != NULL) pd->mp3->dispose    (pd->mp3);
	if (pd->client != NULL) pd->client->dispose (pd->client);
	if (pd->mixer  != NULL) pd->mixer->close ();
}

static struct stream_mpeg_t * hktv_get_mpeg (struct holktv_stb_t *self) {
	return self->pd.mpeg;
}

static int hktv_enable_public_mpeg (struct holktv_stb_t *self, const int onof) {
	struct stream_mpeg_t	*mpeg = self->pd.mpeg;

	return mpeg->set_public (mpeg, onof);
}

static int hktv_set_public_mpeg (struct holktv_stb_t *self,
						const char *fname) {
	struct stream_mpeg_t	*mpeg = self->pd.mpeg;

	self->set_pmpeg_decrypt (self, 1);

	if (mpeg->open_public (mpeg, fname) < 0) return 0;

	return 1;
}

struct holktv_stb_t * new_holktv_stb (const int port, const int mpegbuff) {
	struct holktv_stb_t	*self;
	struct holktv_stb_pd_t	*pd;

	if ((self = malloc (sizeof (struct holktv_stb_t))) != NULL) {
		pd		= &self->pd;

		self->dispose		= hktv_dispose;
		self->setkey		= hktv_setkey;
		self->clearkey		= hktv_clearkey;
		self->mpeg_decrypt	= hktv_mpeg_decrypt;
		self->pmpeg_decrypt	= hktv_pubmpeg_decrypt;
		self->mp3_decrypt[0]	= hktv_mp3_1_decrypt;
		self->mp3_decrypt[1]	= hktv_mp3_2_decrypt;
		self->bind_channel	= hktv_bind_channel;
		self->set_mp3_prefetch	= hktv_set_mp3_prefetch;
		self->set_mpeg_minplay	= hktv_set_mpeg_minplay;
		self->set_mpeg_decrypt	= hktv_set_mpeg_decrypt;
		self->set_pmpeg_decrypt	= hktv_set_pubmpeg_decrypt;
		self->set_mp3_decrypt	= hktv_set_mp3_decrypt;
		self->prepare		= hktv_prepare;
		self->pq_prefetch	= hktv_pq_prefetch;
		self->play		= hktv_play;
		self->replay		= hktv_replay;
		self->is_playing	= hktv_is_playing;
		self->cut		= hktv_cut;
		self->wait_condition	= hktv_wait;
		self->get_mpeg_status	= hktv_get_mpeg_status;
		self->load_volume	= hktv_load_volume;
		self->save_volume	= hktv_save_volume;
		self->set_system_volume	= hktv_set_system_volume;
		self->set_volume_level	= hktv_set_volume_level;
		self->get_volume_level	= hktv_get_volume_level;
		self->volume_switch	= hktv_volume_switch;
		self->cdpcm		= hktv_cdpcm;
		self->prefer_vocal	= hktv_prefer_vocal;
		self->in_fmp		= hktv_in_fmp;
		self->server_ip		= hktv_server_ip;
		self->server_port	= hktv_server_port;
		self->server_id		= hktv_server_id;

		self->streamming_delay	= holktv_streamming_delay;
		self->lyr_rgb		= hktv_lyr_rgb;
		self->lyr_palette	= hktv_lyr_palette;

		self->cmd_regist	= hktvcmd_regist;
		self->command_processing= hktvcmd_main;

		self->get_mpeg		= hktv_get_mpeg;

		self->set_keep_buffer	= hktv_set_keep_buffer;
		self->set_pq_buffer	= hktv_set_pq_buffer;

		self->enable_public_mpeg = hktv_enable_public_mpeg;
		self->set_public_mpeg	= hktv_set_public_mpeg;

		pd->client	= NULL;
		pd->mp3		= NULL;
		pd->mpeg	= NULL;
		pd->lyr		= NULL;
		pd->thr		= NULL;
		pd->fader	= NULL;
		pd->mixer	= NULL;
		pd->have_key	= 0;
		pd->new_file[0]	= 1;
		pd->new_file[1]	= 1;
		pd->new_file[2]	= 1;
		pd->new_file[3]	= 1;
		pd->hol_idx[0]	= 0;
		pd->hol_idx[1]	= 0;
		pd->hol_idx[2]	= 0;
		pd->hol_idx[3]	= 0;
		pd->in_playing	= 0;
		pd->terminate	= 0;
		pd->minplay	= 30;
		pd->prefer_vocal= 0;
		pd->nbuf	= mpegbuff;
		pd->pltype	= &playtype[0];

		pd->mtv_channel[0] = 0;
		pd->mtv_channel[1] = HKTV_CHANNEL_NONE;
		pd->mtv_mode[0]    = 'S';
		pd->mtv_mode[1]    = 'S';

		pd->in_fade	= 0;
		pd->close_fader = 0;
		pd->replaying   = 0;
		pd->can_replay	= 0;

		pd->can_cut	= 0;
		pd->allow_cut	= 0;

		pd->endof_prefetch = 0;
		pd->already_prefetch = 0;

		memcpy (&pd->volume_setting, &default_volume_level,
						sizeof default_volume_level);
		/*
		memcpy (&pd->volume_running, &default_volume_level,
						sizeof default_volume_level);
		*/

		pthread_mutex_init (&pd->mutex, NULL);
		pthread_cond_init  (&pd->condi, NULL);

#if ENABLE_VOCAL_FADER == 1
		pthread_mutex_init (&pd->mutex_fader, NULL);
		pthread_cond_init  (&pd->condi_fader, NULL);
#endif

#if USING_STREAMIO_VERSION == 2
		if ((pd->client = new_streamming_io_v2_client (port)) == NULL) {
			self->dispose (self);
			return NULL;
		}
#elif USING_STREAMIO_VERSION == 3
		if ((pd->client = new_streamming_io_v3_client (
						port, 0)) == NULL) {
			self->dispose (self);
			return NULL;
		}

		pd->client->start (pd->client);
#endif

		if ((pd->mpeg = new_stream_mpeg (mpegbuff)) == NULL) {
			self->dispose (self);
			return NULL;
		} else {
			pd->mpeg->set_vol_cbk (
					pd->mpeg,
					set_original_mpeg_volume, self);
			pd->mpeg->set_mix_cbk (
					pd->mpeg,
					set_delay_mpeg_volume, self);
		}

		if ((pd->mp3 = new_stream_mp3 ()) == NULL) {
			self->dispose (self);
			return NULL;
		}


		if ((pd->lyr = new_lyrics_osd (
				pd->mpeg->chip_id (pd->mpeg) == 0 ? 0 : 1
						)) == NULL) {
			self->dispose (self);
			return NULL;
		}

		if ((pd->thr = new_thread_svc (hktv_play_main, self)) == NULL) {
			self->dispose (self);
			return NULL;
		}

#if ENABLE_VOCAL_FADER == 1
		if ((pd->fader = new_thread_svc (hktv_fader, self)) == NULL) {
			self->dispose (self);
			return NULL;
		} else {
			pd->fader->start (pd->fader);
		}
#endif

		pd->mixer = new_aumixer ();

		self->set_system_volume (self);
		/*
		for (i = 0; i < VOLUME_LEVEL_SOURCE; i++) {
			if (volume_mapping[i] >= 0) {
				pd->mixer->setvolume (volume_mapping[i],
						pd->volume.src[i]);
			}
		}

		pd->mixer->close ();
		*/

		/*
		{
			struct volume_level_t	vtest;
			int			i;

			for (i = 0; i < VOLUME_LEVEL_SOURCE; i++) {
				vtest.src[i] = i + 1;
			}

			if (vtest.fmp != vtest.src[0]) {
				fprintf (stderr, "FMP ERROR\n");
			}
			if (vtest.main != vtest.src[1]) {
				fprintf (stderr, "MAIN ERROR\n");
			}
			if (vtest.linein != vtest.src[2]) {
				fprintf (stderr, "LINEIN ERROR\n");
			}
			if (vtest.lineout != vtest.src[3]) {
				fprintf (stderr, "LINEOUT ERROR\n");
			}
			if (vtest.cdin != vtest.src[4]) {
				fprintf (stderr, "CDIN ERROR\n");
			}
			if (vtest.pcm != vtest.src[5]) {
				fprintf (stderr, "PCM ERROR\n");
			}
			if (vtest.mic != vtest.src[6]) {
				fprintf (stderr, "MIC ERROR\n");
			}

			fprintf (stderr, "End of testing\n");
		}
		*/

		fprintf (stderr, "[*] HOLKTV Library Engine Version %4.2f\n",
				(float) katmai_version () / 100.);
	}

	return self;
}
