/*
 * FILE:    transmit.h
 * PROGRAM: RAT
 * AUTHOR:  Orion Hodson / Isidor Kouvelas
 *
 * $Revision$
 * $Date$
 *
 * Copyright (c) 1995-1999 University College London
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

#ifndef _transmit_h_
#define _transmit_h_

#include "session.h"

struct s_tx_buffer;
struct session_tag;
struct s_speaker_table;
struct s_minibuf;

int   tx_create      (struct s_tx_buffer **tb,
                      struct session_tag  *sp,
                      struct s_time       *clock,
                      u_int16 unit_size, 
                      u_int16 channels);

void  tx_destroy     (struct s_tx_buffer **tb);
void  tx_start       (struct s_tx_buffer  *tb);
void  tx_stop        (struct s_tx_buffer  *tb);
int   tx_is_sending  (struct s_tx_buffer  *tb);

int   tx_read_audio    (struct s_tx_buffer *tb);
int   tx_process_audio (struct s_tx_buffer *tb);
void  tx_send          (struct s_tx_buffer *tb);
void  tx_update_ui     (struct session_tag *sp);
void  tx_igain_update  (struct session_tag *sp);

#endif /* _transmit_h_ */
