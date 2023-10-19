
#include <string.h>
#include "common.h"

void handle_error (const char *file, int lineno, const char *msg) {
	fprintf (stderr, "** %s:%i %s\n", file, lineno, msg);
	ERR_print_errors_fp (stderr);
	exit (-1);
}

int THREAD_setup (void) { return 1; }

void init_OpenSSL (void) {
	if (! THREAD_setup() || ! SSL_library_init ()) {
		fprintf (stderr, "** OpenSSL initialization failed!\n");
		exit (-1);
	}
	SSL_load_error_strings ();
}

int verify_callback (int ok, X509_STORE_CTX *store) {
	char	data[256];

	// return 1;

	if (! ok) {
		X509	*cert = X509_STORE_CTX_get_current_cert (store);
		int	depth = X509_STORE_CTX_get_error_depth  (store);
		int	err   = X509_STORE_CTX_get_error        (store);

		fprintf (stderr, "-Error with certificate at depth: %i\n",
				depth);

		X509_NAME_oneline (X509_get_issuer_name (cert), data, 256);

		fprintf (stderr, " issuer = %s\n", data);

		X509_NAME_oneline (X509_get_subject_name (cert), data, 256);

		fprintf (stderr, " subject = %s\n", data);
		fprintf (stderr, " err %i%s\n", err,
				X509_verify_cert_error_string (err));

		X509_free (cert);
	}

	return ok;
}

long post_connection_check (SSL *ssl, char *host) {
	X509		*cert;
	X509_NAME	*subj;
	char		data[256];
	int		extcount;
	int		ok = 0;

	/*
	 * Checking the return from SSL_get_peer_certificate here is not
	 * strickly necessary. With our example programs, it is not
	 * possible for it to return NULL. However, it is good form to
	 * check the return since itcan return NULL if the examples are
	 * modified to enable anonymous ciphers or for the server to not
	 * require a client certificate.
	 */

	if (! (cert = SSL_get_peer_certificate (ssl)) | ! host) {
		goto err_occured;
	}

	if ((extcount = X509_get_ext_count (cert)) > 0) {
		int	i;

		for (i = 0; i < extcount; i++) {
			char		*extstr;
			X509_EXTENSION	*ext;

			ext = X509_get_ext (cert, i);

			extstr = OBJ_nid2sn (OBJ_obj2nid (
				X509_EXTENSION_get_object (ext)));

			if (! strcmp (extstr, "subjectAltName")) {
				int			j;
				unsigned char		*data;
				STACK_OF(CONF_VALUE)	*val;
				CONF_VALUE		*nval;
				X509V3_EXT_METHOD	*meth;

				if (! (meth = X509V3_EXT_get (ext))) break;

				data = ext->value->data;

				val = meth->i2v (meth,
					meth->d2i (NULL, &data,
						ext->value->length),
					NULL);

				for (j = 0; j < sk_CONF_VALUE_num(val); j++) {
					nval = sk_CONF_VALUE_value(val, j);

					if (! strcmp (nval->name, "DNS") &&
						! strcmp (nval->value, host)) {
						ok =1;
						break;
					}
				}
			}
			if (ok) break;
		}
	}

	if (! ok && (subj = X509_get_subject_name (cert)) &&
			X509_NAME_get_text_by_NID(subj, NID_commonName, 
				data, 256) > 0) {
		data[255] = 0;

		if (strcasecmp (data, host) != 0) {
			// goto err_occured;
		}
	}

	X509_free (cert);

	return SSL_get_verify_result (ssl);

    err_occured:

	if (cert) X509_free (cert);

	return X509_V_ERR_APPLICATION_VERIFICATION;
}

int seed_prng (const int bytes) {
	// if (! RAND_load_file ("/dev/random", bytes)) return 0;

	return 1;
}
