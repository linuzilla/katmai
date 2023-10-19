#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "cmdlintf.h"

static int cmd_logout (struct cmdlintf_t *cli, char *cmd) {
	cli->print ("Bye-Bye\n");
	return 0;
}

int main (int argc, char *argv[]) {
	struct cmdlintf_t	*cli;
	int			is_start_cs = 0;

	is_start_cs = 1;

	cli = init_cmdline_interface ();
	cli->regcmd ();
	cli->add ("exit", 0, cmd_logout, NULL, 0, 1);
	cli->set_timeout (3);

	if (is_start_cs) {
		cli->socket_name ("/tmp/cli.sock");
		if (fork () == 0) {
			if (cli->start (1) == 1) {
				fprintf (stderr, "Server is terminate!\n");
			}
			exit (0);
		}
		sleep (1);
	}

	cli->start (0);
	while (cli->cli () != 0) ;
	return 0;
}
