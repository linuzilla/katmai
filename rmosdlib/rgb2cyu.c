#include <stdio.h>
#include <stdlib.h>

static void RGB_to_YUV (const double r, const double g, const double b) {
	int	y, u, v;

	/*
	*	Y  =  0.257R + 0.504G + 0.098B +  16
	*	Cb = -0.148R - 0.291G + 0.439B + 128
	*	Cr =  0.439R - 0.368G - 0.071B + 128
	*/

	y = (int) ( 0.257 * r + 0.504 * g + 0.098 * b +  16);
	u = (int) (-0.148 * r + 0.291 * g + 0.439 * b + 128);
	v = (int) ( 0.439 * r - 0.368 * g - 0.071 * b + 128);

	printf ("R=%3d, G=%3d, B=%3d\n", (int) r, (int) g, (int) b);
	printf ("Y=%3d, U=%3d, V=%3d\n", y, u, v);
	printf ("Y=0x%02x, U=0x%02x, V=0x%02x\n", y, u, v);
}

static void YUV_to_RGB (const double y, const double u, const double v) {
	int	r, g, b;
	double	Y, Cb, Cr;

	/*
	*	R  = 1.164 (Y - 16)                    + 1.596 (Cr - 128)
	*	G  = 1.164 (Y - 16) - 0.391 (Cb - 128) - 0.813 (Cr - 128)
	*	B  = 1.164 (Y - 16) + 2.018 (Cb - 128)
	*/

	Y  = y +  16.0;
	Cb = u - 128.0;
	Cr = v - 128.0;

	r = (int) (1.164 * Y              + 1.596 * Cr);
	g = (int) (1.164 * Y - 0.391 * Cb - 0.813 * Cr);
	b = (int) (1.164 * Y + 2.018 * Cb);

	printf ("Y=%3d, U=%3d, V=%3d\n", (int) Y, (int) Cb, (int) Cr);
	printf ("R=%3d, G=%3d, B=%3d\n", r, g, b);
}

int main (int argc, char *argv[]) {
	double	r, g, b;

	/*
	*	Y  =  0.257R + 0.504G + 0.098B +  16
	*	Cb = -0.148R - 0.291G + 0.439B + 128
	*	Cr =  0.439R - 0.368G - 0.071B + 128
	*
	*	R  = 1.164 (Y - 16)                    + 1.596 (Cr - 128)
	*	G  = 1.164 (Y - 16) - 0.391 (Cb - 128) - 0.813 (Cr - 128)
	*	B  = 1.164 (Y - 16) + 2.018 (Cb - 128)
	*/

	if (argc != 4) {
		fprintf (stderr, "usage: %s r g b\n", argv[0]);
		return 0;
	}

	r = (double) (atoi (argv[1]) % 256);
	g = (double) (atoi (argv[2]) % 256);
	b = (double) (atoi (argv[3]) % 256);

	RGB_to_YUV (r, g, b);
	fprintf (stderr, "\n---------------\n\n");
	YUV_to_RGB (r, g, b);

	/* ----------------------------------------------------- */

	return 0;
}
