/*
 * FILE:	statistics.c
 * PROGRAM:	RAT
 * AUTHOR(S):   O.Hodson, I.Kouvelas, C.Perkins, D.Miras, and V.J.Hardman
 *
 * $Revision$
 * $Date$
 *
 * Copyright (c) 1995-99 University College London
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, is permitted, for non-commercial use only, provided
 * that the following conditions are met:
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
 * Use of this software for commercial purposes is explicitly forbidden
 * unless prior written permission is obtained from the authors.
 *
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
#include "codec.h"
#include "debug.h"
#include "memory.h"
#include "util.h"
#include "session.h"
#include "source.h"
#include "timers.h"
#include "pckt_queue.h"
#include "rtcp_pckt.h"
#include "rtcp_db.h"
#include "audio.h"
#include "cushion.h"
#include "codec.h"
#include "ui.h"
#include "mbus.h"
#include "statistics.h"

static rtcp_dbentry *
update_database(session_struct *sp, u_int32 ssrc)
{
	rtcp_dbentry   *dbe_source;

	/* This function gets the relevant data base entry */
	dbe_source = rtcp_get_dbentry(sp, ssrc);
	if (dbe_source == NULL) {
		/* We haven't received an RTCP packet for this source,
		 * so we must throw the packets away. This seems a
		 * little extreme, but there are actually a couple of
		 * good reasons for it: 1) If we're receiving
		 * encrypted data, but we don't have the decryption
		 * key, then every RTP packet we receive will have a
		 * different SSRC and if we create a new database
		 * entry for it, we fill up the database with garbage
		 * (which is then displayed in the list of
		 * participants...)  2) The RTP specification says
		 * that we should do it this way (sec 6.2.1) [csp] 
                 */

		return NULL;
	}

	dbe_source->last_active = get_time(sp->device_clock);
	dbe_source->is_sender   = 1;

	sp->db->pckts_received++;

	return dbe_source;
}

