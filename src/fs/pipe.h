#pragma once

#include <common/spinlock.h>
#include <common/defines.h>
#include <fs/file.h>
#include <common/sem.h>

#define PIPE_SIZE 512

typedef struct pipe {
    SpinLock lock;
    Semaphore wlock, rlock;
    char data[PIPE_SIZE];
    u32 nread; // Number of bytes read
    u32 nwrite; // Number of bytes written
    int readopen; // Read fd is still open
    int writeopen; // Write fd is still open
} Pipe;

int pipe_alloc(File **f0, File **f1);
void pipe_close(Pipe *pi, int writable);
int pipe_write(Pipe *pi, u64 addr, int n);
int pipe_read(Pipe *pi, u64 addr, int n);
