#include <stdio.h>
#include <stdlib.h>
#include "xtimer.h"

int main (int argc, char *argv[]) {
	struct xtimer_t	*xt;
	char		buffer[256];


	if ((xt = new_xtimer ()) == NULL) {
		perror (NULL);
		exit (1);
	}

	while (fgets (buffer, sizeof buffer, stdin) != NULL) {
		fprintf (stderr, "%d", xt->elapsed (xt));
		xt->print (xt);
	}

	fprintf (stderr, "%d\n", xt->elapsed (xt));
}