static u_int32
adapt_playout(rtp_hdr_t *hdr, 
              u_int32 arrival_ts, 
              rtcp_dbentry *src,
	      session_struct *sp, 
              struct s_cushion_struct *cushion, 
	      u_int32 real_time)
{
	u_int32	playout, var;
        u_int32 minv, maxv; 

	int	delay, diff;
	codec_id_t            cid;
        const codec_format_t *cf;
	u_int32	ntptime, play_time;
	u_int32	sendtime = 0;
        int     ntp_delay = 0;
	u_int32	rtp_time;

        int64 since_last_sr;
    
	arrival_ts = convert_time(arrival_ts, sp->device_clock, src->clock);
	delay = arrival_ts - hdr->ts;

	if (src->first_pckt_flag == TRUE) {
		src->first_pckt_flag = FALSE;
		diff                 = 0;
		src->delay           = delay;
		src->last_ts         = hdr->ts - 1;
		hdr->m               = TRUE;
                cid = codec_get_by_payload(src->enc);
                if (cid) {
                        cf = codec_get_format(cid);
                        src->jitter  = 3 * 20 * cf->format.sample_rate / 1000;
                } else {
                        src->jitter  = 240; 
                }
	} else {
                if (hdr->seq != src->last_seq) {
                        diff       = abs(delay - src->delay);
                        /* Jitter calculation as in RTP spec */
                        src->jitter += (((double) diff - src->jitter) / 16.0);
                        src->delay = delay;
                }
	}

	/* 
	 * besides having the lip-sync option enabled, we also need to have
	 * a SR received [dm]
	 */	
	if (sp->sync_on && src->mapping_valid) {
		/* calculate delay in absolute (real) time [dm] */ 
		ntptime = (src->last_ntp_sec & 0xffff) << 16 | src->last_ntp_frac >> 16;
		if (hdr->ts > src->last_rtp_ts) {
			since_last_sr = hdr->ts - src->last_rtp_ts;	
		} else {
			since_last_sr = src->last_rtp_ts - hdr->ts;
		}
		since_last_sr = (since_last_sr << 16) / get_freq(src->clock);
		sendtime = (u_int32)(ntptime + since_last_sr); /* (since_last_sr << 16) / get_freq(src->clock); */

		ntp_delay = real_time - sendtime; 

		if (src->first_pckt_flag == TRUE) { 
			src->sync_playout_delay = ntp_delay;
		}
	}

	if (ts_gt(hdr->ts, src->last_ts)) {
		/* IF (a) TS start 
                   OR (b) we've thrown 4 consecutive packets away 
                   OR (c) ts have jumped by 8 packets worth 
                   OR (e) a new/empty playout buffer.
                   THEN adapt playout and communicate it
                   */
		if ((hdr->m) || 
                    src->cont_toged || 
                    ts_gt(hdr->ts, (src->last_ts + (hdr->seq - src->last_seq) * src->inter_pkt_gap * 8 + 1))) {
#ifdef DEBUG
                        if (hdr->m) {
                                debug_msg("New talkspurt\n");
                        } else if (src->cont_toged) {
                                debug_msg("Cont_toged\n");
                        } else {
                                debug_msg("Time stamp jump %ld %ld\n", hdr->ts, src->last_ts);
                        }
#endif
			var = (u_int32) src->jitter * 3;
                        
                        debug_msg("jitter %d\n", var / 3);

                        if (var < (unsigned)src->inter_pkt_gap) {
                                debug_msg("var (%d) < inter_pkt_gap (%d)\n", var, src->inter_pkt_gap);
                                var = src->inter_pkt_gap;
                        }

                        var = max(var, 3 * cushion_get_size(cushion) / 2);

                        debug_msg("Cushion %d\n", cushion_get_size(cushion));
                        minv = sp->min_playout * get_freq(src->clock) / 1000;
                        maxv = sp->max_playout * get_freq(src->clock) / 1000; 

                        assert(maxv > minv);
                        if (sp->limit_playout) {
                                var = max(minv, var);
                                var = min(maxv, var);
                        }

                        assert(var > 0);

                        if (src->delay_in_playout_calc != src->delay) {
                                debug_msg("Old delay (%u) new delay (%u) delta (%u)\n", src->delay_in_playout_calc, src->delay, max(src->delay_in_playout_calc, src->delay) - min(src->delay_in_playout_calc, src->delay));
                        }

                        src->delay_in_playout_calc = src->delay;
                        
                        if (source_get(sp->active_sources, src) != 0) {
                                /* If playout buffer is not empty
                                 * or, difference in time stamps is less than 1 sec,
                                 * we don't want playout point to be before that of existing data.
                                 */
                                debug_msg("Buf exists (%u) (%u)\n", src->playout, src->delay+var);
                                src->playout = max((unsigned)src->playout, src->delay + var);
                        } else {
                                debug_msg("delay (%lu) var (%lu)\n", src->delay, var);
                                src->playout = src->delay + var;
                                debug_msg("src playout %lu\n", src->playout);
                        }

			if (sp->sync_on && src->mapping_valid) {
				/* use the jitter value as calculated
                                 * but convert it to a ntp_ts freq
                                 * [dm] */
				src->sync_playout_delay = ntp_delay + ((var << 16) / get_freq(src->clock));
                                
				/* Communicate our playout delay to
                                   the video tool... */
                                ui_update_video_playout(sp, src->sentry->cname, src->sync_playout_delay);
		
				/* If the video tool is slower than
                                 * us, then adjust to match it...
                                 * src->video_playout is the delay of
                                 * the video in real time */
				debug_msg("ad=%d\tvd=%d\n", src->sync_playout_delay, src->video_playout);
                                if (src->video_playout_received == TRUE &&
                                    src->video_playout > src->sync_playout_delay) {
                                        src->sync_playout_delay = src->video_playout;
                                }
			}
                        src->skew_adjust = 0;
                } else {
                        /* No reason to adapt playout unless too
                         * little or too much audio buffered in
                         * comparison to to amount of audio in a
                         * packet. 
                        codec_id_t id;
                        u_int32 buffered, audio_per_pckt, adjust, samples_per_frame, cs;
                        static u_int32 last_adjust;

                        id = codec_get_by_payload(src->enc);
                        if (id) {
                                samples_per_frame = codec_get_samples_per_frame(id);
                        } else {
                                samples_per_frame = 160;
                        }

                        audio_per_pckt    = src->units_per_packet * samples_per_frame;

                        buffered = receive_buffer_duration_ms(sp->receive_buf_list, src) * 8;
                        cs = cushion_get_size(cushion);
                        if (ts_gt(get_time(sp->device_clock), last_adjust + 4000)) {
                                if (buffered < cs + audio_per_pckt) {
                                        adjust            = samples_per_frame * src->units_per_packet;
                                        src->playout     += adjust;
                                        src->skew_adjust += adjust;
                                        debug_msg("*** shifted %d +samples, previous time %08u\n", adjust, get_time(sp->device_clock) - last_adjust);
                                        last_adjust = get_time(sp->device_clock);
                                } else if (buffered > cs + 3 * audio_per_pckt) {
                                        adjust            = samples_per_frame / 2;
                                        src->playout     -= samples_per_frame;
                                        src->skew_adjust -= adjust;
                                        debug_msg("*** shifted %d -samples, previous time %08u\n", adjust, get_time(sp->device_clock) - last_adjust);
                                        last_adjust = get_time(sp->device_clock);
                                }
                                
                        }
                        */
                }
        }
        src->last_ts        = hdr->ts;
        src->last_seq       = hdr->seq;

	/* Calculate the playout point in local source time for this packet. */
        if (sp->sync_on && src->mapping_valid) {
		/* 	
		 * Use the NTP to RTP ts mapping to calculate the playout time 
		 * converted to the clock base of the receiver
		 */
		play_time = sendtime + src->sync_playout_delay;
		rtp_time = sp->db->map_rtp_time + (((play_time - sp->db->map_ntp_time) * get_freq(src->clock)) >> 16);
                playout = rtp_time;
		src->playout = playout - hdr->ts;
	} else {
		playout = hdr->ts + src->playout;
	}

        if (ts_gt(arrival_ts, playout)) {
                int now = get_time(sp->device_clock);
                now = convert_time(now, sp->device_clock, src->clock);
                debug_msg("Will be discarded - now (%u) arrival (%u) playout (%u) \n", now, arrival_ts, playout);
        }

        if (src->cont_toged > 12) {
                /* something has gone wrong if this assertion fails*/
                if (playout < get_time(src->clock)) {
                        debug_msg("playout before now.\n");
                        src->first_pckt_flag = TRUE;
                }
        }

	return playout;
}

