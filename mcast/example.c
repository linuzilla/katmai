/*
 *	mcast example
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include "mcast.h"

int main (int argc, char *argv[]) {
	struct mcast_lib_t	*mserv;
	struct mcast_lib_t	*mclient;
	int			i;
	char			buf[10];
	char			rcvbuf[1024];
       
	if ((mserv = new_multicast (0)) == NULL) {
		perror ("new_multicast");
		return 1;
	}

	if ((mclient = new_multicast (0)) == NULL) {
		perror ("new_multicast");
		return 1;
	}

	if (mserv->open (mserv, "225.7.7.7", NULL, 1) < 0) {
		return 1;
	}

	if (mclient->open (mclient, "225.3.3.3", NULL, 1) < 0) {
		return 1;
	}

	mserv->bind   (mserv, INADDR_ANY, 1132);
	mclient->bind (mclient, INADDR_ANY, 1133);

	if (fork () == 0) {
		sleep (1);
		for (i = 0; i < 100; i++) {
			sprintf (buf, "%d", i);
			mserv->send (mserv, buf, sizeof buf);
			fprintf (stderr, "Ping\n");
			mclient->receive (mclient, rcvbuf, sizeof rcvbuf);
			fprintf (stderr, "Ping %d (%s:%d)\n", i,
					mclient->remote_addr (mclient),
					mclient->remote_port (mclient));
		}

		fprintf (stderr, "Terminate\n");
	} else {

		do {
			mserv->receive (mserv, rcvbuf, sizeof rcvbuf);
			sscanf (rcvbuf, "%d", &i);
			fprintf (stderr, "Pong %d (%s:%d)\n", i,
					mserv->remote_addr (mserv),
					mserv->remote_port (mserv));
			mclient->send (mclient, buf, sizeof buf);
		} while (i < 99);
	}
	mserv->close (mserv);
	mclient->close (mclient);

	return 0;
}
