/*
 * FILE:      cc_rdncy.c
 * AUTHOR(S): Orion Hodson 
 *	
 * $Revision$
 * $Date$
 * 
 * Copyright (c) 1995-99 University College London
 * All rights reserved.
 *
 */

#include "config_unix.h"
#include "config_win32.h"

#include "codec_types.h"
#include "codec.h"
#include "channel_types.h"
#include "playout.h"
#include "cc_rdncy.h"

#include "memory.h"
#include "util.h"
#include "debug.h"

#include "timers.h"

/* This is pap.  Have run out of time.  Apologies if you ever end
 * up here... [oh] 
 */

#define RED_MAX_LAYERS 3
#define RED_MAX_OFFSET 16383u
#define RED_MAX_LEN    1023u

#define RED_PRIMARY 1
#define RED_EXTRA   2

#define RED_HDR32_INIT(x)      (x)  = 0x80000000
#define RED_HDR32_SET_PT(x,y)  (x) |= ((y)<<24)
#define RED_HDR32_SET_OFF(x,y) (x) |= ((y)<<10)
#define RED_HDR32_SET_LEN(x,y) (x) |= (y)
#define RED_HDR32_GET_PT(z)    (((z) >> 24) & 0x7f)
#define RED_HDR32_GET_OFF(z)   (((z) >> 10) & 0x3fff)
#define RED_HDR32_GET_LEN(z)   ((z) & 0x3ff)

#define RED_HDR8_INIT(x)       (x) = 0
#define RED_HDR8_SET_PT(x,y)   (x) = (y)
#define RED_HDR8_GET_PT(z)     (z)

typedef struct s_red_layer {
        codec_id_t          cid;
        u_int32             pkts_off;
} red_layer;

typedef struct {
        red_layer            layer[RED_MAX_LAYERS];
        u_int32              n_layers;
        struct s_pb          *media_buffer;
        struct s_pb_iterator *media_pos;
        u_int32               units_ready;
} red_enc_state;

int
redundancy_encoder_create(u_char **state, u_int32 *len)
{
        red_enc_state *re = (red_enc_state*)xmalloc(sizeof(red_enc_state));

        if (re == NULL) {
                goto fail_alloc;
        }
        memset(re, 0, sizeof(red_enc_state));

        *state = (u_char*)re;
        *len   = sizeof(red_enc_state);

        if (pb_create(&re->media_buffer,
                      (playoutfreeproc)media_data_destroy) == FALSE) {
                goto fail_pb;
        }

        if (pb_iterator_create(re->media_buffer, &re->media_pos) == FALSE) {
                debug_msg("failed to create iterator\n");
                goto fail_pb_iterator;
        }

        re->n_layers    = 0;        
        re->units_ready = 0;

        return TRUE;

fail_pb_iterator:
        pb_destroy(&re->media_buffer);

fail_pb:
        xfree(re);

fail_alloc:
        *state = NULL;
        return FALSE;
}

void
redundancy_encoder_destroy(u_char **state, u_int32 len)
{
        red_enc_state *re = *((red_enc_state**)state);
        
        assert(len == sizeof(red_enc_state));

        pb_iterator_destroy(re->media_buffer,
                            &re->media_pos);

        pb_destroy(&re->media_buffer);

        xfree(*state);
        *state = NULL;
}

int
redundancy_encoder_reset(u_char *state)
{
        red_enc_state *re = (red_enc_state*)state;
        
        pb_flush(re->media_buffer);
        re->units_ready = 0;

        return TRUE;
}

/* Adds header to next free slot in channel_data */
static void
add_hdr(channel_data *cd, int hdr_type, codec_id_t cid, u_int32 uo, u_int32 len)
{
        channel_unit *chu;
        u_int32 so;             /* sample time offset */
        u_char  pt;

        pt = codec_get_payload(cid);
        assert(payload_is_valid(cid));

        so = codec_get_samples_per_frame(cid) * uo;

        assert(so <= RED_MAX_OFFSET);
        assert(len <= RED_MAX_LEN );

        chu = (channel_unit*)block_alloc(sizeof(channel_unit));
        cd->elem[cd->nelem++] = chu;
        
        if (hdr_type == RED_EXTRA) {
                u_int32 *h;
                h = (u_int32*)block_alloc(4);
                RED_HDR32_INIT(*h);
                RED_HDR32_SET_PT(*h, (u_int32)pt);
                RED_HDR32_SET_OFF(*h, so);
                RED_HDR32_SET_LEN(*h, len);
                *h = htonl(*h);
                chu->data     = (u_char*)h;
                chu->data_len = 4;
        } else {
                u_char *h;
                assert(hdr_type == RED_PRIMARY);
                h = (u_char*)block_alloc(1);
                RED_HDR8_INIT(*h);
                RED_HDR8_SET_PT(*h, pt);
                chu->data     = h;
                chu->data_len = 1;
        }
}

