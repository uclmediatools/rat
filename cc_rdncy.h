/*
 * FILE:      cc_rdncy.h
 * AUTHOR(S): Orion Hodson 
 *	
 * $Revision$
 * $Date$
 * 
 * Copyright (c) 1995-99 University College London
 * All rights reserved.
 *
 */

#ifndef __CC_RDNCY_H__
#define __CC_RDNCY_H__

/* Encoder functions *********************************************************/

int  redundancy_encoder_create  (u_char **state, u_int32_t *len);

void redundancy_encoder_destroy (u_char **state, u_int32_t  len);

int  redundancy_encoder_reset   (u_char  *state);

int  redundancy_encoder_encode  (u_char                  *state,
                                 struct s_pb *in,
                                 struct s_pb *out,
                                 u_int32_t                  units_per_packet);

int  redundancy_encoder_set_parameters(u_char *state, char *cmd);
int  redundancy_encoder_get_parameters(u_char *state, char *buf, u_int32_t blen);

/* Decoder functions *********************************************************/

int  redundancy_decoder_decode  (u_char                  *state,
                                 struct s_pb *in,
                                 struct s_pb *out,
                                 ts_t                     now);

int redundancy_decoder_peek     (u_int8_t   pkt_pt,
                                 u_char  *data,
                                 u_int32_t  len,
                                 u_int16_t  *upp,
                                 u_int8_t   *pt);

int redundancy_decoder_describe (u_int8_t   pkt_pt,
                                 u_char  *data,
                                 u_int32_t  len,
                                 char    *out,
                                 u_int32_t  out_len);
 
#endif /* __CC_RDNCY_H__ */

