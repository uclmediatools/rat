/*
 * FILE:    net.c 
 * PROGRAM: rat - Version 3.0
 * 
 * Modified by Isidor Kouvelas from NT source.
 * 
 * $Revision$ $Date$
 * 
 * Modified by V.J.Hardman 2/3/95 All the network specific routines have been
 * moved to this module
 * 
 * Add queues functionality to simplify interface to rest of program
 * 
 * 21/3/95 Isidor fixed the network select to work for Solaris
 * 
 * Copyright (c) 1995,1996 University College London
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

#include "crypt.h"
#include "net.h"
#include "session.h"
#include "interfaces.h"
#include "audio.h"
#include "util.h"
#include "mbus.h"

#ifndef INADDR_NONE
#define INADDR_NONE     0xffffffff
#endif

void 
network_init(session_struct *sp)
{
	struct in_addr in;
	struct hostent *h;
	sp->net_maddress = get_net_addr(sp->asc_address);  

	if (inet_addr(sp->asc_address) != INADDR_NONE) {
		strcpy(sp->maddress, sp->asc_address);
	} else if ((h = gethostbyname(sp->asc_address))!=NULL) {
		memcpy(&in.s_addr, *(h->h_addr_list), sizeof(in.s_addr));
		sprintf(sp->maddress, "%s", inet_ntoa(in));
	} else {
		fprintf(stderr, "Could not resolve hostname (h_errno = %d): %s", h_errno, sp->asc_address);
		exit(-1);
	}
	
	sp->our_address  = get_net_addr(NULL);
	sp->rtp_fd  = sock_init(sp->net_maddress, sp->rtp_port,  sp->ttl);
	sp->rtcp_fd = sock_init(sp->net_maddress, sp->rtcp_port, sp->ttl);
}

u_long 
get_net_addr(char *dhost)
{
	char            dhosta[MAXHOSTNAMELEN];
	u_long          inaddr = 0;
	struct hostent *addr;

	if (dhost == 0) {
		gethostname(dhosta, MAXHOSTNAMELEN);
		dhost = dhosta;
	}
	if ((inaddr = inet_addr(dhost)) == INADDR_NONE) {
		if ((addr = gethostbyname(dhost)) == NULL) {
			fprintf(stderr, "bad hostname(h_errno = %d): %s\n", \
				h_errno, dhost);
			exit(-1);
		}
#ifndef h_addr
		memcpy(&inaddr, addr->h_addr, sizeof(inaddr));
#else				/* h_addr */
		memcpy(&inaddr, addr->h_addr_list[0], sizeof(inaddr));
#endif				/* h_addr */
	}
	return (inaddr);
}

