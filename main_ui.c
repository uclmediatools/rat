/*
 * FILE:    main_ui.c
 * PROGRAM: RAT
 * AUTHORS: Colin Perkins 
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
#include "mbus_ui.h"
#include "tcltk.h"

char	 m_addr[100];
char	*c_addr, *token, *token_e; 

static void parse_args(int argc, char *argv[])
{
	int 	i;

	if (argc != 5) {
		printf("Usage: %s -ctrl <addr> -token <token>\n", argv[0]);
		exit(1);
	}
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-ctrl") == 0) {
			c_addr = xstrdup(argv[++i]);
		} else if (strcmp(argv[i], "-token") == 0) {
			token   = xstrdup(argv[++i]);
			token_e = mbus_encode_str(token);
		} else {
			printf("Unknown argument \"%s\"\n", argv[i]);
			abort();
		}
	}
}

int main(int argc, char *argv[])
{
	struct mbus	*m;
	struct timeval	 timeout;

	parse_args(argc, argv);
/*	tcl_init(m, argc, argv, e_addr);*/

	m = mbus_init(mbus_ui_rx, NULL);
	sprintf(m_addr, "(media:audio module:ui app:rat instance:%lu)", (u_int32) getpid());
	mbus_addr(m, m_addr);

	/* The first stage is to wait until we hear from our controller. The address of the */
	/* controller is passed to us via a command line parameter, and we just wait until  */
	/* we get an mbus.hello() from that address.                                        */
	while (!mbus_addr_valid(m, c_addr)) {
		debug_msg("Waiting to validate address %s\n", c_addr);
		timeout.tv_sec  = 0;
		timeout.tv_usec = 500000;
		mbus_recv(m, NULL, &timeout);
		mbus_send(m);
		mbus_heartbeat(m, 1);
	}
	debug_msg("Address %s is valid\n", c_addr);

	/* Next, we signal to the controller that we are ready to go. It should be sending  */
	/* us an mbus.waiting(foo) where "foo" is the same as the "-token" argument we were */
	/* passed on startup. We respond with mbus.go(foo) sent reliably to the controller. */
	mbus_ui_wait_handler_init(token);
	mbus_cmd_handler(m, mbus_ui_wait_handler);
	while (!mbus_ui_wait_handler_done()) {
		debug_msg("Waiting for token \"%s\" from controller\n", token);
		timeout.tv_sec  = 0;
		timeout.tv_usec = 500000;
		mbus_recv(m, m, &timeout);
		mbus_send(m);
		mbus_heartbeat(m, 1);
	}
	mbus_cmd_handler(m, mbus_ui_rx);
	mbus_qmsgf(m, c_addr, TRUE, "mbus.go", "%s", token_e);
	mbus_send(m);

	/* ...and sit waiting for it to send us commands... */
	while (1) {
		timeout.tv_sec  = 0;
		timeout.tv_usec = 50000;
		mbus_recv(m, NULL, &timeout);
		mbus_send(m);
		mbus_heartbeat(m, 1);
		tcl_process_all_events();
	}
}
