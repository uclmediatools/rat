/*
 * FILE:      new_channel.c
 * AUTHOR(S): Orion Hodson 
 *	
 * $Revision$
 * $Date$
 * 
 * Copyright (c) 1999 University College London
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, is permitted provided that the following conditions 
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the Computer Science
 *      Department at University College London
 * 4. Neither the name of the University nor of the Department may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "config_unix.h"
#include "config_win32.h"

#include "codec_types.h"
#include "new_channel.h"
#include "playout.h"

#include "memory.h"

typedef struct s_channel_state {
        u_int16 coder;              /* Index of coder in coder table      */
        u_char *state;              /* Pointer to state relevent to coder */
        u_int32 state_len;          /* The size of that state             */
        u_int8  units_per_packet:7; /* The number of units per packet     */
        u_int8  is_encoder:1;       /* For debugging                      */
} channel_state_t;

typedef struct {
        char    name[CC_NAME_LENGTH];

        int     (*enc_create_state)   (u_char                **state,
                                       u_int32                *len);

        void    (*enc_destroy_state)  (u_char                **state, 
                                       u_int32                 len);

        int     (*enc_set_parameters) (u_char                 *state, 
                                       char                   *cmd);

        int     (*enc_get_parameters) (u_char                 *state, 
                                       char                   *cmd, 
                                       u_int32                 cmd_len);

        int     (*enc_reset)          (u_char                  *state);

        int     (*enc_encode)         (u_char                  *state, 
                                       struct s_playout_buffer *in, 
                                       struct s_playout_buffer *out, 
                                       u_int32                  units_per_packet);

        int     (*dec_create_state)   (u_char                 **state,
                                       u_int32                 *len);

        void    (*dec_destroy_state)  (u_char                 **state, 
                                       u_int32                  len);

        int     (*dec_reset)          (u_char                  *state);
        int     (*dec_decode)         (u_char                  *state, 
                                       struct s_playout_buffer *in, 
                                       struct s_playout_buffer *out,
                                       u_int32                  now);
        int     (*dec_peek)           (u_int8                   ccpt,
                                       u_char                  *data,
                                       u_int32                  len,
                                       u_int16                 *upp,
                                       u_int8                  *pt);
} channel_coder_t;

#include "cc_vanilla.h"

static channel_coder_t table[] = {
        /* The vanilla coder goes first. Update channel_get_null_coder 
         * if it moves.
         */
        {"No channel coder",     
         vanilla_encoder_create, 
         vanilla_encoder_destroy,
         NULL,                   /* No parameters to set ...*/
         NULL,                   /* ... or get. */
         vanilla_encoder_reset,
         vanilla_encoder_encode,
         NULL,
         NULL,
         NULL,
         vanilla_decoder_decode,
         vanilla_decoder_peek}
};

#define CC_IDX_TO_ID(x) (((x)+1) | 0x0e00)
#define CC_ID_TO_IDX(x) (((x)-1) & 0x000f)

#define CC_NUM_CODERS (sizeof(table)/sizeof(channel_coder_t))

int
channel_get_coder_count()
{
        return (int)CC_NUM_CODERS;
}

int
channel_get_coder_details(int idx, cc_details *ccd)
{
        if (idx >=  0 && 
            idx < channel_get_coder_count()) {
                ccd->descriptor = CC_IDX_TO_ID(idx);
                strcpy(ccd->name, table[idx].name);
                return TRUE;
        }
        return FALSE;
}

int
channel_get_null_coder(void)
{
        return 0;
}

/* The create, destroy, and reset functions take the same arguments and so use
 * is_encoder to determine which function in the table to call.  It's dirty
 * but it saves typing.
 */

