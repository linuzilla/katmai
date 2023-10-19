/*
 *	conio.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include "conio.h"

static struct consoleIO_t	self;
static struct termios		orig, newtio;
static int			peek = -1;

static int conio_open (void) {
	peek = -1;

	if (tcgetattr (0, &orig) == -1) {
		fprintf (stderr, "Could not tcgetattr");
		return 0;
	}

	newtio = orig;

	newtio.c_lflag &= ~ICANON;
	newtio.c_lflag &= ~ECHO;
	newtio.c_lflag &= ~ISIG;
	newtio.c_cc[VMIN]  =  1;
	newtio.c_cc[VTIME] =  0;

	if (tcsetattr (0, TCSANOW, &newtio)==-1) {
		fprintf (stderr, "Could not tcsetattr");
		return 0;
	}

	return 1;
}

static void conio_close (void) {
	tcsetattr (0, TCSANOW, &orig);
}

static int conio_kbhit (void) {
	char	ch;
	int	nread;

	if (peek != -1) return 1;

	newtio.c_cc[VMIN] = 0;
	tcsetattr (0, TCSANOW, &newtio);
	nread = read (0, &ch, 1);
	newtio.c_cc[VMIN] = 1;
	tcsetattr (0, TCSANOW, &newtio);

	if (nread == 1) {
		peek = ch;
		return 1;
	}

	return 0;
}

static int conio_read (void) {
	char	ch;

	if (peek != -1) {
		ch   = peek;
		peek = -1;
		return ch;
	}

	read (0, &ch, 1);
	return ch;
}

struct consoleIO_t * new_consoleIO (void) {
	self.open	= conio_open;
	self.close	= conio_close;
	self.kbhit	= conio_kbhit;
	self.read	= conio_read;

	return &self;
}
