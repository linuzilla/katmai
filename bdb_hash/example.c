#include <stdio.h>
#include "bdb_hash.h"

int main (int argc, char *argv[]) {
	struct bdb_hash_t	*bdb;
	char			*key, *val;

	if ((bdb = new_bdb_hash (NULL)) == NULL) {
		perror ("new_bdb_hash");
		return 1;
	}

	fprintf (stderr, "%s\n", bdb->bdb_version (NULL, NULL, NULL));

	bdb->put (bdb, "a", "b");
	bdb->put (bdb, "tsdflkajsd", "bsdfasdf");
	bdb->put (bdb, "a", "bsdfasdf");
	bdb->put (bdb, "djfjjl", "b");
	bdb->put (bdb, "a", "bsdf");

	if (bdb->get_first (bdb, &key, &val)) {
		do {
			fprintf (stderr, "[%s][%s]\n", key, val);
		} while (bdb->get_next (bdb, &key, &val));
	}

	fprintf (stderr, "[%s]\n", bdb->get (bdb, "djfjjl"));

	bdb->dispose (bdb);
	return 0;
}
