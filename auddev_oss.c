/*
 * FILE:    auddev_oss.c - Open Sound System audio device driver
 * PROGRAM: RAT
 * AUTHOR:  Colin Perkins
 * MODS:    Orion Hodson
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
#include "debug.h"
#include "memory.h"
#include "audio_types.h"
#include "audio_fmt.h"
#include "auddev_oss.h"
#include "util.h"

#ifdef HAVE_SOUNDCARD_H
# include <soundcard.h>
#else
#ifdef HAVE_SYS_SOUNDCARD_H
#  include <sys/soundcard.h>
#endif
#endif

#ifdef HAVE_ALSA_AUDIO
extern int alsa_get_device_count();
#endif

enum { AUDIO_SPEAKER, AUDIO_HEADPHONE, AUDIO_LINE_OUT, AUDIO_MICROPHONE, AUDIO_LINE_IN, AUDIO_CD};

/* Info to match port id's to port name's */
static audio_port_details_t in_ports[] = {
        {AUDIO_MICROPHONE, AUDIO_PORT_MICROPHONE},
        {AUDIO_LINE_IN,    AUDIO_PORT_LINE_IN},
        {AUDIO_CD,         AUDIO_PORT_CD}
};

#define NUM_IN_PORTS (sizeof(in_ports)/(sizeof(in_ports[0])))

static audio_port_details_t out_ports[] = {
        {AUDIO_SPEAKER,   AUDIO_PORT_SPEAKER}
};

#define NUM_OUT_PORTS (sizeof(out_ports)/(sizeof(out_ports[0])))

int	iport     = AUDIO_MICROPHONE;
int	bytes_per_block;

/* Magic to get device names from OSS */

#define OSS_MAX_DEVICES 3
#define OSS_MAX_NAME_LEN 64

static int ndev;
char   dev_name[OSS_MAX_DEVICES][OSS_MAX_NAME_LEN];

static char the_dev[] = "/dev/dspX";
static int audio_fd[OSS_MAX_DEVICES];


#define OSS_MAX_SUPPORTED_FORMATS 14
/* == 8k,11k,16k,22k,32k,44.1k,48k * mono, stereo */
static u_char       have_probed[OSS_MAX_DEVICES];
static audio_format af_sup[OSS_MAX_DEVICES][OSS_MAX_SUPPORTED_FORMATS];
static int          n_af_sup[OSS_MAX_DEVICES];

static int oss_probe_formats(audio_desc_t);

audio_format format;

#define bat_to_device(x)  ((x) * 100 / MAX_AMP)
#define device_to_bat(x)  ((x) * MAX_AMP / 100)

static int
deve2oss(deve_e encoding)
{
        switch(encoding) {
        	case DEV_PCMU: return AFMT_MU_LAW;
        	case DEV_PCMA: return AFMT_A_LAW;
        	case DEV_S8:   return AFMT_S8;
        	case DEV_S16:  return AFMT_S16_LE;
                case DEV_U8:   return AFMT_U8;
        }
        abort();
	return 0;
}

