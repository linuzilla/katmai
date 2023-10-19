/*
 *	ssl_lib.c
 *
 *	Copyright (c) 2004, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define OPENSSL_NO_KRB5

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#include "ssl_lib.h"

#define SF_CS_CLIENT		1
#define SF_CS_SERVER		2
#define DEFAULT_CIPHER_LIST	"ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"

struct SSL_lib_pd_t {
	SSL_CTX		*client_ctx;
	SSL_CTX		*server_ctx;
	SSL		*client_ssl;
	SSL		*server_ssl;
	BIO		*client_conn;
	int		client_fd;
	int		server_fd;
	short		client_or_server;
	short		verbose_level;
	char		*CAdir;
	char		*CAfile;
	char		*CERTfile;
	char		*cipher_list;
	char		*errstr;
};

static int ssl_init_OpenSSL (struct SSL_lib_t *self) {
	if (!  SSL_library_init ()) {
		self->pd->errstr = "** OpenSSL initialization failed!";
		return 0;
	}

	SSL_load_error_strings ();

	return 1;
}

static int ssl_init_client (struct SSL_lib_t *self) {
	struct SSL_lib_pd_t	*pd = self->pd;

	if (pd->client_ctx != NULL) return 1;

	pd->client_ctx = SSL_CTX_new (SSLv23_method ());

	return 1;
}

static int ssl_init_server (struct SSL_lib_t *self) {
	struct SSL_lib_pd_t	*pd = self->pd;

	if (pd->server_ctx != NULL) return 1;

	pd->server_ctx = SSL_CTX_new (SSLv23_method ());

	if (pd->CAfile != NULL) {
		if (SSL_CTX_load_verify_locations (pd->server_ctx, 
				pd->CAfile, pd->CAdir) != 1) {
			pd->errstr = "Error loading CA file and/or directory";
			return 0;
		}
	}

	if (SSL_CTX_set_default_verify_paths (pd->server_ctx) != 1) {
		pd->errstr = "Error loading default CA file and/or directory";
		return 0;
	}

	if (SSL_CTX_use_certificate_chain_file (pd->server_ctx,
				pd->CERTfile) != 1) {
		pd->errstr = "Error loading certificate from file";
		return 0;
	}

	if (SSL_CTX_use_PrivateKey_file (pd->server_ctx,
				pd->CERTfile, SSL_FILETYPE_PEM) != 1){
		pd->errstr = "Error loading private key from file";
		return 0;
	}

	return 1;
}

static int ssl_disconnect (struct SSL_lib_t *self) {
	struct SSL_lib_pd_t	*pd = self->pd;

	if ((pd->client_or_server & SF_CS_CLIENT) != 0) {
		SSL_clear (pd->client_ssl);
		SSL_shutdown (pd->client_ssl);

		pd->client_or_server &= ~SF_CS_CLIENT;
	}

	if (pd->client_fd >= 0) {
		close (pd->client_fd);
		pd->client_fd = -1;
	}

	return 1;
}

static int set_peer (struct sockaddr_in *sa, const char *host, const int port) {
	int			len;
	struct hostent		*hp;

	memset (sa, 0, sizeof *sa);

	if (inet_aton (host, &sa->sin_addr) != 0) {
		sa->sin_family = AF_INET;
	} else if ((hp = gethostbyname (host)) != NULL) {
		sa->sin_family = hp->h_addrtype;
		len = hp->h_length;

		if (len > sizeof sa->sin_addr) len = sizeof sa->sin_addr;

		memcpy (&sa->sin_addr, hp->h_addr_list[0], len);
	} else {
		return 0;
	}

	sa->sin_port = htons (port);

	return 1;
}

static int ssl_connect (struct SSL_lib_t *self,
				const char *host, const int port) {
	struct SSL_lib_pd_t	*pd = self->pd;
	struct sockaddr_in	sa;

	if (set_peer (&sa, host, port) == 0) {
		pd->errstr = "unknow host";
		return 0;
	}

	self->disconnect (self);

	if ((pd->client_fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		pd->errstr = "open socket";
		return 0;
	}

	ssl_init_client (self);

	if (connect (pd->client_fd, (struct sockaddr*) &sa, sizeof sa) < 0) {
		close (pd->client_fd);
		pd->errstr = "fail to connect";
		return -1;
	}

	/*
	char			*hoststr;

	hoststr = alloca (strlen (host) + 7);

	sprintf (hoststr, "%s:%d", host, port);

	if (! (pd->client_conn = BIO_new_connect (hoststr))) {
		pd->errstr = "Error creating connection BIO";
		return 0;
	}

	if (BIO_do_connect (pd->client_conn) <= 0) {
		pd->errstr = "Error connection to remote machine";
		return 0;
	}
	*/

	if (! (pd->client_ssl = SSL_new (pd->client_ctx))) {
		pd->errstr = "Error creating an SSL context";
		return 0;
	}

	// SSL_set_bio (pd->client_ssl, pd->client_conn, pd->client_conn);

	SSL_set_fd (pd->client_ssl, pd->client_fd);

	if (SSL_connect (pd->client_ssl) <= 0) {
		pd->errstr = "Error connection SSL object";
		return 0;
	}

	pd->client_or_server |= SF_CS_CLIENT;

	if (pd->verbose_level > 2) {
		printf ("SSL connection using %s\n",
				SSL_get_cipher (pd->client_ssl));
	}

	if (pd->verbose_level > 2) {
		X509*	cert;
		char	data[256];

		if ((cert = SSL_get_peer_certificate (pd->client_ssl))
				!= NULL) {
			X509_NAME_oneline (X509_get_issuer_name (cert),
					data, sizeof data);
			fprintf (stderr, " issuer = %s\n", data);

			X509_NAME_oneline (X509_get_subject_name (cert),
					data, sizeof data);

			fprintf (stderr, " subject = %s\n", data);

			X509_free (cert);
		}
	}

	return 1;
}