static u_int32
make_pdu(struct s_pb          *pb, 
         struct s_pb_iterator *pbi, 
         codec_id_t            cid, 
         channel_data         *cd, 
         u_int32               upp)
{
        struct s_pb_iterator *p;
        u_int32        units, got, md_len, r;
        channel_unit  *chu;
        media_data    *md;
        coded_unit    *cdu_target[MAX_UNITS_PER_PACKET];
        u_char        *dp;
        ts_t           playout;

        pb_iterator_dup(&p, pbi);

        chu = (channel_unit*)block_alloc(sizeof(channel_unit));
        chu->data_start = 0;
        chu->data_len   = 0;

        chu->pt = codec_get_payload(cid);
        assert(payload_is_valid(chu->pt));

        /* First work out how much space this is going to need and
         * to save time cache units wanted in cdu_target.
         */

        for(units = 0; units < upp; units++) {        
                pb_iterator_get_at(p, (u_char**)&md, &md_len, &playout);
                if (md == NULL) break;
                assert(md_len == sizeof(media_data));
                for(r = 0; r < md->nrep; r++) {
                        if (md->rep[r]->id == cid) {
                                cdu_target[r] = md->rep[r];
                                chu->data_len += md->rep[r]->data_len;
                                if (units == 0) {
                                        chu->data_len += md->rep[r]->state_len;
                                }
                        }
                }
                pb_iterator_advance(p);
        }

        if (chu->data_len == 0) {
                /* Nothing to do, chu not needed */
                block_free(chu, sizeof(channel_unit));
                return 0;
        }

        chu->data = (u_char*)block_alloc(chu->data_len);
        dp = chu->data;
        for(r = 0; r < units; r++) {
                if (r == 0 && cdu_target[r]->state_len) {
                        memcpy(dp, cdu_target[r]->state, cdu_target[r]->state_len);
                        dp += cdu_target[r]->state_len;
                }
                memcpy(dp, cdu_target[r]->data, cdu_target[r]->data_len);
                dp += cdu_target[r]->data_len;
        }

        assert((u_int32)(dp - chu->data) == chu->data_len);
        assert(cd->nelem < MAX_CHANNEL_UNITS);

        cd->elem[cd->nelem] = chu;
        cd->nelem++;

        pb_iterator_destroy(pb, &p);
        return got;
}


/* redundancy_check_layers - checks codings specified for redundant
 * encoder are present in incomding media_data unit, m.  And only
 * those are present.
 */

__inline static void
redundancy_check_layers(red_enc_state *re, media_data *m)
{
        u_int8     i;
        u_int32    lidx;
        int        found, used[MAX_MEDIA_UNITS];
        assert(re);
        assert(m);

        for(i = 0; i < m->nrep; i++) {
                used[i] = 0;
        }
        
        for(lidx = 0; lidx < re->n_layers ; lidx++) {
                found = FALSE;
                for(i = 0; i < m->nrep; i++) {
                        assert(m->rep[i] != NULL);
                        if (m->rep[i]->id == re->layer[lidx].cid) {
                                found = TRUE;
                                used[i]++;
                                break;
                        }
                }
                assert(found == TRUE); /* Coding for layer not found */
        }
}

