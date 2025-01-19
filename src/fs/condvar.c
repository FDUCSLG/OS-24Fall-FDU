#include "condvar.h"

void cond_wait(struct condvar *cv, SpinLock *lock)
{
    _lock_sem(&cv->sema);
    // incr # waiting
    cv->waitcnt++;
    // release holding lock, and goto sleep
    release_spinlock(lock);
    // unalertable_wait_sem(&cv->sema);
    ASSERT(_wait_sem(&cv->sema, false));
    // reacquire lock
    acquire_spinlock(lock);
}

void cond_signal(struct condvar *cv)
{
    _lock_sem(&cv->sema);
    if (cv->waitcnt == 0) {
        _unlock_sem(&cv->sema);
        return;
    }
    cv->waitcnt--;
    _post_sem(&cv->sema);
    _unlock_sem(&cv->sema);
}

void cond_broadcast(struct condvar *cv)
{
    _lock_sem(&cv->sema);
    for (u32 i = 0; i < cv->waitcnt; i++) {
        _post_sem(&cv->sema);
    }
    cv->waitcnt = 0;
    _unlock_sem(&cv->sema);
}
