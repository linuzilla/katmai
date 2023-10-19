/*
 *	rs232.h
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#ifndef __RS232_H__
#define __RS232_H__

#include <termio.h>

struct rs232_private_data_t {
	int		fd;
	int		ioflags;
	short		swflow;		// Software Flow Control
	short		hwflow;		// Hardware Flow Control
	short		crnl;		// send CR with NL
	short		nbflag;	// blocking mode
	speed_t		speed;		// Baud_Rate
	struct termios	pots;
	struct termios	pts;
};

struct rs232_t {
	struct rs232_private_data_t	pd;
	int	(*open)(struct rs232_t *, const char *str);
	int	(*close)(struct rs232_t *);
	int	(*h_flowcontrol)(struct rs232_t *, const int hf);
	int	(*s_flowcontrol)(struct rs232_t *, const int sf);
	int	(*nonblocking)(struct rs232_t *, const int nb);
	void	(*clear)(struct rs232_t *);
	int	(*fd)(struct rs232_t *);
	speed_t	(*speed)(struct rs232_t *, speed_t sp);
};

struct rs232_t * new_rs232 (void);

#endif
