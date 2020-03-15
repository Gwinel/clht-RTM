// Copyright (c) 2014, Yehuda Afek, Amir Levy, and Adam Morrison.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materials provided with the
//    distribution.
//  * Neither the name of the Tel Aviv University nor the names of the
//    author of this software may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef _LOCKLIB_H_
#define _LOCKLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mcs.h"
#include "ttas.h"
#include "tas.h"
#include "scm_ttas.h"
#include "clh.h"
#include "ticket.h"
#include "slr_scm_lock.h"

#include <pthread.h>

typedef struct {
        union {
                mcs_lock_t 				mcs;
                ttas_lock_t 				ttas;
                tas_lock_t 				tas;
                scm_ttas_lock_t		scm_ttas;
                scm_ttas_mcs_lock_t	scm_ttas_mcs;
                clh_lock_t				clh;
                ticket_lock_t			ticket;
        };
        char cv_lock_pad[128];
        pthread_mutex_t cv_lock;
} locklib_mutex_t;

int locklib_mutex_lock(locklib_mutex_t *mutex);
int locklib_mutex_unlock(locklib_mutex_t *mutex);
int locklib_mutex_init(locklib_mutex_t *mutex, const void *attr);
int locklib_mutex_destroy(locklib_mutex_t *mutex);

int locklib_cond_timedwait(pthread_cond_t* cond, locklib_mutex_t* mutex, const struct timespec* ts);
int locklib_cond_wait(pthread_cond_t* cond, locklib_mutex_t* mutex);

#ifdef __cplusplus
}
#endif

#endif /* _LOCKLIB_H_ */

