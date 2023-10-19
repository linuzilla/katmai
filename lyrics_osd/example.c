/*
 *	osd.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "fmplib.h"
#include "mp3lib.h"
#include "conio.h"
#include "lyrics_osd.h"
#include "stream_mp3.h"
#include "streamio_v2.h"

#include "rmimage256.h"

#define NUMBER_OF_ENTRY	3
static struct {
	char	*music;
	char	*vocal;
	char	*kok;
	int	delay;
} playlist[] = {
	{
		// "/home/cc/liujc/MPEG/Faithfully.mp3",
		"/home/cc/liujc/MPEG/girls.mp3",
		NULL,
		"KOK/1234.kok",
		// "KOK/222222.kok",
		400000
       	},
	{
		"/home/cc/liujc/StairwayToHeaven.mp3",
		NULL,
		// "KOK/sorry3.kok",
		"KOK/123456.kok",
		0
	},
	{
		"/home/cc/liujc/MPEG/kala.mp3",
		"/home/cc/liujc/MPEG/vocal.mp3",
		// "KOK/1234-2.kok",
		"KOK/123456.kok",
		0
	}
};


int main (int argc, char *argv[]) {
	struct streamming_io_v2_client_t	*client;

	struct stream_mp3_t	*mp3;
	struct lyrics_osd_t	*lyr;
	int			plidx = 0;
	int			pause;
	int			step;
	int			next;
	int			streamtype = FMP_PROGRAM_STREAM;
	struct fmplib_t 	*fmp;
	char			*mpegfile = "/home/cc/liujc/MPEG/000001.mpeg";
	int			fd, len;
	int			terminate = 0;
	int			sel, can_select;
	struct consoleIO_t	*conio;
	struct rmimage256_t	*img;
	int			cptr = 0;
	int			cchg = 0;
	unsigned char		aYUV[] = { 0xff, 0, 0, 0 };

	if (0) {
		streamtype = FMP_VIDEO_STREAM;
		//mpegfile = "/home/cc/liujc/MPEG/test4m.m2v";
		mpegfile = "/home/cc/liujc/MPEG/test8m.m2v";
		plidx = 2;
	}
	// mpegfile = "/home/cc/liujc/MPEG/805200.mpeg";

	if ((client = new_streamming_io_v2_client (6502)) == NULL) {
		perror (NULL);
		exit (-1);
	}

//	client->dispose (client);

	if ((mp3 = new_stream_mp3 ()) == NULL) {
		perror ("new_stream_mp3");
		exit (1);
	}

	if ((lyr = new_lyrics_osd (1)) == NULL) {
		perror ("new_lyrics_osd");
		exit (1);
	}

	// lyr->rgb (lyr, 0);

	img = lyr->image (lyr);

	// client->regist_stio (client, 0, mp3->stio_engine (mp3, 0));
	// client->regist_stio (client, 1, mp3->stio_engine (mp3, 1));
	// client->regist_stio (client, 2, lyr->stio_engine (lyr));

	if ((fmp = new_fmplib ()) == NULL) {
		perror ("new_fmplib");
		lyr->dispose (lyr);
		exit (1);
	}

	if ((fd = open (mpegfile, O_RDONLY)) < 0) {
		perror (mpegfile);
		lyr->dispose (lyr);
		exit (1);
	}

	conio = new_consoleIO ();

	do {
		if (! fmp->driver_entry ()) break;

		conio->open ();

		fmp->open (streamtype);
		fmp->set_volume (80);
		fmp->play ();

		while (! terminate) {
			pause = step = next = sel = can_select = 0;

			lyr->init (lyr, playlist[plidx].kok);
			mp3->init (mp3, 0, playlist[plidx].music);

			if (playlist[plidx].vocal != NULL) {
				mp3->init (mp3, 1, playlist[plidx].vocal);
				mp3->select (mp3, 0);
				can_select = 1;
			}

			mp3->run (mp3);

			mp3->start (mp3);
			usleep (playlist[plidx].delay);
			lyr->start (lyr);

			while (! next) {
				if (fmp->get_buffer () == 0) {
					terminate = 1;
					perror ("get_buffer");
					break;
				}

				if ((len = read (fd, fmp->buffer_ptr (),
						fmp->buffer_size ())) <= 0) {
					lseek (fd, 0, SEEK_SET);
					len = read (fd, fmp->buffer_ptr (),
							fmp->buffer_size ());
				}

				fmp->set_data_size (len);
				fmp->push ();

				if (! mp3->is_running (mp3)) next = 1;

				if (conio->kbhit ()) {
					switch (conio->read ()) {
					case 'q':
						next = terminate = 1;
						break;
					case 'n':
						next = 1;
						break;
					case 's':
						if (can_select) {
							sel = (sel + 1) % 2;
							mp3->select (mp3, sel);
						}
						break;
					case '1':
						aYUV[1]++;
						cchg = 1;
						break;
					case '2':
						aYUV[2]++;
						cchg = 1;
						break;
					case '3':
						aYUV[3]++;
						cchg = 1;
						break;
					case '4':
						aYUV[1]--;
						cchg = 1;
						break;
					case '5':
						aYUV[2]--;
						cchg = 1;
						break;
					case '6':
						aYUV[3]--;
						cchg = 1;
						break;
					}
				}

				if (cchg) {
					fprintf (stderr, "[%u,%u,%u]\n",
						aYUV[1], aYUV[2], aYUV[3]);
					img->setPaletteColor (img, cptr, aYUV);
					cchg = 0;
				}
			}
			lyr->stop (lyr);
			mp3->stop (mp3);

			plidx = (plidx + 1) % NUMBER_OF_ENTRY;

			// plidx = 0;
		}
	} while (0);

	mp3->dispose (mp3);
	lyr->dispose (lyr);
	conio->close ();
	fmp->stop ();
	fmp->close ();
	fmp->driver_unload ();

	return 0;
}
