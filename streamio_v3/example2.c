#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "streamio_v3.h"


static int	port = 6508;
static int	cliport = 6509;

static int file_open (const int channel, const char *file) {
	char	fname[256];
	switch (channel) {
	case 0:
		sprintf (fname, "%s", file);
		break;
	case 1:
		sprintf (fname, "%s.1", file);
		break;
	case 2:
		sprintf (fname, "%s.2", file);
		break;
	case 3:
		sprintf (fname, "%s.0", file);
		break;
	}
	return open (fname, O_RDONLY);
}

void client (void) {
	struct streamming_io_v3_client_t	*cli;
	char					buffer[32768];
	char					lbuf[32768];
	int					len, llen;
	int					i, j, pos;
	int					fd;
	struct stat				statbuf;
	long					st_size = 0L;
	char	*file = "/home/cc/liujc/FoveonX3/run/805200";

	srand (time (NULL));

	if ((fd = open (file, O_RDONLY)) < 0) {
		perror (file);
		return;
	} else if (fstat (fd, &statbuf) != 0) {
		perror (file);
		return;
	} else {
		st_size = statbuf.st_size;
	}

	if ((cli = new_streamming_io_v3_client (port, cliport)) == NULL) {
		perror (NULL);
		exit (-1);
	}

	cli->start (cli);

	if (cli->openfile (cli, 0, file) >= 0) {
		fprintf (stderr, "Ok, filesize = %ld\n",
				cli->filesize (cli, 0));

		/*
		j = rand () % 100 + 100;

		for (i = 0; i < j; i++) {
			if ((len = cli->read (cli, 0,
						buffer, sizeof buffer)) < 0) {
				break;
			}
		}
		*/

		for (i = 0; i < 100; i++) {
			pos = rand () % (st_size - sizeof buffer);
			pos = pos - pos % 32768;

			// pos = i * 32768 + 32768;

			cli->seek (cli, 0, pos);
			if (lseek (fd, pos, SEEK_SET) < 0) {
				perror ("lseek");
				break;
			}

			len  = cli->read (cli, 0, buffer, sizeof buffer);
			llen = read (fd, lbuf, sizeof lbuf);

			if (len != llen) {
				fprintf (stderr,
					"%d. %d v.s %d Not same size\n",
					i + 1, len, llen);
				break;
			}

			if (memcmp (buffer, lbuf, len) != 0) {
				fprintf (stderr,
					"%d. POS=%d, (len=%d) "
					"buffer not same data "
					"(%x v.s %x)\n",
					i + 1, pos, len, buffer[0], lbuf[0]);
			}
		}

		cli->closefile (cli, 0);
		fprintf (stderr, "File closed\n");
	} else {
		fprintf (stderr, "Open failed\n");
	}

	cli->stop (cli);

	cli->dispose (cli);
	exit (1);
}

void server (void) {
	struct streamming_io_v3_server_t	*svr;
	int					i;

	// sleep (3);

	if ((svr = new_streamming_io_v3_server (20, port, 1)) == NULL) {
		perror (NULL);
		exit (-1);
	}

	svr->set_parameter (svr, 0, 1048576, 16384);
	// svr->set_parameter (svr, 1, 8192, 4096);
	svr->set_fopen (svr, file_open);


	svr->start (svr);
	svr->start_svc (svr);

	for (i = 0; i < 1000; i++) {
		sleep (360);
	}

	svr->dispose (svr);
	fprintf (stderr, "dispose server ok\n");
}

int main (int argc, char *argv[]) {
	int	pid;
	int	status;
	int	only = 0;

	if (argc > 1) {
		if (strcmp (argv[1], "-s") == 0) {
			only = 1;
		} else if (strcmp (argv[1], "-c") == 0) {
			only = 2;
		}
	}

	if (only == 0) {
		if ((pid = fork ()) == 0) {
			client ();
			exit (1);
		} else if (pid > 0) {
			server ();
		} else {
			perror ("fork");
		}
		wait (&status);

		fprintf (stderr, "Exit code = %d\n", WEXITSTATUS(status));
	} else if (only == 1) {
		server ();
	} else if (only == 2) {
		client ();
	}

	return -1;
}