static channel_data *
redundancy_encoder_output(red_enc_state *re, u_int32 upp)
{
        struct s_pb_iterator *pbm;
        channel_data *cd_part, *cd_out;
        u_int32       i, units, lidx;

        pbm = re->media_pos;
        pb_iterator_ffwd(pbm);

        /* Rewind iterator to start of first pdu */ 
        for(i = 1; i < upp; i++) {
                pb_iterator_retreat(pbm);
        }

        channel_data_create(&cd_part, 0);

        for(lidx = 0; lidx < re->n_layers; lidx++) {
                for(i = 0; i < re->layer[lidx].pkts_off * upp; i++) {
                        pb_iterator_retreat(pbm);
                }
                units = make_pdu(re->media_buffer, pbm, re->layer[lidx].cid, cd_part, upp);
                if (units == 0) break;
                assert(units == upp);
        }

        assert(lidx != 0);

        /* cd_part contains pdu's for layer 0, layer 1, layer 2... in
         * ascending order Now fill cd_out with pdu's with headers in
         * protocol order layer n, n-1, .. 1.  Note we must transfer
         * pointers from cd_part to cd_out and make pointers in
         * cd_part null because channel_data_destroy will free the
         * channel units referred to by cd_part. 
         */
        
        channel_data_create(&cd_out, 0);

        if (lidx != re->n_layers) {
                /* Put maximum offset info if not present in the packet */
                add_hdr(cd_out, RED_EXTRA, 
                        re->layer[re->n_layers - 1].cid, 
                        re->layer[re->n_layers - 1].pkts_off * upp,
                        0);
        }

        /* Slot in redundant payloads and their headers */
        while(lidx != 0) {
                add_hdr(cd_out, RED_EXTRA,
                        re->layer[lidx].cid,
                        re->layer[lidx].pkts_off * upp,
                        cd_part->elem[lidx]->data_len);
                cd_out->elem[cd_out->nelem] = cd_part->elem[lidx];
                cd_out->nelem ++;
                cd_part->elem[lidx] = NULL;
        }

        /* Now the primary and it's header */
        add_hdr(cd_out, RED_PRIMARY,
                re->layer[lidx].cid,
                re->layer[lidx].pkts_off * upp,
                cd_part->elem[lidx]->data_len);
        cd_out->elem[cd_out->nelem] = cd_part->elem[lidx];
        cd_out->nelem++;
        cd_part->elem[lidx] = NULL;

#ifndef NDEBUG
        for(i = 0; i < cd_part->nelem; i++) {
                assert(cd_part->elem[i] == NULL);
        }
#endif
        cd_part->nelem = 0;
        channel_data_destroy(&cd_part, sizeof(channel_data));

        return cd_out;
}

int
redundancy_encoder_encode (u_char      *state,
                           struct s_pb *in,
                           struct s_pb *out,
                           u_int32      upp)
{
        u_int32        m_len;
        ts_t           playout;
        struct s_pb_iterator *pi;
        media_data     *m;
        red_enc_state  *re = (red_enc_state*)state;

        assert(upp != 0 && upp <= MAX_UNITS_PER_PACKET);

        pb_iterator_create(in, &pi);
        assert(pi != NULL);

        while(pb_iterator_advance(pi)) {
                /* Remove element from playout buffer - it belongs to
                 * the redundancy encoder now.  */
                pb_iterator_detach_at(pi, (u_char**)&m, &m_len, &playout);
                assert(m != NULL);

#ifndef NDEBUG
                redundancy_check_layers(re, m);
#endif /* NDEBUG */

                pb_add(re->media_buffer, 
                       (u_char*)m,
                       m_len,
                       playout);
                re->units_ready++;

                if ((re->units_ready % upp) == 0) {
                        channel_data *cd;
                        int s;
                        cd = redundancy_encoder_output(re, upp);
                        assert(cd != NULL);
                        s  = pb_add(out, (u_char*)cd, sizeof(channel_data), playout);
                        assert(s);
                }
        }
        pb_iterator_destroy(in, &pi);

        return TRUE;
}

/* Redundancy {get,set} parameters expects strings like:
 *            dvi-8k-mono/0/lpc-8k-mono/2
 * where number is the offset in number of units.
 */ 

