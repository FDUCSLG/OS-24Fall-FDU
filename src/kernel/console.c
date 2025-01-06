#include <kernel/console.h>
#include <aarch64/intrinsic.h>
#include <kernel/sched.h>
#include <driver/uart.h>

struct console cons;

void console_init()
{
    /* (Final) TODO BEGIN */

    /* (Final) TODO END */
}

/**
 * console_write - write to uart from the console buffer.
 * @ip: the pointer to the inode
 * @buf: the buffer
 * @n: number of bytes to write
 */
isize console_write(Inode *ip, char *buf, isize n)
{
    /* (Final) TODO BEGIN */

    /* (Final) TODO END */
}

/**
 * console_read - read to the destination from the buffer
 * @ip: the pointer to the inode
 * @dst: the destination
 * @n: number of bytes to read
 */
isize console_read(Inode *ip, char *dst, isize n)
{
    /* (Final) TODO BEGIN */

    /* (Final) TODO END */
}

void console_intr(char c)
{
    /* (Final) TODO BEGIN */

    /* (Final) TODO END */
}