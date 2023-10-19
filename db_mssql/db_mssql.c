/*
 *	db_mssql.c
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db_mssql.h"
#include "x_object.h"

#ifndef USE_FREETDS
#define USE_FREETDS	1
#endif

#if USE_FREETDS == 1
#define MSDBLIB
#endif

#include <sybdb.h>

#if USE_FREETDS == 1
#define SQLTEXT SYBTEXT
#define SQLCHAR SYBCHAR
#define SQLVARCHAR SYBVARCHAR
#define SQLINT1 SYBINT1
#define SQLINT2 SYBINT2
#define SQLINT4 SYBINT4
#define SQLINTN SYBINTN
#define SQLBIT SYBBIT
#define SQLFLT4 SYBREAL
#define SQLFLT8 SYBFLT8
#define SQLFLTN SYBFLTN
#define SQLDECIMAL SYBDECIMAL
#define SQLNUMERIC SYBNUMERIC
#define SQLDATETIME SYBDATETIME
#define SQLDATETIM4 SYBDATETIME4
#define SQLDATETIMN SYBDATETIMN
#define SQLMONEY SYBMONEY
#define SQLMONEY4 SYBMONEY4
#define SQLMONEYN SYBMONEYN
#define SQLIMAGE SYBIMAGE
#define SQLBINARY SYBBINARY
#define SQLVARBINARY SYBVARBINARY
#endif

static struct x_object_interface_t	*objintf = NULL;

typedef struct __mssql_link_t {
	LOGINREC	*login;
	DBPROCESS	*link;
} mssql_link_t;

struct db_mssql_pd_t {
	mssql_link_t		mssql;
	short			verbose;
	short			connect_ok;
	short			new_query;
	short			no_more_rows;
	struct x_object_t	*result;
	struct x_object_t	*field_name;
	struct x_object_t	*resobj;
	int			res;
	unsigned int		rows;
	unsigned int		fields;
	unsigned int		mssqlsvr_port;
	char			*mssqlsvr_unix_socket;
	unsigned int		mssqlsvr_clientflag;
	char			*default_db;
};

static int mssql_error_handler (DBPROCESS *dbproc, int severity,
			int dberr, int oserr, char *dberrstr, char *oserrstr) {
	// TSRMLS_FETCH ();

	fprintf (stderr, "ERROR: %s (severity %d)\n", dberrstr, severity);

	return INT_CANCEL;
}

static int mssql_message_handler (DBPROCESS *dbproc,
			DBINT msgno, int msgstate, int severity,char *msgtext,
			char *srvname, char *procname, DBUSMALLINT line) {

	fprintf (stderr, "MESSAGE: %s (severity %d)\n", msgtext, severity);
	return 0;
}

static int dbmssql_connect (struct db_mssql_t *self,
			char *server, const char *account,
			const char *passwd, char *database) {
	struct db_mssql_pd_t	*pd = self->pd;


	DBSETLUSER (pd->mssql.login, account);
	DBSETLPWD  (pd->mssql.login, passwd);
	DBSETLAPP  (pd->mssql.login, "db_mssql");

	if (pd->verbose > 5) {
		fprintf (stderr, "Connect to Microsoft SQL Server [ %s ]... ",
				server);
	}

	if ((pd->mssql.link = dbopen (pd->mssql.login, server)) == FAIL) {
		if (pd->verbose > 5) fprintf (stderr, "failed.\n");
		
		return 0;
	}

	pd->connect_ok = 1;


	if (dbuse (pd->mssql.link, database) == FAIL) {
		if (pd->verbose > 5) {
			fprintf (stderr, "[DB:%s] failed\n", database);
		}

		return 0;
	} else {
		pd->default_db = database;
	}

	if (pd->verbose > 5) {
		// fprintf (stderr, "ok. [%d]\n", dbtds (pd->mssql.link));
		//		dbversion ());
		fprintf (stderr, "TDS Version %d.%d\n",
				pd->mssql.link->tds_socket->major_version,
				pd->mssql.link->tds_socket->minor_version);

		// fprintf (stderr, "Charset: %s\n",
		//		dbservcharset (pd->mssql.link));
	}

	return 1;
}

static int dbmssql_select_db (struct db_mssql_t *self, char *database) {
	struct db_mssql_pd_t	*pd = self->pd;

	if (database == NULL) {
		if (dbuse (pd->mssql.link, pd->default_db) == FAIL) return 0;
	} else {
		if (dbuse (pd->mssql.link, database) == FAIL) return 0;
	}

	return 1;
}

static void dbmssql_disconnect (struct db_mssql_t *self) {
	struct db_mssql_pd_t	*pd = self->pd;

	if (pd->connect_ok) {
		dbclose (pd->mssql.link);
		pd->connect_ok = 0;
		pd->mssql.link = NULL;
	}

	if (pd->mssql.login != NULL) {
		dbloginfree (pd->mssql.login);
		pd->mssql.login = NULL;
	}
}

static int dbmssql_set_verbose (struct db_mssql_t *self, const int level) {
	struct db_mssql_pd_t	*pd = self->pd;
	int			rc = pd->verbose;

	if (level >= 0) pd->verbose = level;

	return rc;
}

static int internal_fetch_batch (struct db_mssql_t *self) {
	struct db_mssql_pd_t	*pd = self->pd;
	int			i;
	char			*fname;
	struct x_object_t	*xobj;
	char			buffer[64];

	if (pd->fields == 0) return 0;

	xobj  = pd->field_name;

	objintf->empty (xobj);

	for (i = 0; i < pd->fields; i++) {
		fname= (char *) dbcolname (pd->mssql.link, i + 1);

		if (fname[0] != '\0') {
			objintf->str_push (xobj, fname);
		} else {
			objintf->str_push (xobj, buffer);
		}
	}

	return 1;
}

static int dbmssql_store_result (struct db_mssql_t *self) {
	struct db_mssql_pd_t	*pd = self->pd;

	if (pd->new_query) {
		pd->new_query = 0;

		if (pd->fields == 0) return 1;


		internal_fetch_batch (self);


		/*

		if ((pd->res_ptr = mssql_store_result (
						pd->mssql_login)) == NULL) {
			return 0;
		}

		pd->rows   = (unsigned int) mssql_num_rows (pd->res_ptr);
		pd->fields = (unsigned int) mssql_num_fields (pd->res_ptr);

		//	fprintf (stderr, "%u rows, %u fields\n",
		//			pd->rows, pd->fields);
		*/
	}

	return 1;
}

