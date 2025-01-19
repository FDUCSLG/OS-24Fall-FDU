#include "e1000.h"
#include "types.h"
#include "net.h"

#include <common/spinlock.h>
#include <common/string.h>
#include <common/defines.h>
#include <kernel/mem.h>
#include <aarch64/mmu.h>

#define panic(fmt) PANIC()

#define TX_RING_SIZE 16
static struct tx_desc tx_ring[TX_RING_SIZE] __attribute__((aligned(16)));
static struct mbuf *tx_mbufs[TX_RING_SIZE];

#define RX_RING_SIZE 16
static struct rx_desc rx_ring[RX_RING_SIZE] __attribute__((aligned(16)));
static struct mbuf *rx_mbufs[RX_RING_SIZE];

// remember where the e1000's registers live.
static volatile uint32 *regs;

// If host is big endian, then should convert byte order.
// Our kernel is little-endian.
// #define CONVERT

/** Write to u16 in little edian. */
static inline void wl16(volatile u16 *addr, u16 val)
{
#ifdef CONVERT
    *addr = bswaps(val);
#else
    *addr = val;
#endif
}
static inline u16 rl16(volatile u16 *addr)
{
#ifdef CONVERT
    return bswaps(*addr);
#else
    return *addr;
#endif
}

static inline void wl32(volatile u32 *addr, u32 val)
{
#ifdef CONVERT
    *addr = bswapl(val);
#else
    *addr = val;
#endif
}
static inline u32 rl32(volatile u32 *addr)
{
#ifdef CONVERT
    return bswapl(*addr);
#else
    return *addr;
#endif
}

static inline void wl64(volatile u64 *addr, u64 val)
{
#ifdef CONVERT
    *addr = bswapp(val);
#else
    *addr = val;
#endif
}
static inline u64 rl64(volatile u64 *addr)
{
#ifdef CONVERT
    return bswapp(*addr);
#else
    return *addr;
#endif
}

SpinLock e1000_lock;

// called by pci_init().
// xregs is the memory address at which the
// e1000's registers are mapped.
void e1000_init(uint32 *xregs)
{
    int i;

    init_spinlock(&e1000_lock);

    regs = xregs;

    // Reset the device
    wl32(&regs[E1000_IMS], 0); // disable interrupts
    wl32(&regs[E1000_CTL], E1000_CTL_RST | rl32(&regs[E1000_CTL]));
    wl32(&regs[E1000_IMS], 0); // redisable interrupts
    __sync_synchronize();

    // [E1000 14.5] Transmit initialization
    memset(tx_ring, 0, sizeof(tx_ring));
    for (i = 0; i < TX_RING_SIZE; i++) {
        tx_ring[i].status = E1000_TXD_STAT_DD;
        tx_mbufs[i] = 0;
    }
    wl32(&regs[E1000_TDBAL], (uint64)K2P(tx_ring));
    if (sizeof(tx_ring) % 128 != 0)
        panic("e1000");
    wl32(&regs[E1000_TDLEN], sizeof(tx_ring));
    wl32(&regs[E1000_TDH], 0);
    wl32(&regs[E1000_TDT], 0);

    // [E1000 14.4] Receive initialization
    memset(rx_ring, 0, sizeof(rx_ring));
    for (i = 0; i < RX_RING_SIZE; i++) {
        rx_mbufs[i] = mbufalloc(0);
        if (!rx_mbufs[i])
            panic("e1000");
        wl64(&(rx_ring[i].addr), (u64)K2P(rx_mbufs[i]->head));
    }
    wl32(&regs[E1000_RDBAL], (u64)K2P(rx_ring));
    if (sizeof(rx_ring) % 128 != 0)
        panic("e1000");
    wl32(&regs[E1000_RDH], 0);
    wl32(&regs[E1000_RDT], RX_RING_SIZE - 1);
    wl32(&regs[E1000_RDLEN], sizeof(rx_ring));

    // filter by qemu's MAC address, 52:54:00:12:34:56
    wl32(&regs[E1000_RA], 0x12005452);
    wl32(&regs[E1000_RA + 1], 0x5634 | (1 << 31));
    // multicast table
    for (int i = 0; i < 4096 / 32; i++)
        regs[E1000_MTA + i] = 0;

    // transmitter control bits.
    u32 tctl = E1000_TCTL_EN | // enable
               E1000_TCTL_PSP | // pad short packets
               (0x10 << E1000_TCTL_CT_SHIFT) | // collision stuff
               (0x40 << E1000_TCTL_COLD_SHIFT);
    wl32(&regs[E1000_TCTL], tctl);
    u32 TIPG = 10 | (8 << 10) | (6 << 20); // inter-pkt gap
    wl32(&regs[E1000_TIPG], TIPG);

    // receiver control bits.
    u32 RCTL = E1000_RCTL_EN | // enable receiver
               E1000_RCTL_BAM | // enable broadcast
               E1000_RCTL_SZ_2048 | // 2048-byte rx buffers
               E1000_RCTL_SECRC; // strip CRC
    wl32(&regs[E1000_RCTL], RCTL);

    // ask e1000 for receive interrupts.
    regs[E1000_RDTR] = 0; // interrupt after every received packet (no timer)
    regs[E1000_RADV] = 0; // interrupt after every packet (no timer)
    u32 IMS = (1 << 7); // RXDW -- Receiver Descriptor Write Back
    // this is recommended by the manual.
    // u32 IMS = (1 << 7) | (1 << 3) | (1 << 6) | (1 << 2) | (1 << 4);
    wl32(&regs[E1000_IMS], IMS);

    __sync_synchronize();
}

