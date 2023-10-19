#include <stdio.h>
#include <unistd.h>
#include "usec_timer.h"

int main (int argc, char *argv[]) {
	struct usec_timer_t	*usec;

	usec = new_usec_timer ();

	usec->start (usec);

	sleep (1);

	fprintf (stderr, "Elapsed time: %.4f\n", usec->elapsed (usec));

	usleep (100);
	fprintf (stderr, "Elapsed time: %.4f\n", usec->ended (usec));

	return 0;
}