static int dbmssql_query (struct db_mssql_t *self, const char *query, ...) {
	struct db_mssql_pd_t	*pd = self->pd;
	int			rc;
	char			qstr[8192];
	va_list			ap;

	va_start (ap, query);
	vsnprintf (qstr, sizeof qstr - 1, query, ap);
	va_end (ap);

	pd->rows   = 0;
	pd->fields = 0;

	self->free_result (self);

	if (dbcmd (pd->mssql.link, qstr) == FAIL) return 0;

	if (dbsqlexec (pd->mssql.link) == FAIL ||
				(rc = dbresults (pd->mssql.link)) == FAIL) {
		fprintf (stderr, "\nQuery failed: %s\n", qstr);
		return 0;
	}

	while ((pd->fields = dbnumcols (pd->mssql.link)) <= 0
					&& rc == SUCCEED) {
		rc = dbresults (pd->mssql.link);
	}

	if ((pd->fields = dbnumcols (pd->mssql.link)) <= 0) return 0;

	pd->new_query = 1;
	pd->no_more_rows = 0;

	// if ((rc = dbnextrow (pd->mssql.link)) == FAIL) return 0;
	// pd->rows = DBCOUNT (pd->mssql.link);

	return 1;
}

static unsigned int dbmssql_num_fields (struct db_mssql_t *self) {
	return self->pd->fields;
}

static unsigned int dbmssql_num_rows (struct db_mssql_t *self) {
	self->store_result (self);

	return self->pd->rows;
}

static int dbmssql_field_index (struct db_mssql_t *self, const char *str) {
	struct x_object_t	*xobj;
	char			**list;
	int			i, num;

	self->store_result (self);
	if ((xobj  = self->pd->field_name) == NULL) return -1;
	if ((list = objintf->str_array (xobj)) == NULL) return -1;

	num = self->num_fields (self);

	for (i = 0; i < num; i++) {
		if (strcmp (str, list[i]) == 0) return i;
	}

	return -1;
}

static struct x_object_t * dbmssql_fetch_array (struct db_mssql_t *self) {
	struct x_object_t	*rxobj;
	struct x_object_t	*fxobj;
	char			**ptr;
	char			**list;
	int			i, num;

	self->store_result (self);

	fxobj = self->pd->field_name;

	if ((list = objintf->str_array (fxobj)) == NULL) return NULL;
	if ((ptr = self->fetch (self)) == NULL) return NULL;

	num = self->num_fields (self);

	rxobj = self->pd->resobj;
	objintf->empty (rxobj);

