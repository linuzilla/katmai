/*
 *	ttpseng.c	(TrueType PS ENGine)
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <locale.h>
#include <pwd.h>
#include "ttpseng.h"

#define MAX_NUMBER_OF_TT_FACE		35
#define PER_BUFFER_LEN			4096
#define MAXLINE				2049
#define MAX_CMD_LEN			5

#ifndef DEFAULT_TRUETYPE_FONTDIR
#define DEFAULT_TRUETYPE_FONTDIR	"/usr/lib/ttf"
#endif

#ifndef DEFAULT_EUDC_FONT_FILE
#define DEFAULT_EUDC_FONT_FILE		"EUDC.TTE"
#endif

#ifndef DEFAULT_BIG5_FONT_FILE
#define DEFAULT_BIG5_FONT_FILE		"moe_kai.ttf"
#endif

#ifndef TT_Flag_On_Curve
#define TT_Flag_On_Curve		1
#endif

#define PCMD_NULL			0
#define PCMD_NEW_COMMAND		1
#define PCMD_FORMFEED			2
#define PCMD_SETRGB			3
#define PCMD_PRINT			4
#define PCMD_DRAW_LINE			5
#define PCMD_DRAW_BOX			6
#define PCMD_FONT_RESET			7
#define PCMD_SETEFONT			8

#define TBCTYPE_ASCII			0
#define TBCTYPE_BIG5			1
#define TBCTYPE_SETRGB			2
#define TBCTYPE_COMMAND			3
#define TBCTYPE_SET_USE_DIRECT_XY	4
#define TBCTYPE_SET_DIRECT_X		5
#define TBCTYPE_SET_DIRECT_Y		6
#define TBCTYPE_SET_DIRECT_FS		7
#define TBCTYPE_SET_DIRECT_FSY		8
#define TBCTYPE_SET_DIRECT_CS		9
#define TBCTYPE_SET_DIRECT_LS		10
#define TBCTYPE_SET_DIRECT_VDIR		11
#define TBCTYPE_SET_DG_SX		12
#define TBCTYPE_SET_DG_SY		13
#define TBCTYPE_SET_DG_EX		14
#define TBCTYPE_SET_DG_EY		15
#define TBCTYPE_SET_DG_WIDTH		16
#define TBCTYPE_SETSEFONT		17
#define TBCTYPE_SETEFSIZE		18
#define TBCTYPE_SETEFONT		19

#define TBCTYPE_SUBCMD_SAVE_STATES	0
#define TBCTYPE_SUBCMD_RESTORE_STATES	1
#define TBCTYPE_SUBCMD_DRAW_LINE	2
#define TBCTYPE_SUBCMD_DRAW_BOX		3


typedef short   fixp;

struct txtbuf_content_t {
	char		type;
	union {
		int	value;
		float	fvalue;
	} v;
};

struct textfile_buffer {
	struct txtbuf_content_t	code[PER_BUFFER_LEN];
	struct textfile_buffer	*next;
};

struct fontface_dict {
	int			fontid;
	int			code;
	int			unicode;
	int			refcnt;
	struct fontface_dict	*next;
};


#if TT_FREETYPE_MAJOR == 1
struct face_info {
	TT_Face			face;
	TT_Face_Properties	properties;
	TT_Glyph		glyph;
	TT_Instance		instance;
	TT_Outline		outline;
	TT_CharMap		charmap;
	short			xmax, xmin, ymax, ymin;
	fixp			*xcoor, *ycoor;
};
#else
struct face_info {
	FT_Face			face;
	//FT_Face_Properties	properties;
	FT_Glyph		glyph;
	//FT_Instance		instance;
	FT_Outline		outline;
	FT_CharMap		charmap;
	short			xmax, xmin, ymax, ymin;
	fixp			*xcoor, *ycoor;
};
#endif

struct LAYOUT {
	const char	*filename;
	short		page_no;
	int		paper_index;
	int		paper_x;
	int		paper_y;
	short		tray;
	short		manual_feed;
	short		outbin;
	short		copy;
	short		landscape;
	short		wrap;
	float		top_margin;
	float		bottom_margin;
	float		left_margin;
	float		right_margin;
	float		font_size;
	float		char_space;
	float		line_space;
	float		doc_name_font_size;
	float		doc_name_char_space;
	char		*docname_top;
	char		*docname_bottom;
	int		line_per_page;
	int		char_per_line;
	int		bgeps_on_all;
	char		*bgeps_file;
};

struct print_cmd {
	char	*cmd;
	int	(*func)(char *str);
};

struct paper {
	char	*paper_type;
	int	xcoor;
	int	ycoor;
};

typedef struct face_info	*fontface_ptr;
typedef struct fontface_dict	*fontface_dict_ptr;

static TrueTypePS_Engine	tteng;

static fontface_dict_ptr	*fontdc_list = NULL;
#if TT_FREETYPE_MAJOR == 1
static TT_Engine		engine;
#else

#  define MAX_CONTROL_POINT	4096
static FT_Library		engine;

static fixp			xcoor_static[MAX_CONTROL_POINT];
static fixp			ycoor_static[MAX_CONTROL_POINT];
#endif
static fontface_ptr		ffinfo[MAX_NUMBER_OF_TT_FACE];
static int			number_of_font_entry = 0;
static const int		res = 265;
static struct fontface_dict	dict;
static short			initialized = 0;
static struct textfile_buffer	*txtbuf = NULL;
static struct textfile_buffer	*txtbuf_current = NULL;
static int			txtbuf_idx = 0;
static fixp			*xcoor, *ycoor;
#if TT_FREETYPE_MAJOR == 1
static TT_CharMap		charmap_unicode, charmap_big5, charmap_Roman;
static TT_Outline		outline;
#else
// static FT_CharMap		charmap_unicode, charmap_big5, charmap_Roman;
static FT_Outline		outline;
#endif
static short			ymin, xmin;
static int			em;
static int			line_per_page = 0;
static int			char_per_line = 0;
static short			big5font_id = -1;
static short			eudcfont_id = -1;
static short			inline_cmd   = 0;
static short			linehead_cmd = 1;

static char			time_str[50];
static char			cmdlead[MAX_CMD_LEN+1] = "~";
static int			cmdleadlen;
static int			current_rgb = 0;
static int			saved_rgb   = 0;

static float			direct_x	= 0.0;
static float			direct_y	= 0.0;
static float			direct_fs	= 0.0;
static float			direct_fsy	= 0.0;
static float			direct_cs	= 0.0;
static float			direct_ls	= 0.0;
//static float			direct_rotate	= 0.0;
static int			direct_vdir	= 0;
static char			*direct_str	= NULL;
static float			dg_sx		= 0.0;
static float			dg_sy		= 0.0;
static float			dg_ex		= 0.0;
static float			dg_ey		= 0.0;
static float			dg_width	= 0.0;
static int			setefont_id	= 0;
static float			setefont_sz	= 0.0;
static char			*psfonts[] = {
					"Courier",   "Courier-Bold",
					"Courier-Oblique",
					"Courier-BoldOblique",
					"Times",     "Times-Bold",
					"Times-Roman",
					"Times-Italic",
					"Times-BoldItalic",
					"Helvetica", "Helvetica-Oblique",
					"Helvetica-Bold",
					"Helvetica-BoldOblique",
					"AvantGarde",
					"NewCenturySchlbk",
					"Palatino",
					"Symbol",
					"ZapfDingbats",
					NULL
				};

static struct LAYOUT	layout = {
				"-stdin-",
				1,			/* page number */
				4,			/* A4 */
				595 , 842,		/* A4 Size */
				-1  ,			/* Tray */
				0   ,			/* Manual feed */
				0   ,			/* Outbin */
				1   ,			/* Number of copy */
				0   ,			/* LandScape */
				0   ,			/* Wrap */
				40  ,  35, 40, 40,	/* 4 Margins */
				10.5,   1,  1,
				0   ,   2,	/* dnfs, dncs */
				"",		// doc name top
				"",		// doc name bottom
				0   ,   0,	/* –︽计, –︽计,
						   0 ボ祘Α衡 */
				1,		/* – */
				NULL		/* 疊 EPS file */
			};

static FILE		*psfp = NULL;

static struct paper paper_list[] = {
	{ "A0"		, 2380, 3368 },
	{ "A1"		, 1684, 2380 },
	{ "A2"		, 1190, 1684 },
	{ "A3"		,  842, 1191 },
	{ "A4"		,  595,  842 },
	{ "A5"		,  421,  595 },
	{ "A6"		,  297,  421 },
	{ "B0"		, 2836, 4008 },
	{ "B1"		, 2004, 2836 },
	{ "B2"		, 1418, 2004 },
	{ "B3"		, 1002, 1418 },
	{ "B4"		,  729, 1032 },
	{ "B5"		,  516,  729 },
	{ "Half"	,  396,  612 },
	{ "Legal"	,  612, 1008 },
	{ "Letter"	,  612,  792 },
	{ "USER"	,    0,    0 },
	{ NULL		,    0,    0 }
};

