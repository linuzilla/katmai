/*
 *	conio.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __CONIO_H__
#define __CONIO_H__

struct consoleIO_t {
	int	(*open)(void);
	void	(*close)(void);
	int	(*kbhit)(void);
	int	(*read)(void);
};

struct consoleIO_t	*new_consoleIO (void);

#endif
