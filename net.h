/*
 * FILE:    net.h
 * PROGRAM: RAT
 * AUTHOR:  Colin Perkins / Orion Hodson
 *
 * $Revision$
 * $Date$
 *
 * Copyright (c) 1995-2000 University College London
 * All rights reserved.
 *
 */

#ifndef _RAT_NET_H_
#define _RAT_NET_H_

void    network_process_mbus(struct s_session *sp);
uint32_t ntp_time32(void);

#endif /* _RAT_NET_H_ */


