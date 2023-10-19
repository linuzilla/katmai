#include <stdio.h>
#include <unistd.h>
#include "stream_mp3.h"
#include "conio.h"
#include "stio_engine.h"
#include "streamio_v2.h"

int main (int argc, char *argv[]) {
	struct stream_mp3_t	*mp3;
	struct consoleIO_t	*conio;
	struct stio_engine_t	*stio;
	int			sel = 0;
	int			next;
	int			terminate = 0;
	int			nbuf  = 500;
	int			bsize = 1024;
	struct streamming_io_v2_client_t	*client;

	char	*file[] = {
			// "/home/cc/liujc/MPEG/StairwayToHeaven.mp3",
			"/home/cc/liujc/MP3/tear/01.mp3",
			"/home/cc/liujc/MP3/tear/02.mp3"
			//"/home/cc/liujc/MPEG/kala.mp3",
			//"/home/cc/liujc/MPEG/vocal.mp3"
		};

	if ((client = new_streamming_io_v2_client (6502)) == NULL) {
		perror (NULL);
		return -1;
	}

	if ((mp3 = new_stream_mp3 ()) == NULL) return -1;

	conio = new_consoleIO ();

	if ((stio = mp3->stio_engine (mp3, 1)) != NULL) {
		stio->prefetch (stio, nbuf, bsize);
		client->regist_stio (client, 1, stio);
	}
	if ((stio = mp3->stio_engine (mp3, 0)) != NULL) {
		stio->prefetch (stio, nbuf, bsize);
		client->regist_stio (client, 0, stio);
	}

	conio->open ();

	while (! terminate) {
		next = 0;

		mp3->init(mp3, 1, file[1]);
		mp3->select (mp3, sel);


		if (mp3->init (mp3, 0, file[0])) {

			mp3->run (mp3);
			fprintf (stderr, "Ready to start ...\n");
			mp3->start (mp3);

			while (mp3->is_running (mp3)) {
				sleep (1);
				fprintf (stderr, "\r%d/%d (%d)",
						stio->freebuf (stio),
						nbuf, bsize);

				if (conio->kbhit ()) {
					switch (conio->read ()) {
					case 'q':
						next = terminate = 1;
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
		} else {
			break;
		}
		mp3->stop (mp3);
	}

	conio->close ();

	mp3->dispose (mp3);
	return 0;
}
