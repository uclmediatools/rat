/*
 * FILE:      source.c
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

#include "ts.h"
#include "playout.h"

#include "new_channel.h"
#include "channel_types.h"
#include "codec_types.h"
#include "codec.h"
#include "codec_state.h"
#include "convert.h"
#include "render_3D.h"
#include "repair.h"
#include "timers.h"
#include "ts.h"
#include "source.h"
#include "mix.h"
#include "debug.h"
#include "util.h"

/* And we include all of the below just so we can get at
 * the render_3d_data field of the rtcp_dbentry for the source!
 */
#include "net_udp.h"
#include "rtcp.h"
#include "rtcp_pckt.h"
#include "rtcp_db.h"


#define HISTORY 1000

typedef struct s_source {
        struct s_source            *next;
        struct s_source            *prev;
        u_int32                     age;
        ts_t                        last_played;
        ts_t                        last_repair;
        u_int16                     consec_lost;
        ts_sequencer                seq;
        struct s_rtcp_dbentry      *dbe;
        struct s_channel_state     *channel_state;
        struct s_codec_state_store *codec_states;
        struct s_pb                *channel;
        struct s_pb                *media;
        struct s_pb_iterator       *media_pos;
        struct s_converter         *converter;
} source;

/* A linked list is used for sources and this is fine since we mostly
 * expect 1 or 2 sources to be simultaneously active and so efficiency
 * is not a killer.  */

typedef struct s_source_list {
        source  sentinel;
        u_int16 nsrcs;
} source_list;

int
source_list_create(source_list **pplist)
{
        source_list *plist = (source_list*)xmalloc(sizeof(source_list));
        if (plist != NULL) {
                *pplist = plist;
                plist->sentinel.next = &plist->sentinel;
                plist->sentinel.prev = &plist->sentinel;
                plist->nsrcs = 0;
                return TRUE;
        }
        return FALSE;
}

void
source_list_clear(source_list *plist)
{
       assert(plist != NULL);
        
       while(plist->sentinel.next != &plist->sentinel) {
               source_remove(plist, plist->sentinel.next);
       }
}

void
source_list_destroy(source_list **pplist)
{
        source_list *plist = *pplist;
        source_list_clear(plist);
        assert(plist->nsrcs == 0);
        xfree(plist);
        *pplist = NULL;
}

/* The following two functions are provided so that we can estimate
 * the sources that have gone in to mixer for transcoding.  
 */

u_int32
source_list_source_count(source_list *plist)
{
        return plist->nsrcs;
}

source*
source_list_get_source_no(source_list *plist, u_int32 n)
{
/* This obviously does not scale, but does not have to for audio! */
        source *curr;
        assert(plist != NULL);
        if (n < plist->nsrcs) {
                curr = plist->sentinel.next;
                while(n != 0) {
                        curr = curr->next;
                        n--;
                }
                return curr;
        }
        return NULL;
}

source*
source_get_by_rtcp_dbentry(source_list *plist, struct s_rtcp_dbentry *dbe)
{
        source *curr, *stop;
        assert(plist != NULL);
        assert(dbe   != NULL);
        
        curr = plist->sentinel.next; 
        stop = &plist->sentinel;
        while(curr != stop) {
                if (curr->dbe == dbe) return curr;
                curr = curr->next;
        }
 
        return NULL;
}

source*
source_create(source_list    *plist, 
              rtcp_dbentry   *dbe,
              converter_id_t  conv_id,
              int             render_3D_enabled,
              u_int16         out_rate,
              u_int16         out_channels)
{
        source *psrc;
        int     success;

        assert(plist != NULL);
        assert(dbe   != NULL);
        assert(source_get_by_rtcp_dbentry(plist, dbe) == NULL);

        psrc = (source*)block_alloc(sizeof(source));
        
        if (psrc == NULL) return NULL;

        memset(psrc, 0, sizeof(source));
        psrc->dbe            = dbe;
        psrc->dbe->first_mix = 1; /* Used to note we have not mixed anything
                                   * for this decode path yet */
        psrc->channel_state  = NULL;        

        /* Allocate channel and media buffers */
        success = pb_create(&psrc->channel, (playoutfreeproc)channel_data_destroy);
        if (!success) {
                debug_msg("Failed to allocate channel buffer\n");
                goto fail_create_channel;
        }

        success = pb_create(&psrc->media, (playoutfreeproc)media_data_destroy);
        if (!success) {
                debug_msg("Failed to allocate media buffer\n");
                goto fail_create_media;
        }

        success = pb_iterator_create(psrc->media, &psrc->media_pos);
        if (!success) {
                debug_msg("Failed to attach iterator to media buffer\n");
                goto fail_create_iterator;
        }

        success = codec_state_store_create(&psrc->codec_states, DECODER);
        if (!success) {
                debug_msg("Failed to allocate codec state storage\n");
                goto fail_create_states;
        }

        /* List maintenance    */
        psrc->next = plist->sentinel.next;
        psrc->prev = &plist->sentinel;
        psrc->next->prev = psrc;
        psrc->prev->next = psrc;
        plist->nsrcs++;

        /* Configure converter */
        source_reconfigure(psrc, 
                           conv_id, 
                           render_3D_enabled,
                           out_rate, 
                           out_channels);

        return psrc;

        /* Failure fall throughs */
fail_create_states:
        pb_iterator_destroy(psrc->media, &psrc->media_pos);        
fail_create_iterator:
        pb_destroy(&psrc->media);
fail_create_media:
        pb_destroy(&psrc->channel);
fail_create_channel:
        block_free(psrc, sizeof(source));

        return NULL;
}

