/*
 * FILE:     audio.c
 * PROGRAM:  RAT
 * AUTHOR:   Orion Hodson / Isidor Kouvelas / Colin Perkins 
 *
 * $Revision$
 * $Date$
 *
 * Copyright (c) 1995-98 University College London
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
#include "memory.h"
#include "debug.h"
#include "audio.h"
#include "audio_fmt.h"
#include "audio_util.h"
#include "session.h"
#include "transcoder.h"
#include "ui.h"
#include "transmit.h"
#include "codec_types.h"
#include "codec.h"
#include "mix.h"
#include "cushion.h"
#include "source.h"
#include "timers.h"
#include "sndfile.h"

/* Zero buf used for writing zero chunks during cushion adaption */
static sample* audio_zero_buf;

int
audio_device_write(session_struct *sp, sample *buf, int dur)
{
        codec_id_t            id;
        const codec_format_t *cf;
        const audio_format *ofmt = audio_get_ofmt(sp->audio_device);

        if (sp->out_file) {
                snd_write_audio(&sp->out_file, buf, (u_int16)(dur * ofmt->channels));
        }
        if (sp->mode == TRANSCODER) {
                return transcoder_write(sp->audio_device, buf, dur * ofmt->channels);
        } else {
                return audio_write(sp->audio_device, buf, dur * ofmt->channels);
        }
        
        id = codec_get_by_payload((u_char)sp->encodings[0]);
        assert(id);
        cf = codec_get_format(id);
        return dur * cf->format.channels;
}

int
audio_device_take(session_struct *sp, audio_desc_t device)
{
	codec_id_t	       id;
        const codec_format_t  *cf;
	audio_format           format;
        u_int32                unit_len;
        int success, failures;

        if (audio_device_is_open(device)) {
                audio_device_is_open(device);
                debug_msg("new device %04x state %d, %04x state %d\n", 
                          device, audio_device_is_open(device),
                          sp->audio_device, audio_device_is_open(sp->audio_device));
                assert(sp->audio_device == device);
                return TRUE;
        }

	id = codec_get_by_payload((u_char)sp->encodings[0]);
        cf = codec_get_format(id);

        memcpy(&format, &cf->format, sizeof(audio_format));

        failures = 0;

        success  = audio_open(device, &format, &format);
        
        if (!success) {
                /* Maybe we cannot support this format. Try minimal
                 * case - ulaw 8k.
                 */
                audio_format fallback;
                
                failures ++;
                id = codec_get_by_name("PCMU-8K-MONO");
                assert(id); /* in case someone changes codec name */
                
                sp->encodings[0] = codec_get_payload(id);
                cf = codec_get_format(id);
                memcpy(&fallback, &cf->format, sizeof(audio_format));
                
                success = audio_open(device, &fallback, &fallback);
                
                if (!success) {
                        /* Make format consistent just in case. */
                        memcpy(&format, &fallback, sizeof(audio_format));
                        debug_msg("Could use requested format, but could use mulaw 8k\n");
                        failures++;
                }
        }
        
        if (!success) {
                /* Both original request and fallback request to
                 * device have failed.  This probably means device is in use.
                 * So now use null device with original format requested.
                 */
                failures ++;
                device  = audio_get_null_device();
                success = audio_open(device, &format, &format);
                debug_msg("Using null audio device\n");
                assert(success);
        }
        
        audio_drain(device);
	
        if (sp->input_mode!=AUDIO_NO_DEVICE) {
                audio_set_iport(device, sp->input_mode);
                audio_set_gain(device, sp->input_gain);
        } else {
                sp->input_mode=audio_get_iport(device);
                sp->input_gain=audio_get_gain(device);
        }
        if (sp->output_mode!=AUDIO_NO_DEVICE) {
                audio_set_oport(device, sp->output_mode);
                audio_set_volume(device, sp->output_gain);
        } else {
                sp->output_mode=audio_get_oport(device);
                sp->output_gain=audio_get_volume(device);
        }
        
        if (audio_zero_buf == NULL) {
                audio_zero_buf = (sample*) xmalloc (format.bytes_per_block * sizeof(sample));
                audio_zero(audio_zero_buf, format.bytes_per_block, DEV_S16);
        }

        if (sp->mode != TRANSCODER) {
                audio_non_block(device);
        }

        /* We initialize the pieces above the audio device here since their parameters
         * depend on what is set here
         */
        sp->audio_device = device;
        if (sp->device_clock) xfree(sp->device_clock);
        sp->device_clock = new_time(sp->clock, format.sample_rate);
        sp->meter_period = format.sample_rate / 15;
        sp->bc           = bias_ctl_create(format.channels, format.sample_rate);
        unit_len         = format.bytes_per_block * 8 / (format.bits_per_sample*format.channels); 
        tx_create(&sp->tb, sp, sp->device_clock, (u_int16)unit_len, (u_int16)format.channels);
        assert(sp->tb != NULL);
        sp->ms           = mix_create(sp, 32640);
        cushion_create(&sp->cushion, unit_len);
        tx_igain_update(sp->tb);
        ui_update(sp);
        return (failures == 0);
}