int
sock_init(u_long inaddr, int port, int t_flag)
{
	struct sockaddr_in sinme;
#ifndef WIN32
	char            ttl = (char)t_flag;
#else
	int		ttl = t_flag;
#endif
	int             tmp_fd;
	int             multi = FALSE;
	int             reuse = 1;

	assert(port > 1024); 	/* ports < 1024 are reserved on unix */

	/*
	 * Must examine inaddr to see if this is a multicast address and set
	 * multi to TRUE
	 */
	multi = IN_MULTICAST(ntohl(inaddr));

	memset((char *) &sinme, 0, sizeof(sinme));

	if ((tmp_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	if (setsockopt(tmp_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(reuse)) < 0) {
		perror("setsockopt SO_REUSEADDR");
		exit(1);
	}
#ifdef SO_REUSEPORT
	if (setsockopt(tmp_fd, SOL_SOCKET, SO_REUSEPORT, (char *) &reuse, sizeof(reuse)) < 0) {
		perror("setsockopt SO_REUSEPORT");
		return -1;
	}
#endif

	sinme.sin_family = AF_INET;
	sinme.sin_addr.s_addr = htonl(INADDR_ANY);
	sinme.sin_port = htons(port);
	if (bind(tmp_fd, (struct sockaddr *) & sinme, sizeof(sinme)) < 0) {
		perror("bind");
		exit(1);
	}

	if (multi) {
		char            loop = 1;
		struct ip_mreq  imr;

		imr.imr_multiaddr.s_addr = inaddr;
		imr.imr_interface.s_addr = htonl(INADDR_ANY);

		if (setsockopt(tmp_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &imr, sizeof(struct ip_mreq)) == -1) {
			fprintf(stderr, "IP multicast join failed!\n");
		}

		setsockopt(tmp_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

		if (setsockopt(tmp_fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
			perror("setsockopt IP_MULTICAST_TTL");
	}
	return tmp_fd;
}

int
net_write(int fd, u_long addr, int port, unsigned char *msg, int msglen, int type)
{
	struct sockaddr_in name;
	int             ret;
	char*		encrypted_msg = NULL;

	if (Null_Key()==0) {
	  switch (type) {
	    case PACKET_RTP    : encrypted_msg=Encrypt(msg, &msglen);
                                 break;
            case PACKET_RTCP   : encrypted_msg=Encrypt_Ctrl(msg, &msglen);
                                 break;
	    default            : printf("oops. net_write type is broken!\n");
	                         abort();
	  }
	} else {
	  encrypted_msg = msg;
	}

	memcpy((char *) &name.sin_addr.s_addr, (char *) &addr, sizeof(addr));
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	if ((ret = sendto(fd, (char *) encrypted_msg, msglen, 0, (struct sockaddr *) & name, sizeof(name))) < 0) {
		perror("net_write: sendto");
	}
	return ret;
}

#define MAXPACKETSIZE    (1500-28)
int net_write_iov(int fd, u_long addr, int port, struct iovec *iov, int len, int type)
{
	unsigned char  wrkbuf[MAXPACKETSIZE];
	unsigned char *cp;
	unsigned char *ep;

	for (cp = wrkbuf, ep = wrkbuf + MAXPACKETSIZE; --len >= 0; ++iov) {
		int plen = iov->iov_len;
		if (cp + plen >= ep) {
			errno = E2BIG;
			return -1;
		}
		memcpy(cp, iov->iov_base, plen);
		cp += plen;
	}
	return net_write(fd, addr, port, wrkbuf, cp - wrkbuf, type);
}

/* This function is used for both rtp and rtcp packets */
static pckt_queue_element_struct *
read_net(int fd, u_int32 cur_time, int type, int *nbdecryption)
{
	char           *data_in, *data_out, *tmp_data;
	struct sockaddr from;
	int             fromlen, read_len;
	pckt_queue_element_struct *pckt;

	/*
	 * Would be better to have a parameter telling us whether this is an
	 * rtp or rtcp packet to allocate right amount of space. Of course
	 * this way we can use the same packet pool later on.
	 *
	 * We've got the parameter now, but I can't be bothered to fix the code :-) [csp]
	 */

	data_in  = block_alloc(PACKET_LENGTH);
	data_out = block_alloc(PACKET_LENGTH);

	fromlen = sizeof(from);
	if ((read_len = recvfrom(fd, data_in, PACKET_LENGTH, 0, &from, &fromlen)) > 0) {
		if (Null_Key()==0) {
			switch (type) {
			case PACKET_RTP:
				*nbdecryption = Decrypt(data_in, data_out, &read_len);
				break;
			case PACKET_RTCP:
				*nbdecryption = Decrypt_Ctrl(data_in, data_out, &read_len);
				break;
			default:
				printf("oops. read_net type is broken!\n");
				abort();
			}
		} else {
			tmp_data      = data_out;
			data_out      = data_in;
			data_in       = tmp_data;
			*nbdecryption = 0;
		}
		pckt = (pckt_queue_element_struct *) block_alloc(sizeof(pckt_queue_element_struct));
		pckt->len               = read_len;
		pckt->pckt_ptr          = data_out;
		pckt->next_pckt_ptr     = NULL;
		pckt->prev_pckt_ptr     = NULL;
		pckt->addr              = ((struct sockaddr_in *) & from)->sin_addr.s_addr;
		pckt->arrival_timestamp = cur_time;
		block_free(data_in, PACKET_LENGTH);
		return pckt;
	} else {
		block_free(data_in, PACKET_LENGTH);
		block_free(data_out, PACKET_LENGTH);
		return NULL;
	}
}


static void
read_packets_and_add_to_queue(int fd, u_int32 cur_time, pckt_queue_struct * queue, int type)
{
	int             l, nb, nbdecryption;
	pckt_queue_element_struct pckt;
	pckt_queue_element_struct *pckt_ptr = &pckt;

#ifndef WIN32
	if (ioctl(fd, FIONREAD, (caddr_t) & nb) != 0) {
		perror("read_all.../FIONREAD");
	}
	for (l = 0; l < 2 && nb > 0; l++) {
		if ((pckt_ptr = read_net(fd, cur_time, type, &nbdecryption)) != NULL) {
			/* There is a bug in SUNOS, IRIX and HPUX, which */
                        /* means that we need to subtract 16 more bytes  */
			/* on each read to get it right...               */
			nb -= pckt_ptr->len;
			nb -= nbdecryption;
#if defined(SunOS_4) || defined(IRIX) || defined(HPUX) || defined(FreeBSD)
			nb -= 16;
#endif
			put_on_pckt_queue(pckt_ptr, queue);
		} else {
			break;
		}
	}
#else				/* WIN32 */
	fd_set          rfds;
	struct timeval  timeout;
	do {
		if ((pckt_ptr = read_net(fd, cur_time, type)) != NULL)
			put_on_pckt_queue(pckt_ptr, queue);
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
	} while (select(fd + 1, &rfds, (fd_set *) 0, (fd_set *) 0, &timeout) > 0);
#endif				/* WIN32 */
}

void
network_read(session_struct    *sp,
	     pckt_queue_struct *netrx_pckt_queue_ptr,
	     pckt_queue_struct *rtcp_pckt_queue_ptr,
	     u_int32       cur_time)
{
	int             sel_fd;
	struct timeval  timeout, *tvp;
	fd_set          rfds;

	sel_fd = sp->rtp_fd;
	sel_fd = max(sel_fd, sp->rtcp_fd);
	sel_fd = max(sel_fd, mbus_fd(sp->mbus_engine_base));
     	sel_fd = max(sel_fd, mbus_fd(sp->mbus_ui_base));
	if (sp->mbus_channel != 0) {
		sel_fd = max(sel_fd, mbus_fd(sp->mbus_engine_chan));
     		sel_fd = max(sel_fd, mbus_fd(sp->mbus_ui_chan));
	}
#if !defined(WIN32)
	if (sp->mode == AUDIO_TOOL) {
		sel_fd = max(sel_fd, sp->audio_fd);
	}
#endif
	sel_fd++;

	for (;;) {
		FD_ZERO(&rfds);
		FD_SET(sp->rtp_fd,                    &rfds);
		FD_SET(sp->rtcp_fd,                   &rfds);
                FD_SET(mbus_fd(sp->mbus_engine_base), &rfds);
     		FD_SET(mbus_fd(sp->mbus_ui_base),     &rfds);
		if (sp->mbus_channel != 0) {
                	FD_SET(mbus_fd(sp->mbus_engine_chan), &rfds);
     			FD_SET(mbus_fd(sp->mbus_ui_chan),     &rfds);
		}
#if defined(WIN32) || defined(HPUX) || defined(Linux) || defined(FreeBSD)
		timeout.tv_sec  = 0;
		timeout.tv_usec = sp->loop_delay;
		tvp = &timeout;
#else
		if ((sp->audio_fd != -1) && (sp->mode == AUDIO_TOOL)) {
			FD_SET(sp->audio_fd, &rfds);
			tvp = NULL;
		} else {
			/* If we dont have control of the audio device then */
			/* use select to do a timeout at 20ms               */
			timeout.tv_sec  = 0;
			timeout.tv_usec = sp->loop_delay;
			tvp = &timeout;
		}
#endif
#ifdef HPUX
		if (select(sel_fd, (int *) &rfds, NULL, NULL, tvp) > 0) {
#else
		if (select(sel_fd, &rfds, (fd_set *) 0, (fd_set *) 0, tvp) > 0) {
#endif
			if (FD_ISSET(sp->rtp_fd, &rfds)) {
				read_packets_and_add_to_queue(sp->rtp_fd, cur_time, netrx_pckt_queue_ptr, PACKET_RTP);
			}
			if (FD_ISSET(sp->rtcp_fd, &rfds)) {
				read_packets_and_add_to_queue(sp->rtcp_fd, cur_time, rtcp_pckt_queue_ptr, PACKET_RTCP);
			}
                        if (FD_ISSET(mbus_fd(sp->mbus_engine_base), &rfds)) {
     				mbus_recv(sp->mbus_engine_base, (void *) sp);
     			}
     			if (FD_ISSET(mbus_fd(sp->mbus_ui_base), &rfds)) {
     				mbus_recv(sp->mbus_ui_base, (void *) sp);
     			}
			if (sp->mbus_channel != 0) {
                        	if (FD_ISSET(mbus_fd(sp->mbus_engine_chan), &rfds)) {
     					mbus_recv(sp->mbus_engine_chan, (void *) sp);
     				}
     				if (FD_ISSET(mbus_fd(sp->mbus_ui_chan), &rfds)) {
     					mbus_recv(sp->mbus_ui_chan, (void *) sp);
     				}
			}
                }
#if !defined(WIN32) && !defined(HPUX) && !defined(Linux) && !defined(FreeBSD)
		if (sp->mode == AUDIO_TOOL) {
			if (sp->audio_fd == -1 || FD_ISSET(sp->audio_fd, &rfds)) {
				break;
			}
		} else {
			break;
		}
#else
			break;
#endif
	}
}

void network_process_mbus(session_struct *sp[], int num_sessions, int delay)
{
	int             sel_fd = 0, i, rc;
	struct timeval  timeout, *tvp;
	fd_set          rfds;

	for (i=0; i<num_sessions; i++) {
		sel_fd = mbus_fd(sp[i]->mbus_engine_base);
     		sel_fd = max(sel_fd, mbus_fd(sp[i]->mbus_ui_base));
		if (sp[i]->mbus_channel != 0) {
			sel_fd = max(sel_fd, mbus_fd(sp[i]->mbus_engine_chan));
     			sel_fd = max(sel_fd, mbus_fd(sp[i]->mbus_ui_chan));
		}
		sel_fd++;
	}

	for (;;) {
		FD_ZERO(&rfds);
		for (i=0; i<num_sessions; i++) {	
                	FD_SET(mbus_fd(sp[i]->mbus_engine_base), &rfds);
     			FD_SET(mbus_fd(sp[i]->mbus_ui_base),     &rfds);
			if (sp[i]->mbus_channel != 0) {
                		FD_SET(mbus_fd(sp[i]->mbus_engine_chan), &rfds);
     				FD_SET(mbus_fd(sp[i]->mbus_ui_chan),     &rfds);
			}
		}
		timeout.tv_sec  = 0;
		timeout.tv_usec = delay;
		tvp = &timeout;
#ifdef HPUX
		rc = select(sel_fd, (int *) &rfds, NULL, NULL, tvp);
#else
		rc = select(sel_fd, &rfds, (fd_set *) 0, (fd_set *) 0, tvp);
#endif
		if (rc > 0) {
			for (i=0; i<num_sessions; i++) {
                        	if (FD_ISSET(mbus_fd(sp[i]->mbus_engine_base), &rfds)) {
     					mbus_recv(sp[i]->mbus_engine_base, (void *) sp[i]);
     				}
     				if (FD_ISSET(mbus_fd(sp[i]->mbus_ui_base), &rfds)) {
     					mbus_recv(sp[i]->mbus_ui_base, (void *) sp[i]);
     				}
				if (sp[i]->mbus_channel != 0) {
                        		if (FD_ISSET(mbus_fd(sp[i]->mbus_engine_chan), &rfds)) {
     						mbus_recv(sp[i]->mbus_engine_chan, (void *) sp[i]);
     					}
     					if (FD_ISSET(mbus_fd(sp[i]->mbus_ui_chan), &rfds)) {
     						mbus_recv(sp[i]->mbus_ui_chan, (void *) sp[i]);
     					}
				}
                	}
		} else if (rc == 0) {
			return;
		} else {
			perror("Waiting for Mbus to settle");
			abort();
		}
	}
}

