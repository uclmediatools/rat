/*****************************************************************************/
/*                                                                           */
/* FILE: auddev_trans.c                                                      */
/*                                                                           */
/* Transcoder audio device.                                                  */
/*                                                                           */
/* Contributed by Michael Wallbaum <wallbaum@informatik.rwth-aachen.de>      */
/*                                                                           */
/* $Id$                                                                      */
/*                                                                           */
/*****************************************************************************/

#include "config_unix.h"
#include "config_win32.h"
#include "audio_types.h"
#include "auddev_trans.h"
#include "memory.h"
#include "debug.h"

#define MAXBUFDEVS 2

typedef struct _bufdevInfo {
        audio_format ifmt;
        audio_format ofmt;
        int audio_fd;
        struct timeval last_time;
        struct timeval curr_time;
        u_char         *channel;
        int             head;
        int             tail;
        int avail_bytes;
        int read_virgin;
        int igain;
        int ogain;
} bufdevInfo;

static bufdevInfo bufdev[MAXBUFDEVS];
static u_int32 devIdMap[MAXBUFDEVS];

static char *bufdevName[2] = {"Transcoder Port 1", "Transcoder Port 2"};

#define CHANNEL_SIZE    8192

static u_int32 mapAudioDescToDeviceID(audio_desc_t ad);

static int
trans_audio_open_dev (audio_desc_t ad, audio_format *infmt, audio_format *outfmt)
{
        /* Open a fake audio channel. The value we return is used to identify the */
        /* channel for the other routines in this module. The #ifdefs in net.c    */
        /* prevent it being used in a select().                                   */
        /* Note: We must open EXACTLY two channels, before the other routines in  */
        /*       this module function correctly.                                  */
        int i;

        ad = mapAudioDescToDeviceID(ad);
        assert(ad >= 0);

        bufdev[ad].head = 0;
        bufdev[ad].tail = 0;
        bufdev[ad].avail_bytes = 0;
        bufdev[ad].read_virgin  = TRUE;
        bufdev[ad].channel = (u_char *) xmalloc(CHANNEL_SIZE * sizeof(u_char));
        for (i=0; i<CHANNEL_SIZE; i++) {
                bufdev[ad].channel[i] = L16_AUDIO_ZERO;
        }

        if (bufdev[ad].audio_fd != -1) {
                debug_msg("Warning device not closed before opening.\n");
                trans_audio_close(ad);
        }

        memcpy(&(bufdev[ad].ifmt), infmt,  sizeof(*infmt));
        memcpy(&(bufdev[ad].ofmt), outfmt, sizeof(*outfmt));

        return TRUE;
}

static u_int32
mapAudioDescToDeviceID(audio_desc_t ad)
{
        return devIdMap[ad];
}

int
trans_audio_init()
{
        audio_format af;
        unsigned int i;

        af.bits_per_sample = 16;
        af.bytes_per_block = 320;
        af.channels        = 1;
        af.encoding        = DEV_S16;
        af.sample_rate     = 8000;
        
        for(i = 0; i < MAXBUFDEVS; i++) {
                bufdev[i].audio_fd = -1;
                if (trans_audio_open_dev(i, &af, &af)) {
                        trans_audio_close(i);
                        devIdMap[i] = i;
                }
        }

        return MAXBUFDEVS;
}

int
trans_audio_open(audio_desc_t ad, audio_format *infmt, audio_format *outfmt)
{
        ad = mapAudioDescToDeviceID(ad);
        return trans_audio_open_dev(ad, infmt, outfmt);
}

/*
 * Shutdown.
 */
void
trans_audio_close(audio_desc_t ad)
{
        ad = mapAudioDescToDeviceID(ad);
        if (bufdev[ad].audio_fd > 0)
                bufdev[ad].audio_fd = -1;
        xfree(bufdev[ad].channel);
        return;
}

/*
 * Flush input buffer.
 */
void
trans_audio_drain(audio_desc_t ad)
{

        // MW
                   ad = mapAudioDescToDeviceID(ad);
        bufdev[ad].read_virgin = TRUE;
        bufdev[ad].head = 0;
        bufdev[ad].tail = 0;
        bufdev[ad].avail_bytes = 0;
        return;
}

/*
 * Set record gain.
 */
void
trans_audio_set_igain(audio_desc_t ad, int gain)
{
        ad = mapAudioDescToDeviceID(ad);
        bufdev[ad].igain = gain;
        return;
}

/*
 * Get record gain.
 */
