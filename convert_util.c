/*
 * FILE:    converter.c
 * PROGRAM: RAT
 * AUTHOR:  O.Hodson <O.Hodson@cs.ucl.ac.uk>
 * 
 * $Revision$
 * $Date$
 *
 * Copyright (c) 1998-99 University College London
 * All rights reserved.
 *
 */

#include "config_unix.h"
#include "config_win32.h"
#include "converter_types.h"
#include "convert_util.h"
#include "debug.h"

/* Mono-Stereo Conversion ***************************************************/ 
/* Note src_len is length block in number of samples                        */
/* i.e nChannels * nSamplingIntervals                                       */

void
converter_change_channels (sample *src, 
                           int src_len, 
                           int src_channels, 
                           sample *dst, 
                           int dst_len, 
                           int dst_channels)
{
        int i;
        sample *s, *d;
        int t;
        assert(src_channels == 1 || src_channels == 2);
        assert(dst_channels == 1 || dst_channels == 2);
        assert(dst_channels != src_channels);
        assert(src_len/src_channels == dst_len/dst_channels);

        /* Differing directions of conversions means we can do in place        
         * conversion if necessary.
         */

        switch(src_channels) {
        case 1:
                s = &src[src_len - 1]; /* clumsy syntax so not to break bounds-checker in debug */
                d = &dst[dst_len - 1];
                for(i = 0; i < src_len; i++) {
                        *d-- = *s;
                        *d-- = *s--;
                }
                break;
        case 2:
                s = src;
                d = dst;
                src_len /= 2;
                for(i = 0; i < src_len; i++) {
                        t    = *s++;
                        t   += *s++;
                        t   /= 2;
                        *d++ = t;
                }
                break;
        }
        UNUSED(dst_channels);
}

int
gcd (int a, int b)
{
        if (b) return gcd(b, a%b);
        return a;
}

int
conversion_steps(int f1, int f2) 
{
        if (f1 == f2) return 0;

        if (((f1 % f2) == f2) ||
            ((f2 % f1) == f1)) {
                /* Integer conversion */
                return 1;
        } 
        /* Non-integer conversion */
        return 2;
}

int
converter_format_valid(const converter_fmt_t *cfmt)
{
        if (cfmt->src_freq % 8000 &&
            cfmt->src_freq % 11025) {
                return FALSE;
        }

        if (cfmt->from_channels != 1 &&
            cfmt->from_channels != 2) {
                return FALSE;
        }

        if (cfmt->dst_freq % 8000 &&
            cfmt->dst_freq % 11025) {
                return FALSE;
        }

        if (cfmt->to_channels != 1 &&
            cfmt->to_channels != 2) {
                return FALSE;
        }

        return TRUE;
}