static int
rtp_header_validation(rtp_hdr_t *hdr, int32 *len, int *extlen)
{
	/* This function checks the header info to make sure that the packet */
	/* is valid. We return TRUE if the packet is valid, FALSE otherwise. */
	/* This follows from page 52 of RFC1889.            [csp 22-10-1996] */

	/* We only accept RTPv2 packets... */
	if (hdr->type != 2) {
		debug_msg("rtp_header_validation: version != 2\n");
		return FALSE;
	}

	/* Check for valid audio payload types... */
	if (((hdr->pt > 23) && (hdr->pt < 96)) || (hdr->pt > 127)) {
		debug_msg("rtp_header_validation: payload-type out of audio range\n");
		return FALSE;
	}

	/* If padding or header-extension is set, we punt on this one... */
	/* We should really deal with it though...                       */
	if (hdr->p) {
                int pad = *((unsigned char *)hdr + *len - 1);
                if (pad < 1) {
                        debug_msg("rtp_header_validation: padding but 0 len\n");
                        return FALSE;
                }
                *len -= pad;
        }

        if (hdr->x) {
                *extlen = *((u_int32*)((unsigned char*)hdr + 4*(3+hdr->cc)))&0x0000ffff;
	} else {
                *extlen = 0;
        }

	return (TRUE);
}