	for (i = 0; i < num; i++) {
		/*
		if (strcmp (list[i], "cor_serial_equal_assign") == 0) {
			fprintf (stderr, "[%s][%s]\n", list[i], ptr[i]);
		}
		*/

		if (ptr[i] == NULL) {
			objintf->put (rxobj, list[i], "");
		} else {
			objintf->put (rxobj, list[i], ptr[i]);
		}
	}

	return rxobj;
}

static char ** dbmssql_fetch (struct db_mssql_t *self) {
	struct db_mssql_pd_t	*pd = self->pd;
	int			i, j, rc, intval;
	char			*ptr;
	int			column_type;
	double			realval;
	char			*buf;
	int			buflen;
	DBDATEREC		dateinfo;

	if (pd->no_more_rows) return NULL;

	self->store_result (self);


	rc = dbnextrow (pd->mssql.link);
	if (rc == FAIL || rc == NO_MORE_ROWS) {
		pd->no_more_rows = 1;
		return NULL;
	}

	for (i = 0, buflen = 8192; i < pd->fields; i++) {
		j = dbcollen (pd->mssql.link,i+1);
		buflen = buflen >= j ? buflen : j;
	}

	// fprintf (stderr, "MAX-Length: %d\n", buflen);

	if ((buf = alloca (buflen + 1)) == NULL) {
		perror ("alloca");
		return NULL;
	}

	objintf->empty (pd->result);

	for (i = 0; i < pd->fields; i++) {
		column_type = dbcoltype (pd->mssql.link, i + 1);

		if ((ptr = dbdata (pd->mssql.link, i + 1)) == NULL) {
			objintf->str_push (pd->result, NULL);
			continue;
		}

		switch (column_type) {
		case SQLFLT4:
			realval = (double) (*(float *) ptr);
			sprintf (buf, "%.5f", realval);
			break;
		case SQLFLT8:
			realval = (double) (*(DBFLT8 *) ptr);
			sprintf (buf, "%.5f", realval);
			break;
		case SQLINTN:
		case SQLINT1:
			intval = (int) * (DBTINYINT *) ptr;
			sprintf (buf, "%d", intval);
			break;
		case SQLINT2:
			intval = (int) * (DBSMALLINT *) ptr;
			sprintf (buf, "%d", intval);
			break;
		case SQLINT4:
			intval = (int) * (DBINT *) ptr;
			sprintf (buf, "%d", intval);
			break;
		case SQLCHAR:
		case SQLVARCHAR:
		case SQLTEXT:
			intval = dbdatlen (pd->mssql.link, i + 1);
			if (intval >= buflen) {
				fprintf (stderr, "!! short read !!\n");
			} else {
				memcpy (buf, ptr, intval);
				buf[intval] = '\0';
			}

			break;
		case SQLDATETIM4:
		case SQLDATETIME:
			if (dbwillconvert (column_type, SQLCHAR)) {
				intval = dbdatlen (pd->mssql.link, i + 1);

				if (column_type == SQLDATETIM4) {
					DBDATETIME	temp;
					dbconvert (NULL, SQLDATETIM4,
							ptr, -1,
							SQLDATETIME,
							(unsigned char *) &temp,
							-1);
					dbdatecrack (pd->mssql.link,
							&dateinfo, &temp);
				} else {
					dbdatecrack (pd->mssql.link,
							&dateinfo,
							(DBDATETIME *) ptr);
				}

				sprintf (buf,
					"%04d-%02d-%02d %02d:%02d:%02d",
					dateinfo.year, dateinfo.month,
					dateinfo.day, dateinfo.hour,
					dateinfo.minute, dateinfo.second);
			} else {
				fprintf (stderr, "OOPS !!\n");
				exit (1);
			}
			break;
		case SQLNUMERIC:
		case SQLDECIMAL:
		default:
			if (dbwillconvert (column_type, SQLCHAR)) {
				intval = dbdatlen (pd->mssql.link, i + 1);

				if (intval >= buflen) {
					fprintf (stderr, "Short read\n");
					dbconvert (NULL, column_type,
						ptr, intval, SQLCHAR, buf, -1);
				} else {
					dbconvert (NULL, column_type,
						ptr, intval, SQLCHAR, buf, -1);
				}
			} else {
				fprintf (stderr, "OOPS !!\n");
				exit (1);
			}
			break;
		}

		// if (i < 20) fprintf (stderr, "<%s>\n", buf);
		objintf->str_push (pd->result, buf);
	}

	dbclrbuf (pd->mssql.link, DBLASTROW (pd->mssql.link));
	pd->rows++;

	// fprintf (stderr, "Fetch\n");

	return objintf->str_array (pd->result);
}

