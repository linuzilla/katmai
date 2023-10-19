#include <stdio.h>
#include <fcntl.h>
#include "bzlib.h"

int main (int argc, char *argv[]) {
	bz_stream	bzs;
	int		rc;
	int		action;
	int		fd, fw, len;
	char		buffer[65536];
	char		outbuf[65536];

	fprintf (stderr, "bz2 Version %s\n", BZ2_bzlibVersion ());

	if (argc < 3) {
		fprintf (stderr, "%s <infile> <outfile>\n", argv[0]);
		return 0;
	}

	bzs.bzalloc = NULL;
	bzs.bzfree  = NULL;
	bzs.opaque  = NULL;
	bzs.next_in = bzs.next_out = NULL;
	bzs.avail_in = bzs.avail_out = 0;

	if ((rc = BZ2_bzCompressInit (&bzs, 9, 4, 0)) != BZ_OK) {
		switch (rc) {
		case BZ_CONFIG_ERROR:
			fprintf (stderr, "Library has been mis-compiled!\n");
			break;
		case BZ_PARAM_ERROR:
			fprintf (stderr, "parameter incorrect!\n");
			break;
		case BZ_MEM_ERROR:
			fprintf (stderr, "Not enought memory is abailable!\n");
			break;
		}
	}

	if ((fd = open (argv[1], O_RDONLY)) < 0) {
		perror (argv[1]);
		return 1;
	} else if ((fw = open (argv[2], O_WRONLY|O_CREAT|O_EXCL, 0644)) < 0) {
		perror (argv[2]);
		return 1;
	}

	action = BZ_RUN;
	while ((len = read (fd, buffer, sizeof buffer)) > 0) {
		bzs.avail_in  = len;
		bzs.next_in   = buffer;
		bzs.next_out  = outbuf;
		bzs.avail_out = sizeof outbuf;

		if (BZ2_bzCompress (&bzs, action)) {
			write (fw, bzs.next_out, bzs.avail_out);
		}

		action = BZ_FINISH;
	}


	close (fd);
	close (fw);
}
