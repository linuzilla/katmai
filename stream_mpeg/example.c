#include <stdio.h>
#include <stdlib.h>

#include "conio.h"
#include "stream_mpeg.h"

int main (int argc, char *argv[]) {
	struct stream_mpeg_t	*mpeg;
	struct consoleIO_t	*conio;
	int			i, next, quit;

	if ((mpeg = new_stream_mpeg (2000)) == NULL) {
		perror ("new_stream_mpeg");
		return 1;
	}

	conio = new_consoleIO ();
	conio->open ();

	for (i = 1; i < argc; i++) {
		if (mpeg->open (mpeg, argv[i])) {
			fprintf (stderr, "Open %s\n", argv[i]);
			mpeg->start (mpeg);

			next = quit = 0;

			while (mpeg->is_playing (mpeg)) {
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
				if (next) break;
			}

			mpeg->stop (mpeg);
			mpeg->close (mpeg);

			if (quit) break;
		}
	}

	conio->close ();

	mpeg->dispose (mpeg);
	return 0;
}
