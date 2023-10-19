#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "streamio_v2.h"

static int	port = 6502;

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
	struct streamming_io_v2_client_t	*cli;
	char					buffer[65536];
	int					len;
	int					i;
	// char	*file = "/home/cc/liujc/MPEG/MPEG/801299.mpeg.enc";
	char	*f1   = "/home/cc/liujc/FoveonX3/run/000001.mpeg";
	char	*file = "/home/cc/liujc/FoveonX3/run/805200";
	/*
	char	*f1   = "/etc/inetd.conf";
	char	*file = "/etc/issue";
	*/
	// file = "/home/cc/liujc/dispatcher91/result.txt";

	if ((cli = new_streamming_io_v2_client (port)) == NULL) {
		perror (NULL);
		exit (-1);
	}

	/*
	while (! cli->find_server (cli)) {
		fprintf (stderr, "Trying to find server ...\n");
	}
	*/

	// /home/cc/liujc/MPEG/MPEG/801299.mpeg.enc
	if (cli->openfile (cli, 0, f1) >= 0) {
		for (i = 0; i < 20; i++) {
			len = cli->read (cli, 0, buffer, sizeof buffer);
			// fprintf (stderr, "Read %d, %d\n", i, len);
		}
		// cli->abort (cli);
		// cli->closefile (cli, 0);
	} else {
		fprintf (stderr, "Remote error return: ");
		perror (f1);
	}

	if (cli->openfile (cli, 0, file) >= 0) {
		fprintf (stderr, "Ok, filesize = %ld\n",
				cli->filesize (cli, 0));

		while ((len = cli->read (cli, 0, buffer, sizeof buffer)) > 0) {
			write (STDOUT_FILENO, buffer, len);
		}

		cli->closefile (cli, 0);
		fprintf (stderr, "File closed\n");
	} else {
		fprintf (stderr, "Open failed\n");
	}

	cli->dispose (cli);
	exit (1);
}

void server (void) {
	struct streamming_io_v2_server_t	*svr;
	int					i;

	// sleep (3);

	if ((svr = new_streamming_io_v2_server (20, port)) == NULL) {
		perror (NULL);
		exit (-1);
	}

	svr->set_parameter (svr, 0, 1048576, 16384);
	// svr->set_parameter (svr, 1, 8192, 4096);
	svr->set_fopen (svr, file_open);

	svr->start (svr);

	for (i = 0; i < 1000; i++) {
		sleep (360);
	}

	svr->dispose (svr);
	fprintf (stderr, "dispose server ok\n");
}

int main (int argc, char *argv[]) {
	int	pid;
	int	status;

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

	return -1;
}
