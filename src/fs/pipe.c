#include <kernel/mem.h>
#include <kernel/sched.h>
#include <fs/pipe.h>
#include <common/string.h>
#include <kernel/printk.h>

void init_pipe(Pipe *pi)
{
    /* (Final) TODO BEGIN */

    /* (Final) TODO END */
}

void init_read_pipe(File *readp, Pipe *pipe)
{
    /* (Final) TODO BEGIN */

    /* (Final) TODO END */
}

void init_write_pipe(File *writep, Pipe *pipe)
{
    /* (Final) TODO BEGIN */

    /* (Final) TODO END */
}

int pipe_alloc(File **f0, File **f1)
{
    /* (Final) TODO BEGIN */

    /* (Final) TODO END */
}

void pipe_close(Pipe *pi, int writable)
{
    /* (Final) TODO BEGIN */

    /* (Final) TODO END */
}

int pipe_write(Pipe *pi, u64 addr, int n)
{
    /* (Final) TODO BEGIN */

    /* (Final) TODO END */
}

int pipe_read(Pipe *pi, u64 addr, int n)
{
    /* (Final) TODO BEGIN */

    /* (Final) TODO END */
}