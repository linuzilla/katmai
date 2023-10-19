/*
 *	ttpseng.h	(TrueType PS ENGine)
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __TrueTypePS_Engine_H__
#define __TrueTypePS_Engine_H__

#include <stdio.h>
#include <freetype/freetype.h>

#if FREETYPE_MAJOR == 2
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/ftmodule.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#define TTPSENG_VERSION			"0.15"

typedef struct __TrueTypePS_Engine {
	void	(*close)(void);
	int	(*new_FontFace)(const char *fontfile);
	int	(*add2dict)(const int fid, const int code, const int unicode);
	int	(*push)(const unsigned int key, const short sel);
	int	(*pushf)(const float key, const short sel);
	int	(*showpage)(void);
	void	(*set_filename)(const char *filename);
	float	(*set_fontsize)(const float fs);
	void	(*set_background_eps)(char *file);
	int	(*set_background_on_all)(const int val);
	void	(*set_fontfile)(char *file);
	void	(*set_eudc_fontfile)(char *file);
	int	(*set_paper_type)(const char *pname);
	int	(*outbin)(const int val);
	int	(*wrap)(const int val);
	int	(*tray)(const int val);
	void	(*scan)(FILE *fp);
} TrueTypePS_Engine;

TrueTypePS_Engine *Initial_TrueTypePS_Engine (FILE *fp, const char *lc_ctype);

// int get_EUDC_char_index (const char *str);
// int get_Big5_char_index (const char *str);

#if defined(__cplusplus)
}
#endif
#endif
