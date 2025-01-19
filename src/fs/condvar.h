#pragma once
#ifndef _FS_CONDVAR_
#define _FS_CONDVAR_

#include <common/sem.h>
#include <common/spinlock.h>
#include <common/defines.h>

struct condvar {
    u32 waitcnt; // number of waiting threads
    Semaphore sema;
};

static inline void cond_init(struct condvar *cv)
{
    init_sem(&cv->sema, 0);
    cv->waitcnt = 0;
}

// wait till cond_signal or cond_broadcast
extern void cond_wait(struct condvar *cv, SpinLock *lock);

// wake up only one sleeping thread(if any)
extern void cond_signal(struct condvar *cv);

// wake up all sleeping thread
extern void cond_broadcast(struct condvar *cv);

#endif