int
redundancy_encoder_set_parameters(u_char *state, char *cmd)
{
        red_enc_state *n, *cur;
        u_int32 nl, po;
        codec_id_t  cid;
        char *s;
        int success = FALSE;

        assert(state != NULL);
        assert(cmd   != NULL);

        /* Create a temporary encoder, try to set it's params */
        redundancy_encoder_create((u_char**)&n, &nl);
        assert(n != NULL);

        s = strtok(cmd, "/");
        cid = codec_get_by_name(s);
        if (!codec_id_is_valid(cid)) {
                debug_msg("codec not recognized\n");
                goto done;
        }

        s = strtok(NULL, "/");
        po = atoi(s);

        if (po > 20) {
                debug_msg("offset too big\n");
                goto done;
        }
        
        n->layer[0].cid       = cid;
        n->layer[0].pkts_off  = po;
        n->n_layers           = 1;

        while (n->n_layers < RED_MAX_LAYERS) {
                s = strtok(NULL, "/");
                if (s == NULL) break;
                cid = codec_get_by_name(s);
                if (!codec_id_is_valid(cid)) {
                        debug_msg("codec not recognized\n");
                        goto done;
                }

                s = strtok(NULL, "/");
                if (s == NULL) {
                        debug_msg("Incomplete layer info\n");
                        goto done;
                }
                po = atoi(s);
                if (po > 20) {
                        debug_msg("offset too big\n");
                        goto done;
                }
        
                n->layer[n->n_layers].cid      = cid;
                n->layer[n->n_layers].pkts_off = po;
                n->n_layers ++;
        }


        redundancy_encoder_reset(state);
        /* Take bits from temporary encoder state we want */
        cur = (red_enc_state*)state;
        memcpy(cur->layer, n->layer, sizeof(red_layer)*RED_MAX_LAYERS);
        cur->n_layers = n->n_layers;
        success = TRUE;

done:
        redundancy_encoder_destroy((u_char**)&n, nl);
        return success;
}

int 
redundancy_encoder_get_parameters(u_char *state, char *buf, u_int32 blen)
{
        const codec_format_t *cf;
        red_enc_state *r;
        u_int32 i, used, flen;

        char frag[CODEC_LONG_NAME_LEN+5]; /* XXX/nn/\0 + 1*/

        assert(blen > 0);
        assert(buf != NULL);

        r = (red_enc_state*)state;
        if (r->n_layers < 2) {
                debug_msg("Redundancy encoder has not had parameters set!\n");
                return FALSE;
        }
        
        *buf = '\0';
	flen = 0;

        for(i = 0, used = 0; i < r->n_layers; i++) {
                cf = codec_get_format(r->layer[i].cid);
                assert(cf != NULL);
                sprintf(frag,
                        "%s/%ld/",
                        cf->long_name,
                        r->layer[i].pkts_off);
                flen += strlen(frag);
                if (used+flen > blen) {
                        debug_msg("buffer overflow would have occured.\n");
                        *buf = '\0';
                        return FALSE;
                }
                strcat(buf + used, frag);
                used += flen;
        }
        buf[used - 1] = '\0';
        debug_msg("red parameters: %s\n", buf);
        return TRUE;
}

int
redundancy_decoder_peek(u_int8   pkt_pt,
                        u_char  *buf,
                        u_int32  len,
                        u_int16  *upp,
                        u_int8   *pt)
{
        const codec_format_t *cf;
        codec_id_t            cid;
        u_char               *p;
        u_int32              hdr32, plen, blen, units;
        
        assert(buf != NULL);
        assert(upp != NULL);
        assert(pt  != NULL);

        /* Just check primary */
        p   = buf;
        hdr32 = ntohl(*(u_int32*)p);
        while (hdr32 & 0x8000000) {
                p += RED_HDR32_GET_LEN(hdr32);
                p += 4; /* hdr */
                hdr32 = ntohl(*(u_int32*)p);
                if (((u_int32)(p - buf) > len)) {
                        debug_msg("Buffer overflow\n");
                        return FALSE;
                }
        }

        *pt = *p;
        p  += 1; /* Step over header */

        cid = codec_get_by_payload(*pt);
        
        if (!cid) {
                debug_msg("Codec not found\n");
                return FALSE;
        }

        /* Primary data length */
        plen = len - (u_int32)(p - buf);

        cf = codec_get_format(cid);
        assert(cf);

        p    += cf->mean_per_packet_state_size;
        plen -= cf->mean_per_packet_state_size;

        units = 0;        
        while (plen != 0) {
                blen = codec_peek_frame_size(cid, p, plen);
                assert(blen != 0);
                p    += blen;
                plen -= blen;
                units ++;
                assert(*upp < 50);
        }

        *upp = units;
        UNUSED(pkt_pt);

        return TRUE;
}

/* redundancy_decoder_describe - produces a text string describing the
 * format from a packet of data.  We go through layers one at a time,
 * shift the text from previous layers on, and insert text for current
 * layer.  Not a very attractive method.
 */