/* All sources need to be reconfigured when anything changes in
 * audio path.  These include change of device frequency, change of
 * the number of channels, etc..
 */

void
source_reconfigure(source        *src,
                   converter_id_t conv_id,
                   int            render_3d,
                   u_int16        out_rate,
                   u_int16        out_channels)
{
        u_int16    src_rate, src_channels;
        codec_id_t            src_cid;
        const codec_format_t *src_cf;
        assert(src->dbe != NULL);

        /* Set age to zero and flush existing media
         * so that repair mechanism does not attempt
         * to patch across different block sizes.
         */

        src->age = 0;
        pb_flush(src->media);

        /* Get rate and channels of incoming media so we know
         * what we have to change.
         */
        src_cid = codec_get_by_payload(src->dbe->enc);
        src_cf  = codec_get_format(src_cid);
        src_rate     = (u_int16)src_cf->format.sample_rate;
        src_channels = (u_int16)src_cf->format.channels;

        if (render_3d) {
                assert(out_channels == 2);
                /* Rejig 3d renderer if there, else create */
                if (src->dbe->render_3D_data) {
                        int azi3d, fil3d, len3d;
                        render_3D_get_parameters(src->dbe->render_3D_data,
                                                 &azi3d,
                                                 &fil3d,
                                                 &len3d);
                        render_3D_set_parameters(src->dbe->render_3D_data,
                                                 (int)src_rate,
                                                 azi3d,
                                                 fil3d,
                                                 len3d);
                } else {
                        src->dbe->render_3D_data = render_3D_init((int)src_rate);
                }
                assert(src->dbe->render_3D_data);
        } else {
                /* Rendering is switched off so destroy info */
                if (src->dbe->render_3D_data != NULL) {
                        render_3D_free(&src->dbe->render_3D_data);
                }
        }

        /* Now destroy converter if it is already there */
        if (src->converter) {
                converter_destroy(&src->converter);
        }

        if (src_rate != out_rate || src_channels != out_channels) {
                converter_fmt_t c;

                c.from_freq     = src_rate;
                c.from_channels = src_channels;
                c.to_freq       = out_rate;
                c.to_channels   = out_channels;
                src->converter  = converter_create(conv_id, &c);
        }
}

void
source_remove(source_list *plist, source *psrc)
{
        assert(plist);
        assert(psrc);
        assert(source_get_by_rtcp_dbentry(plist, psrc->dbe) != NULL);

        psrc->next->prev = psrc->prev;
        psrc->prev->next = psrc->next;

        if (psrc->channel_state) channel_decoder_destroy(&psrc->channel_state);

        pb_iterator_destroy(psrc->media, &psrc->media_pos);
        pb_destroy(&psrc->channel);
        pb_destroy(&psrc->media);
        codec_state_store_destroy(&psrc->codec_states);
        plist->nsrcs--;

        /* This is hook into the playout_adapt, we are signalling
         * there is no source decode path.
         */
        psrc->dbe->first_pckt_flag = TRUE;
        
        block_free(psrc, sizeof(source));
}
              
/* Source Processing Routines ************************************************/

