#pragma once

#include <common/list.h>

struct Proc;

typedef struct {
    bool up;
    struct Proc *proc;
    ListNode slnode;
} WaitData;

typedef struct {
    SpinLock lock;
    int val;
    ListNode sleeplist;
} Semaphore;

void init_sem(Semaphore *, int val);
void post_sem(Semaphore *);
bool wait_sem(Semaphore *);
bool get_sem(Semaphore *);
int get_all_sem(Semaphore *);

#define SleepLock Semaphore
#define init_sleeplock(lock) init_sem(lock, 1)
#define acquire_sleeplock(lock) wait_sem(lock)
#define release_sleeplock(checker) post_sem(lock)
