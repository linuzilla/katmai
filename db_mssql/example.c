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

#ifdef __linux__
#include <getopt.h>
#endif
// #include "predefine.h"

#include "db_mysql.h"
#include "db_mssql.h"
#include "sys_conf.h"

struct sysconf_t	*sysconfig = NULL;
struct db_mysql_t	*mydb = NULL;
struct db_mssql_t	*msdb = NULL;

int main (int argc, char *argv[]) {
	struct utsname		utsname_buf;
	char			*cfgfile;
	const char		*cfg_tailer = ".conf";
	char			*cp, *program_name;
	int			user_opt = 0, i, len;
	char			**result;

	program_name = ((cp = strrchr (argv[0], '/')) != NULL) ? cp+1 : argv[0];

	if ((cfgfile = malloc (strlen (argv[0]) +
					sizeof cfg_tailer + 1)) == NULL) {
		perror ("malloc");
		return 0;
	} else {
		sprintf (cfgfile, "%s%s", argv[0], cfg_tailer);
	}

	if ((sysconfig = initial_sysconf_module (
				cfgfile, "cmdline-opt", user_opt)) == NULL) {
		return 0;
	}


	uname (&utsname_buf);
	fprintf (stderr, "\n%s %s %s [%s]\n",
				utsname_buf.sysname,
				utsname_buf.machine,
				utsname_buf.release,
				utsname_buf.version);

	if ((mydb = new_dbmysql ()) == NULL) {
		perror ("new_dbmysql");
		return 0;
	}

	if ((msdb = new_dbmssql ()) == NULL) {
		perror ("new_dbmssql");
		return 0;
	} else {
		msdb->verbose (msdb, 10);
	}

	if (mydb->connect (mydb,
			sysconfig->getstr ("mysql-server"),
			sysconfig->getstr ("mysql-user"),
			sysconfig->getstr ("mysql-password"),
			sysconfig->getstr ("mysql-database"))) {
	} else {
		mydb->perror (mydb, "mysql");
		mydb->dispose (mydb);
		return 0;
	}

	if (msdb->connect (msdb,
			sysconfig->getstr ("mssql-server"),
			sysconfig->getstr ("mssql-user"),
			sysconfig->getstr ("mssql-password"),
			sysconfig->getstr ("mssql-database"))) {
	} else {
		fprintf (stderr, "Connect to Microsoft SQL error\n");
		return 0;
	}

	msdb->query (msdb, "SELECT * FROM crs_open");
	fprintf (stderr, "MSSQL: Number of rows: %u, Number of fields: %u\n",
			msdb->num_rows (msdb),
			msdb->num_fields (msdb));

	len = msdb->num_fields (msdb);

	while ((result = msdb->fetch (msdb)) != NULL) {
		for (i = 0; i < len; i++) {
			if (result[i] == NULL) {
				fprintf (stderr, "[(null)]");
			} else {
				fprintf (stderr, "[%s]", result[i]);
			}
		}
		fprintf (stderr, "\n");
	}
	msdb->free_result (msdb);


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

	mydb->dispose (mydb);
	msdb->dispose (msdb);

	return 0;
}
