/*
 * FILE: auddev_newpcm.c - Sound interface for newpcm FreeBSD driver.
 *
 * Modified to support newpcm (July 2000).
 *
 * Copyright (c) 1996-2000 University College London
 * All rights reserved.
 */
 
#ifndef HIDE_SOURCE_STRINGS
static const char cvsid[] = 
	"$Id$";
#endif /* HIDE_SOURCE_STRINGS */

#include "config_unix.h"
#include "config_win32.h"
#include "audio_types.h"
#include "audio_fmt.h"
#include "auddev_newpcm.h"
#include "memory.h"
#include "debug.h"

#include <machine/soundcard.h>

static char *port_names[] = SOUND_DEVICE_LABELS;
static int  iport, oport, loop;
static snd_chan_param pa;
static struct snd_size sz;
static int audio_fd = -1;

#define RAT_TO_DEVICE(x) ((x) * 100 / MAX_AMP)
#define DEVICE_TO_RAT(x) ((x) * MAX_AMP / 100)

#define NEWPCM_AUDIO_IOCTL(fd, cmd, val) if (ioctl((fd), (cmd), (val)) < 0) { \
                                            debug_msg("Failed %s - line %d\n",#cmd, __LINE__); \
                                            newpcm_error = __LINE__; \
                                               }

#define NEWPCM_MAX_AUDIO_NAME_LEN 32
#define NEWPCM_MAX_AUDIO_DEVICES  3

static int dev_ids[NEWPCM_MAX_AUDIO_DEVICES];
static char names[NEWPCM_MAX_AUDIO_DEVICES][NEWPCM_MAX_AUDIO_NAME_LEN];
static int ndev = 0;
static int newpcm_error;
static audio_format *input_format, *output_format, *tmp_format;
static snd_capabilities soundcaps[NEWPCM_MAX_AUDIO_DEVICES];

static void newpcm_mixer_save(int fd);
static void newpcm_mixer_restore(int fd);
static void newpcm_mixer_init(int fd);
static void newpcm_audio_loopback_config(int gain);

