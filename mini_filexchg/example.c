#include <stdio.h>
#include <unistd.h>
#include "mini_filexchg.h"

struct mini_filexchg_t	*fxchg;

const int 	port = 6502;

void server (void) {
	fxchg->exports (fxchg, "/etc");
	fxchg->start (fxchg, port);
	sleep (20);
	fxchg->stop (fxchg);
}

void client (void) {
	fxchg->ftpget (fxchg, "127.0.0.1", port, "issue", "issue.test");
}

int main (int argc, char *argv[]) {
	if ((fxchg = new_mini_filexchg ()) == NULL) return 1;

	if (fork () == 0) {
		server ();
	} else {
		sleep (1);
		client ();
	}

	return 0;
}