static void
receiver_change_format(rtcp_dbentry *dbe, codec_id_t cid)
{
        const codec_format_t *cf = codec_get_format(cid);
        debug_msg("Changing Format. %d %d\n", dbe->enc, codec_get_payload(cid));
        dbe->first_pckt_flag = TRUE;
        dbe->enc             = codec_get_payload(cid);
	change_freq(dbe->clock, cf->format.sample_rate);
}  

/* Statistics _init and _end allocate good_pckt_queue which is used
 * as a temporary queue in rtp packet processing.
 */

static struct s_pckt_queue *good_pckt_queue;

void 
statistics_init()
{
        if (good_pckt_queue != NULL) {
                statistics_free();
        }
        good_pckt_queue = pckt_queue_create(PCKT_QUEUE_RTP_LEN);
}

void
statistics_free()
{
        pckt_queue_destroy(&good_pckt_queue);
}

void
statistics(session_struct          *sp,
	   struct s_pckt_queue     *rtp_pckt_queue,
	   struct s_cushion_struct *cushion,
	   u_int32	            real_time)
{
	/*
	 * We expect to take in an RTP packet, and decode it - read fields
	 * etc. This module should do statistics, and keep information on
	 * losses, and re-orderings. Duplicates will be dealt with in the
	 * receive buffer module.
	 * 
	 * Late packets will not be counted as lost. RTP stats reports count
	 * duplicates in with the number of packets correctly received, thus
	 * sometimes invalidating stats reports. We can, if necessary, keep
	 * track of some duplicates, and throw away in the receive module. It
	 * has not yet been decided whether or not loss will be 'indicated'
	 * to later modules - put a dummy unit(s) on the queue for the receive
	 * buffer
         * 
         * Added split between netrx_pckt_queue and good_pckt_queue.
         * Basically, packets on the netrx_pckt_queue are validated
         * and have their playout calculated before being placed on
         * good_pckt_queue.  If all of the packets on the
         * good_pckt_queue will fail playout because calculation is
         * wrong (probably because delay estimate or jitter has
         * suddenly changed) then we up their playout so that they get
         * played out, rather than wait for cont_toged count to go up.
         * This is only really necessary when processing packets at
         * rates other than 8kHz.  
         */

	rtp_hdr_t	*hdr;
	u_char		*data_ptr;
	int		 len;
	rtcp_dbentry	*sender = NULL;
	u_int32		 now, now_device, late_adjust;
	pckt_queue_element *pckt;
        struct s_source *src;
	codec_id_t       cid = 0;
	char 		 update_req = FALSE;
        int pkt_cnt = 0, late_cnt;

        now_device  = get_time(sp->device_clock);
        late_adjust = 0;
        late_cnt    = 0;

	/* Process incoming packets */
        while( (pckt = pckt_dequeue(rtp_pckt_queue)) != NULL ) {
                block_trash_check();
                /* Impose RTP formating on it... */
                hdr = (rtp_hdr_t *) (pckt->pckt_ptr);
        
                if (rtp_header_validation(hdr, &pckt->len, (int*)&pckt->extlen) == FALSE) {
                        debug_msg("RTP Packet failed header validation!\n");
                        block_trash_check();
                        goto release;
                }

                if (sp->playing_audio == FALSE) {
                        /* Don't decode audio if we are not playing it! */
                        goto release;
                }
        
                /* Convert from network byte-order */
                hdr->seq  = ntohs(hdr->seq);
                hdr->ts   = ntohl(hdr->ts);
                hdr->ssrc = ntohl(hdr->ssrc);
        
                if ((hdr->ssrc == sp->db->myssrc) && sp->filter_loopback) {
                        /* Discard loopback packets...unless we have asked for them ;-) */
                        block_trash_check();
                        goto release;
                }
        
                /* Get database entry of participant that sent this packet */
                sender = update_database(sp, hdr->ssrc);
                if (sender == NULL) {
                        debug_msg("Packet from unknown participant discarded\n");
                        goto release;
                }
                rtcp_update_seq(sender, hdr->seq);

                if (hdr->cc) {
                        int k;
                        for(k = 0; k < hdr->cc; k++) {
                                update_database(sp, ntohl(hdr->csrc[k]));
                        }
                }

                data_ptr =  (unsigned char *)pckt->pckt_ptr + 4 * (3 + hdr->cc) + pckt->extlen;
                len      = pckt->len - 4 * (3 + hdr->cc) - pckt->extlen;

                cid = channel_get_compatible_codec(hdr->pt, (u_char*)data_ptr, len);
                if (cid == 0) {
			debug_msg("Cannot decode data (pt %d).\n",hdr->pt);
			goto release;
                }

                sender->units_per_packet = channel_get_units_in_packet(hdr->pt, (u_char*)data_ptr, len);
        
                if ((sender->enc == 0xff) || (sender->enc != codec_get_payload(cid)))
                        receiver_change_format(sender, cid);
                
                if (sender->enc != codec_get_payload(cid)) {
                        /* we should tell update more about coded format */
                        debug_msg("sender enc %d pcp enc %d\n", sender->enc, codec_get_payload(cid));
                        sender->enc = codec_get_payload(cid);
                        update_req = TRUE;
                }

                pckt->sender     = sender;
                pckt->playout = adapt_playout(hdr, pckt->arrival_timestamp, sender, sp, cushion, real_time);

                /* Is this packet going to be played out late */
                now = convert_time(now_device, sp->device_clock, sender->clock);
                if (ts_gt(now, pckt->playout)) {
                        late_cnt ++;
                        late_adjust = max(late_adjust, ts_abs_diff(now, pckt->playout));
                }
                
                pckt_enqueue(good_pckt_queue, pckt);

                pkt_cnt++;
                continue;
        release:
                block_trash_check();
                pckt_queue_element_free(&pckt);
        }

        if (late_cnt) {
                /* Would we through away all the data packets in the queue ? */
                debug_msg("Late %d / %d\n", late_cnt, pkt_cnt);
                if (late_cnt * sender->units_per_packet > 4 || sender->units_per_packet == 0) {
                        /* this would fail the cont_toged test */
                        debug_msg("Would fail cont_toged test so adapting (%u samples)\n", late_adjust);
                        sender->playout += late_adjust;
                } else {
                        late_adjust  = 0;
                } 
                late_adjust = 0;
        }

        while( (pckt = pckt_dequeue(good_pckt_queue)) != NULL) {
                block_trash_check();

                if ((src = source_get(sp->active_sources, pckt->sender)) == NULL) {
                        src = source_create(sp->active_sources, pckt->sender);
                        assert(src != NULL);
                }

                hdr      = (rtp_hdr_t*)pckt->pckt_ptr;
                data_ptr = (unsigned char *)pckt->pckt_ptr + 4 * (3 + hdr->cc) + pckt->extlen;
                len      = pckt->len - 4 * (3 + hdr->cc) - pckt->extlen;

                if (source_add_packet(src, 
                                      pckt_ptr, 
                                      pckt->len, 
                                      data_ptr, 
                                      hdr->pt, 
                                      pckt->playout + late_adjust) == TRUE) {
                        /* Source is now repsonsible for packet so empty pointers */
                        pckt->pckt_ptr = NULL;
                        pckt->len      = 0;
                }

                block_trash_check();
                pckt_queue_element_free(&pckt);
        }

        if (pkt_cnt > 5) {
                debug_msg("Processed lots of packets(%d).\n", pkt_cnt);
        }
}

