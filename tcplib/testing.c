#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "tcplib.h"

const int portnumber = 6500;
struct tcplib_t	*tcp = NULL;

void callback (const int fd, void *param) {
	int		len;
	char		buffer[1024];

	fprintf (stderr, "Callback function: fd=%d\n", fd);

	while ((len = read (fd, buffer, sizeof buffer)) > 0) {
		fprintf (stderr, "Received data, len=%d\n", len);
		write (STDOUT_FILENO, buffer, len);
	}

	close (fd);
}

void server (void) {
	tcp->listen (tcp, callback, portnumber, NULL);
	sleep (20);
	tcp->stop (tcp);
}

void client (void) {
	int	fd;
	char	*message = "hello\n";
	
	sleep (1);
	fd = tcp->connect (tcp, "140.115.11.124", portnumber);

	fprintf (stderr, "Connect ok, fd = %d\n", fd);
	write (fd, message, strlen (message));
	sleep (2);
}

int main (int argc, char *argv[]) {
	if ((tcp = new_tcplib ()) == NULL) return 1;

	if (fork () == 0) {
		server ();
	} else {
		client ();
	}

	tcp->close (tcp);

	return 0;
}
