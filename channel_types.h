/*
 * FILE:      channel_types.h
 * AUTHOR(S): Orion Hodson 
 *	
 * $Revision$
 * $Date$
 * 
 * Copyright (c) 1999 University College London
 * All rights reserved.
 *
 */
#ifndef __CHANNEL_TYPES_H__
#define __CHANNEL_TYPES_H__

/* Channel coder description information */

typedef u_int32 cc_id_t;

#define CC_NAME_LENGTH 32

typedef struct {
        cc_id_t descriptor;
        char    name[CC_NAME_LENGTH];
} cc_details;

/* In and out unit types.  On input channel encoder takes a playout buffer
 * of media_units and puts channel_units on the output playout buffer
 */

#define MAX_CHANNEL_UNITS    8
#define MAX_UNITS_PER_PACKET 8

typedef struct {
        u_int8  pt;
        u_char *data;
        u_int32 data_start; /* We use data_start to indicate offset where
                             * channel data begins relative to data(packet)
                             * since this saves an allocation, copy, and free.
                             * Used only on decode path.
                             */
        u_int32 data_len;   /* This is the length for processing purposes */
} channel_unit;

typedef struct {
        u_int8        nelem;
        channel_unit *elem[MAX_CHANNEL_UNITS];
} channel_data;

int  channel_data_create  (channel_data **cd, 
                           int            nelem);

void channel_data_destroy (channel_data **cd, 
                           u_int32        cdsize);

#endif /* __CHANNEL_TYPES_H__ */
