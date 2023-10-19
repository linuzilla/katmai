/*
 *	aumixer.c
 *
 *	Copyright (c) 2003, Jainn-Ching Liu
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <pthread.h>
#include "aumixer.h"

#define MAXLEVEL	100

#define SOUND_IOCTL(a,b,c)	ioctl(a,b,c)
static char *dev_name[SOUND_MIXER_NRDEVICES] = {
	"Volume  ", "Bass    ", "Treble  ", "Synth   ", "PCM     ",
	"Speaker ", "Line    ", "Mic     ", "CD      ", "Mix     ",
	"PCM 2   ", "Record  ", "Input   ", "Output  ", "Line 1  ",
	"Line 2  ", "Line 3  ", "Digital1", "Digital2", "Digital3",
	"Phone In", "PhoneOut", "Video   ", "Radio   ", "Monitor "
};

// static const char	*moptindx = "vbtswplmcxWrio123";

static int			fd = -1;
static int			in_close_lock = 0;
static int			devmask = 0;
static int			recmask = 0;
static int			stereodevs = 0;
static int			recsrc = 0;
static struct aumixer_t		self;
static pthread_mutex_t		mutex = PTHREAD_MUTEX_INITIALIZER;


static int ReadRecSrc (void) {
	if (SOUND_IOCTL (fd, SOUND_MIXER_READ_RECSRC, &recsrc) == -1)
		return EREADRECSRC;
	return 0;
}

static int WriteLevel (int device, int leftright) {
	if (SOUND_IOCTL (fd, MIXER_WRITE(device), &leftright) == -1)
		return EWRITEMIX;
	return 0;
}

static int ReadLevel (int device, int *leftright) {
	if (SOUND_IOCTL (fd, MIXER_READ(device), leftright) == -1)
		return EREADMIX;
	return 0;
}

static int MixerStatus (void) {
	if (SOUND_IOCTL (fd, SOUND_MIXER_READ_DEVMASK, &devmask) == -1)
		return EREADDEV;

	if (SOUND_IOCTL (fd, SOUND_MIXER_READ_RECMASK, &recmask) == -1)
		return EREADRECMASK;

	if (ReadRecSrc()) return -1;

	if (SOUND_IOCTL (fd, SOUND_MIXER_READ_STEREODEVS, &stereodevs) == -1)
		return EREADSTEREO;

	return 0;
}

static int InitializeMixer (const char *device) {
	const char	*dev = "/dev/mixer";

	if (fd >= 0) return 0;
	if (device != NULL) dev = device;

	if ((fd = open (dev, O_RDWR)) < 0) {
		// trouble opening mixer device
		return EOPENMIX;
	}

	if (MixerStatus()) return -1;
	if (!devmask) return EFINDDEVICE;

	return 0;
}

static int ShowLevel (int dev, int *rl, int *ll, int disp) {
	char	*devstr;
	int	tmp;

	devstr = dev_name[dev];
	if (ReadLevel (dev, &tmp)) return -1;
	*rl = (tmp & 0xFF);
	*ll = (tmp >> 8) & 0xFF;

	if (disp) {
		fprintf (stderr, "%2d. %s %3d, %3d",
					dev, dev_name[dev], *rl, *ll);
	}

	if ((1 << (dev)) & recmask) {
		if (ReadRecSrc()) return -1;
		if (disp) {
			fprintf (stderr, ", %c",
					((1 << dev) & recsrc ? 'R' : 'P'));
		}
	}
	if (disp) fprintf (stderr, "\n");

	return 0;
}

static int query_all (void) {
	int	i, r, l, rc;

	InitializeMixer (NULL);

	for (i = rc = 0; i < SOUND_MIXER_NRDEVICES; i++) {
		if ((1 << i) & (devmask | recmask)) {
			if (ShowLevel (i, &r, &l, 1)) return -1;
			rc++;
		}
	}
	return rc;
}

static int query (const int n, int *rr, int *ll) {
	InitializeMixer (NULL);

	if ((n >= 0) && (n < SOUND_MIXER_NRDEVICES)) {
		if ((1 << n) & (devmask | recmask)) {
			if (ShowLevel (n, rr, ll, 0)) return -1;
			return 1;
		}
	}

	return 0;
}

static void mixer_close_lock (const int lk) {
	pthread_mutex_lock (&mutex);

	if (lk) {
		in_close_lock++;
	} else {
		in_close_lock--;
	}

	pthread_mutex_unlock (&mutex);
}

static void mixer_close (void) {
	if (! in_close_lock) {
		if (fd >= 0) close (fd);
		fd = -1;
	}
}

static char * query_devname (const int n) {
	if ((n >= 0) && (n < SOUND_MIXER_NRDEVICES)) {
		return dev_name[n];
	}

	return NULL;
}

static int setvolume (const int n, const int level) {
	int	tmp;

	InitializeMixer (NULL);

	if ((n >= 0) && (n < SOUND_MIXER_NRDEVICES)) {
		tmp = level;
		tmp = (tmp > MAXLEVEL) ? MAXLEVEL : tmp;
		tmp = (tmp < 0) ? 0 : 257 * tmp;
		WriteLevel (n, tmp);
	}
	return -1;
}

struct aumixer_t * new_aumixer (void) {
	self.query_all	= query_all;
	self.query	= query;
	self.close	= mixer_close;
	self.devname	= query_devname;
	self.setvolume	= setvolume;
	self.close_lock	= mixer_close_lock;

	return &self;
}
