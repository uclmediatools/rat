/*
 * FILE:    cushion.h
 * PROGRAM: RAT
 * AUTHOR:  Isidor Kouvelas
 * MODIFICATIONS: Orion Hodson
 *
 * $Revision$
 * $Date$
 *
 * Copyright (c) 1995-98 University College London
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, is permitted, for non-commercial use only, provided
 * that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the Computer Science
 *      Department at University College London
 * 4. Neither the name of the University nor of the Department may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 * Use of this software for commercial purposes is explicitly forbidden
 * unless prior written permission is obtained from the authors.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef __CUSHION_H__
#define __CUSHION_H__

#define CUSHION_MODE_LECTURE    0
#define CUSHION_MODE_CONFERENCE 1

struct s_cushion_struct;

int     cushion_new               (struct s_cushion_struct **c);
void    cushion_free              (struct s_cushion_struct *c);
void    cushion_update            (struct s_cushion_struct *c, 
                                   u_int32 read_dur, 
                                   int cushion_mode);
u_int32 cushion_get_size          (struct s_cushion_struct *c);
u_int32 cushion_set_size          (struct s_cushion_struct *c, 
                                   u_int32 new_size);
u_int32 cushion_step_up           (struct s_cushion_struct *c);
u_int32 cushion_step_down         (struct s_cushion_struct *c);

u_int32 cushion_get_step          (struct s_cushion_struct *c);

u_int32 cushion_use_estimate      (struct s_cushion_struct *c);
int32   cushion_diff_estimate_size(struct s_cushion_struct *c);

sample *cushion_zero_buffer       (struct s_cushion_struct *c);

#endif __CUSHION_H__