/* Try to open the audio device.              */
/* Return TRUE if successful FALSE otherwise. */
int
oss_audio_open(audio_desc_t ad, audio_format *ifmt, audio_format *ofmt)
{
	int  mode, stereo, speed;
	int  volume   = (100<<8)|100;
	int  frag     = 0x7fff0000; 			/* unlimited number of fragments */
	int  reclb    = 0;
	char buffer[128];				/* sigh. */

        if (ad <0 || ad>OSS_MAX_DEVICES) {
                debug_msg("Invalid audio descriptor (%d)", ad);
                return FALSE;
        }

        sprintf(the_dev, "/dev/dsp%d", ad);
	if (ad == 0) {
		/* My understanding is that /dev/dsp should be a symbolic link to  */
		/* /dev/dsp0, but RedHat-6.0 doesn't do this. If /dev/dsp0 doesn't */
		/* exist we try to open /dev/dsp instead provided it's not a link. */
		struct stat	s;

		if (stat("/dev/dsp", &s) == 0) {
			if (!S_ISLNK(s.st_mode)) {
				debug_msg("/dev/dsp0 doesn't exist, trying /dev/dsp\n");
				sprintf(the_dev, "/dev/dsp");
			}
		}
	}

	audio_fd[ad] = open(the_dev, O_RDWR | O_NDELAY);
	if (audio_fd[ad] > 0) {
		/* Note: The order in which the device is set up is important! Don't */
		/*       reorder this code unless you really know what you're doing! */

                /* Set 20 ms blocksize - only modulates read sizes */
                bytes_per_block = 20 * (ifmt->sample_rate / 1000) * (ifmt->bits_per_sample / 8);
                /* Round to the nearest legal frag size (next power of two lower...) */
                frag |= (int) (log(bytes_per_block)/log(2));
                debug_msg("frag=%x bytes_per_block=%d\n", frag, bytes_per_block);
		if ((ioctl(audio_fd[ad], SNDCTL_DSP_SETFRAGMENT, &frag) == -1)) {
			printf("Cannot set the fragement size\n");
		}

		if (ioctl(audio_fd[ad], SNDCTL_DSP_SETDUPLEX, 0) == -1) {
			printf("Cannot enable full-duplex mode. Are you sure your hardware supports\n");
			printf("full duplex operation? Look for the word `DUPLEX' in /dev/sndstat. \n");
                        return FALSE;
		}

                mode = deve2oss(ifmt->encoding);
		if ((ioctl(audio_fd[ad], SNDCTL_DSP_SETFMT, &mode) == -1)) {
                        if (ifmt->encoding == DEV_S16) {
                                audio_format_change_encoding(ifmt, DEV_PCMU);
                                audio_format_change_encoding(ofmt, DEV_PCMU);
                                if ((ioctl(audio_fd[ad], SNDCTL_DSP_SETFMT, &mode) == -1)) {
                                        oss_audio_close(ad);
                                        return FALSE;
                                }
                                debug_msg("Using mu-law\n");
                        }
		}

                if (!have_probed[ad]) {
                        oss_probe_formats(ad);
                        have_probed[ad] = TRUE;
                }

                stereo = ifmt->channels - 1; 
                assert(stereo == 0 || stereo == 1);
		if ((ioctl(audio_fd[ad], SNDCTL_DSP_STEREO, &stereo) == -1) || (stereo != (ifmt->channels - 1))) {
			printf("Audio device doesn't support %d channels!\n", ifmt->channels);
                        oss_audio_close(ad);
                        return FALSE;
		}

                speed = ifmt->sample_rate;
		if (ioctl(audio_fd[ad], SNDCTL_DSP_SPEED, &speed) == -1) {
			printf("Audio device doesn't support %dHz sampling rate in full duplex!\n", ifmt->sample_rate);
                        oss_audio_close(ad);
                        return FALSE;
		}
		if (speed != ifmt->sample_rate) {
			debug_msg("Audio device sampling rate skew: %d should be %d\n", speed, ifmt->sample_rate);
		}

		/* Set global gain/volume to maximum values. This may fail on */
		/* some cards, but shouldn't cause any harm when it does..... */ 
		ioctl(audio_fd[ad], MIXER_WRITE(SOUND_MIXER_VOLUME), &volume);
		ioctl(audio_fd[ad], MIXER_WRITE(SOUND_MIXER_RECLEV), &volume);
		/* Select microphone input. We can't select output source...  */
		oss_audio_iport_set(ad, iport);
		/* Turn off loopback from input to output... This only works  */
		/* on a few cards, but shouldn't cause problems on the others */
		ioctl(audio_fd[ad], MIXER_WRITE(SOUND_MIXER_IMIX), &reclb);
		/* Device driver bug: we must read some data before the ioctl */
		/* to tell us how much data is waiting works....              */
		read(audio_fd[ad], buffer, 128);	
		return audio_fd[ad];
	} else {
		printf("Unable to open %s\n", the_dev);
		return FALSE;
	}
}

/* Close the audio device */
void
oss_audio_close(audio_desc_t ad)
{
        assert(audio_fd[ad] > 0);
	oss_audio_drain(ad);
	close(audio_fd[ad]);
        audio_fd[ad] = -1;
}

