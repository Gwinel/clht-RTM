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

#ifndef _STATS_H_
#define _STATS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "primitives.h"

typedef struct {
        uint64_t owner;
        uint64_t locks;
        uint64_t contended;
        uint64_t speculative;
        uint64_t failures;
        uint64_t reasons[64];
} stats_t;

void locklib_stats_next_stage(void);
void locklib_stats_next_window(void);
stats_t* locklib_stats_get(void);

#define STATS_SIZE		1023
#define THREAD_STATS_SIZE	8
#define LOCKLIB_STAT_WINDOWS	4000

#if defined(KEEP_STATS) || defined(LOCKLIB_LOCK_IMPL)

static stats_t thread_stats[STATS_SIZE][THREAD_STATS_SIZE][LOCKLIB_STAT_WINDOWS];

typedef stats_t stats_windows_t[LOCKLIB_STAT_WINDOWS];

static uint64_t stats_stage;

#endif

#ifdef KEEP_STATS

static __thread stats_windows_t *stats;

static uint64_t pthread_to_idx(pthread_t id)
{
        return (((uint64_t) id) >> 12) % STATS_SIZE;
}

static inline void stats_begin(void)
{
        if (likely(stats != NULL))
                return;

        uint64_t id = pthread_to_idx(pthread_self());

        for (;;) {
                stats = &thread_stats[id][0];

                if (__sync_lock_test_and_set(&stats[0]->owner, (void*) id) == 0) {
                        return;
                }

                id = (id + 1) % STATS_SIZE;
        }
}

#define stats_count(f) { \
	uint64_t x = stats_stage; \
	stats[(uint32_t)(x >> 32)][(uint32_t)(x)].f++; \
}

static inline void stats_next_stage(void)
{
        stats_stage += (1ull << 32);

}

static inline void stats_next_window(void)
{
        stats_stage++;
}

static inline stats_t* stats_get(void)
{
        return stats[(uint32_t)(stats_stage >> 32)];
}

#else

static inline void stats_next_stage(void) { }
static inline void stats_begin(void) { }
static inline void stats_next_window(void) { }
static inline stats_t* stats_get(void)
{
        return NULL;
}
#define stats_count(f)

#endif /* KEEP_STATS */

#ifdef LOCKLIB_LOCK_IMPL

static void stats_display(int stage)
{
        stats_t all_stats;
        int i, j, w;

        memset(&all_stats, 0, sizeof(all_stats));

        for (i = 0; i < STATS_SIZE; i++) {

                if (thread_stats[i][0][0].owner == 0)
                        continue;

                for (w = 0; w < LOCKLIB_STAT_WINDOWS; w++) {
                        stats_t *s = &thread_stats[i][stage][w];
                        all_stats.locks += s->locks;
                        all_stats.contended += s->contended;
                        all_stats.speculative += s->speculative;
                        all_stats.failures += s->failures;

                        for (j = 0; j < 64; j++)
                                all_stats.reasons[j] += s->reasons[j];
                }
        }

        printf(" stage=%d locks=%ld contended=%ld speculative=%ld failures=%ld", stage, all_stats.locks, all_stats.contended, all_stats.speculative, all_stats.failures);
        printf(" reasons=");
        for (j = 0; j < 64; j++) {
                if (all_stats.reasons[j] == 0)
                        continue;
                printf("0x%x->%ld ", j, all_stats.reasons[j]);
        }
}

static void stats_display_all(int status, void *arg)
{
        int s, nstages = (uint32_t)(stats_stage >> 32);

        printf("LOCKLIB_STATS: name=%s", getenv("LD_PRELOAD"));

        for (s = 0; s <= nstages; s++)
                stats_display(s);

        printf("\n");
}

static __attribute ((constructor)) void register_onexit(void)
{
        if (on_exit(stats_display_all, 0) != 0) {
                fprintf(stderr, "registering on_exit failed\n");
                abort();
        }
}

#endif /* LOCKLIB_LOCK_IMPL */

#endif /* _STATS_H_ */

