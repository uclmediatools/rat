/*
 * FILE:    voxlet.h
 * PROGRAM: RAT
 * AUTHORS: Orion Hodson
 *
 * Copyright (c) 2000 University College London
 * All rights reserved.
 *
 * $Id$
 */

typedef struct s_voxlet voxlet_t;

int  voxlet_create  (voxlet_t          **ppv, 
                     struct s_mixer     *mixer, 
                     struct s_pdb       *pdb, 
                     const char         *sndfile);
int  voxlet_play    (voxlet_t           *ppv, 
                     ts_t                start, 
                     ts_t                end);
void voxlet_destroy (voxlet_t          **ppv);
