/*
 *	osd.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "fmplib.h"
#include "conio.h"
#include "rmosdlib.h"
#include "rmimage256.h"
#include "koklib.h"

// #define _DEFINE_FMP_TYPES_
// #include "fmp.h"
// #include "ifmp.h"
// #include "osthread.h"
// #include "osdsupport.h"
// #include "handlemedia.h"
// #include "osdcontrol.h"

int main (int argc, char *argv[]) {
	// INFO 		pInfo;
	struct rmimage256_t	*img;
	struct rmimage256_t	*img2;
	struct koklib_t		*kok;
	struct koklib_bitmap_t	*kbmp = NULL;
	int			i = 0;
	int			first = 1;
	int			pause;
	int			step;
	int			streamtype = FMP_PROGRAM_STREAM;
	struct fmplib_t 	*fmp;
	struct rmosdlib_t	*osd;
	char			*mpegfile = "/home/cc/liujc/MPEG/000001.mpeg";
	int			fd, len;
	int			imglen;
	int			np = 0;
	int			kidx = 0;
	int			terminate = 0;
	struct consoleIO_t	*conio;
	int			display_fonts = 1;
	int			sid = 0;
	int			cnt = 0;
	int			from_x, to_x;
	int			img_x = 600, img_y = 160;
	//int			fcolor[2] = {   4, 17 };
	//int			bcolor[2] = {   6, 20 };
	int			fcolor[2] = {    7, 4 };
	int			bcolor[2] = {   20, 6 };
	int			cidx1 = 0;
	int			cidx2 = 1;
	int			last_w = 0, last_h = 0;
	const int		border_width = 2;
	const int		start_x = 10;
	const int		start_y = 3;

	if (argc > 1) fcolor[0] = atoi (argv[1]);

	if ((kok = new_koklib ()) == NULL) {
		perror ("new_koklib");
		exit (1);
	} else {
		int	ii;

		if ((np = kok->load (kok, "KOK/1234-2.kok")) < 0) return 2;
		// if ((np = kok->load (kok, "KOK/sorry3.kok")) < 0) return 2;

		if ((kbmp = calloc (np,
				sizeof (struct koklib_bitmap_t))) == NULL) {
			perror ("calloc");
			return 2;
		}

		for (ii = 0; ii < np; ii++) {
			kbmp[ii].size = 0;
			kok->read_para (kok);
			kok->read_bitmap (kok, &kbmp[ii]);
		}
	}

	if ((img = new_rmimage256 (img_x + 1, img_y + 1)) == NULL) {
		perror ("new_rmimage256");
		exit (1);
	} else {
		img->rectangle (img, 0, 0, img_x - 1, img_y - 1, 2, 0);
		/*
		img->rectangle (img, 0, 0, img_x - 1, img_y - 1, 2, 0);
		img->line (img, 0, 0, img_x - 1, img_y - 1, 3);
		img->line (img, img_x - 1, 0, - img_x, img_y, 4);
		*/

	}

	if ((fmp = new_fmplib ()) == NULL) {
		perror ("new_fmplib");
		exit (1);
	}

	if ((osd = new_rmosdlib ()) == NULL) {
		perror ("new_rosdlib");
		exit (1);
	}


	if ((fd = open (mpegfile, O_RDONLY)) < 0) {
		perror (mpegfile);
		exit (1);
	}

	conio = new_consoleIO ();

	from_x = to_x = 0;

	do {
		if (! fmp->driver_entry ()) break;

		conio->open ();

		fmp->open (streamtype);
		fmp->set_volume (80);
		fmp->play ();

		pause = step = 0;

		// if (! initOSD (&pInfo)) terminate = 1;
		osd->open (osd);

		if (first) {
			/*
			img2 = new_rmimage256_from_raw (bmp1);
			free (bmp1);
			*/
			first = 0;

			// img->copyPalette (img, img2);
			// img->paste (img, 10, 10, img2);

			/*
			imglen = img->drawText (img, 48, 55,
					fcolor[cidx1], 0, 2, 9,
					"數位相機的規格介紹");
					*/
			//		"國立中央大學");
			//
			img->packedBitmap (img, start_x, start_y,
					fcolor[cidx1], 0,
					kbmp[kidx].bmp,
					kbmp[kidx].width,
					kbmp[kidx].height,
					kbmp[kidx].real_width);

			imglen = kbmp[kidx].real_width;

			last_w = imglen + border_width * 2;
			last_h = kbmp[kidx].height + border_width * 2;

			img->border (img, start_x - border_width,
					start_y - border_width,
					last_w, last_h, fcolor[cidx1],
					bcolor[cidx1], border_width);
		}

		while (! terminate) {
			if (pause) {
				usleep (10000);
			} else {
				if (step) {
					while (! conio->kbhit ()) {
						usleep (10000);
					}
				}

				if (fmp->get_buffer () == 0) {
					perror ("get_buffer");
					break;
				}

				if ((len = read (fd, fmp->buffer_ptr (),
						fmp->buffer_size ())) <= 0) {
					lseek (fd, 0, SEEK_SET);
					len = read (fd, fmp->buffer_ptr (),
							fmp->buffer_size ());
				}

				if (display_fonts == 1) {
					i = (i + 1) % 4;
					from_x = to_x;
					to_x += 4;

					if (to_x < imglen) {
						img->atob (img, start_x -
							        border_width
						       	        + from_x,
							        start_y,
								to_x - from_x,
								last_h,
								fcolor[cidx1],
								fcolor[cidx2]);
						img->atob (img, start_x -
							        border_width
								+ from_x,
								start_y,
								to_x - from_x,
								last_h,
								bcolor[cidx1],
								bcolor[cidx2]);
					} else {
						img->rectangle (img,
							start_x - border_width,
							start_y - border_width,
							last_w, last_h,
							0, 1);

						kidx = (kidx + 1) % np;

						img->packedBitmap (img,
							start_x,
							start_y,
							fcolor[cidx1], 0,
							kbmp[kidx].bmp,
							kbmp[kidx].width,
							kbmp[kidx].height,
							kbmp[kidx].real_width);

						imglen = kbmp[kidx].real_width;

						last_w = imglen + 2;
						last_h = kbmp[kidx].height + 2;

						img->border (img,
							start_x - border_width,
							start_y - border_width,
								last_w,
								last_h,
								fcolor[cidx1],
								bcolor[cidx1],
								border_width);

						from_x = to_x = 0;
						/*
						int	tmp_idx;

						tmp_idx = cidx1 + 1;
						cidx2 = cidx1;
						cidx1 = tmp_idx % 2;

						from_x = to_x = 0;
						*/
					}
					osd->displayBitmap (osd,
							img->image (img), 0);
					fprintf (stderr, "\r%c", "\\|/-"[i]);
				} else if (display_fonts == 2) {
					img->rectangle (img, 0, 0,
						img_x - 1, img_y - 1, i, 0);
					img->line (img, 0, 0,
						img_x - 1, img_y - 1, i+1);
					img->line (img, img_x - 1, 0,
						- img_x, img_y, i+1);

					// img->paste (img, 10, 10, img2);

					i = (i + 1) % 4;
					osd->displayBitmap (osd,
							img->image (img), 0);
					fprintf (stderr, "\r%c", "\\|/-"[i]);
				} else {
					i = i % 14 + 1;
				}

				fmp->set_data_size (len);
				fmp->push ();
			}

			if (conio->kbhit ()) {
				switch (conio->read ()) {
				case 's':
					step = (step + 1) % 2;
					break;
				case 'q':
					terminate = 1;
					break;
				case 'p':
					pause = (pause + 1) % 2;
					break;
				case 'x':
					display_fonts = (display_fonts + 1) % 3;
					break;
				}
			}
		}

		osd->close (osd);
	} while (0);

	conio->close ();
	fmp->stop ();
	fmp->close ();
	fmp->driver_unload ();

	return 0;
}
