/*
 * FILE:    codec_gsm.h
 * AUTHORS: Orion Hodson
 * 
 * Copyright (c) 1998 University College London
 * All rights reserved.
 *
 */

#ifndef _CODEC_GSM_H_
#define _CODEC_GSM_H_

u_int16                      gsm_get_formats_count (void);
const struct s_codec_format* gsm_get_format(u_int16 idx);

int  gsm_state_create  (u_int16 idx, u_char **state);
void gsm_state_destroy (u_int16 idx, u_char **state);
int  gsm_encoder       (u_int16 idx, u_char *state, sample *in, coded_unit *out);
int  gsm_decoder       (u_int16 idx, u_char *state, coded_unit *in, sample *out);

int  gsm_repair        (u_int16 idx, u_char *state, u_int16 consec_lost,
                        coded_unit *prev, coded_unit *missing, coded_unit *next);

#endif /* _CODEC_GSM_H_ */
