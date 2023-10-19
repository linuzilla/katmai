#ifndef __ETen_Font_library_H_
#define __ETen_Font_library_H_

struct ETlib_t {
	int	(*open)(const char *path);
	int	(*close)(void);
	int	(*isbig5)(const int hi, const int lo);
	char *	(*ffont24)(unsigned char hbp, unsigned char lbp);
	char *	(*hfont24)(unsigned char bp);
	char *	(*ffont15)(unsigned char hbp, unsigned char lbp);
	char *	(*hfont15)(unsigned char bp);
};

struct ETlib_t *	new_etenlib (void);

#endif
