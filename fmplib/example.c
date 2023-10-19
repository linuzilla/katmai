/*
 *	example.c
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
#include "conio.h"
#include "streamio_v2.h"

#define NUMBER_OF_ENTRY	3

int main (int argc, char *argv[]) {
	struct streamming_io_v2_client_t	*client;

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

	if (0) {
		streamtype = FMP_VIDEO_STREAM;
		//mpegfile = "/home/cc/liujc/MPEG/test4m.m2v";
		mpegfile = "/home/cc/liujc/MPEG/test8m.m2v";
		plidx = 2;
	}
	// mpegfile = "/home/cc/liujc/MPEG/805200.mpeg";

	//if ((client = new_streamming_io_v2_client (6502)) == NULL) {
	//	perror (NULL);
	//	exit (-1);
	//}

	// client->regist_stio (client, 0, mp3->stio_engine (mp3, 0));
	// client->regist_stio (client, 1, mp3->stio_engine (mp3, 1));
	// client->regist_stio (client, 2, lyr->stio_engine (lyr));

	if ((fmp = new_fmplib ()) == NULL) {
		perror ("new_fmplib");
		exit (1);
	}

	if ((fd = open (mpegfile, O_RDONLY)) < 0) {
		perror (mpegfile);
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

				if (conio->kbhit ()) {
					switch (conio->read ()) {
					case 'q':
						next = terminate = 1;
						break;
					case 'n':
						next = 1;
						break;
					}
				}
			}

			plidx = (plidx + 1) % NUMBER_OF_ENTRY;
		}
	} while (0);

	conio->close ();
	fmp->stop ();
	fmp->close ();
	fmp->driver_unload ();

	return 0;
}
