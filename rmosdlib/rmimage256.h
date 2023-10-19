/*
 *	rmimage256.h
 *
 *	Copyright (c) 2002-2003, Jiann-Ching Liu
 */

#ifndef __RMIMAGE256_H__
#define __RMIMAGE256_H__

struct rmimage256_pd_t {
	unsigned char	*img;
	int		width;
	int		height;
	int		size;
};

struct rmimage256_t {
	struct rmimage256_pd_t	pd;

	void		(*dispose)(struct rmimage256_t *);
	int		(*size)  (struct rmimage256_t *);
	int		(*width) (struct rmimage256_t *);
	int		(*height)(struct rmimage256_t *);
	unsigned char *	(*image) (struct rmimage256_t *);
	void		(*loadPalette) (struct rmimage256_t *self,
							void *palette);
	void		(*copyPalette) (struct rmimage256_t *self,
						struct rmimage256_t *other);
	void		(*setPaletteColor) (struct rmimage256_t *self,
					const int entry,
					char *aRGB);
	void		(*setPaletteRGB) (struct rmimage256_t *self,
					const int entry,
					const int rr, const int gg,
					const int bb);
	void		(*setPaletteYUV) (struct rmimage256_t *self,
					const int entry,
					const int yy, const int uu,
					const int vv);
	int		(*drawText)(struct rmimage256_t *self,
					const int xx, const int yy,
					const int fidx, const int bidx,
					const int scale, const int space,
					const char *string);
	void		(*border)(struct rmimage256_t *self,
					const int xx, const int yy,
					const int xl, const int yl,
					const int fc , const int bc,
					const int bw);
	void		(*line)(struct rmimage256_t *self,
					const int xx, const int yy,
					const int xl, const int yl,
					const int cidx);
	void		(*lineto)(struct rmimage256_t *self,
					const int x1, const int y1,
					const int x2, const int y2,
					const int cidx);
	void		(*rectangle)(struct rmimage256_t *self,
					const int xx, const int yy,
					const int xl, const int yl,
					const int cidx, const int fill);
	void		(*atob)(struct rmimage256_t *self,
					const int xx, const int yy,
					const int xl, const int yl,
					const int aidx, const int bidx);
	void		(*paste)(struct rmimage256_t *self,
					const int xx, const int yy,
					struct rmimage256_t *img);
	int		(*packedBitmap)(struct rmimage256_t *self,
					const int xx, const int yy,
					const int fidx, const int bidx,
					const char *bmp, const int width,
					const int height,
					const int realwidth);
};

struct rmimage256_t *	new_rmimage256 (const int width, const int height);
struct rmimage256_t *	new_rmimage256_from_raw (const void *raw);

#endif
