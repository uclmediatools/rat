/*
 *  config-unix.h
 *
 *  Unix specific definitions and includes for RAT.
 *  
 *  $Revision$
 *  $Date$
 *
 * Copyright (c) 1995-98 University College London
 * All rights reserved.
 *
 */

#ifndef WIN32
#ifndef _CONFIG_UNIX_H
#define _CONFIG_UNIX_H

/* A lot of includes here that should all probably be in files where they   */
/* are used.  If anyone ever has the time to reverse the includes into      */
/* the files where they are actually used, there would be a couple of pints */
/* in it.                                                                   */

#include "config.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <ctype.h>

#include <stdio.h>
#include <stdarg.h>
#include <memory.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>  
#include <string.h>

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#ifdef HAVE_BSTRING_H
#include <bstring.h>
#endif

#ifdef HAVE_STROPTS_H
#include <stropts.h>
#endif

#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>  
#endif 

#ifdef HAVE_SYS_SOCK_IO_H
#include <sys/sockio.h>
#endif

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <arpa/inet.h>

#ifdef GETTOD_NOT_DECLARED
int gettimeofday(struct timeval *tp, void * );
#endif

#ifdef KILL_NOT_DECLARED
int kill(pid_t pid, int sig);
#endif

#ifndef TRUE
#define FALSE	0
#define	TRUE	1
#endif /* TRUE */

#define USERNAMELEN	8

#define max(a, b)	(((a) > (b))? (a): (b))
#define min(a, b)	(((a) < (b))? (a): (b))

#ifdef NDEBUG
#define assert(x) if ((x) == 0) fprintf(stderr, "%s:%u: failed assertion\n", __FILE__, __LINE__)
#else
#include <assert.h>
#endif

#endif
