/*
 * FILE:    bitstream.h
 * PROGRAM: RAT
 * AUTHOR:  Orion Hodson
 *
 * $Revision$
 * $Date$
 *
 * Copyright (c) 1998-99 University College London
 * All rights reserved.
 *
 */

#ifndef RAT_BITSTREAM_H
#define RAT_BITSTREAM_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct s_bitstream bitstream_t;

int  bs_create     (bitstream_t **b);

int  bs_destroy    (bitstream_t **b);

int  bs_attach     (bitstream_t *b, 
                    u_char *buf, 
                    int blen);

int  bs_put        (bitstream_t *b,
                    u_char       bits,
                    u_int8_t       nbits);

u_char bs_get      (bitstream_t *b,
                    u_int8_t  nbits);

int  bs_bytes_used (bitstream_t *b);

#ifdef __cplusplus
}
#endif

#endif /* RAT_BITSTREAM_H */



