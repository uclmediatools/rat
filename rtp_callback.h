/*
 * FILE:    rtp_callback.h
 * PROGRAM: RAT
 * AUTHOR:  Colin Perkins / Orion Hodson
 * 
 * $Revision$
 * $Date$
 * 
 * Copyright (c) 1999-2000 University College London
 * All rights reserved.
 *
 */

#ifndef __RTP_CALLBACK_H__
#define __RTP_CALLBACK_H__

struct s_session;

void rtp_callback_init (struct rtp *s, struct s_session *sp);
void rtp_callback      (struct rtp *s, rtp_event *e);
void rtp_callback_exit (struct rtp *s);

#endif /* __RTP_CALLBACK_H__ */