int
redundancy_decoder_describe (u_int8   pkt_pt,
                             u_char  *data,
                             u_int32  data_len,
                             char    *out,
                             u_int32  out_len)
{
        const codec_format_t *cf;
        codec_id_t            cid;
        u_int32 hdr32, slen, blksz, off, nlen;
        u_char  *p, pt;

        UNUSED(pkt_pt);
        
        *out = '\0';
        slen = 1;

        p   = data;
        hdr32 = ntohl(*((u_int32*)p));
        while (hdr32 & 0x80000000) {
                pt    = (u_char)RED_HDR32_GET_PT(hdr32);
                off   = RED_HDR32_GET_OFF(hdr32);
                blksz = RED_HDR32_GET_LEN(hdr32);
                cid   = codec_get_by_payload(pt);

                if (cid == 0) {
                        p += 4 + blksz;
                        hdr32 = ntohl(*((u_int32*)p));
                        continue;
                }

                cf = codec_get_format(cid);
                assert(cf != NULL);
                
                nlen = strlen(cf->long_name);

                if (slen + nlen >=  out_len) {
                        debug_msg("Out of buffer space\n");
                        return FALSE;
                }
                memmove(out, out + nlen + 1, slen);
                strncpy(out, cf->long_name, nlen);
                out[slen - 1] = '/';
                slen += nlen + 1;
                p += 4 + blksz;
                assert((u_int32)(p - data) < data_len);
                hdr32 = ntohl(*((u_int32*)p));
        }

        pt  = *p;
        cid = codec_get_by_payload(pt);
        if (cid == 0) {
                return FALSE;
        }

        cf = codec_get_format(cid);
        assert(cf != NULL);
                
        nlen = strlen(cf->long_name);
        
        if (slen + nlen >=  out_len) {
                debug_msg("Out of buffer space\n");
                return FALSE;
        }
        memmove(out, out + nlen + 1, slen);
        strncpy(out, cf->long_name, nlen);
        out[nlen - 1] = '/';
        slen += nlen + 1;

        /* Axe trailing separator */
        out[slen-1] = '\0';

        return TRUE;        
}

static u_char 
red_get_primary_pt(u_char *p)
{
        u_int32 hdr32;
        hdr32 = ntohl(*(u_int32*)p);
        while(hdr32 & 0x80000000) {
                p += 4 + RED_HDR32_GET_LEN(ntohl(hdr32));
                hdr32 = ntohl(*(u_int32*)p);
        }
        return *p;
}

__inline static void
place_unit(media_data *md, coded_unit *cu, int pos)
{
        assert(pos <= md->nrep);

        if (md->rep) {
                memmove(md->rep + pos, 
                       md->rep + pos + 1, 
                       sizeof(coded_unit) * (md->nrep - pos));
        }
        md->rep[pos] = cu;
        md->nrep ++;
        assert(md->nrep <= MAX_MEDIA_UNITS);
}

static media_data *
red_media_data_create_or_get(struct s_pb *p, ts_t playout)
{
        struct s_pb_iterator *pi;
        media_data *md;
        u_int32     md_len, success;
        ts_t        md_playout;

        pb_iterator_create(p, &pi);
        pb_iterator_advance(pi);

        /* This would probably be quicker starting at back and coming
         * forward. */

        while(pb_iterator_get_at(pi, (u_char**)&md, &md_len, &md_playout)) {
                assert(md != NULL);
                assert(md_len == sizeof(media_data));
                if (ts_eq(md_playout, playout)) {
                        goto done;
                } else if (ts_gt(md_playout,playout)) {
                        break;
                }
                pb_iterator_advance(pi); 
        }

        /* Not found in playout buffer */
        media_data_create(&md, 0);
        success = pb_add(p, (u_char*)md, sizeof(media_data), playout);
        assert(success);

done:
        pb_iterator_destroy(p, &pi);
        return md;
}

__inline static void
red_split_unit(u_char  ppt,        /* Primary payload type */
               u_char  bpt,        /* Block payload type   */
               u_char *b,          /* Block pointer        */
               u_int32 blen,       /* Block len            */
               ts_t    playout,    /* Block playout time   */
               struct s_pb *out)   /* media buffer         */
{
        const codec_format_t *cf;
        media_data *md;
        codec_id_t  cid, pid;
        coded_unit *cu;
        u_char     *p,*pe;
        ts_t        step;

        pid = codec_get_payload(pid);
        if (!pid) {
                debug_msg("Payload not recognized\n");
                return;
        }

