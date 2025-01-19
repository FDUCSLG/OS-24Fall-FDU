#pragma once
#ifndef _NET_SOCKET_
#define _NET_SOCKET_

#include <common/defines.h>
#include <common/list.h>
#include <fs/condvar.h>
#include <common/spinlock.h>
#include <net/net.h>

typedef struct socket {
    // hold struct socket::lock when accessing these;
    struct mbufq rxq; // reception queue

    // hold modules' lock when accessing these:
    ListNode snode; // socket node

    // fixed property, read only
    u32 raddr; // remote ip address
    u16 lport; // local port
    u16 rport; // remote portj

    // synchronization
    SpinLock lock; // lock
    struct condvar cv; // condvar
} Socket;

// Init the socket module.
extern void sock_init(void);

/**
 * Open a socket.
 * @param raddr remote ip address(host endianess)
 * @param lport local port
 * @param rport remote port
 */
extern Socket *sock_open(u32 raddr, u16 lport, u16 rport);
extern void sock_close(Socket *sock);

extern isize sock_read(Socket *sock, void *buf, usize n);
extern isize sock_write(Socket *sock, void *buf, usize n);

/** Called by irq handler. Do NOT use this. */
extern void sock_recv(struct mbuf *m, uint32 raddr, uint16 lport, uint16 rport);

#endif // _NET_SOCET_
