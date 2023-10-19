/*
 *	main.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>

#include "db_mysql.h"
#include "x_object.h"


int main (int argc, char *argv[]) {
	int			i, len;
	struct db_mysql_t	*mydb = NULL;
	struct utsname		utsname_buf;
	char			**result;
	char			**fields;
	struct x_object_t	*obj;
	struct x_object_interface_t *objintf;

	objintf = init_x_object_interface ();

	uname (&utsname_buf);
	fprintf (stderr, "\n%s %s %s [%s]\n\n",
				utsname_buf.sysname,
				utsname_buf.machine,
				utsname_buf.release,
				utsname_buf.version);

	if ((mydb = new_dbmysql ()) == NULL) {
		perror ("new_dbmysql");
		return 0;
	}

	if (! mydb->connect (mydb, "localhost", "guest", "", "test")) {
		mydb->perror (mydb, "mysql");
		mydb->dispose (mydb);
		return 0;
	}

	mydb->query (mydb, "SELECT * FROM sysconf");

	fprintf (stderr, "Number of rows: %u, Number of fields: %u\n",
			mydb->num_rows (mydb),
			mydb->num_fields (mydb));

	len = mydb->num_fields (mydb);

	while ((result = mydb->fetch (mydb)) != NULL) {
		for (i = 0; i < len; i++) {
			if (result[i] == NULL) {
				fprintf (stderr, "[(null)]");
			} else {
				fprintf (stderr, "[%s]", result[i]);
			}
		}
		fprintf (stderr, "\n");
	}
	mydb->free_result (mydb);

	mydb->query (mydb, "SHOW TABLES");
	fields = mydb->fetch_fields (mydb);
	len = mydb->num_fields (mydb);

	while ((obj = mydb->fetch_array (mydb)) != NULL) {
		for (i = 0; i < len; i++) {
			fprintf (stderr, "[%s -> %s]",
				fields[i],
				objintf->get (obj, fields[i]));
		}
		fprintf (stderr, "\n");
	}

	mydb->dispose (mydb);

	return 0;
}
