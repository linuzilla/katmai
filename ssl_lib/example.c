#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ssl_lib.h"


#define LISTEN_PORT	6001
static struct SSL_lib_t	*ssl;

static void server (void) {
	int	fd;
	char	buffer[512];
	int	rc;

	// ssl->set_CAdir    (ssl, "./CA");
	ssl->set_CAfile   (ssl, "rootcert.pem");
	ssl->set_CERTfile (ssl, "server.pem");

	if (ssl->listen (ssl, LISTEN_PORT) > 0) {
		fd = ssl->accept (ssl);
		fprintf (stderr, "[fd=%d]\n", fd);
		sleep (10);
		rc = ssl->read (ssl, buffer, sizeof buffer);
		fprintf (stderr, "Server:[");
		fwrite (buffer, 1, rc, stderr);
		fprintf (stderr, "]\n");
	} else {
		fprintf (stderr, "[%s]\n", ssl->errstr (ssl));
	}
}

static void client_pop3 (void) {
	char			buffer[512];
	int			rc;

	if (! ssl->connect (ssl, "dove.cc.ncu.edu.tw", 995)) {
		fprintf (stderr, "%s\n", ssl->errstr (ssl));
		return;
	}

	if ((rc = ssl->read (ssl, buffer, sizeof buffer)) > 0) {
		fprintf (stderr, "<<--");
		fwrite (buffer, 1, rc, stdout);
	}

	fprintf (stderr, "--> QUIT\n");
	sprintf (buffer, "QUIT\n");
	ssl->write (ssl, buffer, strlen (buffer));

	if ((rc = ssl->read (ssl, buffer, sizeof buffer)) > 0) {
		fprintf (stderr, "<<--");
		fwrite (buffer, 1, rc, stdout);
	}
}

static void client (void) {
	char			buffer[512];
	int			rc;

	sleep (10);

	fprintf (stderr, "Client connect ... ");

	if (! ssl->connect (ssl, "127.0.0.1", LISTEN_PORT)) {
		fprintf (stderr, "%s\n", ssl->errstr (ssl));
		return;
	}
	fprintf (stderr, "ok.\n");

	fprintf (stderr, "--> QUIT\n");
	sprintf (buffer, "QUIT\n");
	ssl->write (ssl, buffer, strlen (buffer));
	/*
	if ((rc = ssl->read (ssl, buffer, sizeof buffer)) > 0) {
		fprintf (stderr, "<<--");
		fwrite (buffer, 1, rc, stdout);
	}

	fprintf (stderr, "--> QUIT\n");
	sprintf (buffer, "QUIT\n");
	ssl->write (ssl, buffer, strlen (buffer));

	if ((rc = ssl->read (ssl, buffer, sizeof buffer)) > 0) {
		fprintf (stderr, "<<--");
		fwrite (buffer, 1, rc, stdout);
	}
	*/
}

int main (int argc, char *argv[]) {

	while ((ssl = new_SSL_lib ()) != NULL) {
		switch (fork ()) {
		case 0:
			client ();
			break;
		case -1:
			client_pop3 ();
			break;
		default:
			server ();
			break;
		}

		ssl->disconnect (ssl);
		ssl->dispose (ssl);
		break;
	}

	return 0;
}