int
_channel_coder_create(cc_id_t id, channel_state_t **ppcs, int is_encoder)
{
        channel_state_t *pcs;
        int (*create_state)(u_char**, u_int32 *len);

        pcs = (channel_state_t*)xmalloc(sizeof(channel_state_t));
        
        if (pcs == NULL) {
                return FALSE;
        }

        *ppcs = pcs;

        pcs->coder = CC_ID_TO_IDX(id);
        assert(pcs->coder < CC_NUM_CODERS);

        pcs->units_per_packet = 2;
        pcs->is_encoder       = is_encoder;

        if (is_encoder) {
                create_state = table[pcs->coder].enc_create_state;
        } else {
                create_state = table[pcs->coder].dec_create_state;
        }

        if (create_state) {
                create_state(&pcs->state, &pcs->state_len);
        } else {
                pcs->state     = NULL;
                pcs->state_len = 0;
        }

        return TRUE;
}

void
_channel_coder_destroy(channel_state_t **ppcs, int is_encoder)
{
        channel_state_t *pcs = *ppcs;

        void (*destroy_state)(u_char**, u_int32);

        assert(is_encoder == pcs->is_encoder);

        if (is_encoder) {
                destroy_state = table[pcs->coder].enc_destroy_state;
        } else {
                destroy_state = table[pcs->coder].dec_destroy_state;
        }

        if (destroy_state) {
                destroy_state(&pcs->state, pcs->state_len);
                pcs->state_len = 0;
        }

        assert(pcs->state     == NULL);
        assert(pcs->state_len == 0);

        xfree(pcs);
        *ppcs = NULL;
}

int
_channel_coder_reset(channel_state_t *pcs, int is_encoder)
{
        int (*reset) (u_char *state);
        
        assert(is_encoder == pcs->is_encoder);

        if (is_encoder) {
                reset = table[pcs->coder].enc_reset; 
        } else {
                reset = table[pcs->coder].dec_reset; 
        }
        
        return (reset != NULL) ? reset(pcs->state) : TRUE;
}

/* Encoder specifics */

int
channel_encoder_set_units_per_packet(channel_state_t *cs, u_int16 units)
{
        /* This should not be hardcoded, it should be based on packet 
         *size [oth] 
         */
        assert(cs->is_encoder);
        if (units != 0 && units <= MAX_UNITS_PER_PACKET) {
                cs->units_per_packet = units;
                return TRUE;
        }
        return FALSE;
}

u_int16 
channel_encoder_get_units_per_packet(channel_state_t *cs)
{
        assert(cs->is_encoder);
        return cs->units_per_packet;
}

int
channel_encoder_set_parameters(channel_state_t *cs, char *cmd)
{
        if (table[cs->coder].enc_set_parameters) {
                return table[cs->coder].enc_set_parameters(cs->state, cmd);
        }
        return TRUE;
}

int
channel_encoder_get_parameters(channel_state_t *cs, char *cmd, int cmd_len)
{
        if (table[cs->coder].enc_get_parameters) {
                return table[cs->coder].enc_get_parameters(cs->state, cmd, cmd_len);
        }
        return TRUE;
}

int
channel_encoder_encode(channel_state_t         *cs, 
                       struct s_playout_buffer *media_buffer, 
                       struct s_playout_buffer *channel_buffer)
{
        assert(table[cs->coder].enc_encode != NULL);
        return table[cs->coder].enc_encode(cs->state, media_buffer, channel_buffer, cs->units_per_packet);
}

int
channel_decoder_decode(channel_state_t         *cs, 
                       struct s_playout_buffer *media_buffer, 
                       struct s_playout_buffer *channel_buffer,
                       u_int32                  now)
{
        assert(table[cs->coder].dec_decode != NULL);
        return table[cs->coder].dec_decode(cs->state, media_buffer, channel_buffer, now);
}

int
channel_decoder_matches(cc_id_t          id,
                        channel_state_t *cs)
{
        u_int16 coder = CC_ID_TO_IDX(id);
        return (coder == cs->coder);
}

int
channel_verify_and_stat(cc_id_t  cid,
                        u_int8   pktpt,
                        u_char  *data,
                        u_int32  data_len,
                        u_int16 *units_per_packet,
                        u_char  *codec_pt)
{
        u_int16 idx = CC_ID_TO_IDX(cid);
        assert(idx < CC_NUM_CODERS);
        return table[idx].dec_peek(pktpt, data, data_len, units_per_packet, codec_pt);
}