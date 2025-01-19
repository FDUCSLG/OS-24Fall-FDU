#include "socket.h"
#include <kernel/mem.h>

static ListNode head;
static ListNode tail;
static SpinLock slock;

static INLINE void sock_add(Socket *sock)
{
    ListNode *prev = tail.prev;
    ListNode *nxt = &tail;
    sock->snode.prev = prev;
    sock->snode.next = nxt;
    prev->next = &sock->snode;
    nxt->prev = &sock->snode;
}

static INLINE void sock_rm(Socket *sock)
{
    ListNode *prev = sock->snode.prev;
    ListNode *nxt = sock->snode.next;

    // validate the list.
    ASSERT(prev->next = &sock->snode);
    ASSERT(nxt->prev = &sock->snode);

    // do removal
    prev->next = nxt;
    nxt->prev = prev;
}

void sock_init(void)
{
    init_spinlock(&slock);

    // init list
    head.prev = NULL;
    tail.next = NULL;
    head.next = &tail;
    tail.prev = &head;
}

Socket *sock_open(u32 raddr, u16 lport, u16 rport)
{
    Socket *si = kalloc(sizeof(Socket));
    if (si == NULL) {
        // fail!
        return si;
    }

    // init sock's lock and condvar
    cond_init(&si->cv);
    init_spinlock(&si->lock);

    // fill in properties
    si->raddr = raddr;
    si->lport = lport;
    si->rport = rport;

    // buffer queue
    mbufq_init(&si->rxq);

    // list node.
    acquire_spinlock(&slock);
    sock_add(si);
    release_spinlock(&slock);

    return si;
}

void sock_close(Socket *sock)
{
    ASSERT(sock != NULL);
    // remove from list
    acquire_spinlock(&slock);
    sock_rm(sock);
    release_spinlock(&slock);

    // free any pending mbufs
    struct mbuf *m;
    while (!mbufq_empty(&sock->rxq)) {
        m = mbufq_pophead(&sock->rxq);
        mbuffree(m);
    }

    kfree(sock);
}

extern void net_tx_udp(struct mbuf *m, uint32 dip, uint16 sport, uint16 dport);
isize sock_write(Socket *sock, void *buf, usize n)
{
    struct mbuf *m;
    m = mbufalloc(MBUF_DEFAULT_HEADROOM);
    if (!m)
        return -1;

    char *dst = mbufput(m, n);
    for (usize i = 0; i < n; i++) {
        dst[i] = ((char *)buf)[i];
    }

    // transmit
    net_tx_udp(m, sock->raddr, sock->lport, sock->rport);
    return n;
}

isize sock_read(Socket *si, void *buf, usize n)
{
    struct mbuf *m;
    isize len;
    acquire_spinlock(&si->lock);
    while (mbufq_empty(&si->rxq)) {
        // irq handler will signal when a packet
        // is ready for read.
        cond_wait(&si->cv, &si->lock);
    }
    m = mbufq_pophead(&si->rxq);
    release_spinlock(&si->lock);

    len = (isize)m->len;
    if (len > (isize)n) {
        // only print this number of bytes.
        len = (isize)n;
    }

    for (isize i = 0; i < len; i++) {
        ((char *)buf)[i] = m->head[i];
    }

    mbuffree(m);
    return len;
}

void sock_recv(struct mbuf *m, uint32 raddr, uint16 lport, uint16 rport)
{
    // the socket sleeping on this pack.
    Socket *sleeping = NULL;
    acquire_spinlock(&slock);
    ListNode *it = head.next;
    for (; it != &tail; it = it->next) {
        Socket *s = ListEntr(it, Socket, snode);

        if (s->raddr == raddr && s->lport == lport && s->rport == rport) {
            sleeping = s;
            break;
        }
    }

    if (sleeping == NULL) {
        release_spinlock(&slock);
        mbuffree(m);
        return;
    }

    acquire_spinlock(&sleeping->lock);
    mbufq_pushtail(&sleeping->rxq, m);
    // signal the sleeping one.
    cond_signal(&sleeping->cv);
    release_spinlock(&sleeping->lock);
    release_spinlock(&slock);
}