static int ssl_prepare_socket (struct SSL_lib_t *self,
					const int port, int *portno) {
	struct SSL_lib_pd_t	*pd = self->pd;
	char			on = 1;
	int			fd;
	struct sockaddr_in	sa;

	if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		pd->errstr = "socket";
		return -1;
	}

	setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof on);
	setsockopt (fd, SOL_SOCKET, SO_KEEPALIVE, (char*) &on, sizeof on);

	memset (&sa, 0, sizeof sa);
	sa.sin_family		= AF_INET;
	sa.sin_addr.s_addr	= INADDR_ANY;
	sa.sin_port		= htons (port);

	if (bind (fd, (struct sockaddr *) &sa, sizeof (struct sockaddr)) < 0) {
		pd->errstr = "bind";
		close (fd);
		return -1;
	} else {
		int	len = sizeof sa;

		if (getsockname (fd, (struct sockaddr *) &sa, &len) < 0) {
			pd->errstr = "getsockname";
			close (fd);
			return -1;
		}
	}

	if (portno != NULL) *portno = ntohs (sa.sin_port);

	return fd;
}

static int ssl_listen (struct SSL_lib_t *self, const int port) {
	struct SSL_lib_pd_t	*pd = self->pd;

	if (ssl_init_server (self) == 0) return 0;

	if ((pd->server_fd = ssl_prepare_socket (self, port, NULL)) < 0) {
		return 0;
	}

	listen (pd->server_fd, 5);

	return 1;
}

static int ssl_accept (struct SSL_lib_t *self) {
	struct SSL_lib_pd_t	*pd = self->pd;
	struct sockaddr_in	client_addr;
	int			fd;
	size_t			client_len;

	client_len = sizeof client_addr;

	fd = accept (pd->server_fd,
			(struct sockaddr *) &client_addr, &client_len);

	if (! (pd->server_ssl = SSL_new (pd->server_ctx))) {
		pd->errstr = "Error creating an SSL context";
		close (fd);
		return -1;
	}

	SSL_set_accept_state (pd->server_ssl);
	SSL_set_fd (pd->server_ssl, fd);

	if (SSL_accept (pd->server_ssl) <= 0) {
		pd->errstr = "Error accepting SSL connection";
		close (fd);
		return -1;
	}

	pd->client_or_server |= SF_CS_SERVER;

	return fd;
}

static int ssl_client_read (struct SSL_lib_t *self, void *buf, size_t len) {
	struct SSL_lib_pd_t	*pd = self->pd;

	return SSL_read (pd->client_ssl, buf, len);
}

static int ssl_server_read (struct SSL_lib_t *self, void *buf, size_t len) {
	struct SSL_lib_pd_t	*pd = self->pd;

	return SSL_read (pd->server_ssl, buf, len);
}

static int ssl_do_SSL_read (struct SSL_lib_t *self, void *buf, size_t len) {
	struct SSL_lib_pd_t	*pd = self->pd;

	if ((pd->client_or_server & SF_CS_CLIENT) != 0) {
		return self->client_read (self, buf, len);
	} else if ((pd->client_or_server & SF_CS_SERVER) != 0) {
		return self->server_read (self, buf, len);
	} else {
		self->pd->errstr = "No connection";
	}

	return -1;
}

static int ssl_client_write (struct SSL_lib_t *self, void *buf, size_t len) {
	struct SSL_lib_pd_t	*pd = self->pd;

	return SSL_write (pd->client_ssl, buf, len);
}

static int ssl_server_write (struct SSL_lib_t *self, void *buf, size_t len) {
	struct SSL_lib_pd_t	*pd = self->pd;

	return SSL_write (pd->server_ssl, buf, len);
}

static int ssl_do_SSL_write (struct SSL_lib_t *self, void *buf, size_t len) {
	struct SSL_lib_pd_t	*pd = self->pd;

	if ((pd->client_or_server & SF_CS_CLIENT) != 0) {
		return self->client_write (self, buf, len);
	} else if ((pd->client_or_server & SF_CS_SERVER) != 0) {
		return self->server_write (self, buf, len);
	} else {
		self->pd->errstr = "No connection";
	}

	return -1;
}