        cid = codec_get_payload(bpt);
        if (!cid) {
                debug_msg("Payload not recognized\n");
                return;
        }
        
        if (!codec_audio_formats_compatible(pid, cid)) {
                debug_msg("Primary (%d) and redundant (%d) not compatible\n", ppt, bpt);
                return;
        }

        cf = codec_get_format(cid);
        assert(cf != NULL);
        step = ts_map32(cf->format.sample_rate, codec_get_samples_per_frame(cid));
        
        p  = b;
        pe = b + blen;
        while(p < pe) {
                cu = (coded_unit*)block_alloc(sizeof(coded_unit));
                cu->id = cid;
                if (p == b && cf->mean_per_packet_state_size) {
                        cu->state     = p;
                        cu->state_len = cf->mean_per_packet_state_size;
                        p            += cu->state_len;
                } else {
                        cu->state     = NULL;
                        cu->state_len = 0;
                }
                cu->data     = p;
                cu->data_len = codec_peek_frame_size(cid, p, (u_int32)(pe - p));
                p += cu->data_len;

                md = red_media_data_create_or_get(out, playout);

                if (md->nrep == MAX_MEDIA_UNITS) continue;

                if (bpt == ppt || md->nrep == 0) {
                        /* This is primary, or primary not present already */
                        place_unit(md, cu, 0);
                } else {
                        /* This secondary and goes second since primary present */
                        place_unit(md, cu, 1);
                }
                
                playout = ts_add(playout, step);
        }
}

__inline static void
redundancy_decoder_output(channel_unit *chu, struct s_pb *out, ts_t playout)
{
        const codec_format_t *cf;
        codec_id_t cid;
        u_char  *p, *pe, bpt, ppt;
        u_int32 hdr32, blen, boff;
        ts_t ts_mo, ts_bo, this_playout;

        p  = chu->data + chu->data_start;
        pe = chu->data + chu->data_len;

        /* Max offset should be in first header.  Want max offset
         * as we nobble timestamps to be:
         *              playout + max_offset - this_offset 
         */

        hdr32 = ntohl(*(u_int32*)p);
        assert(hdr32 & 0x80000000);

        ppt   = red_get_primary_pt(p);
        cid   = codec_get_by_payload(ppt);
        if (!cid) {
                debug_msg("Primary not recognized. Bailing out.\n");
                return;
        }

        cf = codec_get_format(cid);
        assert(cf != NULL);

        ts_mo = ts_map32(cf->format.sample_rate, RED_HDR32_GET_OFF(hdr32));
	blen = 0;

        while (hdr32 & 0x80000000) {
                boff  = RED_HDR32_GET_OFF(hdr32);
                blen  = RED_HDR32_GET_LEN(hdr32);
                bpt   = (u_char)RED_HDR32_GET_PT(hdr32);

                /* Calculate playout point = playout + max_offset - offset */
                ts_bo = ts_map32(cf->format.sample_rate, boff);
                this_playout = ts_add(playout, ts_mo);
                this_playout = ts_sub(this_playout, ts_bo);

                p += 4; /* hdr */
                red_split_unit(ppt, bpt, p, blen, playout, out);
                p += blen;
                hdr32 = ntohl(*(u_int32*)p);
        }
        
        this_playout = ts_add(playout, ts_mo);
        p += 1;
        red_split_unit(ppt, ppt, p, blen, playout, out);
}
                          
int
redundancy_decoder_decode(u_char      *state,
                           struct s_pb *in,
                           struct s_pb *out,
                           ts_t         now)
{
        struct s_pb_iterator *pi;
        channel_data         *c;
        u_int32               clen;
        ts_t                  cplayout;

        pb_iterator_create(in, &pi);
        assert(pi != NULL);

        assert(state == NULL); /* No decoder state necesssary */
        UNUSED(state);

        while(pb_iterator_get_at(pi, (u_char**)&c, &clen, &cplayout)) {
                assert(c != NULL);
                assert(clen == sizeof(channel_data));
                if (ts_gt(cplayout, now)) {
                        break;
                }
                pb_iterator_detach_at(pi, (u_char**)&c, &clen, &cplayout);
                assert(c->nelem == 1);
                redundancy_decoder_output(c->elem[0], out, cplayout);
                channel_data_destroy(&c, sizeof(channel_data));
        }
        
        pb_iterator_destroy(in, &pi);
        return TRUE;
}
        