static int		tte_showpage (void);
static int		cmd_remark	(char *str) { return 1; }
static int		cmd_px		(char *str);
static int		cmd_py		(char *str);
static int		cmd_fn		(char *str);
static int		cmd_eudc	(char *str);
static int		cmd_usefn	(char *str);
static int		cmd_mphdr	(char *str);
static int		cmd_fs		(char *str);
static int		cmd_inlinecmd	(char *str);
static int		cmd_linehead_cmd(char *str);
static int		cmd_landscape	(char *str);
static int		cmd_paper	(char *str);
static int		cmd_tray	(char *str);
static int		cmd_outbin	(char *str);
static int		cmd_wrap	(char *str);
static int		cmd_change_cmd	(char *str);
static int		cmd_form_feed	(char *str);
static int		cmd_setrgb	(char *str);
static int		cmd_rgb_save	(char *str);
static int		cmd_rgb_restore	(char *str);
static int		cmd_print	(char *str);
static int		cmd_vprint	(char *str);
static int		cmd_box		(char *str);
static int		cmd_line	(char *str);
static int		cmd_page_no	(char *str);
static int		cmd_setfont	(char *str);
static int		cmd_list_all_fonts	(char *str);

static int		get_EUDC_char_index (const char *str);
static int		get_Big5_char_index (const char *str);

static struct print_cmd	cmdlist[] = {
				{ "rem"		, cmd_remark		},
				{ "px"		, cmd_px 		},
				{ "py"		, cmd_py 		},
				{ "fs"		, cmd_fs		},
				{ "fn"		, cmd_fn		},
				{ "eudc"	, cmd_eudc		},
				{ "usefn"	, cmd_usefn		},
				{ "mphdr"	, cmd_mphdr		},
				{ "paper"	, cmd_paper		},
				{ "tray"	, cmd_tray		},
				{ "outbin"	, cmd_outbin		},
				{ "wrap"	, cmd_wrap		},
				{ "inlinecmd"	, cmd_inlinecmd		},
				{ "lineheadcmd"	, cmd_linehead_cmd	},
				{ "landscape"	, cmd_landscape		},
				{ "chgcmd"	, cmd_change_cmd	},
				{ "formfeed"	, cmd_form_feed		},
				{ "rgb-save"	, cmd_rgb_save		},
				{ "rgb-restore"	, cmd_rgb_restore	},
				{ "setrgb"	, cmd_setrgb		},
				{ "print"	, cmd_print		},
				{ "vprint"	, cmd_vprint		},
				{ "box"		, cmd_box		},
				{ "line"	, cmd_line		},
				{ "pageno"	, cmd_page_no		},
				{ "setfont"	, cmd_setfont		},
				{ "list-all-fonts"	, cmd_list_all_fonts},
				{ NULL		, NULL   		}
			};

// -----------------------------------------------------------------------

static int txtbuf_allocate (void) {
	int	idx = 0;

	if (txtbuf_current == NULL) {
		txtbuf = malloc (sizeof (struct textfile_buffer));
		txtbuf_current = txtbuf;
		txtbuf_current->next = NULL;
		txtbuf_idx = 0;
	} else if (txtbuf_idx >= PER_BUFFER_LEN) {
		struct textfile_buffer	*ptr;

		ptr = malloc (sizeof (struct textfile_buffer));
		txtbuf_current->next = ptr;
		ptr->next = NULL;
		txtbuf_current = ptr;
		txtbuf_idx = 0;
	}

	idx = txtbuf_idx;
	txtbuf_idx++;

	return idx;
}

static int tte_push (const unsigned int key, const short sel) {
	int	idx;

	idx = txtbuf_allocate ();
	txtbuf_current->code[idx].type = sel;

	if (sel == TBCTYPE_ASCII) {
		txtbuf_current->code[idx].v.value = (key & 0xff);
	} else {
		txtbuf_current->code[idx].v.value = key;
	}

	return 1;
}

static int tte_push_float (const float key, const short sel) {
	int	idx;

	txtbuf_current->code[(idx = txtbuf_allocate ())].type = sel;
	txtbuf_current->code[idx].v.fvalue = key;

	return 1;
}

/*
static struct fontface_dict *getfromdict (const int n) {
	struct fontface_dict	*ptr, *prev;
	int			i = 0;

	for (prev = &dict; (ptr = prev->next) != NULL; prev = ptr) {
		if (++i == n) return ptr;
	}

	return NULL;
}
*/

static void chomp (char *buffer) {
	int	i, len = strlen (buffer);

	for (i = len - 1; i >= 0; i--) {
		if ((buffer[i] == '\r') || (buffer[i] == '\n')) {
			buffer[i] = '\0';
		} else {
			break;
		}
	}
}


static int font_file_autoload (int i) {
	char	msg[4096];

	switch (i) {
	case 1:
		if (big5font_id == -1) {
			big5font_id = tteng.new_FontFace (
					DEFAULT_TRUETYPE_FONTDIR
					"/" DEFAULT_BIG5_FONT_FILE);

			if (big5font_id != -1) {
				fprintf (stderr,
					"Using BIG5 font from: %s/%s\n",
						DEFAULT_TRUETYPE_FONTDIR,
						DEFAULT_BIG5_FONT_FILE);
			}
		}

		return (big5font_id == -1) ? 0 : 1;
	case 0:
		if (eudcfont_id == -1) {
			eudcfont_id = tteng.new_FontFace (
					DEFAULT_EUDC_FONT_FILE);

			sprintf (msg, "%s", DEFAULT_EUDC_FONT_FILE);

			if (eudcfont_id == -1) {
				eudcfont_id = tteng.new_FontFace (
					DEFAULT_TRUETYPE_FONTDIR
					"/" DEFAULT_EUDC_FONT_FILE);

				sprintf (msg, "%s/%s",
					DEFAULT_TRUETYPE_FONTDIR,
					DEFAULT_EUDC_FONT_FILE);
			}

			if (eudcfont_id == -1) {
				fprintf (stderr, "!! No EUDC font file !!\n");
			} else {
				fprintf (stderr,
					"Using EUDC font from: %s\n", msg);
			}
		}

		return (eudcfont_id == -1) ? 0 : 1;
	}

	return 1;
}

static int read_and_do_cmd (char *buffer) {
	int	i, j, blen, found, len = 0;
	char	*ptr = NULL;

	// fprintf (stderr, "command: [%s]\n", buffer);

	for (i = found = 0; cmdlist[i].cmd != NULL; i++) {
		len = strlen (cmdlist[i].cmd);

		if (strncasecmp (cmdlist[i].cmd, buffer, len) == 0) {
			if ((buffer[len] == ' ') || (buffer[len] == '=')) {
				blen = strlen (buffer);

				for (j = len + 1; j < blen; j++) {
					if (buffer[j] == ';') {
						buffer[j] = '\0';
						break;
					}
				}

				found = 1;
				ptr = &buffer[len+1];
				break;
			} else if ((buffer[len] == '\0') ||
						(buffer[len] == ';')) {
				found = 1;
				buffer[len] = '\0';
				ptr = &buffer[len];
				break;
			}
		}
	}

	if (found) {
		return cmdlist[i].func (ptr);
	} else {
		fprintf (stderr, "Unknow command: [%s]\n", buffer);
	}
	return -1;
}

static int push_word (char *buffer) {
	int	didx, c;
	int	advance = 0;
	char	big5str[3];
	wchar_t	unicode;

	if ((c = get_Big5_char_index (buffer)) >= 0) {
		// fprintf (stderr,
		//	"Big5(%c%c)", hi, lo);
		if (big5font_id == -1) font_file_autoload (1);

		big5str[0] = buffer[0];
		big5str[1] = buffer[1];
		big5str[2] = '\0';
		mbstowcs (&unicode, big5str, 1);

		didx = tteng.add2dict (big5font_id, c, unicode);

		tteng.push (didx, TBCTYPE_BIG5);

		advance = 1;
	} else if ((c = get_EUDC_char_index (buffer)) >= 0){
		// fprintf (stderr,
		//	"EUDC(%c%c)=0x%x", hi, lo, c);
		if (eudcfont_id == -1) font_file_autoload (0);

		didx = tteng.add2dict (eudcfont_id, c, c);
		tteng.push (didx, TBCTYPE_BIG5);

		advance = 1;
	} else {
		// fprintf (stderr, "%c", hi);
		tteng.push (buffer[0], TBCTYPE_ASCII);
		advance = 0;
	}

	return advance;
}

