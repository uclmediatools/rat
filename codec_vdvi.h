/*
 * FILE:    codec_vdvi.h
 * AUTHORS: Orion Hodson
 * 
 * Copyright (c) 1998 University College London
 * All rights reserved.
 *
 */

/* Just wrote the RAT interface, see codec_dvi.c for coder copyright [oth] */

#ifndef _CODEC_VDVI_H_
#define _CODEC_VDVI_H_

u_int16               vdvi_get_formats_count (void);
const codec_format_t* vdvi_get_format        (u_int16 idx);
int                   vdvi_state_create      (u_int16 idx, u_char **state);
void                  vdvi_state_destroy     (u_int16 idx, u_char **state);
int                   vdvi_encoder           (u_int16 idx, u_char *state, sample     *in, coded_unit *out);
int                   vdvi_decoder           (u_int16 idx, u_char *state, coded_unit *in, sample     *out);
int                   vdvi_peek_frame_size   (u_int16 idx, u_char *data, int data_len);

#endif /* _CODEC_VDVI_H_ */