/* Returns true if fn takes ownership responsibility for data */
int
source_add_packet (source *src, 
                   u_char *pckt, 
                   u_int32 pckt_len, 
                   u_int32 data_start,
                   u_int8  payload,
                   ts_t    playout)
{
        channel_data *cd;
        channel_unit *cu;
        cc_id_t       cid;

        assert(src != NULL);
        assert(pckt != NULL);
        assert(data_start != 0);

        /* If last_played is valid then enough audio is buffer
         * for the playout check to be sensible
         */
        if (ts_valid(src->last_played) &&
            ts_gt(src->last_played, playout)) {
                debug_msg("Packet late (%u > %u)- discarding\n", 
                          src->last_played.ticks,
                          playout.ticks);
                /* Up src->dbe jitter toged */
                return FALSE;
        }

        if (channel_data_create(&cd, 1) == 0) {
                return FALSE;
        }
        
        cu               = cd->elem[0];
        cu->data         = pckt;
        cu->data_start   = data_start;
        cu->data_len     = pckt_len;
        cu->pt           = payload;

        /* Check we have state to decode this */
        cid = channel_coder_get_by_payload(cu->pt);
        if (src->channel_state && 
            channel_decoder_matches(cid, src->channel_state) == FALSE) {
                debug_msg("Channel coder changed - flushing\n");
                channel_decoder_destroy(&src->channel_state);
                pb_flush(src->channel);
        }

        /* Make state if not there and create decoder */
        if (src->channel_state == NULL && 
            channel_decoder_create(cid, &src->channel_state) == FALSE) {
                debug_msg("Cannot decode payload %d\n", cu->pt);
                channel_data_destroy(&cd, sizeof(channel_data));
        }

        if (pb_add(src->channel, (u_char*)cd, sizeof(channel_data), playout) == FALSE) {
                debug_msg("Packet addition failed - duplicate ?\n");
                src->dbe->duplicates++;
                channel_data_destroy(&cd, sizeof(channel_data));
        }

        return TRUE;
}

static void
source_repair(source *src,
              int     repair_type,
              ts_t    step)
{
        media_data* fill_md, *prev_md;
        ts_t        fill_ts,  prev_ts;
        u_int32     success,  prev_len;

        /* Check for need to reset of consec_lost count */

        if (ts_valid(src->last_repair) == FALSE || 
            ts_eq(src->last_played, src->last_repair) == FALSE) {
                src->consec_lost = 0;
        }

        debug_msg("Repair %d\n", src->consec_lost);

        /* We repair one unit at a time since it may be all we need */
        pb_iterator_retreat(src->media_pos);
        pb_iterator_get_at(src->media_pos,
                           (u_char**)&prev_md,
                           &prev_len,
                           &prev_ts);
        assert(prev_md != NULL);
        assert(ts_eq(prev_ts, src->last_played));

        media_data_create(&fill_md, 1);
        repair(repair_type,
               src->consec_lost,
               src->codec_states,
               prev_md,
               fill_md->rep[0]);
        fill_ts = ts_add(src->last_played, step);
        success = pb_add(src->media, 
                         (u_char*)fill_md,
                         sizeof(media_data),
                         fill_ts);
        if (success) {
                src->consec_lost ++;
                src->last_repair = fill_ts;
                pb_iterator_advance(src->media_pos);

#ifndef NDEBUG
        /* Reusing prev_* - bad style */
        pb_iterator_get_at(src->media_pos,
                           (u_char**)&prev_md,
                           &prev_len,
                           &prev_ts);
        assert(ts_eq(prev_ts, fill_ts));
#endif
        } else {
                /* This should only ever fail at when source changes
                 * sample rate in less time than playout buffer
                 * timeout.  This should be a very very rare event...  
                 */
                debug_msg("Repair add data failed.\n");
                media_data_destroy(&fill_md, sizeof(media_data));
                src->consec_lost = 0;
        }
}

