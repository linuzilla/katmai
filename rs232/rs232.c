/*
 *	rs232.c
 *
 *	Copyright (c) 2002, Jiann-Ching Liu
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "rs232.h"

static int rs232_open (struct rs232_t *self, const char *device) {
	struct rs232_private_data_t	*pd = &self->pd;
	short				noflow = 0;

	if ((pd->fd = open (device, O_RDWR)) < 0) {
		perror (device);
		return 0;
	}

	pd->ioflags = fcntl (pd->fd, F_GETFL);

	/* modify the port configuration */
	tcgetattr (pd->fd, &pd->pts);
	pd->pots = pd->pts;

	/* some things we want to set arbitrarily */
	pd->pts.c_lflag &= ~ICANON;
	pd->pts.c_lflag &= ~(ECHO | ECHOCTL | ECHONL);
	pd->pts.c_cflag |= HUPCL;
	pd->pts.c_cc[VMIN] = 1;
	pd->pts.c_cc[VTIME] = 0;

	/* Standard CR/LF handling: this is a dumb terminal.
	 * Do no translation:
	 *   no NL -> CR/NL mapping on output, and
	 *   no CR -> NL mapping on input.
	 */

	pd->pts.c_oflag &= ~ONLCR;
	pd->pts.c_iflag &= ~ICRNL;

	/* Now deal with the local terminal side */

	if (pd->crnl)   pd->pts.c_oflag |= ONLCR;       // send CR with NL

	if (pd->hwflow) {   // Hardware flow control
		pd->pts.c_cflag |= CRTSCTS;
		pd->pts.c_iflag &= ~(IXON | IXOFF | IXANY);
	}

	if (pd->swflow) { // software flow control
		pd->pts.c_cflag &= ~CRTSCTS;
		pd->pts.c_iflag |= IXON | IXOFF | IXANY;
	}

	if (noflow) { // no flow control
		pd->pts.c_cflag &= ~CRTSCTS;
		pd->pts.c_iflag &= ~(IXON | IXOFF | IXANY);
	}

	cfsetospeed (&pd->pts, pd->speed);
	cfsetispeed (&pd->pts, pd->speed);
	 
	/* set the signal handler to restore the old
	 * termios handler */

	tcsetattr (pd->fd, TCSANOW, &pd->pts);


	return 1;
}

static int rs232_close (struct rs232_t *self) {
	struct rs232_private_data_t	*pd = &self->pd;

	if (pd->fd >= 0) {
		tcsetattr (pd->fd, TCSANOW, &pd->pots);
		close (pd->fd);
		pd->fd = -1;
		return 1;
	}

	return 0;
}

static int rs232_hardware_flowcontrol (struct rs232_t *self, const int hwfc) {
	int	rc = self->pd.hwflow;

	if (hwfc >= 0) self->pd.hwflow = (hwfc == 0) ? 0 : 1;
	return rc;
}

static int rs232_software_flowcontrol (struct rs232_t *self, const int swfc) {
	int	rc = self->pd.swflow;

	if (swfc >= 0) self->pd.swflow = (swfc == 0) ? 0 : 1;
	return rc;
}

static speed_t rs232_speed (struct rs232_t *self, speed_t sp) {
	speed_t	rc = self->pd.speed;
	self->pd.speed = sp;
	return rc;
}

static int rs232_nonblocking (struct rs232_t *self, const int nb) {
	struct rs232_private_data_t	*pd = &self->pd;
	int				rc = self->pd.nbflag;

	if ((nb == 0) && (rc != 0)) {
		// ioflags = fcntl (pd->fd, F_GETFL);
		fcntl (pd->fd, F_SETFL, pd->ioflags);
		pd->nbflag =  0;
	} else if ((nb > 0) && (rc == 0)) {
		fcntl (pd->fd, F_SETFL, pd->ioflags | O_NDELAY);
		pd->nbflag = 1;
	}

	return rc;
}

static void rs232_clear (struct rs232_t *self) {
	struct rs232_private_data_t	*pd = &self->pd;
	int				val;
	char				buffer[512];

	val = self->nonblocking (self, 1);

	while (read (pd->fd, buffer, sizeof buffer) > 0);

	self->nonblocking (self, 0);
}

static int rs232_fd (struct rs232_t *self) {
	return self->pd.fd;
}

struct rs232_t * new_rs232 (void) {
	struct rs232_t *	self;

	if ((self = malloc (sizeof (struct rs232_t))) != NULL) {
		self->open		= rs232_open;
		self->close		= rs232_close;
		self->clear		= rs232_clear;
		self->h_flowcontrol	= rs232_hardware_flowcontrol;
		self->s_flowcontrol	= rs232_software_flowcontrol;
		self->nonblocking	= rs232_nonblocking;
		self->speed		= rs232_speed;
		self->fd		= rs232_fd;

		self->pd.swflow		= 0;
		self->pd.hwflow		= 0;
		self->pd.crnl		= 0;
		self->pd.nbflag		= 0;
		self->pd.speed		= B9600;
	}

	return self;
}