int 
newpcm_audio_open(audio_desc_t ad, audio_format *ifmt, audio_format *ofmt)
{
        int32_t         fragment;
        char            thedev[64];
        
        assert(ad >= 0 && ad < ndev); 
	sprintf(thedev, "/dev/audio%d", dev_ids[ad]);

        debug_msg("Opening %s\n", thedev);

        audio_fd = open(thedev, O_RDWR);
        if (audio_fd >= 0) {
                /* Ignore any earlier errors */
                newpcm_error = 0;

		newpcm_mixer_save(audio_fd);

                NEWPCM_AUDIO_IOCTL(audio_fd, AIOGCAP, &soundcaps[ad]);
		debug_msg("soundcaps[%d].rate_min = %d\n", ad, soundcaps[ad].rate_min);
		debug_msg("soundcaps[%d].rate_max = %d\n", ad, soundcaps[ad].rate_max);
		debug_msg("soundcaps[%d].formats  = 0x%08lx\n", ad, soundcaps[ad].formats);
                debug_msg("soundcaps[%d].bufsize  = %d\n", ad, soundcaps[ad].bufsize);
		debug_msg("soundcaps[%d].mixers   = 0x%08lx\n", ad, soundcaps[ad].mixers);
		debug_msg("soundcaps[%d].inputs   = 0x%08lx\n", ad, soundcaps[ad].inputs);
		debug_msg("soundcaps[%d].left     = 0x%04lx\n", ad, soundcaps[ad].left);
		debug_msg("soundcaps[%d].right    = 0x%04lx\n", ad, soundcaps[ad].right);

                /* Setup input and output format settings */
                assert(ofmt->channels == ifmt->channels);
                memset(&pa, 0, sizeof(pa));
                if (ifmt->channels == 2) {
                        if (!soundcaps[ad].formats & AFMT_STEREO) {
                                fprintf(stderr,"Driver does not support stereo for this soundcard\n");
                                newpcm_audio_close(ad);
                                return FALSE;
                        }
                        pa.rec_format  = AFMT_STEREO;
                        pa.play_format = AFMT_STEREO;
                }

                switch(ifmt->encoding) {
                case DEV_PCMU: pa.rec_format |= AFMT_MU_LAW; break;
                case DEV_PCMA: pa.rec_format |= AFMT_A_LAW;  break;
                case DEV_S8:   pa.rec_format |= AFMT_S8;     break;
                case DEV_S16:  pa.rec_format |= AFMT_S16_LE; break;
                case DEV_U8:   pa.rec_format |= AFMT_U8;     break;
                }

                switch(ofmt->encoding) {
                case DEV_PCMU: pa.play_format |= AFMT_MU_LAW; break;
                case DEV_PCMA: pa.play_format |= AFMT_A_LAW;  break;
                case DEV_S8:   pa.play_format |= AFMT_S8;     break;
                case DEV_S16:  pa.play_format |= AFMT_S16_LE; break;
                case DEV_U8:   pa.play_format |= AFMT_U8;     break;
                }
                pa.play_rate = ofmt->sample_rate;
                pa.rec_rate = ifmt->sample_rate;
                NEWPCM_AUDIO_IOCTL(audio_fd, AIOSFMT, &pa);

                sz.play_size = ofmt->bytes_per_block;
                sz.rec_size  = ifmt->bytes_per_block;
                NEWPCM_AUDIO_IOCTL(audio_fd, AIOSSIZE, &sz);

                NEWPCM_AUDIO_IOCTL(audio_fd, AIOGSIZE, &sz);
                debug_msg("rec size %d, play size %d bytes\n",
                          sz.rec_size, sz.play_size);

		/* Fragment :  8msb = #frags, 16lsbs = log2 fragsize */
		fragment = 0x08000007;
		NEWPCM_AUDIO_IOCTL(audio_fd, SNDCTL_DSP_SETFRAGMENT, &fragment);
                
                if (newpcm_error != 0) {
                        /* Failed somewhere in initialization - reset error and exit*/
                        newpcm_audio_close(ad);
                        newpcm_error = 0;
                        return FALSE;
                }

                /* Store format in case we have to re-open device because
                 * of driver bug.  Careful with freeing format as input format
                 * could be static input_format if device reset during write.
                 */
                tmp_format = audio_format_dup(ifmt);
                if (input_format != NULL) {
                        audio_format_free(&input_format);
                }
                input_format = tmp_format;

                tmp_format = audio_format_dup(ofmt);
                if (output_format != NULL) {
                        audio_format_free(&output_format);
                }
                output_format = tmp_format;

		newpcm_mixer_init(audio_fd);
                /* Turn off loopback from input to output... not fatal so
                 * after error check.
                 */
		newpcm_audio_loopback(ad, 0);

                read(audio_fd, thedev, 64);
                return TRUE;
        } else {
		fprintf(stderr, 
			"Could not open device: %s (half-duplex?)\n", 
			names[ad]);
		perror("newpcm_audio_open");
                newpcm_audio_close(ad);
                return FALSE;
        }
}

/* Close the audio device */
void
newpcm_audio_close(audio_desc_t ad)
{
        UNUSED(ad);
	
	if (audio_fd < 0) {
                debug_msg("Device already closed!\n");
                return;
        }
        if (input_format != NULL) {
                audio_format_free(&input_format);
        }
        if (output_format != NULL) {
                audio_format_free(&output_format);
        }
	newpcm_mixer_restore(audio_fd);
	newpcm_audio_drain(audio_fd);
	close(audio_fd);
        audio_fd = -1;
}

/* Flush input buffer */
void
newpcm_audio_drain(audio_desc_t ad)
{
        u_char buf[4];
        int pre, post;
        
        assert(audio_fd > 0);

        NEWPCM_AUDIO_IOCTL(audio_fd, FIONREAD, &pre);
        NEWPCM_AUDIO_IOCTL(audio_fd, SNDCTL_DSP_RESET, 0);
        NEWPCM_AUDIO_IOCTL(audio_fd, SNDCTL_DSP_SYNC, 0);
        NEWPCM_AUDIO_IOCTL(audio_fd, FIONREAD, &post);
        debug_msg("audio drain: %d -> %d\n", pre, post);
        read(audio_fd, buf, sizeof(buf));

        UNUSED(ad);
}