int
trans_audio_get_igain(audio_desc_t ad)
{
        ad = mapAudioDescToDeviceID(ad);
        return bufdev[ad].igain;
}

int
trans_audio_duplex(audio_desc_t ad)
{
        UNUSED(ad);
        return TRUE;
}

/*
 * Set play gain.
 */
void
trans_audio_set_ogain(audio_desc_t ad, int vol)
{
        ad = mapAudioDescToDeviceID(ad);
        bufdev[ad].ogain = vol;
        return;
}

/*
 * Get play gain.
 */
int
trans_audio_get_ogain(audio_desc_t ad)
{
        ad = mapAudioDescToDeviceID(ad);
        return bufdev[ad].ogain;
}

static int
time_diff_to_bytes(struct timeval *start, struct timeval *end,  audio_format ifmt)
{
        int diff_ms, diff_bytes;
        diff_ms = (end->tv_sec  - start->tv_sec) * 1000 + (end->tv_usec - start->tv_usec) / 1000;
        diff_bytes = diff_ms * (ifmt.bits_per_sample / 8 ) * (ifmt.sample_rate / 1000) * ifmt.channels;

        return diff_bytes;
}


/*
 * Record audio data.
 */
int
trans_audio_read(audio_desc_t ad, u_char *buf, int buf_bytes)
{
        int i, read_size, copy_size;

        ad = mapAudioDescToDeviceID(ad);
        assert(ad >= 0);

        assert(buf != 0);
        assert(buf_bytes > 0);
        assert(bufdev[ad].head <= CHANNEL_SIZE);
        assert(bufdev[ad].tail <= CHANNEL_SIZE);
        assert(bufdev[ad].avail_bytes <= CHANNEL_SIZE);

        if (bufdev[ad].read_virgin == TRUE) {
                gettimeofday(&(bufdev[ad].last_time), NULL);
                bufdev[ad].avail_bytes = 0;
                bufdev[ad].read_virgin = FALSE;
        }
        gettimeofday(&(bufdev[ad].curr_time), NULL);
        read_size = time_diff_to_bytes(&(bufdev[ad].last_time), &(bufdev[ad].curr_time), bufdev[ad].ifmt);

        if (read_size + bufdev[ad].avail_bytes < bufdev[ad].ifmt.bytes_per_block) {
                return 0;
        }

        if (read_size > buf_bytes) read_size = buf_bytes;

        bufdev[ad].last_time = bufdev[ad].curr_time;

        copy_size = bufdev[ad].avail_bytes;     /* The amount of data available in this module... */
        if (copy_size >= read_size) {
                copy_size = read_size;
        } else {
#ifdef DEBUG_TRANSCODER
                printf("transcoder_read: underflow, silence substituted -- want %d got %d channel %d\n", read_size, copy_size, ad);
#endif
        }
        for (i=0; i<copy_size; i++) {
                buf[i] = bufdev[ad].channel[(bufdev[ad].head + i) % CHANNEL_SIZE];
        }
        for (i=copy_size; i<read_size; i++) {
                buf[i] = L16_AUDIO_ZERO;
        }

        bufdev[ad].head = (bufdev[ad].head + copy_size) % CHANNEL_SIZE;
        bufdev[ad].avail_bytes -= copy_size;

        assert(bufdev[ad].head <= CHANNEL_SIZE);
        assert(bufdev[ad].tail <= CHANNEL_SIZE);
        assert(bufdev[ad].avail_bytes >= 0);
        return read_size;
}

/*
 * Playback audio data.
 */
int
trans_audio_write(audio_desc_t ad, u_char *buf, int write_bytes)
{
        int i;

        ad = mapAudioDescToDeviceID(ad);
        assert(ad >= 0);

        assert(buf != 0);
        assert(write_bytes > 0);
        assert(bufdev[ad].head <= CHANNEL_SIZE);
        assert(bufdev[ad].tail <= CHANNEL_SIZE);
        assert(bufdev[ad].avail_bytes <= CHANNEL_SIZE);

        for (i=0; i<write_bytes; i++) {
                bufdev[ad].channel[(bufdev[ad].tail + i) % CHANNEL_SIZE] = buf[i];
        }
        bufdev[ad].tail = (bufdev[ad].tail + write_bytes) % CHANNEL_SIZE;

        // reposition head if necessary
                if(write_bytes > CHANNEL_SIZE - bufdev[ad].avail_bytes)
                        bufdev[ad].head = (bufdev[ad].tail + 1) % CHANNEL_SIZE;

        bufdev[ad].avail_bytes = (bufdev[ad].avail_bytes + write_bytes) % CHANNEL_SIZE;

        assert(bufdev[ad].head <= CHANNEL_SIZE);
        assert(bufdev[ad].tail <= CHANNEL_SIZE);
        return write_bytes;
}

