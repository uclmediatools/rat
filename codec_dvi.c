/*
 * FILE:    codec_dvi.c
 * AUTHORS: Orion Hodson
 * 
 * Copyright (c) 1998 University College London
 * All rights reserved.
 *
 */

#include "config_unix.h"
#include "config_win32.h"
#include "memory.h"
#include "util.h"
#include "debug.h"
#include "audio_types.h"
#include "codec_types.h"
#include "codec_dvi.h"
#include "cx_dvi.h"

static codec_format_t cs[] = {
        {"DVI", "DVI-8K-Mono",  
         "IMA ADPCM codec. (c) 1992 Stichting Mathematisch Centrum, Amsterdam, Netherlands.", 
         5,                     4, 80, {DEV_S16,  8000, 16, 1, 160 * BYTES_PER_SAMPLE}}, /* 20  ms */
        {"DVI", "DVI-16K-Mono", 
         "IMA ADPCM codec. (c) 1992 Stichting Mathematisch Centrum, Amsterdam, Netherlands.",
         6,                     4, 80, {DEV_S16, 16000, 16, 1, 160 * BYTES_PER_SAMPLE}}, /* 10  ms */
        {"DVI", "DVI-32K-Mono", 
         "IMA ADPCM codec. (c) 1992 Stichting Mathematisch Centrum, Amsterdam, Netherlands.",
         CODEC_PAYLOAD_DYNAMIC, 4, 80, {DEV_S16, 32000, 16, 1, 160 * BYTES_PER_SAMPLE}}, /* 5   ms */
        {"DVI", "DVI-48K-Mono", 
         "IMA ADPCM codec. (c) 1992 Stichting Mathematisch Centrum, Amsterdam, Netherlands.",
         CODEC_PAYLOAD_DYNAMIC, 4, 80, {DEV_S16, 48000, 16, 1, 160 * BYTES_PER_SAMPLE}}  /* 3.3 ms */
};

#define DVI_NUM_FORMATS sizeof(cs)/sizeof(codec_format_t)

u_int16
dvi_get_formats_count()
{
        return (u_int16)DVI_NUM_FORMATS;
}

const codec_format_t *
dvi_get_format(u_int16 idx)
{
        assert(idx < DVI_NUM_FORMATS);
        return &cs[idx];
}

int 
dvi_state_create(u_int16 idx, u_char **s)
{
        struct adpcm_state *as;
        int                 sz;

        if (idx < DVI_NUM_FORMATS) {
                sz = sizeof(struct adpcm_state);
                as = (struct adpcm_state*)xmalloc(sz);
                if (as) {
                        memset(as, 0, sz);
                        *s = (u_char*)as;
                        return sz;
                }
        }
        return 0;
}

void
dvi_state_destroy(u_int16 idx, u_char **s)
{
        UNUSED(idx);
        assert(idx < DVI_NUM_FORMATS);
        xfree(*s);
        *s = (u_char*)NULL;
}

int
dvi_encode(u_int16 idx, u_char *encoder_state, sample *inbuf, coded_unit *c)
{
        int samples;

        assert(encoder_state);
        assert(inbuf);
        assert(idx < DVI_NUM_FORMATS);
        UNUSED(idx);
        
        /* Transfer state and fix ordering */
        c->state     = (u_char*)block_alloc(sizeof(struct adpcm_state));
        c->state_len = sizeof(struct adpcm_state);
        memcpy(c->state, encoder_state, sizeof(struct adpcm_state));

        /* Fix coded state for byte ordering */
	((struct adpcm_state*)c->state)->valprev = htons(((struct adpcm_state*)c->state)->valprev);
        
        samples = cs[idx].format.bytes_per_block * 8 / cs[idx].format.bits_per_sample;
        c->data     = (u_char*)block_alloc(samples / sizeof(sample)); /* 4 bits per sample */
        c->data_len = samples / sizeof(sample);
        adpcm_coder(inbuf, c->data, samples, (struct adpcm_state*)encoder_state);
        return samples / 2;
}


int
dvi_decode(u_int16 idx, u_char *decoder_state, coded_unit *c, sample *data)
{
        int samples; 
        assert(decoder_state);
        assert(c);
        assert(data);
        assert(idx < DVI_NUM_FORMATS);

	if (c->state_len > 0) {
		assert(c->state_len == sizeof(struct adpcm_state));
		memcpy(decoder_state, c->state, sizeof(struct adpcm_state));
		((struct adpcm_state*)decoder_state)->valprev = ntohs(((struct adpcm_state*)decoder_state)->valprev);
	}

        samples = cs[idx].format.bytes_per_block / sizeof(sample);
	adpcm_decoder(c->data, data, samples, (struct adpcm_state*)decoder_state);

        return samples;
}
