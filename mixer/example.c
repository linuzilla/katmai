#include <stdio.h>
#include "aumixer.h"

int main (int argc, char *argv[]) {
	struct aumixer_t	*mixer;
	int			i = 5;
	int			r, l;

	mixer = new_aumixer ();
	mixer->query_all ();

	if (mixer->query (i, &r, &l) > 0) {
		printf ("%s[%d,%d]\n", mixer->devname (i), r, l);
	}
	mixer->setvolume (i, 67);
	mixer->close ();

	return 0;
}