/*
 * Set options on audio device to be non-blocking.
 */
void
trans_audio_non_block(audio_desc_t ad)
{
        UNUSED(ad);
}

/*
 * Set options on audio device to be blocking.
 */
void
trans_audio_block(audio_desc_t ad)
{
        UNUSED(ad);
}

#define TRANS_SPEAKER    0x0101
#define TRANS_MICROPHONE 0x0201

static audio_port_details_t out_ports[] = {
        {TRANS_SPEAKER, AUDIO_PORT_SPEAKER}
};

static audio_port_details_t in_ports[] = {
        {TRANS_MICROPHONE, AUDIO_PORT_MICROPHONE}
};

/*
 * Set output port.
 */
void
trans_audio_oport_set(audio_desc_t ad, audio_port_t port)
{
        UNUSED(ad); UNUSED(port);

        return;
}

/*
 * Get output port.
 */

audio_port_t
trans_audio_oport_get(audio_desc_t ad)
{
        UNUSED(ad);
        return out_ports[0].port;
}

int
trans_audio_oport_count(audio_desc_t ad)
{
        UNUSED(ad);
        return 1;
}

const audio_port_details_t*
trans_audio_oport_details(audio_desc_t ad, int idx)
{
        UNUSED(ad);
        assert(idx == 0);
        return &out_ports[0];
}

/*
 * Set input port.
 */
void
trans_audio_iport_set(audio_desc_t ad, audio_port_t port)
{
        UNUSED(ad);
        UNUSED(port);
        return;
}

/*
 * Get input port.
 */
audio_port_t
trans_audio_iport_get(audio_desc_t ad)
{
        UNUSED(ad);
        return in_ports[0].port;
}

int
trans_audio_iport_count(audio_desc_t ad)
{
        UNUSED(ad);
        return 1;
}

const audio_port_details_t*
trans_audio_iport_details(audio_desc_t ad, int idx)
{
        UNUSED(ad);
        assert(idx == 0);
        return &in_ports[0];
}

/*
 * Enable hardware loopback
 */
void 
trans_audio_loopback(audio_desc_t ad, int gain)
{
        UNUSED(ad);
        UNUSED(gain);
        /* Nothing doing... */
}

/*
 * For external purposes this function returns non-zero
 * if audio is ready.
 */
int
trans_audio_is_ready(audio_desc_t ad)
{
        struct timeval now;
        u_int32 diff;

        ad = mapAudioDescToDeviceID(ad);
        gettimeofday(&now,NULL);
        diff = (now.tv_sec  - bufdev[ad].last_time.tv_sec) * 1000 + (now.tv_usec - bufdev[ad].last_time.tv_usec)/1000;
        diff *= (bufdev[ad].ifmt.bits_per_sample / 8) * bufdev[ad].ifmt.sample_rate / 1000 * bufdev[ad].ifmt.channels;

        if ((diff + bufdev[ad].avail_bytes) > (unsigned)bufdev[ad].ifmt.bytes_per_block) return TRUE;

        return FALSE;
}

static void 
trans_audio_select(audio_desc_t ad, int delay_ms)
{
        int needed, dur;

        ad = mapAudioDescToDeviceID(ad);
        needed = bufdev[ad].ifmt.bytes_per_block - bufdev[ad].avail_bytes;
        assert(needed >= 0);

        dur = needed * 1000 * 8 / (bufdev[ad].ifmt.sample_rate * bufdev[ad].ifmt.bits_per_sample * bufdev[ad].ifmt.channels);
        dur = min(dur, delay_ms);
        usleep(dur * 1000);
}

void
trans_audio_wait_for(audio_desc_t ad, int delay_ms)
{
        if (trans_audio_is_ready(ad) == FALSE) {
                trans_audio_select(ad, delay_ms);
        }
}

int
trans_audio_device_count()
{
        return MAXBUFDEVS;
}

const char*
trans_audio_device_name(audio_desc_t ad)
{
        ad = mapAudioDescToDeviceID(ad);
        return bufdevName[ad];
}

int
trans_audio_supports(audio_desc_t ad, audio_format *fmt)
{
        UNUSED(ad);
        if ((!(fmt->sample_rate % 8000) || !(fmt->sample_rate % 11025)) && 
            (fmt->channels == 1 || fmt->channels == 2)) {
                return TRUE;
        }
        return FALSE;
}
