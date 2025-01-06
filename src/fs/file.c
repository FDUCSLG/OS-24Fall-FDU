#include "file.h"
#include <common/defines.h>
#include <common/spinlock.h>
#include <common/sem.h>
#include <fs/inode.h>
#include <common/list.h>
#include <kernel/mem.h>

// the global file table.
static struct ftable ftable;

void init_ftable() {
    // TODO: initialize your ftable.
}

void init_oftable(struct oftable *oftable) {
    // TODO: initialize your oftable for a new process.
}

/* Allocate a file structure. */
struct file* file_alloc() {
    /* (Final) TODO BEGIN */
    
    /* (Final) TODO END */
    return 0;
}

/* Increment ref count for file f. */
struct file* file_dup(struct file* f) {
    /* (Final) TODO BEGIN */
    
    /* (Final) TODO END */
    return f;
}

/* Close file f. (Decrement ref count, close when reaches 0.) */
void file_close(struct file* f) {
    /* (Final) TODO BEGIN */
    
    /* (Final) TODO END */
}

/* Get metadata about file f. */
int file_stat(struct file* f, struct stat* st) {
    /* (Final) TODO BEGIN */
    
    /* (Final) TODO END */
    return -1;
}

/* Read from file f. */
isize file_read(struct file* f, char* addr, isize n) {
    /* (Final) TODO BEGIN */
    
    /* (Final) TODO END */
    return 0;
}

/* Write to file f. */
isize file_write(struct file* f, char* addr, isize n) {
    /* (Final) TODO BEGIN */
    
    /* (Final) TODO END */
    return 0;
}