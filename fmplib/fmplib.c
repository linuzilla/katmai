/*
 *	fmplib.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <sys/time.h> 
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "fmplib.h"

#define _DEFINE_FMP_TYPES_
#include "fmp.h"
#include "ifmp.h"

enum fmp_func_t {
	FF_NOTHING,
	FF_MPEGDriverEntry,
	FF_MPEGDriverUnload,
	FF_FMPOpen,
	FF_FMPClose,
	FF_FMPPush,
	FF_FMPPause,
	FF_FMPPlay,
	FF_FMPStop,
	FF_FMPGetBuffer,
	FF_FMPSet
};

static int		em84xx_id = 0;
static short		driver_loaded = 0;
static short		mpeg_playing  = 0;
static FMP_BUFFER	FMPBuf;
static char		FMPBuf_pBuffer[32768];
static enum fmp_func_t	in_fmp_state = FF_NOTHING;
static struct timeval	tv;

static void entering_fmp_driver (const enum fmp_func_t what) {
	in_fmp_state = what;
	gettimeofday (&tv, (struct timezone *) 0);
};

static void leaving_fmp_driver (void) {
	in_fmp_state = FF_NOTHING;
};

static int fmp_is_infmp (struct timeval *tdiff) {
	struct timeval	tnow;

	if (in_fmp_state == FF_NOTHING) return 0;

	gettimeofday (&tnow, (struct timezone *) 0);

	tdiff->tv_sec  = tnow.tv_sec  - tv.tv_sec;
	tdiff->tv_usec = tnow.tv_usec - tv.tv_usec;

	if (tdiff->tv_usec < 0) {
		tdiff->tv_sec--;
		tdiff->tv_usec += 1000000;
	}

	return 1;
}

static int check_if_em84xx_exists (void) {
	FILE	*fp;
	char	buffer[4096];
	char	*pci_info = "/proc/pci";
	char    *ptr;
	int	rc = 0;

	if ((fp = fopen (pci_info, "r")) != NULL) {
		while (fgets (buffer, sizeof buffer, fp) != NULL) {
			if ((strstr (buffer, "Multimedia controller") != NULL)
				&& (strstr (buffer, "Sigma Designs") != NULL)) {
				if ((ptr = strstr (buffer,
							"1105:84")) != NULL) {
					rc = 8400
						+ (ptr[7] - '0') * 10
						+ (ptr[8] - '0');
					break;
				}
			}
		}
		fclose (fp);
	}

	return rc;
}

static int fmp_driver_entry (void) {
	int	ok = 0;
	int	rc;

	if (driver_loaded) return 1;

	if (em84xx_id != 0) {
		while (1) {
			entering_fmp_driver (FF_MPEGDriverEntry);
			rc = MPEGDriverEntry (NO_DRIVE);
			leaving_fmp_driver ();

			if (rc == FMPE_OK) break;

			switch (rc) {
			case FMPE_ENTRY_MOD_INIT_FAILED:
				fprintf (stderr, "MPEGDriverEntry: "
						"Module loader failed "
						"to initialize\n");
				break;
			case FMPE_ENTRY_HWL_INIT_FAILED:
				fprintf (stderr, "MPEGDriverEntry: "
						"Fail to create MPEG Hardware "
						"driver");
				break;
			case FMPE_ENTRY_DVDDEV_INIT_FAILED:
				fprintf (stderr, "MPEGDriverEntry: "
						"Failed to create DVD-ROM "
						"device driver\n");
				break;
			default:
				fprintf (stderr, "MPEGDriverEntry: "
						"Unknow error (%d)\n", rc);
				break;
			}

			sleep (1);
		}

		if (rc == FMPE_OK) {
			fprintf (stderr, "[*] MPEGDriverEntry: OK\n");
			driver_loaded = 1;
			ok = 1;
		}
	} else {
		ok = 1;
	}

	return ok;
}

static void fmp_driver_unload (void) {
	if (driver_loaded) {
		entering_fmp_driver (FF_MPEGDriverUnload);
		MPEGDriverUnload ();
		leaving_fmp_driver ();
		fprintf (stderr, "[*] MPEGDriverUnload: OK\n");

		driver_loaded = 0;
	}
}

static void fmp_set_volume (const int vol) {
	if (! driver_loaded) return;
	if (em84xx_id != 0) {
		entering_fmp_driver (FF_FMPSet);
		FMPSet (FMPI_LEFT_VOLUME,  vol);
		FMPSet (FMPI_RIGHT_VOLUME, vol);
		leaving_fmp_driver ();
	}
}

static void fmp_set_audio_mode (const enum fmp_audio_mode_t mode) {
	int	amode = FMPV_AUDIO_MODE_STEREO;

	switch (mode) {
	case FMP_AM_STEREO:
		amode = FMPV_AUDIO_MODE_STEREO;
		break;
	case FMP_AM_RIGHT:
		amode = FMPV_AUDIO_MODE_RIGHT_ONLY;
		break;
	case FMP_AM_LEFT:
		amode = FMPV_AUDIO_MODE_LEFT_ONLY;
		break;
	case FMP_AM_MIX:
		amode = FMPV_AUDIO_MODE_MONOMIX;
		break;
	}

	if (! driver_loaded) return;
	if (em84xx_id != 0) {
		entering_fmp_driver (FF_FMPSet);
		FMPSet (FMPI_AUDIO_MODE, amode);
		leaving_fmp_driver ();
	}
}

static void fmp_set_audio_channel (const int channel) {
	if (! driver_loaded) return;
	if (em84xx_id != 0) {
		entering_fmp_driver (FF_FMPSet);
		FMPSet (FMPI_AUDIO_SELECT, channel);
		leaving_fmp_driver ();
	}
}

static int fmp_open (const enum fmp_stream_t stream) {
	int	rc = 0;
	int	streamtype = FMPF_SYSTEM;

	if (em84xx_id == 0) return 1;

	if (! driver_loaded) {
		if (! fmp_driver_entry ()) return 1;
	}

	switch (stream) {
	case FMP_SYSTEM_STREAM:
		streamtype =  FMPF_SYSTEM;
		break;
	case FMP_PROGRAM_STREAM:
		streamtype =  FMPF_PROGRAM;
		break;
	case FMP_TRANSPORT_STREAM:
		streamtype =  FMPF_TRANSPORT;
		break;
	case FMP_VIDEO_STREAM:
		streamtype =  FMPF_VIDEO;
		break;
	case FMP_UNKNOW_STREAM:
		streamtype =  FMPF_SYSTEM;
		break;
	}

	while (1) {
		entering_fmp_driver (FF_FMPOpen);
		rc = FMPOpen (streamtype, 32 * 1024, 10, NULL, 0);
		leaving_fmp_driver ();

		if (rc == FMPE_OK) break;

		switch (rc) {
		case FMPE_DRIVER_OPEN:
			fprintf (stderr, "FMPOpen: Driver Already Open\n");
			break;
		case FMPE_CANNOT_OPEN_DRIVER:
			fprintf (stderr, "FMPOpen: Cannot open driver\n");
			break;
		case FMPE_NOT_ENOUGH_MEMORY:
			fprintf (stderr, "FMPOpen: No more memory available\n");
			break;
		default:
			fprintf (stderr, "FMPOpen: Unknow error %d\n", rc);
			break;
		}

		entering_fmp_driver (FF_FMPClose);
		FMPClose ();
		leaving_fmp_driver ();

		sleep (1);
	}

	return rc;
}

static void fmp_play (void) {
	if (! driver_loaded) return;

	if (em84xx_id != 0) {
		entering_fmp_driver (FF_FMPPlay);
		FMPPlay ();
		leaving_fmp_driver ();
		mpeg_playing = 1;
	}
}

static void fmp_stop (void) {
	if (! driver_loaded) return;

	if (em84xx_id != 0) {
		entering_fmp_driver (FF_FMPStop);
		FMPStop ();
		mpeg_playing = 0;
		leaving_fmp_driver ();
	}
}

static void fmp_pause (void) {
	if (! driver_loaded) return;

	if (em84xx_id != 0) {
		entering_fmp_driver (FF_FMPPause);
		FMPPause ();
		leaving_fmp_driver ();
	}
}

static void fmp_close (void) {
	if (! driver_loaded) return;

	if (em84xx_id != 0) {
		if (mpeg_playing) fmp_stop ();

		entering_fmp_driver (FF_FMPClose);
		FMPClose ();
		leaving_fmp_driver ();
	}
}

static int fmp_get_buffer (void) {
	int	rc;

	if (em84xx_id == 0) return 1;
	if (! driver_loaded) return 0;

	entering_fmp_driver (FF_FMPGetBuffer);
	rc = FMPGetBuffer (&FMPBuf, TRUE);
	leaving_fmp_driver ();

	if (rc != FMPE_OK) {
		fprintf (stderr, "FMPError : %s\n", FMPGetLastError (rc));
		return 0;
	}

	return 1;
}

static unsigned long fmp_buffer_size (void) { return FMPBuf.dwBufferSize; }
static void fmp_set_data_size (unsigned long ds) { FMPBuf.dwDataSize = ds; }
static void * fmp_buffer_ptr (void) { return FMPBuf.pBuffer; }

static void fmp_push (void) {
	if (em84xx_id != 0) {
		if (! driver_loaded) return;

		entering_fmp_driver (FF_FMPPush);
		FMPPush (&FMPBuf);
		leaving_fmp_driver ();
	} else {
		usleep (58000);
		/*
		struct timespec	req;
		struct timespec	rem;

		req.tv_sec = 0;
		req.tv_nsec = 1000000;

		while (nanosleep (&req, &rem) < 0) {
			req.tv_sec  = rem.tv_sec;
			req.tv_nsec = rem.tv_nsec;
		}
		*/
	}
}