/* Flush input buffer */
void
oss_audio_drain(audio_desc_t ad)
{
        u_char buf[160];

        assert(ad < OSS_MAX_DEVICES);
        assert(audio_fd[ad] > 0);

	debug_msg("Draining audio buffer...\n");
        while(oss_audio_read(ad, buf, 160) == 160);
}

int
oss_audio_duplex(audio_desc_t ad)
{
        /* We don't open device if not full duplex. */
        UNUSED(ad);
        return TRUE;
}

/* Gain and volume values are in the range 0 - MAX_AMP */
void
oss_audio_set_igain(audio_desc_t ad, int gain)
{

	int volume = bat_to_device(gain) << 8 | bat_to_device(gain);

        assert(ad < OSS_MAX_DEVICES);
        assert(audio_fd[ad] > 0);

	switch (iport) {
	case AUDIO_MICROPHONE : 
		if (ioctl(audio_fd[ad], MIXER_WRITE(SOUND_MIXER_MIC), &volume) == -1) {
			perror("Setting gain");
		}
		return;
	case AUDIO_LINE_IN : 
		if (ioctl(audio_fd[ad], MIXER_WRITE(SOUND_MIXER_LINE), &volume) == -1) {
			perror("Setting gain");
		}
		return;
	case AUDIO_CD:
		if (ioctl(audio_fd[ad], MIXER_WRITE(SOUND_MIXER_CD), &volume) < 0) {
			perror("Setting gain");
		}
		return;
	}
	printf("ERROR: Unknown iport in audio_set_igain!\n");
	abort();
}

int
oss_audio_get_igain(audio_desc_t ad)
{
	int volume;

        UNUSED(ad); assert(audio_fd[ad] > 0); assert(ad < OSS_MAX_DEVICES);

	switch (iport) {
	case AUDIO_MICROPHONE : 
		if (ioctl(audio_fd[ad], MIXER_READ(SOUND_MIXER_MIC), &volume) == -1) {
			perror("Getting gain");
		}
		break;
	case AUDIO_LINE_IN : 
		if (ioctl(audio_fd[ad], MIXER_READ(SOUND_MIXER_LINE), &volume) == -1) {
			perror("Getting gain");
		}
		break;
	case AUDIO_CD:
		if (ioctl(audio_fd[ad], MIXER_READ(SOUND_MIXER_CD), &volume) < 0) {
			perror("Getting gain");
		}
		break;
	default : 
		printf("ERROR: Unknown iport in audio_set_igain!\n");
		abort();
	}
	return device_to_bat(volume & 0xff);
}

void
oss_audio_set_ogain(audio_desc_t ad, int vol)
{
	int volume;

        UNUSED(ad); assert(audio_fd[ad] > 0);

	volume = vol << 8 | vol;
	if (ioctl(audio_fd[ad], MIXER_WRITE(SOUND_MIXER_PCM), &volume) == -1) {
		perror("Setting volume");
	}
}

void
oss_audio_loopback(audio_desc_t ad, int gain)
{
        UNUSED(ad); assert(audio_fd[ad] > 0);

        gain = gain << 8 | gain;
        if (ioctl(audio_fd[ad], MIXER_WRITE(SOUND_MIXER_IMIX), &gain) == -1) {
                perror("loopback");
        }
}

int
oss_audio_get_ogain(audio_desc_t ad)
{
	int volume;

        UNUSED(ad); assert(audio_fd[ad] > 0);

	if (ioctl(audio_fd[ad], MIXER_READ(SOUND_MIXER_PCM), &volume) == -1) {
		perror("Getting volume");
	}
	return device_to_bat(volume & 0x000000ff); /* Extract left channel volume */
}

