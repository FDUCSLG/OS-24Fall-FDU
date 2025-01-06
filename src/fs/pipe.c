#include <kernel/mem.h>
#include <kernel/sched.h>
#include <fs/pipe.h>
#include <common/string.h>
#include <kernel/printk.h>

void init_pipe(Pipe *pi)
{
    /* (Final) TODO Begin */

    /* (Final) TODO End */
}

void init_read_pipe(File *readp, Pipe *pipe)
{
    /* (Final) TODO Begin */

    /* (Final) TODO End */
}

void init_write_pipe(File *writep, Pipe *pipe)
{
    /* (Final) TODO Begin */

    /* (Final) TODO End */
}

int pipe_alloc(File **f0, File **f1)
{
    /* (Final) TODO Begin */

    /* (Final) TODO End */
}

void pipe_close(Pipe *pi, int writable)
{
    /* (Final) TODO Begin */

    /* (Final) TODO End */
}

int pipe_write(Pipe *pi, u64 addr, int n)
{
    /* (Final) TODO Begin */

    /* (Final) TODO End */
}

int pipe_read(Pipe *pi, u64 addr, int n)
{
    /* (Final) TODO Begin */

    /* (Final) TODO End */
}