static void fmp_disable (void) {
	em84xx_id = 0;

	FMPBuf.pBuffer      = FMPBuf_pBuffer;
	FMPBuf.dwBufferSize = sizeof FMPBuf_pBuffer;
}

static int fmp_em84xx_id (void) { return em84xx_id; }

struct fmplib_t *new_fmplib (void) {
	static struct fmplib_t		self;

	fprintf (stderr, "[*] RealMagic detection  : ");
	if ((em84xx_id = check_if_em84xx_exists ()) == 0) {
		FMPBuf.pBuffer      = FMPBuf_pBuffer;
		FMPBuf.dwBufferSize = sizeof FMPBuf_pBuffer;
		fprintf (stderr, "Not present (using emulation)\n");
	} else {
		fprintf (stderr, "EM%d detected\n", em84xx_id);
	}

	self.driver_entry	= fmp_driver_entry;
	self.driver_unload	= fmp_driver_unload;
	self.set_volume		= fmp_set_volume;
	self.set_audio_channel	= fmp_set_audio_channel;
	self.set_audio_mode	= fmp_set_audio_mode;
	self.open		= fmp_open;
	self.close		= fmp_close;
	self.pause		= fmp_pause;
	self.play		= fmp_play;
	self.stop		= fmp_stop;
	self.push		= fmp_push;
	self.disable		= fmp_disable;
	self.get_buffer		= fmp_get_buffer;
	self.buffer_size	= fmp_buffer_size;
	self.buffer_ptr		= fmp_buffer_ptr;
	self.set_data_size	= fmp_set_data_size;
	self.is_infmp		= fmp_is_infmp;
	self.em84xx_id		= fmp_em84xx_id;

	return &self;
}