int
oss_audio_read(audio_desc_t ad, u_char *buf, int read_bytes)
{
        int 		read_len, available;
        audio_buf_info 	info;

        assert(audio_fd[ad] > 0);        

        /* Figure out how many bytes we can read before blocking... */
        ioctl(audio_fd[ad], SNDCTL_DSP_GETISPACE, &info);

        available = min(info.bytes, read_bytes);
        read_len  = read(audio_fd[ad], (char *)buf, available);
	if (read_len != available) {
		debug_msg("Amount of audio read != amount available\n");
	}
	if (read_len < 0) {
                perror("audio_read");
		return 0;
        }

        return read_len;
}

int
oss_audio_write(audio_desc_t ad, u_char *buf, int write_bytes)
{
        int    		 done, len;
        char  		*p;

        assert(audio_fd[ad] > 0);
        
        p   = (char *) buf;
        len = write_bytes;
        while (1) {
                if ((done = write(audio_fd[ad], p, len)) == len) {
                        break;
                }
                if (errno != EINTR) {
				perror("audio_write");
				return write_bytes - (len - done);
                }
                len -= done;
                p   += done;
        }
        return write_bytes;
}

/* Set ops on audio device to be non-blocking */
void
oss_audio_non_block(audio_desc_t ad)
{
	int  on = 1;

        assert(audio_fd[ad] > 0);

	if (ioctl(audio_fd[ad], FIONBIO, (char *)&on) < 0) {
		debug_msg("Failed to set non-blocking mode on audio device!\n");
	}
}

/* Set ops on audio device to block */
void
oss_audio_block(audio_desc_t ad)
{
	int  on = 0;

        assert(audio_fd[ad] > 0);

	if (ioctl(audio_fd[ad], FIONBIO, (char *)&on) < 0) {
		debug_msg("Failed to set blocking mode on audio device!\n");
	}
}

void
oss_audio_oport_set(audio_desc_t ad, audio_port_t port)
{
	/* There appears to be no-way to select this with OSS... */
        assert(audio_fd[ad] > 0);
	UNUSED(port);
	return;
}

audio_port_t
oss_audio_oport_get(audio_desc_t ad)
{
	/* There appears to be no-way to select this with OSS... */
        assert(audio_fd[ad] > 0);
	return out_ports[0].port;
}

int 
oss_audio_oport_count(audio_desc_t ad)
{
        UNUSED(ad);
        return (int)NUM_OUT_PORTS;
}

const audio_port_details_t*
oss_audio_oport_details(audio_desc_t ad, int idx)
{
        UNUSED(ad);
        if (idx >= 0 && idx < (int)NUM_OUT_PORTS) {
                return &out_ports[idx];
        }
        return NULL;
}

void
oss_audio_iport_set(audio_desc_t ad, audio_port_t port)
{
	int recmask, portmask;
	int recsrc;
	int gain;

        UNUSED(ad); assert(audio_fd[ad] > 0);

	if (ioctl(audio_fd[ad], MIXER_READ(SOUND_MIXER_RECMASK), &recmask) == -1) {
		debug_msg("WARNING: Unable to read recording mask!\n");
		return;
	}

        switch (port) {
		case AUDIO_MICROPHONE: 
			recsrc = SOUND_MASK_MIC;  
			break;
		case AUDIO_LINE_IN:    
			recsrc = SOUND_MASK_LINE; 
			break;
		case AUDIO_CD:         
			recsrc = SOUND_MASK_CD;   
			break;
		default:
			debug_msg("Port not recognized\n");
			return;
        }

        /* Can we select chosen port ? */
        if (recmask & recsrc) {
                portmask = recsrc;
                if ((ioctl(audio_fd[ad], MIXER_WRITE(SOUND_MIXER_RECSRC), &recsrc) == -1) && !(recsrc & portmask)) {
                        debug_msg("WARNING: Unable to select recording source!\n");
                        return;
                }
                gain = oss_audio_get_igain(ad);
                iport = port;
                oss_audio_set_igain(ad, gain);
        } else {
                debug_msg("Audio device doesn't support recording from port %d\n", port);
        }
}

audio_port_t
oss_audio_iport_get(audio_desc_t ad)
{
        assert(audio_fd[ad] > 0);
	return iport;
}

int
oss_audio_iport_count(audio_desc_t ad)
{
        UNUSED(ad);
        return (int)NUM_IN_PORTS;
}

