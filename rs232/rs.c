#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>             /* for strerror() */
#include <termios.h>
#include <unistd.h>
#include <sys/reboot.h>
#include "global_var.h"

#define BUFSIZE 1024

static struct termios	pots; 
static int		pf;
static int		done = 0;

/* restore original terminal settings on exit */

static void cleanup_termios (int signal) {
	done = 1;
	fprintf (stderr, "(timeout)");
}

int set_code_from_rs232 (int (*myip)[], const char *portname, speed_t speed,
					int swflow, int hwflow, int seconds) {
	/*
	speed_t			speed = B9600;	// baud rate
	char			*portname = "/dev/ttyS0";
	int			swflow = 0;	// software flow control
	int			hwflow = 0;	// hardware flow control
	*/
	int			rc = 0;
	int			crnl = 0;	// send CR with NL
	struct termios		pts;		// termios settings on port

	fd_set			ready;		// used for select
	int			i = 0;		// used in the multiplex loop
	int			gotit = 0;
	int			j;
	char			buf[BUFSIZE];
	char			cmdbuffer[BUFSIZE];
	int			cmdbufidx = 0;
	char			STB_request_string[] = "STBidReq";
	char			STB_done_string[]    = "idDone\r\n";
	char			Master_ID_string[]   = "STBid";
	int			ioflags;
	struct timeval		tv;


	fprintf (stderr, "[*] Reading data from [ %s ] ... ", portname);

	if ((pf = open (portname, O_RDWR)) < 0) {
		perror (portname);
		return 0;
	}


	/* modify the port configuration */
	tcgetattr (pf, &pts);
	pots = pts;

	/* some things we want to set arbitrarily */
	pts.c_lflag &= ~ICANON; 
	pts.c_lflag &= ~(ECHO | ECHOCTL | ECHONL);
	pts.c_cflag |= HUPCL;
	pts.c_cc[VMIN] = 1;
	pts.c_cc[VTIME] = 0;

	/* Standard CR/LF handling: this is a dumb terminal.
	 * Do no translation:
	 *  no NL -> CR/NL mapping on output, and
	 *  no CR -> NL mapping on input.
	 */

	pts.c_oflag &= ~ONLCR;
	pts.c_iflag &= ~ICRNL;

	/* Now deal with the local terminal side */

	if (crnl)   pts.c_oflag |= ONLCR;	// send CR with NL

	if (hwflow) {	// Hardware flow control
		pts.c_cflag |= CRTSCTS;
		pts.c_iflag &= ~(IXON | IXOFF | IXANY);
	}

	if (swflow) { // software flow control
		pts.c_cflag &= ~CRTSCTS;
		pts.c_iflag |= IXON | IXOFF | IXANY;
	}

	/*
	if (noflow) { // no flow control
		pts.c_cflag &= ~CRTSCTS;
		pts.c_iflag &= ~(IXON | IXOFF | IXANY);
	}
	*/

	cfsetospeed (&pts, speed);
	cfsetispeed (&pts, speed);

	/* set the signal handler to restore the old
	 * termios handler */

	signal (SIGALRM, cleanup_termios);

	/* Now set the modified termios settings */
	tcsetattr (pf, TCSANOW, &pts);

	// read out from pf ....

	ioflags = fcntl (pf, F_GETFL);
	fcntl (pf, F_SETFL, ioflags | O_NDELAY);
	while (read (pf, buf, BUFSIZE) > 0) fprintf (stderr, "clean\n");
	// fcntl (pf, F_SETFL, ioflags);

	sprintf (buf, "%s%03d.%03d\r\n", STB_request_string,
					(*myip)[2], (*myip)[3]);
	// fprintf (stderr, "SEND %s", buf);
	write (pf, buf, strlen (buf));

	alarm (seconds);

	do {
		FD_ZERO (&ready);
		FD_SET  (pf, &ready);

		tv.tv_sec = 1;
		tv.tv_usec = 0;

		select (pf+1, &ready, NULL, NULL, &tv);

		if (FD_ISSET (pf, &ready)) {
			alarm (seconds);
			/* pf has characters for us */
			i = read (pf, buf, BUFSIZE);
			if (i >= 1) {
				for (j = 0; j < i; j++) {
					if ((buf[j] == '\r') ||
					    (buf[j] == '\n')) {
						cmdbuffer[cmdbufidx] = '\0';

						if ((cmdbufidx >=
							sizeof Master_ID_string
							+ 4) &&
							(strncmp (
							   Master_ID_string,
							   cmdbuffer,
							   sizeof
							      Master_ID_string
							   -1) == 0)) {
							// fprintf (stderr,
							//	"ok [%s]\n",
							//	cmdbuffer);

							done = 1;
							gotit = 1;
						} else if (cmdbufidx != 0) {
							fprintf (stderr,
								"unknow [%s]\n",
								cmdbuffer);
						}

						cmdbufidx = 0;
					} else if (cmdbufidx < BUFSIZE-1) {
						cmdbuffer[cmdbufidx++] = buf[j];
					}
				}
			} else {
				done = 1;
			}
		} else {
			fprintf (stderr, "select\n");
		}
	} while (!done);

	alarm (0);
	signal (SIGALRM, SIG_IGN);

	if (gotit) {
		int	i, j;

		// fprintf (stderr, "[%s]\n",
		// 		&cmdbuffer[sizeof Master_ID_string - 1]);
		sscanf (&cmdbuffer[sizeof Master_ID_string - 1], "%d.%d",
				&i, &j);

		if (((*myip)[2] != i) || ((*myip)[3] != j)) {
			(*myip)[2] = i;
			(*myip)[3] = j;
			// system_setting_writeback ();
		}
		rc = 1;

		write (pf, STB_done_string, sizeof STB_done_string - 1);
	}

	/* restore original terminal settings and exit */
	tcsetattr (pf, TCSANOW, &pots);
	close (pf);

	fprintf (stderr, " OK\n");

	if (rc) reboot (RB_POWER_OFF);

	return rc;
}
