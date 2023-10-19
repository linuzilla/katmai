/*
 *	rmimage256.c
 *
 *	Copyright (c) 2002-2003, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rmimage256.h"
#include "rmpalette.h"
#include "etenlib.h"

#define PALETTE_OFFSET	8
#define IMAGE_OFFSET	1032

#ifdef ABS
#undef ABS
#endif

#define ABS(x)	(((x) >= 0) ? (x) : (-(x)))

struct ETlib_t	*et = NULL;
static int	have_eten = 0;	// Unknow

static int init_eten (const char *path) {
	if (have_eten != 0) return have_eten;

	if (et == NULL) {
		if ((et = new_etenlib ()) == NULL) {
			have_eten = -1;
			return -1;
		}
	}

	if (! et->open (path)) {
		fprintf (stderr, "ET fonts not found !!\n");
		have_eten = -1;
		return -1;
	}

	have_eten = 1;

	return 1;
}

static int img_size   (struct rmimage256_t *self) { return self->pd.size;   }
static int img_width  (struct rmimage256_t *self) { return self->pd.width;  }
static int img_height (struct rmimage256_t *self) { return self->pd.height; }

static void img_dispose (struct rmimage256_t *self) {
	free (self->pd.img);
	free (self);
}

static unsigned char * img_image (struct rmimage256_t *self) {
	return self->pd.img;
}

static void img_loadPalette (struct rmimage256_t *self, void *palette) {
	memcpy (&(self->pd.img[PALETTE_OFFSET]), palette, 1024);
}

static void img_copyPalette (struct rmimage256_t *self,
						struct rmimage256_t *other) {
	memcpy (&(self->pd.img[PALETTE_OFFSET]),
				&(other->pd.img[PALETTE_OFFSET]), 1024);
}

static void img_setPaletteRGB (struct rmimage256_t *self, const int entry,
				const int rr, const int gg, const int bb) {
	char	*ptr = &(self->pd.img[PALETTE_OFFSET]);
	int	i, r, g, b, yuv[3];
	unsigned char	ayuv[] = { 0xff, 0, 0, 0 };

	r = (double) (rr % 256);
	g = (double) (gg % 256);
	b = (double) (bb % 256);

#ifdef USE_YCBCR_TO_RGB	
	yuv[0] = (int) ( 0.257 * r + 0.504 * g + 0.098 * b +  16);
	yuv[1] = (int) (-0.148 * r - 0.291 * g + 0.439 * b + 128);
	yuv[2] = (int) ( 0.439 * r - 0.368 * g - 0.071 * b + 128);
#else
	yuv[0] = (int) ( 0.298 * r + 0.587 * g + 0.114 * b);
	yuv[1] = (int) (-0.169 * r - 0.331 * g + 0.500 * b + 128);
	yuv[2] = (int) ( 0.500 * r - 0.419 * g - 0.081 * b + 128);
#endif

	for (i = 0; i < 3; i++) {
		if (yuv[i] < 0) {
			yuv[i] = 0;
		} else if (yuv[i] > 0xff) {
			yuv[i] = 0xff;
		}

		ayuv[i + 1] = (unsigned char) yuv[i];
	}

	if ((entry >= 0) && (entry <= 255)) {
		memcpy (&ptr[entry * 4], ayuv, 4);
	}
}

static void img_setPaletteYUV (struct rmimage256_t *self, const int entry,
				const int yy, const int uu, const int vv) {
	char		*ptr = &(self->pd.img[PALETTE_OFFSET]);
	unsigned char	ayuv[] = { 0xff, 0, 0, 0 };

	ayuv[1] = (unsigned char) (yy % 256);
	ayuv[2] = (unsigned char) (uu % 256);
	ayuv[3] = (unsigned char) (vv % 256);

	if ((entry >= 0) && (entry <= 255)) {
		memcpy (&ptr[entry * 4], ayuv, 4);
	}
}

static void img_setPaletteColor (struct rmimage256_t *self,
					const int entry, char *aRGB) {
	char	*ptr = &(self->pd.img[PALETTE_OFFSET]);

	if ((entry >= 0) && (entry <= 255)) {
		memcpy (&ptr[entry * 4], aRGB, 4);
	}
}

static void img_line (struct rmimage256_t *self,
				const int xx, const int yy,
				const int xl, const int yl,
				const int cidx) {
	char	*ptr = &(self->pd.img[IMAGE_OFFSET]);
	int	w    = self->pd.width;
	int	h    = self->pd.height;
	int	i, j, k;
	float	r;

	if ((cidx < 0) || (cidx > 255)) return;

	if (xl >= yl) {
		r = (float) yl / (float) xl;
		for (i = xx, j = 0; j < xl; i++, j++) {
			k = (int)(r * j) + yy;
			if ((i >= 0) && (i < w) && (k >= 0) && (k < h)) {
				ptr[i + k * w] = cidx;
			}
		}
	} else {
		r = (float) xl / (float) yl;
		for (i = yy, j = 0; j < yl; i++, j++) {
			k = (int)(r * j) + xx;
			if ((k >= 0) && (k < w) && (i >= 0) && (i < h)) {
				ptr[k + i * w] = cidx;
			}
		}
	}
}

static void img_lineto (struct rmimage256_t *self,
				const int x1, const int y1,
				const int x2, const int y2,
				const int cidx) {
	img_line (self, x1, y1, x2 - x1, y2 - y1, cidx);
}

static void img_rectangle (struct rmimage256_t *self,
				const int xx, const int yy,
				const int xl, const int yl,
				const int cidx, const int fill) {
	char	*ptr = &(self->pd.img[IMAGE_OFFSET]);
	int	w    = self->pd.width;
	int	h    = self->pd.height;
	int	i, j, k, x, y, xc, yc;

	if ((cidx < 0) || (cidx > 255)) return;

	if (fill) {
		for (y = yy, yc = 0; yc < yl; y++, yc++) {
			if ((y >= 0) && (y < h)) {
				k = y * w;
				for (x = xx, xc = 0; xc < xl; x++, xc++) {
					if ((x >= 0) && (x < w)) {
						ptr[x + k] = cidx;
					}
				}
			}
		}
	} else {
		k = yy + yl;
		x = yy * w;
		y = k  * w;

		for (i = xx, j = 0; j < xl; i++, j++) {
			if ((i >= 0) && (i < w)) {
				if ((yy >= 0) && (yy < h)) ptr[i + x] = cidx;
				if ((k  >= 0) && (k  < h)) ptr[i + y] = cidx;
			}
		}

		x = xx;
		k = xx + xl;
		for (i = yy, j = 0; j < yl; i++, j++) {
			if ((i >= 0) && (i < h)) {
				y = i * w;

				if ((x >= 0) && (x < w)) ptr[x + y] = cidx;
				if ((k >= 0) && (k < w)) ptr[k + y] = cidx;
			}
		}
	}
}

static void img_atob (struct rmimage256_t *self,
				const int xx, const int yy,
				const int xl, const int yl,
				const int aidx, const int bidx) {
	char	*ptr = &(self->pd.img[IMAGE_OFFSET]);
	int	w    = self->pd.width;
	int	h    = self->pd.height;
	int	k, x, y, xc, yc;

	for (y = yy, yc = 0; yc < yl; y++, yc++) {
		if ((y >= 0) && (y < h)) {
			k = y * w;
			for (x = xx, xc = 0; xc < xl; x++, xc++) {
				if ((x >= 0) && (x < w)) {
					if (ptr[x + k] == aidx) {
						ptr[x + k] = bidx;
					}
				}
			}
		}
	}
}

static void img_paste (struct rmimage256_t *self,
			const int xx, const int yy, struct rmimage256_t *img) {
	char	*ptr  = &(self->pd.img[IMAGE_OFFSET]);
	int	w     = self->pd.width;
	int	h     = self->pd.height;
	char	*iptr = &(img->pd.img[IMAGE_OFFSET]);
	int	xl    = img->pd.width;
	int	yl    = img->pd.height;
	int	i, k, x, y, xc, yc;

	for (y = yy, yc = 0; yc < yl; y++, yc++) {
		if ((y >= 0) && (y < h)) {
			k = y  * w;
			i = yc * xl;
			for (x = xx, xc = 0; xc < xl; x++, xc++) {
				if ((x >= 0) && (x < w)) {
					ptr[x + k] = iptr[xc + i];
				}
			}
		}
	}
}

static void img_border (struct rmimage256_t *self,
				const int xx, const int yy,
				const int xl, const int yl,
				const int fc , const int bc,
				const int bw) {
	char	*ptr = &(self->pd.img[IMAGE_OFFSET]);
	int	w    = self->pd.width;
	int	h    = self->pd.height;
	int	xb, yb, xe, ye;
	int	i, j, k, dist, pos, x, y;

	xe = xx + xl;
	ye = yy + yl;
	xb = (xx > 0) ? xx : 0;
	yb = (yy > 0) ? yy : 0;
	xe = (xe < w) ? xe : w;
	ye = (ye < h) ? ye : h;

	for (y = yb; y < ye; y++) {
		k = y * w;

		for (x = xb; x < xe; x++) {
			if (ptr[x + k] == fc) continue;
			if (ptr[x + k] == bc) continue;

			for (i = -bw; i <= bw; i++) {
				if (x + i <  0) continue;
				if (x + i >= w) continue;

				for (j = -bw; j <= bw; j++) {
					if (y + j <  0) continue;
					if (y + j >= h) continue;

					dist = ABS(i) + ABS(j);

					if (dist == 0) continue;
					if (dist > bw) continue;

					pos = (x + i) + (y + j) * w;

					if (ptr[pos] == fc) {
						ptr[x + k] = bc;
						i = j = bw;
						break;
					}
				}
			}
		}
	}
}

#ifdef OLD_BORDER_ALGORITHMS
static void img_border2 (struct rmimage256_t *self,
				const int xx, const int yy,
				const int xl, const int yl,
				const int fc , const int bc,
				const int bw) {
	char	*ptr = &(self->pd.img[IMAGE_OFFSET]);
	int	w    = self->pd.width;
	int	h    = self->pd.height;
	int	xb, yb, xe, ye;
	int	i, j, k, u, d, x, y;

	xe = xx + xl;
	ye = yy + yl;
	xb = (xx > 0) ? xx : 0;
	yb = (yy > 0) ? yy : 0;
	xe = (xe < w) ? xe : w;
	ye = (ye < h) ? ye : h;

	for (y = yb; y < ye; y++) {
		k = y * w;

		for (x = xb; x < xe; x++) {
			if (ptr[x + k] != fc) continue;

			for (i = 1; i <= bw; i++) {
				if (x - i >= 0) {
					if (ptr[x - i + k] != fc) {
						ptr[x - i + k] = bc;
					}
				}

				if (x + i < w) {
					if (ptr[x + i + k] != fc) {
						ptr[x + i + k] = bc;
					}
				}
			}

			for (i = 1; i <= bw; i++) {
				u = k - i * w;
				d = k + i * w;

				if (y - i >= 0) {
					if (ptr[x + u] != fc) {
						ptr[x + u] = bc;
					}
				}

				if (y + i < h) {
					if (ptr[x + d] != fc) {
						ptr[x + d] = bc;
					}
				}
			}

			for (j = 1; j < bw; j++) {
				u = k - w * j;
				d = k + w * j;

				if (y - i >= 0) {
					for (i = 1; i < bw; i++) {
						if (x - i >= 0) {
							if (ptr[x+u-i] != fc) {
								ptr[x+u-i] = bc;
							}
						}

						if (x + i >= w) {
							if (ptr[x+u+i] != fc) {
								ptr[x+u+i] = bc;
							}
						}
					}
				}

				if (y + i < h) {
					for (i = 1; i < bw; i++) {
						if (x - i >= 0) {
							if (ptr[x+d-i] != fc) {
								ptr[x+d-i] = bc;
							}
						}

						if (x + i >= w) {
							if (ptr[x+d+i] != fc) {
								ptr[x+d+i] = bc;
							}
						}
					}
				}
			}
		}
	}
}
#endif

static int img_packedBitmap (struct rmimage256_t *self,
				const int xx, const int yy,
				const int fidx, const int bidx,
				const char *bmp,
				const int width, const int height,
				const int realwidth) {
	char		*ptr  = &(self->pd.img[IMAGE_OFFSET]);
	int		w     = self->pd.width;
	int		h     = self->pd.height;
	int		i, j, k, ww, hh, x, y;
	int		bmb;
	int		c;

	for (hh = height - 1, y = yy; hh >= 0; hh--, y++) {
		if (y < 0) continue;
		if (y >= h) break;

		j = y * w;
		k = hh * width;

		for (ww = 0, x = xx; ww < width; ww++) {
			bmb = bmp[k + ww];

			for (i = 0; i < 8; i++, x++, bmb <<= 1) {
				if (x < 0) continue;
				if (x >= w) break;
				if (x - xx > realwidth) break;

				c = ((bmb & 0x80) != 0) ? fidx : bidx;
				ptr[j + x] = c;
			}
		}
	}

	return 1;
}

static int img_drawText (struct rmimage256_t *self,
				const int xx, const int yy,
				const int fidx, const int bidx,
				const int scale, const int space,
				const char *string) {
	char		*ptr  = &(self->pd.img[IMAGE_OFFSET]);
	int		w     = self->pd.width;
	int		h     = self->pd.height;
	int		i, j, v, k, len;
	int		x, y, xw, yh, xc, yc, xs, ys;
	int		cidx;
	unsigned char	word, lookahead;
	unsigned char	*pattern;
	int		rc = 0;
	int		full, fs, fm, ff, fw = 0;

	if (init_eten ("etfonts") <= 0) return 0;

	len = strlen (string);

	x = xx;

	for (i = 0; i < len; i++) {
		word	  = string[i];
		lookahead = string[i + 1];

		y  = yy;
		xw = x;

		if (et->isbig5 (word, lookahead)) {
			pattern = et->ffont24 (word, lookahead);
			i++;
			full = 1;
			fs = 24;
			fw = 24;
			ff = 3;
			fm = 0x800000;
		} else {
			pattern = et->hfont24 (word);
			full = 0;
			fs = 16;
			fw = 12;
			ff = 2;
			fm = 0x8000;
		}

		for (yh = 0; yh < 24; yh++) {
			k = yh * ff;
			x = xw;

			if (full) {
				v =     (pattern[k] << 16) +
					(pattern[k+1] << 8) +
					pattern[k+2];
			} else {
				v =     (pattern[k] << 8) + pattern[k+1];
			}
				ys = y;

			for (j = 0; j < fw; j++) {
				y = ys;

				if ((v & fm) != 0) {
					cidx = fidx;
				} else {
					cidx = bidx;
				}

				xs = x;

				for (yc = 0; yc < scale; yc++, y++) {
					if ((y < 0)||(y >= h)) continue;

					x = xs;

					for (xc = 0; xc < scale; xc++, x++) {
						if ((x < 0)||(x >= w)) continue;
						ptr[x + y * w] = cidx;
					}
				}

				x = xs + scale;
				v <<= 1;
			}
			y = ys + scale;
		}
		rc = x;
		x += space;
	}

	return rc;
}

struct rmimage256_t * new_rmimage256 (const int width, const int height) {
	struct rmimage256_t	*self;
	struct rmimage256_pd_t	*pd;
	unsigned char		*img;
	int			isize;

	while ((self = malloc (sizeof (struct rmimage256_t))) != NULL) {
		pd = &self->pd;
		isize = width * height + 1032;

		if ((img = pd->img = calloc (isize, sizeof (char))) == NULL) {
			free (self);
			self = NULL;
			break;
		}

		img[0] = 0x3e;

		img[1] = (isize & 0xff0000) >> 16;
		img[2] = (isize & 0x00ff00) >>  8;
		img[3] = (isize & 0x0000ff);

		img[4] = (width & 0xff00) >> 8;
		img[5] = (width & 0x00ff);

		img[6] = (height & 0xff00) >> 8;
		img[7] = (height & 0x00ff);

		memcpy (&(self->pd.img[PALETTE_OFFSET]),
					rm_default_palette, 1024);

		pd->width  = width;
		pd->height = height;
		pd->size   = isize;

		self->dispose	= img_dispose;

		self->height	= img_height;
		self->width	= img_width;
		self->size	= img_size;
		self->image	= img_image;

		self->loadPalette	= img_loadPalette;
		self->copyPalette	= img_copyPalette;
		self->setPaletteColor	= img_setPaletteColor;
		self->setPaletteRGB	= img_setPaletteRGB;
		self->setPaletteYUV	= img_setPaletteYUV;
		self->line		= img_line;
		self->lineto		= img_lineto;
		self->rectangle		= img_rectangle;
		self->atob		= img_atob;
		self->paste		= img_paste;
		self->drawText		= img_drawText;
		self->border		= img_border;
		self->packedBitmap	= img_packedBitmap;

		break;
	}

	return self;
}

struct rmimage256_t * new_rmimage256_from_raw (const void *ptr) {
	struct rmimage256_t	*self;
	const unsigned char	*raw = ptr;
	int			width, height;

	if (raw[0] != 0x3e) return NULL;

	width  = (((int)(raw[4])) << 8) | raw[5];
	height = (((int)(raw[6])) << 8) | raw[7];

	if ((self = new_rmimage256 (width, height)) != NULL) {
		memcpy (self->pd.img, ptr, self->pd.size);
	}

	return self;
}
