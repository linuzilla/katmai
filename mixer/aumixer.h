/*
 *	aumixer.h
 *
 *	Copyright (c) 2003, Jainn-Ching Liu
 */

#ifndef __AUMIXER_H__
#define __AUMIXER_H__

enum {
	ENOERROR,
	EOPENMIX,               /* trouble opening mixer device */
	EFINDDEVICE,            /* no device found */
	EREADDEV,               /* SOUND_MIXER_READ_DEVMASK */
	EREADRECMASK,           /* SOUND_MIXER_READ_RECMASK */
	EREADRECSRC,            /* SOUND_MIXER_READ_RECSRC */
	EREADSTEREO,            /* SOUND_MIXER_READ_STEREODEVS */
	EWRITERECSRC,           /* SOUND_MIXER_WRITE_RECSRC */
	EREADMIX,               /* MIXER_READ */
	EWRITEMIX,              /* MIXER_WRITE */
	ENOTOPEN,               /* mixer not open */
	EFILE                   /* unable to open settings file */
};

struct aumixer_t {
	int	(*query_all)(void);
	int	(*query)(const int n, int *rr, int *ll);
	int	(*setvolume)(const int n, const int v);
	char *	(*devname)(const int n);
	void	(*close)(void);
	void	(*close_lock)(const int cl);
};

struct aumixer_t	* new_aumixer (void);

#endif
