/*
 * FILE:      codec_types.c
 * AUTHOR(S): Orion Hodson 
 *	
 * $Revision$
 * $Date$
 * 
 * Copyright (c) 1999 University College London
 * All rights reserved.
 *
 */
#include "config_unix.h"
#include "config_win32.h"

#include "codec_types.h"

#include "util.h"
#include "debug.h"

int  
media_data_create(media_data **ppmd, int nrep)
{
        media_data *pmd;
        int i;

        *ppmd = NULL;
        pmd   = (media_data*)block_alloc(sizeof(media_data));
        
        if (pmd) {
                memset(pmd, 0, sizeof(media_data));
                for(i = 0; i < nrep; i++) {
                        pmd->rep[i] = block_alloc(sizeof(coded_unit));
                        if (pmd->rep[i] == NULL) {
                                pmd->nrep = i;
                                media_data_destroy(&pmd, sizeof(media_data));
                                return FALSE;
                        }
                        memset(pmd->rep[i], 0, sizeof(coded_unit));
                }
                pmd->nrep    = nrep;
                *ppmd = pmd;
                return TRUE;
        }
        return FALSE;
}

void 
media_data_destroy(media_data **ppmd, u_int32_t md_size)
{
        media_data *pmd;
        coded_unit *pcu;
        int         i;
        
        pmd = *ppmd;

        assert(pmd != NULL);
        assert(md_size == sizeof(media_data));

        for(i = 0; i < pmd->nrep; i++) {
                pcu = pmd->rep[i];
                if (pcu->state) {
                        block_free(pcu->state, pcu->state_len);
                        pcu->state     = 0;
                        pcu->state_len = 0;
                }
                assert(pcu->state_len == 0);
                if (pcu->data) {
                        block_free(pcu->data, pcu->data_len);
                        pcu->data     = 0;
                        pcu->data_len = 0;
                }
                assert(pcu->data_len == 0);
                block_free(pcu, sizeof(coded_unit));
        }
#ifdef DEBUG_MEM
        for(i = pmd->nrep; i < MAX_MEDIA_UNITS; i++) {
                if (pmd->rep[i] != NULL) {
                        assert(pmd->rep[i]->state == NULL);
                        assert(pmd->rep[i]->data  == NULL);
                }
        }
#endif /* DEBUG_MEM */

        block_free(pmd, sizeof(media_data));
        *ppmd = NULL;
}

int
coded_unit_dup(coded_unit *dst, coded_unit *src)
{
        assert(dst != NULL);
        assert(src != NULL);
        assert(src->data_len != 0);

        dst->data     = (u_char*)block_alloc(src->data_len);
        dst->data_len = src->data_len;

        memcpy(dst->data, src->data, src->data_len);

        if (src->state_len != 0) {
                dst->state     = (u_char*)block_alloc(src->state_len);
                dst->state_len = src->state_len;
                memcpy(dst->state, src->state, src->state_len);
        } else {
                dst->state     = NULL;
                dst->state_len = 0;
        }

        dst->id = src->id;

        return TRUE;
}


void
coded_unit_layer_split(coded_unit *in, coded_unit *out, u_int8_t layer, u_int8_t *layer_markers)
{
        u_int16_t tmp_datalen;
        u_int8_t i;

        tmp_datalen = (u_int16_t)(layer_markers[layer] - layer_markers[layer-1]);

        out->data = (u_char*)block_alloc(tmp_datalen);
        out->data_len = tmp_datalen;

        for(i=layer_markers[layer-1];i<layer_markers[layer];i++) {
                out->data[i-layer_markers[layer-1]] = in->data[i];
        }
        
        if (in->state_len != 0) {
                out->state     = (u_char*)block_alloc(in->state_len);
                out->state_len = in->state_len;
                memcpy(out->state, in->state, in->state_len);
        } else {
                in->state     = NULL;
                in->state_len = 0;
        }

        out->id = in->id;
}
