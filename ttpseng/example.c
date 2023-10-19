/*
 *	main.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef linux
#include <getopt.h>
#endif
//#include "big5ps.h"
#include "ttpseng.h"

#define MAXLINE			4096
#define MAX_CMD_LEN		5

int main (int argc, char *argv[]) {
	TrueTypePS_Engine	*engine;
	FILE			*fp = stdin;
	FILE			*psfp = NULL;
	char			*infile = NULL;
	int			c, outbin = 0, tray = 0;
	int			wrap = 0;
	int			errflag = 0;
	char			*outfile = "-";
	char			*lc_ctype = NULL;
	
	while ((c = getopt (argc, argv, "Ehwf:o:b:t:")) != EOF) {
		switch (c) {
		case 'E':
			if ((lc_ctype = getenv ("LC_CTYPE")) != NULL) {
				fprintf (stderr, "setlocal (LC_CTYPE, %s)\n",
						lc_ctype);
			}
			break;
		case 'w':
			wrap = 1;
			break;
		case 'b':
			outbin = atoi (optarg);
			break;
		case 't':
			tray = atoi (optarg);
			break;
		case 'o':
			outfile = optarg;
			break;
		case 'f':
			// fontfile[0] = optarg;
			break;
		case 'h':
		case '?':
		default:
			errflag++;
			break;
		}
	}

	fprintf (stderr, "big5ps %s, Copyright (c) 2002, Jiann-Ching Liu\n\n",
			TTPSENG_VERSION);
	if (errflag) {
		fprintf (stderr, "usage: "
			"big5ps [-Ew][-b outbin][-t tray][-o outfile] file\n");
		return 1;
	}

	if (optind < argc) {
		infile = argv[optind];

		if (strcmp (argv[optind], "-") == 0) {
			fprintf (stderr, "Input from stdin\n");
			fp = stdin;
		} else {
			//fprintf (stderr, "Input file = [%s]\n", argv[optind]);

			if ((fp = fopen (argv[optind], "r")) == NULL) {
				perror (argv[optind]);
				return 1;
			}
		}
	} else {
		fprintf (stderr, "No input file\n");
		return 1;
	}

	if (strcmp (outfile, "-") == 0) {
		psfp = stdout;
	} else {
		if ((psfp = fopen (outfile, "w")) == NULL) {
			perror (outfile);
			return 1;
		}
	}

	if ((engine = Initial_TrueTypePS_Engine (psfp, lc_ctype)) == NULL) {
		fprintf (stderr, "Fail on  Initial_TrueTypePS_Engine\n");
		return 1;
	}

	if (fp != stdin) engine->set_filename (infile);
	if (outbin != 0) engine->outbin (outbin);
	if (tray   != 0) engine->tray (tray);
	if (wrap   != 0) engine->wrap (1);
	/*
	for (i = 0; i < 2; i++) {
		if ((fontid[i] = engine->new_FontFace (fontfile[i])) < 0) {
			fprintf (stderr, "%s: some error\n", fontfile[i]);
			return 1;
		}
	}
	*/

	//fprintf (stderr, "Old font size = %.2f\n", engine->set_fontsize (24));

	// engine->set_background_eps (
	//		"/home/cc/liujc/dispatcher-0.1.25/hdr/ncucc.eps");

	engine->scan (fp);
	fclose (fp);

	fprintf (stderr, "%d page(s) printed\n", engine->showpage ());
	engine->close ();

	return 0;
}