static void tte_scan_file (FILE *fp) {
	char			buffer[MAXLINE + 1];
	int			i, j, found, len = 0;
//	int			c, didx;
	unsigned char		hi, lo;
//	wchar_t			unicode;
//	char			big5str[3];
	int			to_be_continue;
	char			*ptr = NULL;
	int			in_pcmd_print = 0;
	//struct print_cmd	*ptr;

//	big5str[2] = '\0';
	cmdleadlen = strlen (cmdlead);

	while (fgets (buffer, MAXLINE, fp)) {
		ptr = buffer;
		in_pcmd_print = 0;

		if (linehead_cmd) {
			if (strncmp (buffer, cmdlead, cmdleadlen) == 0) {
				chomp (buffer);

				// fprintf (stderr, "[%s]\n", buffer);

				to_be_continue = 1;

				switch (read_and_do_cmd (&buffer[cmdleadlen])) {
				case PCMD_SETRGB:
					tteng.push (current_rgb,
							TBCTYPE_SETRGB);
					break;
				case PCMD_FORMFEED:
					buffer[0] = '\f';
					buffer[1] = '\0';
					to_be_continue = 0;
					break;
				case PCMD_PRINT:
					tteng.pushf (direct_x,
							TBCTYPE_SET_DIRECT_X);
					tteng.pushf (direct_y,
							TBCTYPE_SET_DIRECT_Y);
					tteng.pushf (direct_fs,
							TBCTYPE_SET_DIRECT_FS);
					tteng.pushf (direct_fsy,
							TBCTYPE_SET_DIRECT_FSY);
					tteng.pushf (direct_cs,
							TBCTYPE_SET_DIRECT_CS);
					tteng.pushf (direct_ls,
							TBCTYPE_SET_DIRECT_LS);
					tteng.push (direct_vdir,
						TBCTYPE_SET_DIRECT_VDIR);
					tteng.push (1,
						TBCTYPE_SET_USE_DIRECT_XY);

					ptr = direct_str;

					in_pcmd_print = 1;
					to_be_continue = 0;
					//fprintf (stderr,"Start PCMD_PRINT\n");
					break;
				case PCMD_DRAW_LINE:
					tteng.pushf (dg_sx, TBCTYPE_SET_DG_SX);
					tteng.pushf (dg_sy, TBCTYPE_SET_DG_SY);
					tteng.pushf (dg_ex, TBCTYPE_SET_DG_EX);
					tteng.pushf (dg_ey, TBCTYPE_SET_DG_EY);
					tteng.pushf (dg_width,
							TBCTYPE_SET_DG_WIDTH);
					tteng.push (TBCTYPE_SUBCMD_DRAW_LINE,
						TBCTYPE_COMMAND);
					break;
				case PCMD_DRAW_BOX:
					tteng.pushf (dg_sx, TBCTYPE_SET_DG_SX);
					tteng.pushf (dg_sy, TBCTYPE_SET_DG_SY);
					tteng.pushf (dg_ex, TBCTYPE_SET_DG_EX);
					tteng.pushf (dg_ey, TBCTYPE_SET_DG_EY);
					tteng.pushf (dg_width,
							TBCTYPE_SET_DG_WIDTH);
					tteng.push (TBCTYPE_SUBCMD_DRAW_BOX,
						TBCTYPE_COMMAND);
					break;
				case PCMD_FONT_RESET:
					tteng.push (1, TBCTYPE_SETSEFONT);
					break;
				case PCMD_SETEFONT:
					if (setefont_sz == 0.0) {
						tteng.push (setefont_id,
							TBCTYPE_SETSEFONT);
					} else {
						tteng.pushf (setefont_sz,
							TBCTYPE_SETEFSIZE);
						tteng.push (setefont_id,
							TBCTYPE_SETEFONT);
					}
					break;
				case PCMD_NULL:
				case PCMD_NEW_COMMAND:
				default:
					break;
				}

				if (to_be_continue) continue;
			}
		}

		len = strlen (ptr);

		for (i = 0; i < len; i++) {
			hi = ptr[i];
			lo = ptr[i + 1];

			if (inline_cmd) {
				if (strncmp (&ptr[i], cmdlead,
							cmdleadlen) == 0) {
					found = 0;
					for (j = i; j < len; j++) {
						if (ptr[j] == ';') {
							ptr[j] = '\0';
							found = 1;
							break;
						}
					}

					if (found) {
						i += cmdleadlen;
						read_and_do_cmd (&ptr[i]);
						switch (read_and_do_cmd
								(&ptr[i])) {
						case PCMD_SETRGB:
							tteng.push (current_rgb,
								TBCTYPE_SETRGB);
							break;
						}

						i = j;
						continue;
					}
				}
			}

			i += push_word (&ptr[i]);
#if 0
			if ((c = get_Big5_char_index (&buffer[i])) >= 0) {
				// fprintf (stderr,
				//	"Big5(%c%c)", hi, lo);
				if (big5font_id == -1) font_file_autoload (1);

				big5str[0] = buffer[i];
				big5str[1] = buffer[i+1];
				mbstowcs (&unicode, big5str, 1);

				didx = tteng.add2dict (big5font_id, c, unicode);
				i++;

				tteng.push (didx, TBCTYPE_BIG5);
			} else if ((c = get_EUDC_char_index (&buffer[i])) >= 0){
				// fprintf (stderr,
				//	"EUDC(%c%c)=0x%x", hi, lo, c);
				if (eudcfont_id == -1) font_file_autoload (0);

				didx = tteng.add2dict (eudcfont_id, c, c);
				i++;
				tteng.push (didx, TBCTYPE_BIG5);
			} else {
				// fprintf (stderr, "%c", hi);
				tteng.push (hi, TBCTYPE_ASCII);
			}
#endif
		}

		if (in_pcmd_print) {
			// fprintf (stderr, "End of in_pcmd_print\n");
			in_pcmd_print = 0;
			tteng.push (0, TBCTYPE_SET_USE_DIRECT_XY);
		}
	}
}

static void tte_set_fontfile (char *fontfile) {
	big5font_id = tteng.new_FontFace (fontfile);
}

static void tte_set_eudc_fontfile (char *fontfile) {
	eudcfont_id = tteng.new_FontFace (fontfile);
}

static int tte_set_paper_type (const char *pname) {
	int	i;

	for (i = 0;paper_list[i].paper_type != NULL; i++) {
		if (strcasecmp (pname, paper_list[i].paper_type) == 0) {
			layout.paper_x = paper_list[i].xcoor;
			layout.paper_y = paper_list[i].ycoor;
			layout.paper_index = i;

			return 1;
		}
	}

	return 0;
}

static int tte_set_outbin (const int val) {
	int	old_outbin = layout.outbin;

	if ((val == 1) || (val == 2)) {
		layout.outbin = val;
	}
	return old_outbin;
}

static int tte_set_wrap (const int val) {
	int	old_wrap = layout.wrap;

	layout.wrap = val;
	return old_wrap;
}

static int tte_set_tray (const int val) {
	int	old_tray = layout.tray;

	layout.tray = val;

	return old_tray;
}

static void tte_set_filename (const char *filename) {
	layout.filename = filename;
}

static float tte_set_fontsize (const float fs) {
	float	oldfs = layout.font_size;

	if (fs > 0) layout.font_size = fs;

	return oldfs;
}

static void tte_set_background_eps (char *file) {
	layout.bgeps_file = file;
}

static int tte_set_background_on_all (const int val) {
	int	retval = layout.bgeps_on_all;

	layout.bgeps_on_all = val;
	return retval;
}

static int tte_add2dict (const int fid, const int code, const int unicode) {
	struct fontface_dict	*ptr, *prev;
	int			found = 0;
	int			retval = 0;

	prev = &dict;

	while ((ptr = prev->next) != NULL) {
		if ((ptr->fontid == fid) && (ptr->code == code)) {
			found = 1;
			ptr->refcnt++;
			break;
		}
		retval++;
		prev = ptr;
	}

	if (! found) {
		ptr = prev->next = malloc (sizeof (struct fontface_dict));
		ptr->next    = NULL;
		ptr->fontid  = fid;
		ptr->code    = code;
		ptr->unicode = unicode;
		ptr->refcnt  = 1;
	}

	// fprintf (stderr, "Return: %d\n", retval);
	return retval;
}

static int tte_new_FontFace (const char *fontfile) {
	int		fid, em;
	fontface_ptr	ptr;

	if ((fid = number_of_font_entry) >= MAX_NUMBER_OF_TT_FACE) return -1;

	if ((ptr = malloc (sizeof (struct face_info))) == NULL) {
		return -1;
	}

#if TT_FREETYPE_MAJOR == 1
	if (TT_Open_Face (engine, fontfile, &ptr->face)) {
		// fprintf (stderr, "%s: Could not open as font file\n",
		//		fontfile);
		free (ptr);
		return -1;
	}
#else
	if (FT_New_Face (engine, fontfile, 0, &ptr->face)) {
		// fprintf (stderr, "%s: Could not open as font file\n",
		//		fontfile);
		free (ptr);
		return -1;
	}
#endif

#if TT_FREETYPE_MAJOR == 1
	TT_Get_Face_Properties (ptr->face, &ptr->properties);

	em = ptr->properties.header->Units_Per_EM;

	ptr->xmin = ptr->properties.header->xMin;
        ptr->ymin = ptr->properties.header->yMin;
	ptr->xmax = ptr->properties.header->xMax - ptr->xmin;  // to original
	ptr->ymax = ptr->properties.header->yMax - ptr->ymin;

	ptr->xcoor = calloc (ptr->properties.max_Points, sizeof (fixp));
	ptr->ycoor = calloc (ptr->properties.max_Points, sizeof (fixp));
#else
	em = ptr->face->units_per_EM;

	ptr->xmin  = ptr->face->bbox.xMin;
	ptr->ymin  = ptr->face->bbox.yMin;
	ptr->xmax  = ptr->face->bbox.xMax - ptr->xmin;
	ptr->ymax  = ptr->face->bbox.yMax - ptr->ymin;

	ptr->xcoor = xcoor_static;
	ptr->ycoor = ycoor_static;
#endif
	ptr->xmax = (short) (((long) ptr->xmax) * res / em);
	ptr->ymax = (short) (((long) ptr->ymax) * res / em);

#if TT_FREETYPE_MAJOR == 1
	if (TT_New_Glyph (ptr->face, &ptr->glyph)) {
		fprintf (stderr, "Could not create glyph container.\n" );
		return -1;
	}
#else
#endif

#if TT_FREETYPE_MAJOR == 1
	if (TT_New_Instance (ptr->face, &ptr->instance)) {
		fprintf (stderr, "Could not create instance for %s.\n",
					fontfile);
		return -1;
	}
#else
#endif

	ffinfo[fid] = ptr;
	number_of_font_entry++;

	return fid;
}

