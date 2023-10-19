/*
 */

#ifndef __CLI_PREDEF_CMDS_H__
#define __CLI_PREDEF_CMDS_H__

static int cmd_history (struct cmdlintf_t *cli, char *cmd) {
	if (cmd != NULL) {
		cli->print ("%% Invalid input detected: \"%s\"\n", cmd);
	} else {
		HIST_ENTRY	**the_list;
		int		i;

		if ((the_list = history_list ())!= NULL) {
			for (i = 0; the_list[i] != NULL; i++) {
				cli->print ("%5d %s\n",
					i + history_base,
					the_list[i]->line);
			}
		}

		return 2;
	}

	return 1;
}

static int cmd_clear_history (struct cmdlintf_t *cli, char *cmd) {
	if (cmd != NULL) {
		cli->print ("%% Invalid input detected: \"%s\"\n", cmd);
	} else {
		clear_history ();
		return 2;
	}
	return 1;
}

static int cmd_terminate (struct cmdlintf_t *cli, char *cmd) {
	if (cmd != NULL) {
		cli->print ("%% Invalid input detected: \"%s\"\n", cmd);
	} else {
		cli->pd->terminate = 1;
	}

	return 0;
}

#endif
