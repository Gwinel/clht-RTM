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

#ifndef _SCM_TTAS_H_
#define _SCM_TTAS_H_

#include "primitives.h"
#include "mcs.h"
#include "ticket.h"

#define LOCK_IS_TAKEN			1
#define	XABORT_ARG_POS			24
#define	USER_XABORT_POS			0
#define	USER_XABORT   			1<<USER_XABORT_POS
#define	MAX_AUX_PATH_RETRIES	        10
#define	INVALID_THREAD_HANDLE	        0

typedef struct {
        uint32_t		lock;
        uint32_t 		pad[128/4 - 1];
        uint32_t		aux_lock;
        uint32_t		pad1[128/4 - 1];
        uint64_t		aux_lock_owner;
        uint32_t		aux_retries;
        uint64_t		lock_owner;
        uint32_t 		pad2[128/4 - 5];
} scm_ttas_lock_t;

typedef struct {
        uint32_t		lock;
        uint32_t 		pad[128/4 - 1];
        mcs_lock_t		aux_lock;
        uint32_t		pad1[128/4 - sizeof(mcs_lock_t)];
        uint64_t		aux_lock_owner;
        uint32_t		aux_retries;
        uint64_t		lock_owner;
        uint32_t 		pad2[128/4 - 5];
} scm_ttas_mcs_lock_t;

typedef struct {
        uint32_t		lock;
        uint32_t 		pad[128/4 - 1];
        ticket_lock_t	aux_lock;
        uint32_t		pad1[128/4 - sizeof(mcs_lock_t)];
        uint64_t		aux_lock_owner;
        uint32_t		aux_retries;
        uint64_t		lock_owner;
        uint32_t 		pad2[128/4 - 5];
} scm_ttas_ticket_lock_t;

#endif /* _SCM_TTAS_H_ */