static void tte_close (void) {
#if TT_FREETYPE_MAJOR == 1
	int	i;

	for (i = 0; i <  number_of_font_entry; i++) {
		TT_Done_Instance (ffinfo[i]->instance);
		TT_Close_Face (ffinfo[i]->face);
	}

	TT_Done_FreeType (engine);
#else
	FT_Done_Library (engine);
#endif
}


// ------------------------------------------------------

TrueTypePS_Engine *Initial_TrueTypePS_Engine (FILE *fp, const char *lc_ctype) {
	int		major, minor, patchlevel;
	time_t		curtime;       /* The time file is proceed */
	struct tm	*loctime;

	if (! initialized) {
#if TT_FREETYPE_MAJOR == 1
		patchlevel = 0;

		if (TT_FreeType_Version (&major, &minor)) return NULL;


		if (TT_Init_FreeType (&engine)) {
			fprintf (stderr, "Failure to initialize font engine\n");
			return NULL;
		}
#else
		if (FT_Init_FreeType (&engine)) {
			fprintf (stderr,
				"Failure to initialize font library\n");
			return NULL;
		}

#  if (FREETYPE_MAJOR == 2) && (FREETYPE_MINOR == 0)
		major = FREETYPE_MAJOR;
		minor = FREETYPE_MINOR;
		patchlevel = 0;
#  else
		FT_Library_Version (engine, &major, &minor, &patchlevel);
#  endif
#endif

		fprintf (stderr, "FreeType Version %d.%d.%d\n",
				major, minor, patchlevel);

		dict.next			= NULL;

		tteng.close			= tte_close;
		tteng.new_FontFace		= tte_new_FontFace;
		tteng.add2dict			= tte_add2dict;
		tteng.push			= tte_push;
		tteng.pushf			= tte_push_float;
		tteng.showpage			= tte_showpage;
		tteng.set_filename		= tte_set_filename;
		tteng.set_fontsize		= tte_set_fontsize;
		tteng.set_background_eps	= tte_set_background_eps;
		tteng.set_background_on_all	= tte_set_background_on_all;
		tteng.outbin			= tte_set_outbin;
		tteng.tray			= tte_set_tray;
		tteng.wrap			= tte_set_wrap;
		tteng.scan			= tte_scan_file;
		tteng.set_fontfile		= tte_set_fontfile;
		tteng.set_eudc_fontfile		= tte_set_eudc_fontfile;
		tteng.set_paper_type		= tte_set_paper_type;


		// tteng.set_paper_type ("Letter");

		psfp				= fp;

		curtime = time (NULL);
		loctime = localtime (&curtime);
		strftime (time_str, 30, "%Y/%m/%d %a %H:%M", loctime);

		if (lc_ctype == NULL) {
			setlocale (LC_CTYPE, "zh_TW.Big5");
		} else {
			setlocale (LC_CTYPE, lc_ctype);
		}

		initialized = 1;
	}

	return &tteng;
}

// ***************************************************************


static void PSmoveto (const int k) {
	fprintf (psfp, "%d %d M\n", (int) xcoor[k], (int) ycoor[k]);
}

static void PSlineto (const int k) {
	fprintf (psfp, "%d %d L\n", (int) xcoor[k], (int) ycoor[k]);
}

static void PScurveto (const int k, const int s, const int t) {
	int	i, n;
	double	sx[3], sy[3], cx[4], cy[4];

	n = t - s + 2;

	for (i = 0; i < n - 1; i++) {
		if (i == 0) {
			sx[0] = xcoor[s - 1];
			sy[0] = ycoor[s - 1];
		} else {
			sx[0] = (xcoor[i + s] + xcoor[i + s - 1]) / 2.;
			sy[0] = (ycoor[i + s] + ycoor[i + s - 1]) / 2.;
		}

		sx[1] = xcoor[s + i];
		sy[1] = ycoor[s + i];

		if (i == (n - 2)) {
			sx[2] = xcoor[k];
			sy[2] = ycoor[k];
		} else {
			sx[2] = (xcoor[s + i] + xcoor[s + i + 1]) / 2.;
			sy[2] = (ycoor[s + i] + ycoor[s + i + 1]) / 2.;
		}

		cx[3] = sx[2];
		cy[3] = sy[2];

		cx[1] = (2.0 * sx[1] + sx[0]) / 3.0;
		cy[1] = (2.0 * sy[1] + sy[0]) / 3.0;

		cx[2] = (sx[2] + 2.0 * sx[1]) / 3.0;
		cy[2] = (sy[2] + 2.0 * sy[1]) / 3.0;

		fprintf (psfp,
			"%.0f %.0f %.0f %.0f %.0f %.0f C\n",
			cx[1], cy[1], cx[2], cy[2], cx[3], cy[3]);
	}
}

static void ttf2ps (const int fontid, const int code, const int unicode) {
	int			i, j, k, m, n, fst, cidx = 0;
	int			start_offpt, end_offpt = 0;
	struct face_info	*ptr;
	unsigned short		platform, encoding;
	short			have_unicode_encoding = 0;
	short			have_big5_encoding = 0;
	char			*ol_flags = NULL;
#if TT_FREETYPE_MAJOR == 1
	int			flags = 0;
	TT_Error		err;
#else
	FT_Error		err;
#endif

	// fprintf (stderr, "Fontid = %d, code = %d, cidx = %d\n",
	//		fontid, code, cidx);

	ptr = ffinfo[fontid];

	xcoor = ptr->xcoor;
	ycoor = ptr->ycoor;
#if TT_FREETYPE_MAJOR == 1
	em    = ptr->properties.header->Units_Per_EM;
#else
	em    = ptr->face->units_per_EM;
#endif
	xmin  = ptr->xmin;
	ymin  = ptr->ymin;

#if TT_FREETYPE_MAJOR == 1
	n = ptr->properties.num_CharMaps;
#else
	n = ptr->face->num_charmaps;
#endif

	for (i = 0; i < n; i++) {
#if TT_FREETYPE_MAJOR == 1
		TT_Get_CharMap_ID (ptr->face, i, &platform, &encoding);
#else
		platform = ptr->face->charmaps[i]->platform_id;
		encoding = ptr->face->charmaps[i]->encoding_id;
		// fprintf (stderr, "%2d. Platform=%d, Encoding=%d\n",
		//		i, platform, encoding);
#endif

		if (platform == 3) { // Windows
			switch (encoding) {
			case 0:	// Windows Symbol
				//fprintf (stderr, "Windows Symbol\n");
				break;
			case 1: // Windows Unicode
				have_unicode_encoding = 1;
#if TT_FREETYPE_MAJOR == 1
				TT_Get_CharMap (ptr->face, i, &charmap_unicode);
#endif
				//fprintf (stderr, "Windows Unicode\n");
				break;
			case 2: // Windows Sjis
				//fprintf (stderr, "Windows Sjis\n");
				break;
			case 4: // Windows BIG5
				have_big5_encoding = 1;
#if TT_FREETYPE_MAJOR == 1
				TT_Get_CharMap (ptr->face, i, &charmap_big5);
#endif
				//fprintf (stderr, "Windows BIG5\n");
				break;
			default:
				//fprintf (stderr, "Unknow\n");
				break;
			}
		} else if (platform == 2) { // ISO ASCII
#if TT_FREETYPE_MAJOR == 1
			TT_Get_CharMap (ptr->face, i, &charmap_Roman);
#endif
			//fprintf (stderr, "ISO ASCII\n");
		} else if (platform == 1) { // Apple Roman
#if TT_FREETYPE_MAJOR == 1
			TT_Get_CharMap (ptr->face, i, &charmap_Roman);
#endif
			//fprintf (stderr, "Apple Roman\n");
		} else if (platform == 0) { // Apple Unicode
			//fprintf (stderr, "Apple Unicode\n");
#if TT_FREETYPE_MAJOR == 1
			TT_Get_CharMap (ptr->face, i, &charmap_unicode);
#endif
			have_unicode_encoding = 1;
		} else {
			//fprintf (stderr, "Unknow\n");
		}
	}

#if FREETYPE_MAJOR == 2
	// have_big5_encoding = 0;
	// have_unicode_encoding = 1;
#endif
	if (have_big5_encoding) {
#if TT_FREETYPE_MAJOR == 1
		cidx = TT_Char_Index (charmap_big5, code);
#else
		FT_Select_Charmap (ptr->face, ft_encoding_big5);
		cidx = FT_Get_Char_Index (ptr->face, code);
#endif
	} else if (have_unicode_encoding) {
#if TT_FREETYPE_MAJOR == 1
		cidx = TT_Char_Index (charmap_unicode, unicode);
#else
		FT_Select_Charmap (ptr->face, ft_encoding_unicode);
		cidx = FT_Get_Char_Index (ptr->face, unicode);
#endif
	} else {
		return;
	}


#if TT_FREETYPE_MAJOR == 1
	err = TT_Load_Glyph (ptr->instance, ptr->glyph, cidx, flags);

	if (err) {
		fprintf (stderr, "fail on TT_Load_Glyph\n");
	} else {
		err = TT_Get_Glyph_Outline (ptr->glyph, &outline);

		if (err) {
			fprintf (stderr, "fail on TT_Get_Glyph_Outline\n");
		} else {
			ol_flags = outline.flags;
		}
	}

#else
	// fprintf (stderr, "cidx = %d\n", cidx);

	// err = FT_Load_Glyph (ptr->face, cidx, FT_LOAD_DEFAULT);
	err = FT_Load_Glyph (ptr->face, cidx, FT_LOAD_NO_SCALE);
	//		FT_LOAD_NO_SCALE|FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
	//			FT_LOAD_NO_SCALE | FT_LOAD_NO_RECURSE);

	if (err) {
		fprintf (stderr, "fail on FT_Load_Glyph\n");
	} else {
		outline = ptr->face->glyph->outline;
		ol_flags = outline.tags;

		switch (ptr->face->glyph->format) {
		case ft_glyph_format_outline:
			// fprintf (stderr, "Outline\n");
			break;
		case ft_glyph_format_composite:
			fprintf (stderr, "Composite\n");
			break;
		case ft_glyph_format_bitmap:
			fprintf (stderr, "Bitmap\n");
		case ft_glyph_format_none:
			break;
		case ft_glyph_format_plotter:
			fprintf (stderr, "Plotter\n");
			break;
		}
	}
#endif


// #define DEBUG
	if (! err) {
// #ifdef DEBUG
#if FREETYPE_MAJOR == 2
		// fprintf (stderr, "pts=%d,contours=%d\n",
		//			outline.n_points,
		//			outline.n_contours);
#endif

		for (j = 0; j < outline.n_points; j++) {
			xcoor[j] = outline.points[j].x;
			ycoor[j] = outline.points[j].y;
		}

#ifdef DEBUG
		k = outline.contours[0];
		for (j = 1; j < outline.n_contours; j++) {
			if ((k + 1) == (m = outline.contours[j+1])) {
				fprintf (stderr,
					"Seems like a composite font\n");
				break;
			}
			k = m;
		}
#endif

		for (j = k = 0; j < outline.n_points; j++) {
			m = outline.contours[k];
#ifdef DEBUG
			if (j == m) k++;

			fprintf (stderr, "%s", j == m ? "*" : " ");
			fprintf (stderr, "%3d. flags=0x%x (%4d, %4d)",
					j, ol_flags[j],
					xcoor[j], ycoor[j]);
#endif
			xcoor[j] = (fixp) (((long)(xcoor[j] - xmin))
					* res / (em / 4)) - 2;
			ycoor[j] = (fixp) (((long)(ycoor[j] - ymin))
					* res / (em / 4)) - 2;
			//fprintf (stderr,"(%d,%d)",xcoor[j],ycoor[j]);

#ifdef DEBUG
			fprintf (stderr, "-> (%4d, %4d)\n", xcoor[j], ycoor[j]);
#endif
		}
		//fprintf (stderr, "\n");

		fst = 0;

		for (j = k = 0, m = -1; j < outline.n_contours; j++) {
			// if (j > 5) continue;
			/*
			if (outline.contours[j] == m + 1) {
			}
			*/

			m = outline.contours[j];

			PSmoveto (fst = k);

			start_offpt = 0;


			/*
			if (j < outline.n_contours - 1) {
				if (m + 1 == outline.contours[j + 1]) {
					j++;
				}
			}
			*/


			for (k++; k <= outline.contours[j]; k++) {
				if (! (ol_flags[k] & TT_Flag_On_Curve)) {
					if (! start_offpt) {
						start_offpt = end_offpt = k;
					} else {
						end_offpt++;
					}
				} else { // On Curve
					if (start_offpt) {
						PScurveto (k, start_offpt,
								end_offpt);
						start_offpt = 0;
					} else {
						PSlineto (k);
					}
				}
			}

			if (start_offpt) {
				PScurveto (fst, start_offpt, end_offpt);
			} else {
				PSlineto (fst);
			}

#if 0
			for (; k <= outline.contours[j]; k++) {
				// fprintf (stderr, "(%d,%d",
				//		(int) outline.points[k].x,
				//		(int) outline.points[k].y);

				if (! (outline.flags[k] & 1)) {
					//	fprintf (stderr, "]");
						// off curve
				} else {
					//	fprintf (stderr, ")");
						// on curve
				}
			}
			// fprintf (stderr, "\n");
#endif
		}
	}
}

