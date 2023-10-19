#include <stdio.h>
#include <stdlib.h>
#include "koklib.h"

int main (int argc, char *argv[]) {
	struct koklib_t		*kok;
	int			i, np;
	struct koklib_bitmap_t	bmp;

	if ((kok = new_koklib ()) == NULL) {
		perror (NULL);
		return 1;
	}

	if ((np = kok->load (kok, "KOK-V4/555555.0")) < 0) {
		return 2;
	} else {
		int	j, k, ii, *rgb;
		int	bw_rgb[2];

		for (j = 0; j < 2; j++) {
			rgb = kok->get_RGB (kok, j);

			// common(b,w), boy, girl, mix
			for (i = ii = 0; i < 4; i++, ii += 2) {
				bw_rgb[0] = rgb[ii];
				bw_rgb[1] = rgb[ii + 1];

				/*
				for (k = 0; k < 2; k++) {
					img->setPaletteRGB (img,
						convmap[j][i][k],
						bw_rgb[k] & 0xff,
						(bw_rgb[k] >> 8) & 0xff,
						(bw_rgb[k] >> 16) & 0xff);
				}
				*/

				fprintf (stderr, "[%06x,%06x]",
						bw_rgb[0], bw_rgb[1]);
			}
			fprintf (stderr, "\n");
		}
	}


	/*
	for (i = 0; i < np; i++) {
		kok->read_para (kok);
		kok->read_bitmap (kok, &bmp);
		fprintf (stderr, "[%d,%d]%s\n", bmp.width, bmp.height);
		free (bmp.bmp);
	}
	*/

	kok->dispose (kok);

	return 0;
}
