#include <stdio.h>
#include "simconf.h"

int main (int argc, char *argv[]) {
	struct simconf_t	*simconf;

	if ((simconf = new_simconf ()) == NULL) exit (1);

	simconf->load (simconf, "config.conf");

	simconf->dispose (simconf);

	return 1;
}