static void StartDoc (FILE *fp) {
	int	i;

	fprintf (fp,
		"<< /DeferredMediaSelection true "
		"/PageSize [%d %d] /ImagingBBox [ 0 0 %d %d ] "
		"/Orientation %d",
		layout.paper_x, layout.paper_y, layout.paper_x, layout.paper_y,
		(layout.landscape ? 1 : 0));


	if (layout.tray  >=  0) fprintf (fp, " /MediaPosition %d", layout.tray);
	if (layout.manual_feed) fprintf (fp, " /ManualFeed false");
	if (layout.outbin > 0) {
		fprintf (fp, " /OutputType (%s OUTPUT BIN)",
				layout.outbin == 1 ? "TOP" : "LEFT");
	}
	if (layout.copy > 1) fprintf (fp, " /NumCopies %d", layout.copy);

	fprintf (fp, " >> setpagedevice\n\n");

	fprintf (fp,
		"/BeginEPSF {\n"
		"/b4_Inc_state save def\n"
		"/dict_count countdictstack def\n"
		"/op_count count 1 sub def\n"
		"userdict begin\n"
		"/showpage {} def\n"
		"0 setgray 0 setlinecap\n"
		"1 setlinewidth 0 setlinejoin\n"
		"10 setmiterlimit [] 0 setdash newpath\n"
		"/languagelevel where\n"
		"{pop languagelevel 1 ne\n"
		"{false setstrokeadjust false setoverprint\n"
		"} if\n"
		"} if\n"
		"} bind def\n\n");

	fprintf (fp,
		"/EndEPSF {\n"
		"count op_count sub {pop} repeat\n"
		"countdictstack dict_count sub {end} repeat\n"
		"b4_Inc_state restore\n"
		"} bind def\n\n");

	fprintf (fp, "/I {bind def} bind def\n");

	for (i = 0; psfonts[i] != NULL; i++) {
		fprintf (fp,
			"/F%d {/%s findfont %.2f scalefont setfont} I\n",
			i + 1, psfonts[i], layout.font_size);
	}

	fprintf (fp,
		"F1\n"
		"/M {moveto} I\n"
		"/L {lineto} I\n"
		"/C {curveto} I\n"
		"/D {closepath fill grestore} I\n"
		"/S {ashow} I\n"
		"/B {%.4f %.4f scale newpath} I\n"
		"/N {gsave translate B} I\n"
		"/BOX {\n"
		"  gsave\n"
		"  setlinewidth\n"
		"  exch dup 0 rlineto\n"
		"  exch 0 exch rlineto\n"
		"  neg 0 rlineto\n"
		"  closepath stroke\n"
		"  grestore\n"
		"} I\n"
		"/LINE {\n"
		"  gsave\n"
		"  setlinewidth\n"
		"  rlineto stroke\n"
		"  grestore\n"
		"} I\n"
		"/X {gsave translate} I\n"
		"/Z {scale newpath} I\n",
		layout.font_size / 1000, layout.font_size / 1000);

	fprintf (fp,
		"/F {%.2f %.2f M gsave "
		"/Courier findfont 9 scalefont setfont ( ) show grestore\n"
		"} I\n"
		"/Q {show F %.2f %.2f M /Courier findfont 6 "
		"scalefont setfont (%s) show "
		"grestore showpage} I\n",
		20.0, layout.paper_y - 12.0,
		layout.left_margin - 5, layout.bottom_margin - 8,
		time_str);

	fprintf (fp,
		"/P {%.2f %.2f M gsave /Courier-Bold findfont "
		"%.2f scalefont setfont %.1f 0 (%s) "
		"ashow %.2f %.2f M %.1f 0 (%s) ashow %.2f %.2f M "
		"/Courier-Bold findfont 10 scalefont setfont} I\n"
		"/O {grestore showpage} I\n\n",
		(layout.paper_x - strlen (layout.docname_top) *
 		(layout.doc_name_font_size - layout.doc_name_char_space)) / 2,
		layout.paper_y - layout.top_margin +
		layout.doc_name_font_size / 5,
		layout.doc_name_font_size,
		layout.doc_name_char_space,
		layout.docname_top,
		(layout.paper_x - strlen (layout.docname_bottom) *
		(layout.doc_name_font_size - layout.doc_name_char_space)) / 2,
		layout.bottom_margin - layout.doc_name_font_size - 2,
		layout.doc_name_char_space,
		layout.docname_bottom,
		layout.paper_x - layout.right_margin - 5,
		layout.bottom_margin - 8);
}

static void start_new_page (const int page) {
	int	fd, len;
	char	buffer[512];

	if (layout.bgeps_file != NULL) {
		if ((layout.bgeps_on_all == 1) || (page == 0)) {
			if ((fd = open (layout.bgeps_file, O_RDONLY)) >= 0) {
				fprintf (psfp, "\nBeginEPSF\n"
					       "%%%%BeginDocument: Header\n");

				while ((len = read (fd, buffer, 512)) > 0) {
					fwrite (buffer, 1, len, psfp);
				}

				fprintf (psfp, "\n%%%%EndDocument\nEndEPSF\n");
				close (fd);
			}
		}
	}
}

static void write_next_page (const int page) {
	if (layout.page_no) {
		fprintf (psfp, "P (%d) Q\n%%%%Pages: %d\n", page, page);
	} else {
		fprintf (psfp, "P O\n%%%%Pages: %d\n", page);
	}
}

