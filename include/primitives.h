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

#ifndef _PRIMITIVES_H_
#define _PRIMITIVES_H_

#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <math.h>
#include <stdint.h>
#include <sys/timeb.h>
#include <sys/mman.h>
#include <malloc.h>

#include "defs.h"
#include "stats.h"

#ifdef __GNUG__
extern "C" {
#endif

#define barrier()                  asm volatile ("": : :"memory")
#define FullFence()                asm volatile ("mfence":::"memory")

#define XBEGIN(abort_label, abort_reason)       \
   asm goto(                                    \
       ".byte 0xc7, 0xf8, 0x2, 0x0, 0x0, 0x0\n" \
       "1:\n"                                   \
       "jmp 3f\n"                               \
       "2:\n"                                   \
       "mov %%eax,%0\n"                         \
       "jmp %l1 \n"                             \
       "3:\n"                                   \
: : "m"(abort_reason) :"memory" : abort_label)


inline static void XEND(void) {
        asm(".byte 0xf, 0x1, 0xd5":::"memory");
}

inline static bool XTEST(void) {
        bool in_txn;
        asm(".byte 0xf, 0x1, 0xd6;"
            "setnz %b0" : "=r"(in_txn) : "0"(0) : "cc");
        return in_txn;
}

#define XABORT(why) \
        asm(".byte 0xc6, 0xf8, " #why )


/* REP NOP (PAUSE) is a good thing to insert into busy-wait loops. */
static inline void rep_nop(void) {
        asm volatile("rep; nop" ::: "memory");
}

static inline void cpu_relax(void) {
        rep_nop();
#if defined(HTM_HLE)
        if (XTEST()) XABORT(253);
#endif
}


inline static bool CASPTR(void *A, void *B, void *C) {
        uint64_t prev;
        uint64_t *p = (uint64_t *)A;

        asm volatile("lock;cmpxchgq %1,%2"
                     : "=a"(prev)
                     : "r"((uint64_t)C), "m"(*p), "0"((uint64_t)B)
                     : "memory", "cc");
        return (prev == (uint64_t)B);
}

inline static bool XACQUIRE_CASPTR(void *A, void *B, void *C) {
        uint64_t prev;
        uint64_t *p = (uint64_t *)A;

        asm volatile(".byte 0xf2;lock;cmpxchgq %1,%2"
                     : "=a"(prev)
                     : "r"((uint64_t)C), "m"(*p), "0"((uint64_t)B)
                     : "memory", "cc");
        return (prev == (uint64_t)B);
}

inline static bool XRELEASE_CASPTR(void *A, void *B, void *C) {
        uint64_t prev;
        uint64_t *p = (uint64_t *)A;

        asm volatile(".byte 0xf3;lock;cmpxchgq %1,%2"
                     : "=a"(prev)
                     : "r"((uint64_t)C), "m"(*p), "0"((uint64_t)B)
                     : "memory", "cc");
        return (prev == (uint64_t)B);
}

inline static bool HTM_ACQUIRE_CASPTR(void *A, void *B, void *C) {
        uint64_t *ptr = (uint64_t*)A;
        uint32_t reason;

        XBEGIN(onabort, reason);
        return (*ptr == (uint64_t)B);

onabort:
        stats_count(failures);
        stats_count(reasons[reason & 63]);
        return CASPTR(A, B, C);
}

inline static bool HTM_RELEASE_CASPTR(void *A, void *B, void *C) {
        if (!XTEST())
                return CASPTR(A, B, C);

        uint64_t *ptr = (uint64_t*)A;

        if (*ptr != (uint64_t)C)
                XABORT(255);

        XEND();
        return true;
}

inline static bool CAS64(uint64_t *A, uint64_t B, uint64_t C) {
        uint64_t prev;
        uint64_t *p = (uint64_t *)A;

        asm volatile("lock;cmpxchgq %1,%2"
                     : "=a"(prev)
                     : "r"(C), "m"(*p), "0"(B)
                     : "memory", "cc");
        return (prev == B);
}

inline static bool XACQUIRE_CAS64(uint64_t *A, uint64_t B, uint64_t C) {
        uint64_t prev;
        uint64_t *p = (uint64_t *)A;

        asm volatile(".byte 0xf2;lock;cmpxchgq %1,%2"
                     : "=a"(prev)
                     : "r"(C), "m"(*p), "0"(B)
                     : "memory", "cc");
        return (prev == B);
}

inline static bool XRELEASE_CAS64(uint64_t *A, uint64_t B, uint64_t C) {
        uint64_t prev;
        uint64_t *p = (uint64_t *)A;

        asm volatile(".byte 0xf3;lock;cmpxchgq %1,%2"
                     : "=a"(prev)
                     : "r"(C), "m"(*p), "0"(B)
                     : "memory", "cc");
        return (prev == B);
}

inline static bool HTM_ACQUIRE_CAS64(uint64_t *A, uint64_t B, uint64_t C) {
        uint32_t reason;

        XBEGIN(onabort, reason);
        return (*A == B);

onabort:
        stats_count(failures);
        stats_count(reasons[reason & 63]);
        return CAS64(A, B, C);
}

inline static bool HTM_RELEASE_CAS64(uint64_t *A, uint64_t B, uint64_t C) {
        if (!XTEST())
                return CAS64(A, B, C);

        if (*A != C)
                XABORT(255);

        XEND();
        return true;
}

inline static bool CAS32(uint32_t *A, uint32_t B, uint32_t C) {
        uint32_t prev;
        uint32_t *p = (uint32_t *)A;

        asm volatile("lock;cmpxchgl %1,%2"
                     : "=a"(prev)
                     : "r"(C), "m"(*p), "0"(B)
                     : "memory", "cc");
        return (prev == B);
}

inline static bool XACQUIRE_CAS32(uint32_t *A, uint32_t B, uint32_t C) {
        uint32_t prev;
        uint32_t *p = (uint32_t *)A;

        asm volatile(".byte 0xf2;lock;cmpxchgl %1,%2"
                     : "=a"(prev)
                     : "r"(C), "m"(*p), "0"(B)
                     : "memory", "cc");
        return (prev == B);
}

inline static bool XRELEASE_CAS32(uint32_t *A, uint32_t B, uint32_t C) {
        uint32_t prev;
        uint32_t *p = (uint32_t *)A;

        asm volatile(".byte 0xf3;lock;cmpxchgl %1,%2"
                     : "=a"(prev)
                     : "r"(C), "m"(*p), "0"(B)
                     : "memory", "cc");
        return (prev == B);
}

inline static bool HTM_ACQUIRE_CAS32(uint32_t *A, uint32_t B, uint32_t C) {
        uint32_t reason;

        XBEGIN(onabort, reason);
        return (*A == B);

onabort:
        stats_count(failures);
        stats_count(reasons[reason & 63]);
        return CAS32(A, B, C);
}

inline static bool HTM_RELEASE_CAS32(uint32_t *A, uint32_t B, uint32_t C) {
        if (!XTEST())
                return CAS32(A, B, C);

        if (*A != C)
                XABORT(255);

        XEND();
        return true;
}

inline static void *SWAP(void *A, void *B) {
        uint64_t *p = (uint64_t *)A;

        asm volatile("lock;"
                     "xchgq %0, %1"
                     : "=r"(B), "=m"(*p)
                     : "0"(B), "m"(*p)
                     : "memory");
        return B;
}

inline static void *XACQUIRE_SWAP(void *A, void *B) {
        uint64_t *p = (uint64_t *)A;

        asm volatile(".byte 0xf2;lock;"
                     "xchgq %0, %1"
                     : "=r"(B), "=m"(*p)
                     : "0"(B), "m"(*p)
                     : "memory");
        return B;
}

inline static void *XRELEASE_SWAP(void *A, void *B) {
        uint64_t *p = (uint64_t *)A;

        asm volatile(".byte 0xf3;lock;"
                     "xchgq %0, %1"
                     : "=r"(B), "=m"(*p)
                     : "0"(B), "m"(*p)
                     : "memory");
        return B;
}

inline static void *HTM_ACQUIRE_SWAP(void *A, void *B) {
        uint32_t reason;
        uint64_t a = 0;

        XBEGIN(onabort, reason);
        a = *((uint64_t *)A);
        return (void *) a;

onabort:
        stats_count(failures);
        stats_count(reasons[reason & 63]);
        return SWAP(A, B);
}

inline static void *HTM_RELEASE_SWAP(void *A, void *B) {
        if (!XTEST())
                return SWAP(A, B);

        uint64_t a = *((uint64_t *)A);

        if (a != (uint64_t)B)
                XABORT(255);

        XEND();
        return (void *)a;
}

inline static uint64_t FAA64(uint64_t *A, uint64_t B) {
        asm("lock; xaddq %0,%1": "+r" (B), "+m" (*(A)): : "memory", "cc");
        return B;
}

inline static uint64_t XACQUIRE_FAA64(uint64_t *A, uint64_t B) {
        asm(".byte 0xf2; lock; xaddq %0,%1": "+r" (B), "+m" (*(A)): : "memory", "cc");
        return B;
}

inline static uint64_t XRELEASE_FAA64(uint64_t *A, uint64_t B) {
        asm(".byte 0xf3; lock; xaddq %0,%1": "+r" (B), "+m" (*(A)): : "memory", "cc");
        return B;
}

inline static uint32_t FAA32(uint32_t *A, uint32_t B) {
        asm("lock; xaddl %0,%1": "+r" (B), "+m" (*(A)): : "memory", "cc");
        return B;
}

inline static uint32_t XACQUIRE_FAA32(uint32_t *A, uint32_t B) {
        asm(".byte 0xf2; lock; xaddl %0,%1": "+r" (B), "+m" (*(A)): : "memory", "cc");
        return B;
}

inline static uint32_t XRELEASE_FAA32(uint32_t *A, uint32_t B) {
        asm(".byte 0xf3; lock; xaddl %0,%1": "+r" (B), "+m" (*(A)): : "memory", "cc");
        return B;
}

inline static uint32_t HTM_ACQUIRE_FAA32(uint32_t *A, uint32_t B) {
        uint32_t reason;
        uint32_t a = 0;

        XBEGIN(onabort, reason);
        a = *A;
        return a;

onabort:
        stats_count(failures);
        stats_count(reasons[reason & 63]);
        return FAA32(A, B);
}

inline static void STORE64(uint64_t *A, uint32_t v) {
        asm volatile("movq %1, %0\n" : "+m"(*A) : "ri"(v) : "memory");
}

inline static void XRELEASE_STORE64(uint64_t *A, uint32_t v) {
        asm volatile(".byte 0xf3; movq %1, %0\n" : "+m"(*A) : "ri"(v) : "memory");
}

inline static void HTM_RELEASE_STORE64(uint64_t *A, uint32_t v) {
        if (!XTEST()) {
                STORE64(A, v);
                return;
        }

        if (*A != v)
                XABORT(255);

        XEND();
}

inline static void STORE32(uint32_t *A, uint32_t v) {
        asm volatile("movl %1, %0\n" : "+m"(*A) : "ri"(v) : "memory");
}

inline static void XRELEASE_STORE32(uint32_t *A, uint32_t v) {
        asm volatile(".byte 0xf3; movl %1, %0\n" : "+m"(*A) : "ri"(v) : "memory");
}

inline static void HTM_RELEASE_STORE32(uint32_t *A, uint32_t v) {
        if (!XTEST()) {
                STORE32(A, v);
                return;
        }

        if (*A != v)
                XABORT(255);

        XEND();
}

#if defined(HLE)

#define ACQUIRE_CASPTR	XACQUIRE_CASPTR
#define RELEASE_CASPTR	XRELEASE_CASPTR
#define ACQUIRE_CAS64   XACQUIRE_CAS64
#define RELEASE_CAS64   XRELEASE_CAS64
#define ACQUIRE_CAS32   XACQUIRE_CAS32
#define RELEASE_CAS32   XRELEASE_CAS32
#define ACQUIRE_SWAP    XACQUIRE_SWAP
#define RELEASE_SWAP    XRELEASE_SWAP
#define ACQUIRE_FAA64   XACQUIRE_FAA64
#define RELEASE_FAA64   XRELEASE_FAA64
#define ACQUIRE_FAA32   XACQUIRE_FAA32
#define RELEASE_FAA32   XRELEASE_FAA32
#define RELEASE_STORE64 XRELEASE_STORE64
#define RELEASE_STORE32 XRELEASE_STORE32

#elif defined(HTM_HLE)

#define ACQUIRE_CASPTR  HTM_ACQUIRE_CASPTR
#define RELEASE_CASPTR  HTM_RELEASE_CASPTR
#define ACQUIRE_CAS64   HTM_ACQUIRE_CAS64
#define RELEASE_CAS64   HTM_RELEASE_CAS64
#define ACQUIRE_CAS32   HTM_ACQUIRE_CAS32
#define RELEASE_CAS32   HTM_RELEASE_CAS32
#define ACQUIRE_SWAP    HTM_ACQUIRE_SWAP
#define RELEASE_SWAP    HTM_RELEASE_SWAP
#define ACQUIRE_FAA32   HTM_ACQUIRE_FAA32
#define RELEASE_STORE64 HTM_RELEASE_STORE64
#define RELEASE_STORE32 HTM_RELEASE_STORE32

#else

#define ACQUIRE_CASPTR  CASPTR
#define RELEASE_CASPTR  CASPTR
#define ACQUIRE_CAS64   CAS64
#define RELEASE_CAS64   CAS64
#define ACQUIRE_CAS32   CAS32
#define RELEASE_CAS32   CAS32
#define ACQUIRE_SWAP    SWAP
#define RELEASE_SWAP    SWAP
#define ACQUIRE_FAA64   FAA64
#define RELEASE_FAA64   FAA64
#define ACQUIRE_FAA32   FAA32
#define RELEASE_FAA32   FAA32
#define RELEASE_STORE64 STORE64
#define RELEASE_STORE32 STORE32

#endif /* HLE */


#ifdef __GNUG__
};
#endif

#endif /* _PRIMITIVES_H_ */
