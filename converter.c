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
#include "memory.h"
#include "util.h"
#include "converter_types.h"
#include "converter.h"
#include "convert_util.h"
#include "debug.h"

typedef struct s_converter {
        int                     idx;
        struct s_converter_fmt *cfmt;
        u_char                 *state;
        u_int32                 state_len;
} converter_t;

typedef int  (*cv_startup)     (void);  /* converter specific one time initialization */
typedef void (*cv_shutdown)    (void);  /* converter specific one time cleanup */
typedef int  (*cv_conv_init_f) (const converter_fmt_t *c, u_char **state, u_int32 *state_len);
typedef void (*cv_conv_do_f)   (const converter_fmt_t *c, u_char *state, 
                                sample* src_buf, int src_len, 
                                sample *dst_buf, int dst_len);
typedef void (*cv_conv_free_f) (u_char **state, u_int32 *state_len);

typedef struct s_pcm_converter{
        char          *name;
        u_char         enabled;
        cv_startup     startf;
        cv_shutdown    shutdownf;
        cv_conv_init_f initf;
        cv_conv_do_f   convertf;
        cv_conv_free_f freef;
} pcm_converter_t;

/* In this table of converters the platform specific converters should go at the
 * beginning, before the default (and worst) linear interpolation conversion.  The
 * intension is to have a mechanism which enables/disables more complex default schemes
 * such as interpolation with filtering, cubic interpolation, etc...
 */

#include "convert_acm.h"
#include "convert_extra.h"
#include "convert_linear.h"

pcm_converter_t converter_tbl[] = {
#ifdef WIN32
        {
         "Microsoft Converter", 
         FALSE, 
         acm_cv_startup, 
         acm_cv_shutdown, 
         acm_cv_create, 
         acm_cv_convert,  
         acm_cv_destroy 
        },
#endif
#ifdef SRF_GOOD
        {
         "High Quality",
         TRUE,
         srf_tbl_init,
         srf_tbl_free,
         srf_init,
         srf_convert,
         srf_free,
         2 * sizeof(srf_state_t)
        },
#endif /* SRF_GOOD */
        {
         "Intermediate Quality",
         TRUE,  
         NULL,
         NULL,
         linear_create,
         linear_convert,
         linear_destroy
        },
        {
         "Low Quality",
         TRUE,
         NULL,
         NULL,
         extra_create,
         extra_convert,
         extra_destroy
        },
        { /* This must be last converter */
         "None",
         TRUE,
         NULL,
         NULL,
         NULL,
         NULL,
         NULL
        }
};

#define NUM_CONVERTERS sizeof(converter_tbl)/sizeof(pcm_converter_t)
#define CONVERTER_NONE (NUM_CONVERTERS - 1)

/* Index to converter_id_t mapping macros */
#define CONVERTER_ID_TO_IDX(x) (((x)>>2) - 17)
#define IDX_TO_CONVERTER_ID(x) ((x+17) << 2)

int 
converter_create(const converter_id_t   cid, 
                 const converter_fmt_t *cfmt,
                 converter_t          **cvtr)
{
        converter_t *c  = NULL;
        u_int32      tbl_idx;
        
        tbl_idx = CONVERTER_ID_TO_IDX(cid);

        if (tbl_idx >= NUM_CONVERTERS) {
                debug_msg("Converter ID invalid\n");
                return FALSE;
        }

        if (tbl_idx == CONVERTER_NONE) {
                return FALSE;
        }

        if (cfmt == NULL) {
                debug_msg("No format specified\n");
                return FALSE;
        }
        
        c  = (converter_t*)xmalloc(sizeof(converter_t));
        if (c == NULL) {
                debug_msg("Could not allocate converter\n");
                return FALSE;
        }

        memset(c, 0, sizeof(converter_t));

        /* Copy format */
        c->cfmt = (converter_fmt_t*)xmalloc(sizeof(converter_fmt_t));
        if (c->cfmt == NULL) {
                converter_destroy(&c); 
                return FALSE;
        }
        memcpy(c->cfmt, cfmt, sizeof(converter_fmt_t));
        c->idx = tbl_idx;

        /* Initialize */
        if ((converter_tbl[tbl_idx].initf) && 
            (converter_tbl[tbl_idx].initf(cfmt, &c->state, &c->state_len) == FALSE)) {
                converter_destroy(&c);
                return FALSE;
        }
        *cvtr = c;
        xmemchk();
        return TRUE;
}

void 
converter_destroy(converter_t **cvtr)
{
        converter_t *c = *cvtr;

        if (c == NULL) return;

        if (converter_tbl[c->idx].freef) {
                converter_tbl[c->idx].freef(&c->state, &c->state_len);
        }

        if (c->cfmt) {
                xfree(c->cfmt);
        }

        xfree(c); 
        (*cvtr) = NULL;
}

void         
converters_init()
{
        u_int32 i = 0;

        for(i = 0; i < NUM_CONVERTERS; i++) {
                if (converter_tbl[i].startf) {
                        converter_tbl[i].enabled = converter_tbl[i].startf();
                }
        }
}

void
converters_free()
{
        u_int32 i = 0;

        for(i = 0; i < NUM_CONVERTERS; i++) {
                if (converter_tbl[i].shutdownf) {
                        converter_tbl[i].shutdownf();
                }
        }
}

int
converter_get_details(u_int32 idx, converter_details_t *cd)
{
        if (idx < NUM_CONVERTERS && cd != NULL) {
                cd->id   = IDX_TO_CONVERTER_ID(idx);
                cd->name = converter_tbl[idx].name;
                return TRUE;
        }
        debug_msg("Getting invalid converter details\n");
        return FALSE;
}

u_int32 
converter_get_count()
{
        return NUM_CONVERTERS;
}

__inline converter_id_t
converter_get_null_converter()
{
        return IDX_TO_CONVERTER_ID(CONVERTER_NONE);
}

#include "codec_types.h"
#include "codec.h"

int
converter_process (converter_t *c, coded_unit *in, coded_unit *out)
{
        converter_fmt_t *cf;
        u_int32          n_in, n_out;

#ifdef DEBUG
        {
                u_int16 sample_rate, channels;
                codec_get_native_info(in->id, &sample_rate, &channels);
                assert(sample_rate == c->cfmt->from_freq);
                assert(channels == c->cfmt->from_channels);
        }
#endif /* DEBUG */

        assert(c);
        assert(in->data != NULL);
        assert(in->data_len != 0);

        cf = c->cfmt;

        n_in  = in->data_len / sizeof(sample);
        n_out = n_in * cf->to_channels * cf->to_freq / (cf->from_channels * cf->from_freq); 

        assert(converter_format_valid(cf));
        assert(out->state     == NULL);
        assert(out->state_len == 0);
        assert(out->data      == NULL);
        assert(out->data_len  == 0);

        out->id       = codec_get_native_coding(cf->to_freq, cf->to_channels);
        out->data_len = sizeof(sample) * n_out;
        out->data     = (u_char*)block_alloc(out->data_len);

        converter_tbl[c->idx].convertf(c->cfmt,
                                       c->state,
                                       (sample*)in->data, 
                                       n_in,
                                       (sample*)out->data, 
                                       n_out);

        xmemchk();
        return TRUE;
}

const converter_fmt_t*
converter_get_format (converter_t *c)
{
        assert(c != NULL);
        return c->cfmt;
}