static void write_dict_char_xy (const double x, const double y,
			const int idx, const double fs, const double fsy) {
	struct fontface_dict	*fontdc;

	if ((fontdc = fontdc_list[idx]) == NULL) {
		if (fs != 0.0) {
			fprintf (psfp, "%.1f %.1f X %.4f %.4f Z g%03x D\n",
					x, y, fs / 1000, fsy / 1000, idx);
		} else {
			fprintf (psfp, "%.1f %.1f N g%03x D\n", x, y, idx);
		}
	} else {
		if (fs != 0.0) {
			fprintf (psfp, "%.1f %.1f X %.4f %.4f Z ",
					x, y, fs / 1000, fsy / 1000);
		} else {
			fprintf (psfp, "%.1f %.1f N ", x, y);
		}
		ttf2ps (fontdc->fontid, fontdc->code, fontdc->unicode);
		fprintf (psfp, "D\n");
	}
}

static void write_dict_char (const int x, const int y, const int idx) {
	double			xx, yy;

	xx = x * (layout.font_size + layout.char_space * 2) / 2
		+ layout.left_margin;

	yy = layout.paper_y -
		y * (layout.font_size + layout.line_space) - layout.top_margin -
		layout.font_size - layout.line_space;

	write_dict_char_xy (xx, yy, idx, 0.0, 0.0);
}

static void write_ascii_string_xy (const double x, const double y,
				const double z, char *str, const int len,
				int fontid, double fs) {
	str[len] = '\0';
	if (len == 0) return;

	if (fs == 0.0) {
		fprintf (psfp, "%.1f %.1f M %.2f 0 (%s) S\n", x, y, z, str);
	} else {
		if (fontid < 1) fontid = 1;

		fprintf (psfp, "%.1f %.1f M gsave %.2f 0 "
				"/%s findfont %.2f "
				"scalefont setfont (%s) S grestore\n",
				x, y, z,
				psfonts[fontid - 1], fs, str);
	}
}

static void write_ascii_string (const int x, const int y,
					char *str, const int len) {
	double	xx, yy, zz;

	xx = x * (layout.font_size + layout.char_space * 2) / 2
		+ layout.left_margin;

	yy = layout.paper_y -
		y * (layout.font_size + layout.line_space) - layout.top_margin +
		layout.font_size / 6 - layout.font_size - layout.line_space;

	zz = layout.char_space - layout.font_size / 10;

	write_ascii_string_xy (xx, yy, zz, str, len, 1, 0.0);
}

