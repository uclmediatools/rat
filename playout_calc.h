/*
 * FILE:    playout_calc.h
 * PROGRAM: RAT
 * AUTHOR:  Orion Hodson
 * 
 * $Revision$
 * $Date$
 * 
 * Copyright (c) 1999-2000 University College London
 * All rights reserved.
 *
 */

#ifndef __PLAYOUT_CALC_H__
#define __PLAYOUT_CALC_H__

ts_t playout_calc(struct s_session *sp, uint32_t ssrc, ts_t transit_ts, int new_spurt);

#endif /* __PLAYOUT_CALC_H__ */