int
newpcm_audio_duplex(audio_desc_t ad)
{
        /* We only ever open device full duplex! */
        UNUSED(ad);
        return TRUE;
}

int
newpcm_audio_read(audio_desc_t ad, u_char *buf, int read_bytes)
{
        int done, this_read;
        int len;
        /* Figure out how many bytes we can read before blocking... */

        UNUSED(ad); assert(audio_fd > 0);

        NEWPCM_AUDIO_IOCTL(audio_fd, FIONREAD, &len);

        len = min(len, read_bytes);

        /* Read the data... */
        done = 0;
        while(done < len) {
                this_read = read(audio_fd, (void*)buf, len - done);
                done += this_read;
                buf  += this_read;
        }
        return done;
}

int
newpcm_audio_write(audio_desc_t ad, u_char *buf, int write_bytes)
{
	int            done;

        UNUSED(ad); assert(audio_fd > 0);

        done = write(audio_fd, (void*)buf, write_bytes);
        if (done != write_bytes && errno != EINTR) {
                /* Only ever seen this with soundblaster cards.
                 * Driver occasionally packs in reading.  Seems to be
                 * no way to reset cleanly whilst running, even
                 * closing device, waiting a few 100ms and re-opening
                 * seems to fail.  
                 */
                perror("Error writing device.");
		fprintf(stderr, "Please email this message to rat-trap@cs.ucl.ac.uk with output of:\n\t uname -a\n\t cat /dev/sndstat\n");
                return (write_bytes - done);
        }

        return write_bytes;
}

/* Set ops on audio device to be non-blocking */
void
newpcm_audio_non_block(audio_desc_t ad)
{
	int             frag = 1;

	UNUSED(ad); assert(audio_fd != -1);

        NEWPCM_AUDIO_IOCTL(audio_fd, SNDCTL_DSP_NONBLOCK, &frag);
}

/* Set ops on audio device to be blocking */
void
newpcm_audio_block(audio_desc_t ad)
{
  	int             frag = 0;
        
        UNUSED(ad); assert(audio_fd > 0);
        
        NEWPCM_AUDIO_IOCTL(audio_fd, SNDCTL_DSP_NONBLOCK, &frag);
} 


static int recmask, playmask;

static void
newpcm_mixer_init(int fd) 
{
	int devmask;

	NEWPCM_AUDIO_IOCTL(fd, SOUND_MIXER_READ_RECMASK, &recmask);

	/* Remove Vol from Rec mask - it is a play control! */
	recmask = recmask & ~SOUND_MASK_VOLUME;
	if (recmask & SOUND_MASK_MIC) {
		iport = SOUND_MASK_MIC;
	} else {
		iport = 1;
		while ((iport & recmask) == 0) {
			iport <<= 1;
		}
	}

	NEWPCM_AUDIO_IOCTL(fd, SOUND_MIXER_READ_DEVMASK, &devmask);
	playmask = devmask & ~recmask & ~SOUND_MASK_RECLEV ;
	debug_msg("devmask 0x%08x recmask 0x%08x playmask 0x%08x\n",
		  devmask,
		  recmask,
		  playmask);
}

static int
newpcm_count_ports(int mask) 
{
	int n = 0, m = mask;

	while (m > 0) {
		n += (m & 0x01);
		m >>= 1;
	}

	return n;
}

static int
newpcm_get_nth_port_mask(int mask, int n)
{
	static int lgmask;

	lgmask = -1;
	do {
		lgmask ++;
		if ((1 << lgmask) & mask) {
			n--;
		}
	} while (n >= 0);

	assert((1 << lgmask) & mask);
	return lgmask;
}

