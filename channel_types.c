/*
 * FILE:      channel_types.c
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

#include "channel_types.h"

#include "util.h"
#include "debug.h"

int 
channel_data_create(channel_data **ppcd, int nelem)
{
        channel_data *pcd;
        int i;

        *ppcd = NULL;
        pcd = (channel_data*)block_alloc(sizeof(channel_data));

        if (pcd) {
                memset(pcd, 0, sizeof(channel_data));
                for(i = 0; i < nelem; i++) {
                        pcd->elem[i] = (channel_unit*)block_alloc(sizeof(channel_unit));
                        if (pcd->elem[i] == NULL) {
                                pcd->nelem = i;
                                channel_data_destroy(&pcd, sizeof(channel_data));
                                return FALSE;
                        }
                        memset(pcd->elem[i], 0, sizeof(channel_unit));
                }
                pcd->nelem   = nelem;
                *ppcd = pcd;
                return TRUE;
        }
        return FALSE;
}

void
channel_data_destroy(channel_data **ppcd, u_int32 cd_size)
{
        channel_data *pcd;
        channel_unit *pcu;
        int i;

        pcd = *ppcd;
        assert(pcd != NULL);
        assert(cd_size == sizeof(channel_data));

        for(i = 0; i < pcd->nelem; i++) {
                pcu = pcd->elem[i];
                if (pcu->data) {
                        block_free(pcu->data, pcu->data_len);
                        pcu->data_len = 0;
                }
                assert(pcu->data_len == 0);
                block_free(pcu, sizeof(channel_unit));
        }
        block_free(pcd, sizeof(channel_data));
        *ppcd = NULL;
}