static int dbmssql_free_result (struct db_mssql_t *self) {
	struct db_mssql_pd_t	*pd = self->pd;
	int			rc;

	if (pd->no_more_rows) return 1;

	while (1) {
		rc = dbnextrow (pd->mssql.link);
		if (rc == FAIL || rc == NO_MORE_ROWS) {
			pd->no_more_rows = 1;
			return 1;
		}
	}

	return 1;
	// struct db_mssql_pd_t	*pd = self->pd;

	/*
	if (pd->res_ptr != NULL) {
		mssql_free_result (pd->res_ptr);
		pd->res_ptr = NULL;
		return 1;
	}
	*/

	return 0;
}
/*


static int dbmssql_perror (struct db_mssql_t *self, const char *str) {
	struct db_mssql_pd_t	*pd = self->pd;

	if (pd->res) {
		fprintf (stderr, "%s: %s\n", str, mssql_error (pd->mssql_login));
		return 1;
	}

	return 0;
}
*/

static void dbmssql_dispose (struct db_mssql_t *self) {
	self->disconnect (self);

	/*
	self->free_result (self);
	self->unix_socket (self, NULL);
	*/

	free (self->pd);
	free (self);
}

struct db_mssql_t * new_dbmssql (void) {
	struct db_mssql_t	*self;
	struct db_mssql_pd_t	*pd;
	struct x_object_t	*result;
	struct x_object_t	*field_name;
	struct x_object_t	*resobj;

	// putenv ("TDSVER=7.0");

	if (dbinit() == FAIL) {
		fprintf (stderr, "dbinit () failed!\n");
		return NULL;
	}

	if (objintf == NULL) objintf = init_x_object_interface ();

	while ((self = malloc (sizeof (struct db_mssql_t))) != NULL) {
		if ((self->pd = pd = malloc (
				sizeof (struct db_mssql_pd_t))) == NULL) {
			free (self);
			self = NULL;
			break;
		}

		if ((result = objintf->newobj ()) == NULL) {
			free (self->pd);
			free (self);
			self = NULL;
			break;
		}

		if ((field_name = objintf->newobj ()) == NULL) {
			objintf->dispose (result);
			free (self->pd);
			free (self);
			self = NULL;
			break;
		}

		if ((resobj = objintf->newobj ()) == NULL) {
			objintf->dispose (field_name);
			objintf->dispose (result);
			free (self->pd);
			free (self);
			self = NULL;
			break;
		}


		self->dispose		= dbmssql_dispose;
		self->connect		= dbmssql_connect;
		self->disconnect	= dbmssql_disconnect;
		self->verbose		= dbmssql_set_verbose;
		self->query		= dbmssql_query;
		self->num_fields	= dbmssql_num_fields;
		self->num_rows		= dbmssql_num_rows;
		self->store_result	= dbmssql_store_result;
		self->free_result	= dbmssql_free_result;
		self->fetch		= dbmssql_fetch;
		self->fetch_array	= dbmssql_fetch_array;
		self->findex		= dbmssql_field_index;
		self->select_db		= dbmssql_select_db;

		pd->connect_ok		= 0;
		pd->verbose		= 1;
		pd->result		= result;
		pd->field_name		= field_name;
		pd->resobj		= resobj;
		pd->no_more_rows	= 1;

		pd->default_db		= NULL;
		/*
		self->perror		= dbmssql_perror;
		self->unix_socket	= dbmssql_unix_socket;
		self->set_port		= dbmssql_set_port;

		pd->res			= 0;
		pd->new_query		= 0;
		pd->res_ptr		= NULL;

		pd->mssqlsvr_port		= 0;
		pd->mssqlsvr_unix_socket	= NULL;
		pd->mssqlsvr_clientflag		= 0;

		pd->rows			= 0;
		pd->fields			= 0;
		*/

		if ((pd->mssql.login = dblogin ()) == NULL) {
			fprintf (stderr,
				"Unable to allocate login record");
			self->dispose (self);
			self = NULL;
			break;
		} else {
			// dbloginfree (pd->mssql_login);

			if (0) {
				dberrhandle (mssql_error_handler);
				dbmsghandle ((MHANDLEFUNC)
						mssql_message_handler);
			}

			/*
			fprintf (stderr, "Microsoft SQL Library Version: %s\n",
					MSSQL_VERSION);
			*/
		}

		break;
	}

	return self;
}