void
audio_device_give(session_struct *sp)
{
	gettimeofday(&sp->device_time, NULL);

        tx_stop(sp->tb);
        if (sp->mode == TRANSCODER) {
                transcoder_close(sp->audio_device);
        } else {
                sp->input_mode = audio_get_iport(sp->audio_device);
                sp->output_mode = audio_get_oport(sp->audio_device);
                audio_close(sp->audio_device);
        }
        
        if (audio_zero_buf) {
                xfree(audio_zero_buf);
                audio_zero_buf = NULL;
        }
        
        bias_ctl_destroy(sp->bc);
        sp->bc = NULL;

        cushion_destroy(sp->cushion);
        mix_destroy(sp->ms);
        tx_destroy(&sp->tb);
        source_list_clear(sp->active_sources);
}

void
audio_device_reconfigure(session_struct *sp)
{
        if (sp->next_selected_device != -1 &&
            sp->next_selected_device != sp->audio_device) {
                        audio_device_give(sp);
                        audio_device_take(sp, sp->next_selected_device);

        } 
        sp->next_selected_device = -1;
        
        if (sp->next_encoding != -1) {
                codec_id_t  curr_id, next_id;
                curr_id = codec_get_by_payload(sp->encodings[0]);
                next_id = codec_get_by_payload((u_char)sp->next_encoding);

                if (codec_audio_formats_compatible(curr_id, next_id)) {
                        /* Formats compatible device needs no reconfig */
                        sp->encodings[0]  = sp->next_encoding;
                        sp->next_encoding = -1;
                        return;
                } else {
                        u_char      oldpt;
                        /* Changing encoding */
                        oldpt            = sp->encodings[0];
                        sp->encodings[0] = sp->next_encoding;
                        if (audio_device_take(sp, sp->audio_device) == FALSE) {
                                /* we failed, fallback */
                                sp->encodings[0] = oldpt;
                        }
                        sp->next_encoding = -1;
                }
        } else {
                /* Just changing device */
        }
}

/* This function needs to be modified to return some indication of how well
 * or not we are doing.                                                    
 */
int
read_write_audio(session_struct *spi, session_struct *spo,  struct s_mix_info *ms)
{
        u_int32 cushion_size, read_dur;
        struct s_cushion_struct *c;
	int	trailing_silence, new_cushion, cushion_step, diff;
        const audio_format* ofmt;
	sample	*bufp;

        c = spi->cushion;
	if ((read_dur = tx_read_audio(spi->tb)) <= 0) {
		return 0;
	} else {
                if (!spi->audio_device) {
                        /* no device means no cushion */
                        return read_dur;
                }
	}

	/* read_dur now reflects the amount of real time it took us to get
	 * through the last cycle of processing. 
         */

	if (spo->lecture == TRUE && spo->auto_lecture == 0) {
                cushion_update(c, read_dur, CUSHION_MODE_LECTURE);
	} else {
                cushion_update(c, read_dur, CUSHION_MODE_CONFERENCE);
	}

	/* Following code will try to achieve new cushion size without
	 * messing up the audio...
	 * First case is when we are in trouble and the output has gone dry.
	 * In this case we write out a complete new cushion with the desired
	 * size. We do not care how much of it is going to be real audio and
	 * how much silence so long as the silence is at the head of the new
	 * cushion. If the silence was at the end we would be creating
	 * another silence gap...
	 */
        cushion_size = cushion_get_size(c);
        ofmt         = audio_get_ofmt(spi->audio_device);

	if ( cushion_size < read_dur ) {
		/* Use a step for the cushion to keep things nicely rounded  */
                /* in the mixing. Round it up.                               */
                new_cushion = cushion_use_estimate(c);
                /* The mix routine also needs to know for how long the       */
                /* output went dry so that it can adjust the time.           */
                mix_get_new_cushion(ms, 
                                    cushion_size, 
                                    new_cushion, 
                                    (read_dur - cushion_size), 
                                    &bufp);
                audio_device_write(spo, bufp, new_cushion);
                debug_msg("catch up! read_dur(%d) > cushion_size(%d)\n",
                        read_dur,
                        cushion_size);
                cushion_size = new_cushion;
        } else {
                trailing_silence = mix_get_audio(ms, read_dur * ofmt->channels, &bufp);
                cushion_step = cushion_get_step(c);
                diff  = 0;

                if (trailing_silence > cushion_step) {
                        /* Check whether we need to adjust the cushion */
                        diff = cushion_diff_estimate_size(c);
                        if (abs(diff) < cushion_step) {
                                diff = 0;
                        }
                } 
                
                /* If diff is less than zero then we must decrease the */
                /* cushion so loose some of the trailing silence.      */
                if (diff < 0 && 
                    mix_active(ms) == FALSE && 
                    source_list_source_count(spi->active_sources) == 0) {
                        /* Only decrease cushion if not playing anything out */
#ifdef DEBUG
                        u_int32 old_cushion;
                        old_cushion = cushion_get_size(c);
#endif
                        read_dur -= cushion_step;
                        cushion_step_down(c);
#ifdef DEBUG
                        if (cushion_get_size(c) != old_cushion) {
                                debug_msg("Decreasing cushion\n");
                        }
#endif
                        
                }
                audio_device_write(spo, bufp, read_dur);
                /*
                 * If diff is greater than zero then we must increase the
                 * cushion so increase the amount of trailing silence.
                 */
                if (diff > 0) {
                        audio_device_write(spo, audio_zero_buf, cushion_step);
                        cushion_step_up(c);
                        debug_msg("Increasing cushion.\n");
                }
        }
        return (read_dur);
}
