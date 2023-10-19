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

static struct playlist_t {
	int	type;
	int	crypt;
	char	*mpeg;
	char	*mp3;
} playlist[] = {
	{ 2, 1, "99812", "555556" }
//	{ 2, 1, "805200", "123456" }
//	{ 0, 1, "805200", NULL }
	// { 3, 0, "000001", "123456" },
	// { 2, 0, "000001", "test" }
};

static void check_key (struct consoleIO_t *conio, int *next, int *terminate) {
	if (conio->kbhit ()) {
		switch (conio->read ()) {
		case 'q':
			*next = *terminate = 1;
			break;
		case 'n':
			*next = 1;
			break;
		}
	}
}

int main (int argc, char *argv[]) {
	struct consoleIO_t	*conio;
	struct holktv_stb_t	*stb;
	int			i;
	int			terminate = 0;
	int			next = 0;
	const int		port = 8088;
	const int		nbuf = 1200;
	int			num;

	num = sizeof playlist / sizeof (struct playlist_t);

	fprintf (stderr, "%d entry\n", num);

	if ((stb = new_holktv_stb (port, nbuf)) == NULL) {
		perror ("new_holktv_stb");
		return 1;
	} else {
		unsigned char   *keystr =
				"We're planning to holiday at the Holiday";

		stb->setkey (stb, keystr, strlen (keystr));
	}

	return 1;

	conio = new_consoleIO ();
	conio->open ();

	stb->set_mpeg_minplay (stb, 100);
	stb->bind_channel (stb, 0);
	stb->bind_channel (stb, 1);
	stb->bind_channel (stb, 3);

	stb->bind_channel (stb, 2);

	while (! terminate) {
		for (i = 0; i < num; i++) {
			next = 0;
			if (playlist[i].crypt) {
				stb->set_mpeg_decrypt (stb, 1);
			} else {
				stb->set_mpeg_decrypt (stb, 0);
			}

			if (stb->prepare (stb, playlist[i].type,
							playlist[i].mpeg,
							playlist[i].mp3) != 0) {
				check_key (conio, &next, &terminate);
				if (terminate) break;
				continue;
			}

			stb->play (stb);

			while (stb->is_playing (stb)) {
				check_key (conio, &next, &terminate);

				if (next) {
					stb->cut (stb);
					break;
				}
			}

			if (terminate) break;
		}
		check_key (conio, &next, &terminate);
	}

	stb->dispose (stb);
	conio->close ();

	return 0;
}
