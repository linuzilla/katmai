#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "conio.h"
#include "stream_mpeg.h"
#include "holktv_stb.h"
#include "lyrics_osd.h"
#include "stio_engine.h"
#include "streamio_v2.h"
#include "stream_mp3.h"

int main (int argc, char *argv[]) {
	struct streamming_io_v2_client_t	*client;
	struct stream_mpeg_t	*mpeg;
	struct lyrics_osd_t	*lyr;
	struct consoleIO_t	*conio;
	struct holktv_stb_t	*stb;
	int			i, next, quit;
	int			sel = 0;
	struct stream_mp3_t	*mp3;
	struct stio_engine_t	*stio;
	char			*lyr_kok = "KOK/1234-2.kok";
	char			*mp3_chanel_0 =
					"/home/cc/liujc/MP3/stories/08.mp3";
	char			*mp3_chanel_1 =
					"/home/cc/liujc/MP3/stories/01.mp3";

	if ((client = new_streamming_io_v2_client (6502)) == NULL) {
		perror (NULL);
		exit (-1);
	}

	if ((mp3 = new_stream_mp3 ()) == NULL) {
		perror ("new_stream_mp3");
		exit (1);
	} else {
		stio = mp3->stio_engine (mp3, 0);
		stio->prefetch (stio, 30, 32768);
		client->regist_stio (client, 1, stio);

		stio = mp3->stio_engine (mp3, 1);
		stio->prefetch (stio, 30, 32768);
		client->regist_stio (client, 2, stio);
	}

	if ((lyr = new_lyrics_osd ()) == NULL) {
		perror ("new_lyrics_osd");
		exit (1);
	} else {
		client->regist_stio (client, 3, lyr->stio_engine (lyr));
	}

	if ((stb = new_holktv_stb ()) == NULL) {
		perror ("new_holktv_stb");
		return 1;
	} else {
		unsigned char   *keystr =
				"We're planning to holiday at the Holiday";

		stb->setkey (stb, keystr, strlen (keystr));
	}

	if ((mpeg = new_stream_mpeg (1000)) == NULL) {
		perror ("new_stream_mpeg");
		return 1;
	} else {
		stio = mpeg->stio_engine (mpeg);
		stio->set_read_plugin (stio, stb->decode, stb);
		fprintf (stderr, "Set plugin ok\n");
		client->regist_stio (client, 0, stio);

		mpeg->set_minplay (mpeg, 100);
	}

	conio = new_consoleIO ();
	conio->open ();

	while (argc > 1) {
		for (i = 1; i < argc; i++) {
			if (mpeg->open (mpeg, argv[i])) {
				fprintf (stderr, "Open %s\n", argv[i]);

				next = quit = 0;

				mp3->init (mp3, 0, mp3_chanel_0);
				mp3->init (mp3, 1, mp3_chanel_1);
				mp3->select (mp3, sel = 0);
				lyr->init (lyr, lyr_kok);

				mp3->run (mp3);

				mpeg->start (mpeg);

				mpeg->wait_for_playing (mpeg);
				mp3->start (mp3);
				lyr->start (lyr);

				while (mpeg->is_playing (mpeg)) {
					if (conio->kbhit ()) {
						switch (conio->read ()) {
						case 'q':
							next = quit = 1;
							break;
						case 'n':
							next = 1;
							break;
						case 's':
							sel = (sel + 1) % 2;
							mp3->select (mp3, sel);
							break;
						}
					}
					if (next) break;
				}

				lyr->stop (lyr);
				mp3->stop (mp3);
				mpeg->stop (mpeg);
				mpeg->close (mpeg);

				if (conio->kbhit ()) {
					switch (conio->read ()) {
					case 'q':
						next = quit = 1;
						break;
					case 'n':
						next = 1;
						break;
					}
				}
				if (quit) break;
			}
			if (quit) break;
		}
		if (quit) break;
	}

	conio->close ();

	mp3->dispose (mp3);
	mpeg->dispose (mpeg);
	return 0;
}
