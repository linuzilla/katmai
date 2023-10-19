/*
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "rbmcast.h"

static int	port = 1234;


static void do_client (struct rbmcast_lib_t *rbm) {
	fprintf (stderr, "I am client\n");

	rbm->open (rbm, "224.0.1.2", NULL);
	usleep (1);
	rbm->bind (rbm, port, 0, 0);
	sleep (1);

	if (rbm->connect (rbm, 1000)) {
		fprintf (stderr, "Connect ok!\n");
	} else {
		fprintf (stderr, "Connect fail!\n");
	}
}

void do_server (struct rbmcast_lib_t *rbm) {
	char	*str = "Hello !!";

	fprintf (stderr, "I am server\n");

	rbm->open (rbm, "224.0.1.2", NULL);
	rbm->bind (rbm, port, 0, 1);
	rbm->listen (rbm, 20);
	sleep (20);

	fprintf (stderr, "%d client(s) registed!\n",
			rbm->num_of_client (rbm));

	if (rbm->num_of_client (rbm) > 0) {
		rbm->send (rbm, str, strlen (str) + 1);
	}
}

int main (int argc, char *argv[]) {
	struct rbmcast_lib_t	*rbm;

	if ((rbm = new_rbmcast ()) == NULL) {
		perror ("new_rbmcast");
		return 1;
	}

	rbm->loopback (rbm, 1);

	(fork () == 0) ?  do_client (rbm) : do_server (rbm);

	return 0;
}
