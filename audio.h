/*
 * FILE:    audio.h
 * PROGRAM: RAT
 * AUTHOR:  Isidor Kouvelas / Orion Hodson / Colin Perkins
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

#ifndef _RAT_AUDIO_H_
#define _RAT_AUDIO_H_

#include "audio_types.h"


#define BD_THRESHOLD    16
#define BD_CONSECUTIVE  54

#define AUDIO_NO_DEVICE -1

/* Structures used in function declarations below */
struct s_cushion_struct;
struct session_tag;
struct s_bias_ctl;
struct s_mix_info;

/* Audio interface fn's for dealing with multiple devices/device interfaces */
int     audio_init_interfaces(void);
int     audio_free_interfaces(void);
int     audio_get_number_of_interfaces(void);
char*   audio_get_interface_name(int idx);
void    audio_set_interface(int idx);
int     audio_get_interface(void);

/* Audio functions implemented by device interfaces */
int	audio_open          (audio_format *format);
void	audio_close         (audio_desc_t ad);
void	audio_drain         (audio_desc_t ad);
void	audio_set_gain      (audio_desc_t ad, int gain);
int     audio_duplex        (audio_desc_t ad);
int	audio_get_gain      (audio_desc_t ad);
void	audio_set_volume    (audio_desc_t ad, int vol);
int	audio_get_volume    (audio_desc_t ad);
void    audio_loopback      (audio_desc_t ad, int gain);
int	audio_read          (audio_desc_t ad, sample *buf, int samples);
int	audio_write         (audio_desc_t ad, sample *buf, int samples);
void	audio_non_block     (audio_desc_t ad);
void	audio_block         (audio_desc_t ad);
void	audio_set_oport     (audio_desc_t ad, int port);
int	audio_get_oport     (audio_desc_t ad);
int	audio_next_oport    (audio_desc_t ad);
void	audio_set_iport     (audio_desc_t ad, int port);
int	audio_get_iport     (audio_desc_t ad);
int	audio_next_iport    (audio_desc_t ad);
int	audio_get_blocksize (audio_desc_t ad);
int	audio_get_channels  (audio_desc_t ad);
int     audio_get_freq      (audio_desc_t ad);
int     audio_is_ready      (audio_desc_t ad);
void    audio_wait_for      (audio_desc_t ad, int granularity_ms);

/* General audio processing functions */
void	mix_init     (void);
void	mix2_pcmu    (u_int8 *v0, u_int8 *v1, size_t len);
void	mix2_l16     (int16 *v0, int16 *v1, size_t len);
void	mix2_l8      (int8 *v0, int8 *v1, size_t len);
void	audio_zero   (sample *buf, int len, deve_e type);
void    audio_unbias (struct s_bias_ctl *bc, sample *buf, int len);

int     read_write_audio (struct session_tag *spi, struct session_tag *spo, struct s_mix_info *ms);
void    read_write_init  (struct session_tag *session_pointer);

int     audio_device_read        (struct session_tag *sp, sample *buf, int len);
int     audio_device_write       (struct session_tag *sp, sample *buf, int samples);
int     audio_device_take        (struct session_tag *sp);
void	audio_device_give        (struct session_tag *sp);
void    audio_device_reconfigure (struct session_tag *sp);

#endif /* _RAT_AUDIO_H_ */