/* Gain and volume values are in the range 0 - MAX_AMP */
void
newpcm_audio_set_ogain(audio_desc_t ad, int vol)
{
	int volume, lgport, op;

        UNUSED(ad); assert(audio_fd > 0);

	volume = vol << 8 | vol;

	lgport = -1;
	op = oport;
	while (op > 0) {
		op >>= 1;
		lgport ++;
	}

	NEWPCM_AUDIO_IOCTL(audio_fd, MIXER_WRITE(lgport), &volume);
}

int
newpcm_audio_get_ogain(audio_desc_t ad)
{
	int volume, lgport, op;

        UNUSED(ad); assert(audio_fd > 0);

	lgport = -1;
	op     = oport;
	while (op > 0) {
		op >>= 1;
		lgport ++;
	}

	NEWPCM_AUDIO_IOCTL(audio_fd, MIXER_READ(lgport), &volume);

	return DEVICE_TO_RAT(volume & 0xff); /* Extract left channel volume */
}

void
newpcm_audio_oport_set(audio_desc_t ad, audio_port_t port)
{
	UNUSED(ad);
	oport = port;
	return;
}

audio_port_t
newpcm_audio_oport_get(audio_desc_t ad)
{
	UNUSED(ad);
	return oport;
}

int
newpcm_audio_oport_count(audio_desc_t ad)
{
        UNUSED(ad);
	return newpcm_count_ports(playmask);
}

const audio_port_details_t*
newpcm_audio_oport_details(audio_desc_t ad, int idx)
{
	static audio_port_details_t ap;
	int lgmask;

	UNUSED(ad);

	lgmask = newpcm_get_nth_port_mask(playmask, idx);
	ap.port = 1 << lgmask;
	sprintf(ap.name, "%s", port_names[lgmask]);

        return &ap;
}

void
newpcm_audio_set_igain(audio_desc_t ad, int gain)
{
	int volume = RAT_TO_DEVICE(gain) << 8 | RAT_TO_DEVICE(gain);

        UNUSED(ad); assert(audio_fd > 0);
	newpcm_audio_loopback_config(gain);
	NEWPCM_AUDIO_IOCTL(audio_fd, SOUND_MIXER_WRITE_RECLEV, &volume);
}

int
newpcm_audio_get_igain(audio_desc_t ad)
{
	int volume;

        UNUSED(ad); assert(audio_fd > 0);
	NEWPCM_AUDIO_IOCTL(audio_fd, SOUND_MIXER_READ_RECLEV, &volume);
	return (DEVICE_TO_RAT(volume & 0xff));
}

void
newpcm_audio_iport_set(audio_desc_t ad, audio_port_t port)
{
	/* Check port is in record mask */
	int gain;

	debug_msg("port 0x%08x recmask 0x%08x\n", port, recmask);

	assert((port & recmask) != 0);

	if (ioctl(audio_fd, SOUND_MIXER_WRITE_RECSRC, &port) < 0) {
		perror("Unable to write record mask\n");
		return;
	}
	iport = port;
	gain = newpcm_audio_get_igain(ad);
	newpcm_audio_loopback_config(gain);
	UNUSED(ad);
}

audio_port_t
newpcm_audio_iport_get(audio_desc_t ad)
{
	UNUSED(ad); assert(audio_fd > 0);
	return iport;
}

int
newpcm_audio_iport_count(audio_desc_t ad)
{
	UNUSED(ad);
	return newpcm_count_ports(recmask);
}

const audio_port_details_t *
newpcm_audio_iport_details(audio_desc_t ad, int idx)
{
	static audio_port_details_t ap;
	int lgmask;

	UNUSED(ad);

	lgmask = newpcm_get_nth_port_mask(recmask, idx);
	ap.port = 1 << lgmask;
	sprintf(ap.name, "%s", port_names[lgmask]);

        return &ap;
}

void
newpcm_audio_loopback(audio_desc_t ad, int gain)
{
        UNUSED(ad); assert(audio_fd > 0);
        loop = gain;
}