int
source_process(source *src, struct s_mix_info *ms, int render_3d, int repair_type, ts_t now)
{
        media_data  *md;
        coded_unit  *cu;
        codec_state *cs;
        u_int32     md_len, src_freq;
        ts_t        playout, step, cutoff;
        int         i, success;

        /* Split channel coder units up into media units */
        channel_decoder_decode(src->channel_state,
                               src->channel,
                               src->media,
                               now);

        src_freq = get_freq(src->dbe->clock);
        step = ts_map32(src_freq,src->dbe->inter_pkt_gap / src->dbe->units_per_packet);

        while (pb_iterator_advance(src->media_pos)) {
                pb_iterator_get_at(src->media_pos, 
                                  (u_char**)&md, 
                                  &md_len, 
                                  &playout);
                assert(md != NULL);
                assert(md_len == sizeof(media_data));

                /* Conditions for repair: 
                 * (a) last_played has meaning. 
                 * (b) playout point does not what we expect.
                 * (c) repair type is not no repair.
                 * (d) last decoded was not too long ago.
                 */
                cutoff = ts_sub(now, ts_map32(src_freq, HISTORY));
                if (ts_valid(src->last_played) && 
                    ts_eq(playout, ts_add(src->last_played, step)) == FALSE &&
                    repair_type != REPAIR_TYPE_NONE &&
                    ts_gt(src->last_played, cutoff)) {
                        /* If repair was successful media_pos is moved,
                         * so get data at media_pos again.
                         */
                        source_repair(src, repair_type, step);
                        success = pb_iterator_get_at(src->media_pos, 
                                                     (u_char**)&md, 
                                                     &md_len, 
                                                     &playout);
                        assert(success);
                }

                if (ts_gt(playout, now)) {
                        /* This playout point is after now so stop */
                        pb_iterator_retreat(src->media_pos);
                        break;
                }

                if (codec_is_native_coding(md->rep[0]->id) == FALSE) {
                        /* There is data to be decoded.  There may not be
                         * when we have used repair.
                         */
#ifdef DEBUG
                        for(i = 0; i < md->nrep; i++) {
                                /* if there is a native coding this
                                 * unit has already been decoded and
                                 * this would be bug */
                                assert(md->rep[i] != NULL);
                                assert(codec_id_is_valid(md->rep[i]->id));
                                assert(codec_is_native_coding(md->rep[i]->id) == FALSE);
                        }
#endif /* DEBUG */
                        cu = (coded_unit*)block_alloc(sizeof(coded_unit));
                        /* Decode frame */
                        assert(cu != NULL);
                        memset(cu, 0, sizeof(coded_unit));
                        cs = codec_state_store_get(src->codec_states, md->rep[0]->id);
                        codec_decode(cs, md->rep[0], cu);
                        xmemchk();
                        md->rep[md->nrep] = cu;
                        md->nrep++;
                }

                if (render_3d && src->dbe->render_3D_data) {
                        /* 3d rendering necessary */
                        coded_unit *decoded, *render;
                        decoded = md->rep[md->nrep - 1];
                        assert(codec_is_native_coding(decoded->id));
                        
                        render = (coded_unit*)block_alloc(sizeof(coded_unit));
                        memset(render, 0, sizeof(coded_unit));
                        
                        render_3D(src->dbe->render_3D_data,decoded,render);
                        xmemchk();
                        md->rep[md->nrep] = render;
                        md->nrep++;
                }

                if (src->converter) {
                        /* convert frame */
                        coded_unit *decoded, *render;
                        decoded = md->rep[md->nrep - 1];
                        assert(codec_is_native_coding(decoded->id));

                        render = (coded_unit*)block_alloc(sizeof(coded_unit));
                        memset(render, 0, sizeof(coded_unit));
                        converter_process(src->converter,
                                          decoded,
                                          render);
                        xmemchk();
                        md->rep[md->nrep] = render;
                        md->nrep++;
                }

                if (mix_process(ms, src->dbe, md->rep[md->nrep - 1], playout) == FALSE) {
                        /* Sources sampling rate changed mid-flow?,
                         * dump data, make source look irrelevant, it
                         * should get destroyed and the recreated with
                         * proper decode path when new data arrives.
                         * Not graceful..  A better way would be just
                         * to flush media then invoke source_reconfigure 
                         * if this is ever really an issue.
                         */
                        pb_flush(src->media);
                        pb_flush(src->channel);
                }

                src->last_played = playout;
        }

        src->age++;

        UNUSED(i); /* Except for debugging */
        
        return TRUE;
}

int
source_audit(source *src) {
        if (src->age != 0) {
                ts_t history;
                /* Keep 1/2 seconds worth of audio */
                history =  ts_map32(8000,4000);
                pb_iterator_audit(src->media_pos,history);
                return TRUE;
        }
        return FALSE;
}


ts_sequencer*
source_get_sequencer(source *src)
{
        return &src->seq;
}

ts_t
source_get_audio_buffered (source *src)
{
        ts_t start, end;

        /* Total audio buffered is start of media buffer
         * to end of channel buffer.
         */

        if (pb_get_start_ts(src->media, &start) &&
            pb_get_end_ts(src->channel, &end)) {
                assert(ts_gt(end, start));
                return ts_sub(end, start);
        }

        return ts_map32(8000,0);
}

ts_t
source_get_playout_delay (source *src)
{
        ts_t start, end;

        /* Current playout is pretty close to src->media_pos point,
         * delay is diff between this and last packet received.
         */

        if (pb_get_start_ts(src->channel,  &start) &&
            pb_get_end_ts(src->channel, &end)) {
                assert(ts_gt(end, start));
                return ts_sub(end, start);
        }

        return ts_map32(8000,0);
}

int
source_relevant(source *src, ts_t now)
{
        assert(src);
        
        if (pb_relevant(src->media, now) ||
            pb_relevant(src->channel, now)) {
                return TRUE;
        }
        return FALSE;
}

struct s_pb*
source_get_decoded_buffer(source *src)
{
        return src->media;
}

struct s_rtcp_dbentry*
source_get_rtcp_dbentry(source *src)
{
        return src->dbe;
}