static int tte_showpage (void) {
	struct fontface_dict	*fontdc;
	struct textfile_buffer	*ptr;
	int			i, c, tp, upto;
	int			fonts_use, fonts_in_dict;
	int			line_count;
	int			page_count;
	char			char_str[MAXLINE * 2];
	int			char_cnt  = 0;
	int			rx = 0;
	int			slash_cnt = 0;
	int			xx, yy;
	int			rr, gg, bb;
	int			need_start_new_page = 1;
	int			isascii = 0;
	int			use_direct_xy = 0;
	double			dir_x = 0.0, dir_y = 0.0;
	double			dir_x_save = 0.0;
	double			dir_fs  = 0.0;
	double			dir_fsy = 0.0;
	double			dir_cs  = 0.0;
	double			dir_ls  = 0.0;
	double			dirg_sx = 0.0;
	double			dirg_sy = 0.0;
	double			dirg_ex = 0.0;
	double			dirg_ey = 0.0;
	double			dirg_width = 0.0;
	double			fontsz  = 1;
	int			efontid = 1;
	int			dir_vdir = 0;
	char			dir_str[3];
	int			len;
	struct passwd 		*pw;
	char			*name = "Leica M6";
	float			cxfs, cyfs;	// 莱 FONT SIZE ︽计–︽计
						// ┮眔ぇ糴

	if ((pw = getpwuid(getuid ())) != NULL) {
		name = pw->pw_gecos;
	}

	if (layout.doc_name_font_size == 0) {
		layout.doc_name_font_size = layout.font_size * 1.2;
	}

	fprintf (psfp,
			"%%!PS-Adobe-3.0\n"
			"%%%%Title: (%s)\n"
			"%%%%Creator: big5ps version %s, by Jiann-Ching Liu\n"
			"%%%%CreationDate: %s\n"
			"%%%%For: %s\n"
			"%%%%Orientation: %s\n"
			"%%%%TargetDevice: (LaserWriter II NT) (47.0) 1\n"
			"%%%%LanguageLevel: 1\n"
			"%%%%DocumentData: Clean7Bit\n"
			"%%%%Pages: (atend)\n"
			"%%%%PageOrder: Ascend\n"
			"%%%%DocumentMedia: %s %d %d 0 () ()\n"
			"%%%%EndComments\n\n",
			layout.filename,
			TTPSENG_VERSION,
			time_str,
			name,
			layout.landscape ? "Landscape" : "Portrait",
			paper_list[layout.paper_index].paper_type,
			paper_list[layout.paper_index].xcoor,
			paper_list[layout.paper_index].ycoor);

//			"%%%%BoundingBox: 0 0 %d %d\n"
//			layout.paper_x, layout.paper_y);
//		%%BoundingBox: 24 24 588 768
//		%%Pages: 6
//		%%DocumentMedia: Letter 612 792 0 () ()

	if (layout.landscape) {
		int	tmp;

		tmp		= layout.paper_x;
		layout.paper_x	= layout.paper_y;
		layout.paper_y	= tmp;
	}


	if ((layout.line_per_page != 0) && (layout.char_per_line != 0)) {
		// ﹚︽计の–︽计
		cxfs = ((layout.paper_x - layout.left_margin
			- layout.right_margin) / layout.char_per_line) * 2;
		cyfs = (layout.paper_y - layout.top_margin
				- layout.bottom_margin) / layout.line_per_page
			- layout.line_space;

		if (cxfs > cyfs) {
			layout.font_size = cyfs;
			layout.char_space = layout.char_space +
							(cxfs - cyfs) / 2;
		} else {
			layout.font_size = cxfs;
			layout.line_space = layout.line_space + (cyfs - cxfs);
		}

		line_per_page = layout.line_per_page;
		char_per_line = layout.char_per_line;
	} else if ((layout.line_per_page == 0) && (layout.char_per_line != 0)) {
		layout.font_size = ((layout.paper_x - layout.left_margin -
					layout.right_margin) /
					layout.char_per_line -
					layout.char_space) * 2;
		char_per_line = layout.char_per_line;
		line_per_page = (int) ((float) layout.paper_y -
				layout.top_margin - layout.bottom_margin) /
				(layout.font_size + layout.line_space);
	} else if ((layout.line_per_page != 0) && (layout.char_per_line == 0)) {
		// ﹚–︽计
		layout.font_size = (layout.paper_y - layout.top_margin -
					layout.bottom_margin) /
					layout.line_per_page -
					layout.line_space;
		line_per_page = layout.line_per_page;
		char_per_line = (int) (((float) layout.paper_x -
				layout.left_margin - layout.right_margin) * 2) /
				(layout.font_size + layout.char_space * 2);
	} else {
		char_per_line = (int) (((float) layout.paper_x - 
				layout.left_margin - layout.right_margin) * 2) /
				(layout.font_size + layout.char_space * 2);

		line_per_page = (int) ((float) layout.paper_y -
				layout.top_margin - layout.bottom_margin) /
				(layout.font_size + layout.line_space);
	}

	fprintf (stderr, "CPL=%d, LPP=%d\n", char_per_line, line_per_page);

	StartDoc (psfp);

	fonts_in_dict = fonts_use = 0;

	for (fontdc = dict.next; fontdc != NULL; fontdc = fontdc->next) {
		fonts_use++;
	}

	fontdc_list = malloc (fonts_use * sizeof (fontface_dict_ptr));

	for (fontdc = dict.next, i = 0; fontdc != NULL; fontdc = fontdc->next) {
		if (fontdc->refcnt > 1) {
			fprintf (psfp, "/g%03x {\n", i);
			ttf2ps (fontdc->fontid, fontdc->code, fontdc->unicode);
			fprintf (psfp, "} I\n\n");
			fontdc_list[i] = NULL;
			fonts_in_dict++;
		} else {
			fontdc_list[i] = fontdc;
		}

		i++;
	}

	fprintf (stderr, "%d fonts use, %d in dict\n",
					fonts_use, fonts_in_dict);

	fprintf (psfp, "%%EndSetup\n\n");
	// fprintf (psfp, "%%EndSetup\n");


	page_count = line_count = char_cnt = slash_cnt = 0;
	xx = yy = rx = 0;
	need_start_new_page = 1;


	for (ptr = txtbuf; ptr != NULL; ptr = ptr->next) {
		// fprintf (stderr, "Load buffer\n");
		upto = (ptr != txtbuf_current) ? PER_BUFFER_LEN : txtbuf_idx;

		for (i = 0; i < upto; i++) {
			if (need_start_new_page) {
				start_new_page (page_count);
				need_start_new_page = 0;
			}

			c  = ptr->code[i].v.value;
			tp = ptr->code[i].type;

			if (use_direct_xy) {
			} else if (layout.wrap && (c != '\r') && (c != '\n')) {
				if (((tp == TBCTYPE_ASCII) && 
				     (xx + char_cnt - slash_cnt >
						      	char_per_line - 1)) ||
				     ((tp == TBCTYPE_BIG5) && 
				     (xx + char_cnt - slash_cnt >=
						      	char_per_line - 1))) {
					c = '\n';
					i--;
				}
			}

			isascii = 0;

			// fprintf (stderr, "code[i] = %x\n", ptr->code[i]);

			switch (tp) {
			case TBCTYPE_COMMAND:
				switch (c) {
				case TBCTYPE_SUBCMD_SAVE_STATES:
					break;
				case TBCTYPE_SUBCMD_RESTORE_STATES:
					break;
				case TBCTYPE_SUBCMD_DRAW_LINE:
					// fprintf (stderr,
					// 	"line (%.2f,%.2f) "
					//	"-> (%.2f,%.2f) [%.2f]\n",
					//	dirg_sx, dirg_sy,
					//	dirg_ex, dirg_ey,
					//	dirg_width);
					fprintf (psfp,
						"%.2f %.2f M "
						"%.2f %.2f %.2f LINE\n",
						dirg_sx, dirg_sy,
						dirg_ex, dirg_ey,
						dirg_width);
					break;
				case TBCTYPE_SUBCMD_DRAW_BOX:
					// fprintf (stderr, "box (%.2f,%.2f) "
					//	"-> (%.2f,%.2f) [%.2f]\n",
					//	dirg_sx, dirg_sy,
					//	dirg_ex, dirg_ey,
					//	dirg_width);
					fprintf (psfp,
						"%.2f %.2f M "
						"%.2f %.2f %.2f BOX\n",
						dirg_sx, dirg_sy,
						dirg_ex, dirg_ey,
						dirg_width);
					break;
				}
				break;
			case TBCTYPE_SET_USE_DIRECT_XY:
				use_direct_xy = c;
				break;
			case TBCTYPE_SET_DIRECT_X:
				dir_x = dir_x_save = ptr->code[i].v.fvalue;
				break;
			case TBCTYPE_SET_DIRECT_Y:
				dir_y = ptr->code[i].v.fvalue;
				break;
			case TBCTYPE_SET_DIRECT_FS:
				dir_fsy = dir_fs = ptr->code[i].v.fvalue;
				fontsz = dir_fs;
				break;
			case TBCTYPE_SET_DIRECT_FSY:
				dir_fsy = ptr->code[i].v.fvalue;
				break;
			case TBCTYPE_SET_DIRECT_CS:
				dir_cs = ptr->code[i].v.fvalue;
				break;
			case TBCTYPE_SET_DIRECT_LS:
				dir_ls = ptr->code[i].v.fvalue;
				break;
			case TBCTYPE_SET_DIRECT_VDIR:
				dir_vdir = ptr->code[i].v.value;
				break;
			case TBCTYPE_SET_DG_SX:
				dirg_sx = ptr->code[i].v.fvalue;
				break;
			case TBCTYPE_SET_DG_SY:
				dirg_sy = ptr->code[i].v.fvalue;
				break;
			case TBCTYPE_SET_DG_EX:
				dirg_ex = ptr->code[i].v.fvalue;
				break;
			case TBCTYPE_SET_DG_EY:
				dirg_ey = ptr->code[i].v.fvalue;
				break;
			case TBCTYPE_SET_DG_WIDTH:
				dirg_width = ptr->code[i].v.fvalue;
				break;
			case TBCTYPE_SETSEFONT:
				efontid = ptr->code[i].v.value;
				fprintf (psfp, "F%d\n", efontid);
				break;
			case TBCTYPE_SETEFSIZE:
				fontsz = ptr->code[i].v.fvalue;
				break;
			case TBCTYPE_SETEFONT:
				efontid = ptr->code[i].v.value;
				fprintf (psfp,
					"/%s findfont %.2f scalefont setfont\n",
					psfonts[efontid - 1],
					fontsz);
				break;
			case TBCTYPE_SETRGB:
				rr = (c & 0xff0000) >> 16;
				gg = (c & 0x00ff00) >> 8;
				bb = (c & 0x0000ff);

				write_ascii_string (xx, yy, char_str, char_cnt);
				xx = xx + char_cnt - slash_cnt;
				char_cnt = slash_cnt = 0;

				fprintf (psfp, "%.3f %.3f %.3f setrgbcolor\n",
						(float) rr / 255,
						(float) gg / 255,
						(float) bb / 255);
				break;
			case TBCTYPE_BIG5:
				write_ascii_string (xx, yy, char_str, char_cnt);
				xx = xx + char_cnt - slash_cnt;
				char_cnt = slash_cnt = 0;

				if (use_direct_xy) {
					if (dir_vdir && (dir_x != dir_x_save)) {
						dir_x = dir_x_save;
						dir_y -= dir_fs + dir_ls;
					}

					write_dict_char_xy (dir_x, dir_y, c,
							dir_fs, dir_fsy);

					if (dir_vdir) {
						dir_y -= dir_fs + dir_ls;
					} else {
						dir_x += dir_fs + dir_cs;
					}
				} else {
					write_dict_char (xx, yy, c);

					xx += 2;	// いゅ絏, 禲ㄢ
					rx += 2;
				}
				break;
			default:
				isascii = 1;
				break;
			}

			if (! isascii) {
				// do nothing !!
			} else if (use_direct_xy) {
				len = 0;

				if ((c == '(')||(c == ')')||(c == '\\')) {
					dir_str[len++] = '\\';
				}
				dir_str[len++] = c;
				dir_str[len++] = '\0';

				write_ascii_string_xy (dir_x,
					dir_y + fontsz / 6,
					dir_cs - fontsz / 10, dir_str, len,
					efontid,
					fontsz);

				dir_x += fontsz / 2 + dir_cs;
			} else if (c == '\n') {
				write_ascii_string (xx, yy, char_str, char_cnt);
				char_cnt = slash_cnt = 0;
				line_count++;

				if (++yy >= line_per_page) {
					write_next_page (++page_count);
					yy = 0;
					need_start_new_page = 1;
				}

				xx = rx = 0;
			} else if (c == '\r') {
			} else if (c == '\f') {	// form feed
				write_next_page (++page_count);
				yy = 0;
				need_start_new_page = 1;
				char_cnt = slash_cnt = 0;
				xx = rx = 0;
			} else if (c == '\t') {
				rx++;
				char_str[char_cnt++] = ' ';

				while ((rx % 8) != 0) {
					char_str[char_cnt++] = ' ';
					rx++;
				}
			} else if ((c == '(') || (c == ')') || (c == '\\')) {
				rx++;
				slash_cnt++;
				char_str[char_cnt++] = '\\';
				char_str[char_cnt++] = c;
			} else {
				rx++;
				char_str[char_cnt++] = c;
			}
		}
	}

	write_ascii_string (xx, yy, char_str, char_cnt);

	if ((char_cnt > 0) || (yy > 0)) {
		write_next_page (++page_count);
	}

	fprintf (psfp, "\n%%%%Trailer\n%%%%EOF\n\n"); 
	fclose (psfp);

	return page_count;
}

// -----------------------------------------------------------------------

static int assicmd_getint (char *str) {
	int	val = 0;

	if (str[0] == '\0') return 1;

	sscanf (str, "%d", &val);
	return val;
}

static int cmd_px (char *str) {
	return PCMD_NULL;
}

static int cmd_py (char *str) {
	return PCMD_NULL;
}

static int cmd_fs (char *str) {
	float	fs;

	sscanf (str, "%f", &fs);
	fprintf (stderr, "Set font size to [%.2f]\n", fs);
	tteng.set_fontsize (fs);
	return PCMD_NULL;
}

static int cmd_usefn (char *str) {
	int	i, j, fnum;

	sscanf (str, "%d", &fnum);

	for (i = j = 0; i < number_of_font_entry; i++) {
		if (i != eudcfont_id) {
			if (++j == fnum) {
				big5font_id = i;
				return 1;
			}
		}
	}

	return PCMD_NULL;
}

static int cmd_eudc (char *str) {
	char	fontfile[MAXLINE];

	sscanf (str, "%s", fontfile);
	tteng.set_eudc_fontfile (fontfile);
	// fprintf (stderr, "MPHDR [%s]\n", file);
	// big5font_id = tteng.new_FontFace (fontfile);

	return PCMD_NULL;
}

static int cmd_fn (char *str) {
	char	fontfile[MAXLINE];

	sscanf (str, "%s", fontfile);
	tteng.set_fontfile (fontfile);
	// fprintf (stderr, "MPHDR [%s]\n", file);
	// big5font_id = tteng.new_FontFace (fontfile);

	return PCMD_NULL;
}

static int cmd_mphdr (char *str) {
	static char	file[MAXLINE];

	sscanf (str, "%s", file);
	tteng.set_background_eps (file);
	// fprintf (stderr, "MPHDR [%s]\n", file);
	return PCMD_NULL;
}

static int cmd_paper (char *str) {
	static char	paper[MAXLINE];

	sscanf (str, "%s", paper);

	tteng.set_paper_type (paper);
	return PCMD_NULL;
}

static int cmd_outbin (char *str) {
	tteng.outbin (assicmd_getint (str));
	return PCMD_NULL;
}

static int cmd_inlinecmd (char *str) {
	int	i;

	i = assicmd_getint (str);

	if ((i == 1) || (i == 0)) inline_cmd = i;
	return PCMD_NULL;
}

static int cmd_tray (char *str) {
	tteng.tray (assicmd_getint (str));
	return PCMD_NULL;
}