static int ssl_set_CAdir (struct SSL_lib_t *self, const char *path) {
	if (self->pd->CAdir != NULL) free (self->pd->CAdir);
	if ((self->pd->CAdir = strdup (path)) != NULL) return 1;
	return 0;
}

static int ssl_set_CAfile (struct SSL_lib_t *self, const char *path) {
	if (self->pd->CAfile != NULL) free (self->pd->CAfile);
	if ((self->pd->CAfile = strdup (path)) != NULL) return 1;
	return 0;
}

static int ssl_set_CERTfile (struct SSL_lib_t *self, const char *path) {
	if (self->pd->CERTfile != NULL) free (self->pd->CERTfile);
	if ((self->pd->CERTfile = strdup (path)) != NULL) return 1;
	return 0;
}

static const char * ssl_errstr (struct SSL_lib_t *self) {
	return self->pd->errstr;
}

static int ssl_server_fd (struct SSL_lib_t *self) {
	return self->pd->server_fd;
}

static int ssl_client_fd (struct SSL_lib_t *self) {
	return self->pd->client_fd;
}

static void ssl_free_server (struct SSL_lib_t *self) {
	SSL_free (self->pd->server_ssl);
	self->pd->server_ssl = NULL;
}

static void ssl_free_client (struct SSL_lib_t *self) {
	SSL_free (self->pd->client_ssl);
	self->pd->client_ssl = NULL;
}

static int ssl_clear_server (struct SSL_lib_t *self) {
	return SSL_clear (self->pd->server_ssl);
}

static int ssl_clear_client (struct SSL_lib_t *self) {
	return SSL_clear (self->pd->client_ssl);
}

static int ssl_shutdown_server (struct SSL_lib_t *self) {
	return SSL_shutdown (self->pd->server_ssl);
}

static int ssl_shutdown_client (struct SSL_lib_t *self) {
	return SSL_shutdown (self->pd->client_ssl);
}

static int ssl_verbose (struct SSL_lib_t *self, const int l) {
	int	rc = self->pd->verbose_level;

	self->pd->verbose_level = l;
	return rc;
}

static void ssl_dispose (struct SSL_lib_t *self) {
	struct SSL_lib_pd_t	*pd;

	if ((pd = self->pd) != NULL) {
		if (pd->CAdir       != NULL) free (pd->CAdir);
		if (pd->CAfile      != NULL) free (pd->CAfile);
		if (pd->CERTfile    != NULL) free (pd->CERTfile);
		if (pd->cipher_list != NULL) free (pd->cipher_list);

		if (pd->server_ssl  != NULL) SSL_free (pd->server_ssl);
		if (pd->client_ssl  != NULL) SSL_free (pd->client_ssl);

		if (pd->server_ctx  != NULL) SSL_CTX_free (pd->server_ctx);
		if (pd->client_ctx  != NULL) SSL_CTX_free (pd->client_ctx);

		free (self->pd);
	}

	free (self);
}

struct SSL_lib_t *new_SSL_lib (void) {
	struct SSL_lib_t	*self = NULL;
	struct SSL_lib_pd_t	*pd = NULL;

	while ((self = malloc (sizeof *self)) != NULL) {
		if ((pd = malloc (sizeof *pd)) == NULL) {
			free (self);
			break;
		}

		self->pd = pd;

		pd->verbose_level	= 1;

		pd->client_fd		= -1;
		pd->server_fd		= -1;

		pd->client_ctx		= NULL;
		pd->server_ctx		= NULL;

		pd->client_ssl		= NULL;
		pd->server_ssl		= NULL;

		pd->client_conn		= NULL;
		pd->client_or_server	= 0;

		pd->errstr		= NULL;

		pd->CAdir		= NULL;
		pd->CAfile		= NULL;
		pd->cipher_list		= strdup (DEFAULT_CIPHER_LIST);
		

		self->dispose		= ssl_dispose;

		self->verbose		= ssl_verbose;
		self->set_CAdir		= ssl_set_CAdir;
		self->set_CAfile	= ssl_set_CAfile;
		self->set_CERTfile	= ssl_set_CERTfile;

		self->client_read	= ssl_client_read;
		self->client_write	= ssl_client_write;

		self->server_read	= ssl_server_read;
		self->server_write	= ssl_server_write;

		self->read		= ssl_do_SSL_read;
		self->write		= ssl_do_SSL_write;

		self->server_fd		= ssl_server_fd;
		self->client_fd		= ssl_client_fd;

		self->errstr		= ssl_errstr;

		self->listen		= ssl_listen;
		self->accept		= ssl_accept;
		self->connect		= ssl_connect;
		self->disconnect	= ssl_disconnect;

		self->clear_server	= ssl_clear_server;
		self->clear_client	= ssl_clear_client;
		self->free_server	= ssl_free_server;
		self->free_client	= ssl_free_client;
		self->shutdown_server	= ssl_shutdown_server;
		self->shutdown_client	= ssl_shutdown_client;

		if (! ssl_init_OpenSSL (self)) {
			self->dispose (self);
			self = NULL;
		}

		break;
	}

	return self;
}