/**
 * Place a pointer to the packet data in a descriptor in the TX (transmit) ring.
 * Will need to ensure that each mbuf is eventually freed, but only after the 
 * E1000 has finished transmitting the packet(the E1000 sets the E1000_TXD_STAT_DD 
 * bit in the descriptor to indicate this)
 * 
 * @param m the packet to be sent
 */
int e1000_transmit(struct mbuf *m)
{
    acquire_spinlock(&e1000_lock);
    //
    // Your code here.
    //
    // the mbuf contains an ethernet frame; program it into
    // the TX descriptor ring so that the e1000 sends it. Stash
    // a pointer so that it can be freed after sending.
    //

    // the tail of tx queue
    uint32 tail = rl32(&regs[E1000_TDT]);
    // the head of tx queue
    uint32 head = rl32(&regs[E1000_TDH]);
    if (head == tail && !(tx_ring[tail].status & E1000_TXD_STAT_DD)) {
        // cannot overwrite
        release_spinlock(&e1000_lock);
        return -1;
    } else {
        // free previous descriptor
        if (tx_mbufs[tail] != 0) {
            mbuffree(tx_mbufs[tail]);
        }
    }
    // stash m for later freeing
    tx_mbufs[tail] = m;

    // transmit descriptor
    struct tx_desc *txd = &(tx_ring[tail]);
    wl64(&txd->addr, (u64)K2P(m->head));
    wl16(&txd->length, (uint16)m->len);
    txd->cmd = E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP;
    txd->status = 0x00;

    volatile uint8 finished __attribute__((unused));
    finished = 0;
    // update the ring position
    wl32(&regs[E1000_TDT], (tail + 1) % TX_RING_SIZE);
    __sync_synchronize();

    release_spinlock(&e1000_lock);
    return 0;
}

extern void net_rx(struct mbuf *);
extern struct mbuf *mbufalloc(unsigned int headroom);

static void e1000_recv(void)
{
    //
    // Your code here.
    //
    // Check for packets that have arrived from the e1000
    // Create and deliver an mbuf for each packet (using net_rx()).
    //
    uint32 tail;

// start of loop
recv_start:
    tail = regs[E1000_RDT];
    tail = (tail + 1) % RX_RING_SIZE;

    // check if a new packet is available
    if (!(rx_ring[tail].status & E1000_RXD_STAT_DD)) {
        return;
    }
    struct mbuf *m = rx_mbufs[tail];
    m->len = rx_ring[tail].length;

    net_rx(m);

    rx_mbufs[tail] = mbufalloc(0);
    if (!rx_mbufs[tail]) {
        panic("e1000_recv");
    }
    rx_ring[tail].status = 0x00;
    rx_ring[tail].addr = K2P(rx_mbufs[tail]->head);

    regs[E1000_RDT] = tail;

    goto recv_start;
}

void e1000_intr(void)
{
    // tell the e1000 we've seen this interrupt;
    // without this the e1000 won't raise any
    // further interrupts.
    regs[E1000_ICR] = 0xffffffff;

    e1000_recv();
}