static int cmd_page_no (char *str) {
	int	i = assicmd_getint (str);

	if ((i == 1) || (i == 0)) layout.page_no = i;
	return PCMD_NULL;
}

static int cmd_wrap (char *str) {
	int	i = assicmd_getint (str);

	if ((i == 1) || (i == 0)) layout.wrap = i;
	return PCMD_NULL;
}

static int cmd_linehead_cmd (char *str) {
	int	i;

	i = assicmd_getint (str);
	if ((i == 1) || (i == 0)) linehead_cmd = i;
	return PCMD_NULL;
}

static int cmd_form_feed (char *str) {
	return PCMD_FORMFEED;
}

static int cmd_change_cmd (char *str) {
	int	i;

	for (i = 0; i < MAX_CMD_LEN; i++) {
		if ((str[i] == ' ') || (str[i] == '\0')) {
			break;
		}
		cmdlead[i] = str[i];
	}
	cmdlead[i] = '\0';

	cmdleadlen = i;

	fprintf (stderr, "New command: [%s]\n", cmdlead);
	return PCMD_NEW_COMMAND;
}

static int cmd_landscape (char *str) {
	int	i;

	i = assicmd_getint (str);

	if ((i == 1) || (i == 0)) {
		fprintf (stderr, "Landscape = %d\n", i);
		layout.landscape = i;
	}

	return PCMD_NULL;
}

static int cmd_setrgb (char *str) {
	int	value;

	sscanf (str, "%x", &value);
	current_rgb = value;
	return PCMD_SETRGB;
}

static int cmd_rgb_save (char *str) {
	saved_rgb = current_rgb;
	return PCMD_NULL;
}

static int cmd_rgb_restore (char *str) {
	current_rgb = saved_rgb;
	return PCMD_SETRGB;
}

static int cmd_vprint (char *str) {
	int	retval = cmd_print (str);

	direct_vdir = 1;
	return retval;
}

static int cmd_print (char *str) {
	float	k, n = 1.0;
	int	i, j, m, len, loop_break;
	int	have_dot = 0;
	int	err = 0;

	len = strlen (str);
	direct_vdir = 0;

	for (i = j = m = loop_break = 0, k = 0.0; i < len; i++) {
		if (str[i] == ',') {
			k /= n;
			switch (j++) {
			case 0:	// x
				direct_x = k;
				break;
			case 1: // y
				direct_y = k;
				break;
			case 2: // font size
				direct_fs = (m == 0) ? layout.font_size : k;
				break;
			case 3: // font size (y)
				direct_fsy = (m == 0) ? direct_fs : k;
				break;
			case 4: // character space
				direct_cs = (m == 0) ? layout.char_space : k;
				break;
			case 5: // line space
				direct_ls = (m == 0) ? layout.line_space : k;
				// direct_rotate = k;
				loop_break = 1;
				i++;
				break;
			}
			k = m = 0;

			if (loop_break) break;
		} else if ((str[i] >= '0') && (str[i] <= '9')) {
			k = k * 10 + (int) (str[i] - '0');
			m++;
			if (have_dot) n *= 10;
		} else if (str[i] == '.') {
			if (have_dot) err = 1;
			have_dot = 1;
			n = 1.0;
		} else {
			err = 1;
		}

		if (err) {
			fprintf (stderr, "Incorrect format [%s]\n", str);
			return PCMD_NULL;
		}
	}

	direct_str = &str[i];

	// fprintf (stderr, "@(%.2f,%.2f)x%.2f[%s]\n", x, y, fs, &str[i]);
	return PCMD_PRINT;
}

static int cmd_box (char *str) {
	sscanf (str, "%f,%f,%f,%f,%f",
			&dg_sx, &dg_sy, &dg_ex, &dg_ey, &dg_width);

	// fprintf (stderr, "(%.2f, %.2f) width = %.2f\n", x, y, width);

	return (dg_width > 0.0) ? PCMD_DRAW_BOX : PCMD_NULL;
}

static int cmd_line (char *str) {
	sscanf (str, "%f,%f,%f,%f,%f",
			&dg_sx, &dg_sy, &dg_ex, &dg_ey, &dg_width);

	// fprintf (stderr, "(%.2f, %.2f) width = %.2f\n", x, y, width);

	return (dg_width > 0.0) ? PCMD_DRAW_LINE : PCMD_NULL;
}

static int cmd_list_all_fonts (char *str) {
	int	i;

	for (i = 0; psfonts[i] != NULL; i++) {
		fprintf (stderr, "[%s]\n", psfonts[i]);
	}

	return PCMD_NULL;
}

static int cmd_setfont (char *str) {
	int	i, j, m, n, len;
	int	fid = 0;
	float	fs = 0.0;

	if ((len = strlen (str)) ==0) {
		return PCMD_FONT_RESET;
	}


	for (i = j = 0; i <= len; i++) {
		if (j == 0) {
			if ((str[i] == ',') || (str[i] == '\0')) {
				j = 1;

				for (m = 0; psfonts[m] != NULL; m++) {
					if ((n = strlen (psfonts[m])) == i) {
						if (strncmp (psfonts[m],
								str, n) == 0) {
							fid = m + 1;
							break;
						}
					}
				}
			}
		} else {
			fs = atof (&str[i]);
			break;
		}
	}

	if (fid > 0) {
		setefont_id = fid;
		setefont_sz = fs;

		//fprintf (stderr, "Fonts: [%s][%.2f]\n",
		//		psfonts[setefont_id - 1],
		//		setefont_sz);

		return PCMD_SETEFONT;
	}

	return PCMD_NULL;
}

// -----------------------------------------------------------------------

static int get_Big5_char_index (const char *str) {
	unsigned char	hi, lo;
	int		area, l_area;
	int		seq_num = -1;

	hi = (unsigned char) str[0];
	lo = (unsigned char) str[1];

	if ((0x40 <= lo) && (lo <= 0x7e)) {
		l_area = 1;
	} else if ((0xa1 <= lo) && (lo <= 0xfe)) {
		l_area = 2;
	} else {
		l_area = 0;
	}

	if ((0xa4 <= hi) && (hi <= 0xc6) && (l_area == 1)) {
		area = 1;
	} else if ((0xa4 <= hi) && (hi < 0xc6) && (l_area == 2)) {
		area = 2;
	} else if ((0xc9 <= hi) && (hi <= 0xf9) && (l_area == 1)) {
		area = 3;
	} else if ((0xc9 <= hi) && (hi <= 0xf9) && (l_area == 2)) {
		area = 4;
	} else if ((0xa1 <= hi) && (hi <= 0xa3) && (l_area == 1)) {
		area = 5;
	} else if ((0xa1 <= hi) && (hi <= 0xa3) && (l_area == 2)) {
		area = 6;
	} else {
		area = 0;
	}

	switch (area) {
	case 0: // Not a Big5-Encoding
		// seq_num = 13184;
		seq_num = -1;
		break;
	case 1:
		seq_num = (hi - 0xa4) * 157 + (lo - 0x40);
		break;
	case 2:
		seq_num = (hi - 0xa4) * 157 + (lo - 0xa1) + 63;
		break;
	case 3:
		seq_num = (hi - 0xc9) * 157 + (lo - 0x40) + 5401;
		break;
	case 4:
		seq_num = (hi - 0xc9) * 157 + (lo - 0xa1) + 5464;
		break;
	case 5:
		seq_num = (hi - 0xa1) * 157 + (lo - 0x40) + 13094;
		break;
	case 6:
		seq_num = (hi - 0xa1) * 157 + (lo - 0xa1) + 13157;
		break;
	}

	if (seq_num != -1) {
		seq_num = ((unsigned int) hi * 256) + ((unsigned int) lo);
	}

	return seq_num;
}

static int get_EUDC_char_index (const char *str) {
	unsigned char	hi, lo;
	int		l_area;
	int		seq_num = -1;
	int		l_seq = 0;
	int		eh, el;

	hi = (unsigned char) str[0];
	lo = (unsigned char) str[1];

	eh = 0x7e - 0x40 + 1;
	el = 0xfe - 0xa1 + 1;

	if ((0x40 <= lo) && (lo <= 0x7e)) {
		l_area = 1;
		l_seq = lo - 0x40;
	} else if ((0xa1 <= lo) && (lo <= 0xfe)) {
		l_area = 2;
		l_seq = lo - 0xa1 + eh;
	} else {
		return -1;
	}

	if ((0xfa <= hi) && (hi <= 0xfe)) {
		seq_num = (hi - 0xfa) * (eh + el) + l_seq;
	} else if ((0x8e <= hi) && (hi <= 0xa0)) {
		seq_num = (hi - 0x8e) * (eh + el) + l_seq +
				(0xfe - 0xfa + 1) * (eh + el);
	} else if ((0x81 <= hi) && (hi <= 0x8d)) {
		seq_num = (hi - 0x81) * (eh + el) + l_seq +
				(0xfe - 0xfa + 1 + 0xa0 - 0x8e + 1) * (eh + el);
	} else if ((0xc6 <= hi) && (hi <= 0xc8) && (l_area == 2)) {
		seq_num = (hi - 0xc6) * (eh + el) + lo - 0xa1 +
				(0xfe - 0xfa + 1 + 0xa0 - 0x8e + 1 +
				 0x8d - 0x81 + 1) * (eh + el);
	} else {
		return -1;
	}

	return seq_num + 0xe000;
}
