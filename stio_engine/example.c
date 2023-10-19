#include <stdio.h>
#include <unistd.h>
#include "stio_engine.h"

static int eop_callback (void *ptr) {
	struct stio_engine_t	*stio = ptr;

	fprintf (stderr, "End of prefetch\n");

	// stio->open_next (stio, "/etc/hosts");
	stio->open_next (stio, "/etc/hosts");
	stio->start_pq_prefetch (stio);
		
	return 1;
}

int main (int argc, char *argv[]) {
	struct stio_engine_t	*stio;
	char			buffer[3000];
	int			len;

	if ((stio = new_stio_engine ()) == NULL) {
		perror (NULL);
		return -1;
	}

	stio->prefetch (stio, 20, 3);
	stio->set_prefetch_buffer (stio, 3);
	stio->set_eop_callback (stio, eop_callback, stio);

	stio->set_prefetch_buffer (stio, 1);

	if (stio->open (stio, "/etc/passwd") > 0) {
		// stio->set_repeat (stio, 2);

		while ((len = stio->read (stio, buffer, sizeof buffer)) > 0) {
			// write (STDOUT_FILENO, buffer, len);
		}
		stio->close (stio);
		fprintf (stderr, "Read first file ok\n");
	}

	stio->set_eop_callback (stio, NULL, NULL);

	if (stio->open (stio, "/etc/hosts") > 0) {
		sleep (1);
		// stio->open_next (stio, "/etc/hosts");
		while ((len = stio->read (stio, buffer, sizeof buffer)) > 0) {
			write (STDOUT_FILENO, buffer, len);
		}
		stio->close (stio);
		fprintf (stderr, "Read second file ok\n");
	}

	stio->dispose (stio);

	return 0;
}