static void
newpcm_audio_loopback_config(int gain) 
{
	int lgport, vol;

	/* Find current input port id */
	lgport = newpcm_get_nth_port_mask(iport, 0);

	if (loop) {
		vol = RAT_TO_DEVICE(gain) << 8 | RAT_TO_DEVICE(gain);
	} else {
		vol = 0;
	}

	NEWPCM_AUDIO_IOCTL(audio_fd, MIXER_WRITE(lgport), &vol);
}

void
newpcm_audio_wait_for(audio_desc_t ad, int delay_ms)
{
        if (!newpcm_audio_is_ready(ad)) {
                usleep((unsigned int)delay_ms * 1000);
        }
}

int 
newpcm_audio_is_ready(audio_desc_t ad)
{
        int avail;

        UNUSED(ad);

        NEWPCM_AUDIO_IOCTL(audio_fd, FIONREAD, &avail);

        return (avail >= sz.rec_size);
}

int 
newpcm_audio_supports(audio_desc_t ad, audio_format *fmt)
{
        snd_capabilities s;

        UNUSED(ad);

        NEWPCM_AUDIO_IOCTL(audio_fd, AIOGCAP, &s);
        if (!newpcm_error) {
                if ((unsigned)fmt->sample_rate < s.rate_min || (unsigned)fmt->sample_rate > s.rate_max) return FALSE;
                if (fmt->channels == 1) return TRUE;                    /* Always supports mono */
                assert(fmt->channels == 2);
                if (s.formats & AFMT_STEREO) return TRUE;
        }
        return FALSE;
}

int
newpcm_audio_query_devices()
{
        FILE *f;
        char buf[128], *p;
        int n, newpcm = FALSE;

        f = fopen("/dev/sndstat", "r");
        if (f) {
                while (!feof(f) && ndev < NEWPCM_MAX_AUDIO_DEVICES) {
                        p = fgets(buf, 128, f);
                        n = sscanf(buf, "pcm%d: <%[A-z0-9 ]>", dev_ids + ndev, names[ndev]);
                        if (p && n == 2) {
                                debug_msg("dev (%d) name (%s)\n", dev_ids[ndev], names[ndev]);
                                ndev++;
                        } else if (strstr(buf, "newpcm")) {
				newpcm = TRUE;
			} 
                }
                fclose(f);
        }

	if (newpcm == FALSE) {
		ndev = 0; /* Should be using Luigi's interface */
	}

	printf("XXX %d newpcm devices - newpcm (%d)\n", ndev, newpcm);

        return (ndev);
}

int
newpcm_get_device_count()
{
        return ndev;
}

char *
newpcm_get_device_name(audio_desc_t idx)
{
        if (idx >=0 && idx < ndev) {
                return names[idx];
        }
        return NULL;
}

/* Functions to save and restore recording source and mixer levels */

static int saved_rec_mask, saved_gain_values[SOUND_MIXER_NRDEVICES];

static void
newpcm_mixer_save(int fd)
{
	int devmask, i;
	NEWPCM_AUDIO_IOCTL(fd, SOUND_MIXER_READ_RECSRC, &saved_rec_mask); 
	NEWPCM_AUDIO_IOCTL(fd, SOUND_MIXER_READ_DEVMASK, &devmask);
	for (i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
		if ((1 << i) & devmask) {
			NEWPCM_AUDIO_IOCTL(fd, MIXER_READ(i), &saved_gain_values[i]);
		} else {
			saved_gain_values[i] = 0;
		}
	}
}

static void
newpcm_mixer_restore(int fd)
{
	int devmask, i;
	NEWPCM_AUDIO_IOCTL(fd, SOUND_MIXER_WRITE_RECSRC, &saved_rec_mask); 

	NEWPCM_AUDIO_IOCTL(fd, SOUND_MIXER_READ_DEVMASK, &devmask);
	for (i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
		if ((1 << i) & devmask) {
			NEWPCM_AUDIO_IOCTL(fd, MIXER_WRITE(i), &saved_gain_values[i]);
		}
	}
}