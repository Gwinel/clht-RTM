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

#ifndef _SLR_SCM_LOCK_H_
#define _SLR_SCM_LOCK_H_

#include "primitives.h"
#include "mcs.h"

#define TXN_MAY_SUCCEED				0x02
#define INTERNAL_BUFFER_OVERFLOWED		0x08

typedef struct {
        uint32_t		lock;
        uint32_t 		pad[128/4 - 1];
} spec_ttas_lock_t;

typedef struct {
        uint32_t		lock;
        uint32_t 		pad[128/4 - 1];
        uint32_t		aux_lock;
        uint32_t		pad1[128/4 - 1];
        uint64_t		aux_lock_owner;
        uint32_t		aux_retries;
        uint32_t 		pad2[128/4 - 3];
} spec_aux_ttas_lock_t;

typedef struct {
        mcs_lock_t		lock;
        uint32_t		pad1[128/4 - sizeof(mcs_lock_t)];
} spec_mcs_lock_t;

typedef struct {
        mcs_lock_t		lock;
        uint32_t 		pad[128/4 - 1];
        mcs_lock_t		aux_lock;
        uint32_t		pad1[128/4 - sizeof(mcs_lock_t)];
        uint64_t		aux_lock_owner;
        uint32_t		aux_retries;
        uint32_t 		pad2[128/4 - 3];
} spec_aux_mcs_lock_t;

#endif /* _SLR_SCM_LOCK_H_ */




