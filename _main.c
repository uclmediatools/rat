/*
 * FILE:    main.c
 * PROGRAM: RAT - controller
 * AUTHOR:  Colin Perkins 
 * 
 * This is the main program for the RAT controller.  It starts the 
 * media engine and user interface, and controls them via the mbus.
 *
 * $Revision$
 * $Date$
 * 
 * Copyright (c) 1999 University College London
 * All rights reserved.
 *
 */

#include "config_unix.h"
#include "config_win32.h"
#include "debug.h"
#include "mbus.h"
#include "version.h"
#include "mbus_control.h"

#define UI_NAME     "rat-"##VERSION_NUM##"-ui"
#define ENGINE_NAME "rat-"##VERSION_NUM##"-media"

static char *fork_process(struct mbus *m, char *proc_name, char *ctrl_addr, pid_t *pid)
{
	struct timeval	 timeout;
	char		*token = xmalloc(100);
	char		*token_e;
	char		*peer_addr;

	sprintf(token, "rat-controller-waiting-%ld", lrand48());

	*pid = fork();
	if (*pid == -1) {
		perror("Cannot fork");
		abort();
	} else if (*pid == 0) {
		execl(proc_name, proc_name, "-ctrl", ctrl_addr, "-token", token, NULL);
		perror("Cannot execute subprocess");
		/* Note: this MUST NOT be exit() or abort(), since they affects the standard */
		/* IO channels in the parent process (fork duplicates file descriptors, but  */
		/* they still point to the same underlying file).                            */
		_exit(1);	
	}

	/* This is the parent: we have to wait for the child to say hello before continuing. */
	token_e = mbus_encode_str(token);
	mbus_control_wait_init(token);
	while ((peer_addr = mbus_control_wait_done()) == NULL) {
		debug_msg("Waiting for token \"%s\" from sub-process\n", token);
		timeout.tv_sec  = 0;
		timeout.tv_usec = 500000;
		mbus_heartbeat(m, 1);
		mbus_qmsgf(m, "()", FALSE, "mbus.waiting", "%s", token_e);
		mbus_send(m);
		mbus_recv(m, (void *) m, &timeout);
	}
	debug_msg("forked %s\n", proc_name);
	xfree(token);
	xfree(token_e);
	return peer_addr;
}

static void kill_process(pid_t proc)
{
	kill(proc, SIGINT);
}

int main(int argc, char *argv[])
{
	struct mbus	*m;
	char		 m_addr[60];
	char		*u_addr, *e_addr;
	pid_t		 pid_ui, pid_engine;
        int		 seed = (gethostid() << 8) | (getpid() & 0xff);

	srand48(seed);

	UNUSED(argc);
	UNUSED(argv);

	m = mbus_init(mbus_control_rx, NULL);
	snprintf(m_addr, 60, "(media:audio module:control app:rat instance:%lu)", (u_int32) getpid());
	mbus_addr(m, m_addr);

	u_addr = fork_process(m, UI_NAME,     m_addr, &pid_ui);
	e_addr = fork_process(m, ENGINE_NAME, m_addr, &pid_engine);

	while (1) {
		sleep(1);
	}

	kill_process(pid_ui);
	kill_process(pid_engine);
	return 0;
}