const audio_port_details_t*
oss_audio_iport_details(audio_desc_t ad, int idx)
{
        UNUSED(ad);
        if (idx >= 0 && idx < (int)NUM_IN_PORTS) {
                return &in_ports[idx];
        }
        return NULL;
}

static int
oss_audio_select(audio_desc_t ad, int delay_us)
{
        fd_set rfds;
        struct timeval tv;

        assert(audio_fd[ad] > 0);
        
        tv.tv_sec = 0;
        tv.tv_usec = delay_us;

        FD_ZERO(&rfds);
        FD_SET(audio_fd[ad], &rfds);

        select(audio_fd[ad]+1, &rfds, NULL, NULL, &tv);

        return FD_ISSET(audio_fd[ad], &rfds);
}

void
oss_audio_wait_for(audio_desc_t ad, int delay_ms)
{
        oss_audio_select(ad, delay_ms * 1000);
}

int 
oss_audio_is_ready(audio_desc_t ad)
{
        return oss_audio_select(ad, 0);
}

static int
oss_set_mode(audio_desc_t ad, int speed, int stereo)
{
        int sp, st;

	debug_msg("Testing support for %dHz %s...\n", speed, stereo?"stereo":"mono");
        ioctl(audio_fd[ad], SNDCTL_DSP_RESET, 0);

        st = stereo;
        if (ioctl(audio_fd[ad], SNDCTL_DSP_STEREO, &st) == -1 || st != stereo) {
		return FALSE;
        }

        sp = speed;
        if (ioctl(audio_fd[ad], SNDCTL_DSP_SPEED, &sp) == -1) {
		return FALSE;
        }
	if (sp != speed) {
		debug_msg("Sampling clock skew: %d should be %d\n", sp, speed);
	}
	if (((100 * abs(sp - speed)) / speed) > 5) {
		debug_msg("Sampling clock skew of more than 5%, mode disabled\n");
		return FALSE;
	}
        
        return TRUE;
}

static int
oss_probe_formats(audio_desc_t ad)
{
	int	speed[] = {8000, 11025, 16000, 22050, 32000, 44100, 48000};
        int 	stereo;
	int	i;

	for (i = 0; i < 7; i++) {
                for (stereo = 0; stereo < 2; stereo++) {
                        if (oss_set_mode(ad, speed[i], stereo)) {
				af_sup[ad][n_af_sup[ad]].sample_rate = speed[i];
				af_sup[ad][n_af_sup[ad]].channels    = stereo + 1;
				n_af_sup[ad]++;
			}
                }
        }
        return TRUE;
}

int
oss_audio_supports(audio_desc_t ad, audio_format *fmt)
{
        int i;

        for(i = 0; i < n_af_sup[ad]; i++) {
                if (af_sup[ad][i].channels    == fmt->channels &&
                    af_sup[ad][i].sample_rate == fmt->sample_rate) return TRUE;
        }
        return FALSE;
}

int
oss_audio_query_devices(void)
{
	int	 	fd;
	mixer_info	info;

	ndev = 0;
	fd = open("/dev/mixer", O_RDWR);
	if (fd > 0) {
		if (ioctl(fd, SOUND_MIXER_INFO, &info) == 0) {
			strncpy(dev_name[ndev], info.name, OSS_MAX_NAME_LEN);
			purge_chars(dev_name[ndev],"\n");
			debug_msg("Mixer device found: %s\n", dev_name[ndev]);
			ndev++;
		} else {
			debug_msg("Cannot query mixer capabilities\n");
		}
		close(fd);
	} else {
		debug_msg("Cannot open /dev/mixer - no soundcard present?\n");
	}

	return ndev;
}

int
oss_get_device_count()
{
#ifdef HAVE_ALSA_AUDIO
  if (alsa_get_device_count() > 0)
    return 0;
#endif
  
    return ndev;
}

const char *
oss_get_device_name(audio_desc_t idx)
{
        if (idx >=0 && idx < ndev) {
                return dev_name[idx];
        }
        debug_msg("Invalid index\n");
        return NULL;
}
