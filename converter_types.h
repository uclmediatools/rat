/*
 * FILE:    convert_types.h
 * PROGRAM: RAT
 * AUTHOR:  O.Hodson
 * 
 * $Revision$
 * $Date$
 *
 * Copyright (c) 1998-99 University College London
 * All rights reserved.
 *
 */

#ifndef __CONVERT_TYPES_H__
#define __CONVERT_TYPES_H__

struct s_coded_unit;

typedef struct s_converter_fmt {
        u_int16_t from_channels;
        u_int16_t src_freq;
        u_int16_t to_channels;
        u_int16_t dst_freq;
} converter_fmt_t;

typedef u_int32_t converter_id_t;

typedef struct {
        converter_id_t id;
        const char*    name;
} converter_details_t;

struct  s_converter;

#endif /* __CONVERT_TYPES_H__ */
