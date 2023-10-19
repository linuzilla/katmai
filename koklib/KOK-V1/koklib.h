/*
 *	koklib.h
 *
 *	Copyright (c) 2003, Jiann-Ching Liu
 */

#ifndef __KOKLIB_H__
#define __KOKLIB_H__

struct stio_engine_t;

struct koklib_hdr_ds_t {
	short		timeoffset;
	short		lyricspara;
	char		realsizes;
	char		borders;
	char		song[25];
	char		singer[25];
	int		orgb[8];
	int		rgb[8];
}  __attribute__ ((__packed__));

struct koklib_lyricspara_t {
	char		leadtime;		//  0
	char		sentence[25];		//  1 - 25
	char		length;			// 26
	char		division;		// 27
	//char		padding;
	int		time[25];		// 28 - 127
	char		jump[25];		// 128 - 152
	char		alignment;		// 153
	char		singer;			// 154
	short		xoffset;		// 155 - 156
	char		updown;			// 157
}  __attribute__ ((__packed__));

struct koklib_bitmap_t {
	int		size;
	int		width;
	int		height;
	int		real_width;
	char		*bmp;
};

struct koklib_pd_t {
	struct stio_engine_t		*stio;
	struct koklib_hdr_ds_t		header;
	struct koklib_lyricspara_t	*lptr;
	int				lidx;
	int				bidx;
	int				idx[2];
};

struct koklib_t	{
	struct koklib_pd_t	pd;

	void	(*dispose)(struct koklib_t *self);
	int	(*load)(struct koklib_t *self, const char *file);
	void	(*cleanup)(struct koklib_t *self);
	void	(*close)(struct koklib_t *self);
	int	(*read_para)(struct koklib_t *self);
	int	(*read_bitmap)(struct koklib_t *self,
					struct koklib_bitmap_t *bmp);
	void	(*reset)(struct koklib_t *self);
	struct koklib_hdr_ds_t * (*header)(struct koklib_t *self);
	struct koklib_lyricspara_t *
		(*get)(struct koklib_t *self, const int n, int *i);
	struct stio_engine_t * (*stio_engine)(struct koklib_t *self);
};

struct koklib_t *	new_koklib (void);

#endif
