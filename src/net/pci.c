#include "types.h"
#include "e1000.h"

#include <aarch64/mmu.h>
#include <kernel/pt.h>
#include <driver/interrupt.h>

// see also for the memory layout of qemu's virtio:
// https://github.com/qemu/qemu/blob/master/hw/arm/virt.c

/** See also: output of qemu monitor `info qtree`
 * dev: gpex-pcihost, id ""
    gpio-out "sysbus-irq" 4
    allow-unmapped-accesses = true
    x-config-reg-migration-enabled = true
    bypass-iommu = false
    mmio ffffffffffffffff/0000000010000000
    mmio ffffffffffffffff/ffffffffffffffff
    mmio 000000003eff0000/0000000000010000
    bus: pcie.0
      type PCIE
      dev: e1000, id ""
        mac = "52:54:00:12:34:56"
        netdev = "net0"
        extra_mac_registers = true
        migrate_tso_props = true
        init-vet = true
        addr = 01.0
        romfile = "efi-e1000.rom"
        romsize = 524288 (0x80000)
        rombar = 1 (0x1)
        multifunction = false
        x-pcie-lnksta-dllla = true
        x-pcie-extcap-init = true
        failover_pair_id = ""
        acpi-index = 0 (0x0)
        x-pcie-err-unc-mask = true
        x-pcie-ari-nextfn-1 = false
        class Ethernet controller, addr 00:01.0, pci id 8086:100e (sub 1af4:1100)
        bar 0: mem at 0xffffffffffffffff [0x1fffe]
        bar 1: i/o at 0xffffffffffffffff [0x3e]
        bar 6: mem at 0xffffffffffffffff [0x7fffe] 
 */

#ifdef _MEMLAYOUT_
static const MemMapEntry base_memmap[] = {
    /* Space up to 0x8000000 is reserved for a boot ROM */
    [VIRT_FLASH] = { 0, 0x08000000 },
    [VIRT_CPUPERIPHS] = { 0x08000000, 0x00020000 },
    /* GIC distributor and CPU interfaces sit inside the CPU peripheral space */
    [VIRT_GIC_DIST] = { 0x08000000, 0x00010000 },
    [VIRT_GIC_CPU] = { 0x08010000, 0x00010000 },
    [VIRT_GIC_V2M] = { 0x08020000, 0x00001000 },
    [VIRT_GIC_HYP] = { 0x08030000, 0x00010000 },
    [VIRT_GIC_VCPU] = { 0x08040000, 0x00010000 },
    /* The space in between here is reserved for GICv3 CPU/vCPU/HYP */
    [VIRT_GIC_ITS] = { 0x08080000, 0x00020000 },
    /* This redistributor space allows up to 2*64kB*123 CPUs */
    [VIRT_GIC_REDIST] = { 0x080A0000, 0x00F60000 },
    [VIRT_UART0] = { 0x09000000, 0x00001000 },
    [VIRT_RTC] = { 0x09010000, 0x00001000 },
    [VIRT_FW_CFG] = { 0x09020000, 0x00000018 },
    [VIRT_GPIO] = { 0x09030000, 0x00001000 },
    [VIRT_UART1] = { 0x09040000, 0x00001000 },
    [VIRT_SMMU] = { 0x09050000, 0x00020000 },
    [VIRT_PCDIMM_ACPI] = { 0x09070000, MEMORY_HOTPLUG_IO_LEN },
    [VIRT_ACPI_GED] = { 0x09080000, ACPI_GED_EVT_SEL_LEN },
    [VIRT_NVDIMM_ACPI] = { 0x09090000, NVDIMM_ACPI_IO_LEN },
    [VIRT_PVTIME] = { 0x090a0000, 0x00010000 },
    [VIRT_SECURE_GPIO] = { 0x090b0000, 0x00001000 },
    [VIRT_MMIO] = { 0x0a000000, 0x00000200 },
    /* ...repeating for a total of NUM_VIRTIO_TRANSPORTS, each of that size */
    [VIRT_PLATFORM_BUS] = { 0x0c000000, 0x02000000 },
    [VIRT_SECURE_MEM] = { 0x0e000000, 0x01000000 },
    [VIRT_PCIE_MMIO] = { 0x10000000, 0x2eff0000 },
    [VIRT_PCIE_PIO] = { 0x3eff0000, 0x00010000 },
    [VIRT_PCIE_ECAM] = { 0x3f000000, 0x01000000 },
    /* Actual RAM size depends on initial RAM and device memory settings */
    [VIRT_MEM] = { GiB, LEGACY_RAMLIMIT_BYTES },
};

/*
 * Highmem IO Regions: This memory map is floating, located after the RAM.
 * Each MemMapEntry base (GPA) will be dynamically computed, depending on the
 * top of the RAM, so that its base get the same alignment as the size,
 * ie. a 512GiB entry will be aligned on a 512GiB boundary. If there is
 * less than 256GiB of RAM, the floating area starts at the 256GiB mark.
 * Note the extended_memmap is sized so that it eventually also includes the
 * base_memmap entries (VIRT_HIGH_GIC_REDIST2 index is greater than the last
 * index of base_memmap).
 *
 * The memory map for these Highmem IO Regions can be in legacy or compact
 * layout, depending on 'compact-highmem' property. With legacy layout, the
 * PA space for one specific region is always reserved, even if the region
 * has been disabled or doesn't fit into the PA space. However, the PA space
 * for the region won't be reserved in these circumstances with compact layout.
 */
static MemMapEntry extended_memmap[] = {
    /* Additional 64 MB redist region (can contain up to 512 redistributors) */
    [VIRT_HIGH_GIC_REDIST2] = { 0x0, 64 * MiB },
    [VIRT_HIGH_PCIE_ECAM] = { 0x0, 256 * MiB },
    /* Second PCIe window */
    [VIRT_HIGH_PCIE_MMIO] = { 0x0, 512 * GiB },
};

static const int a15irqmap[] = {
    [VIRT_UART0] = 1,          [VIRT_RTC] = 2,   [VIRT_PCIE] = 3, /* ... to 6 */
    [VIRT_GPIO] = 7,           [VIRT_UART1] = 8, [VIRT_ACPI_GED] = 9,
    [VIRT_MMIO] = 16, /* ...to 16 + NUM_VIRTIO_TRANSPORTS - 1 */
    [VIRT_GIC_V2M] = 48, /* ...to 48 + NUM_GICV2M_SPIS - 1 */
    [VIRT_SMMU] = 74, /* ...to 74 + NUM_SMMU_IRQS - 1 */
    [VIRT_PLATFORM_BUS] = 112, /* ...to 112 + PLATFORM_BUS_NUM_IRQS -1 */
};
#endif // _MEMLAYOUT_

extern void e1000_intr(void);

void pci_init()
{
    // we'll place the e1000 registers at this address.
    // vm.c maps this range.
    uint64 e1000_regs = 0x10000000L;

    // qemu -machine virt puts PCIe config space here.
    // vm.c maps this range.
    uint32 *pio = (uint32 *)0x3f000000L;
    uint32 *ecam = pio;

    // both convert to kernel virtual address
    e1000_regs = P2K(e1000_regs);
    ecam = (u32 *)P2K((u64)ecam);

    // look at each possible PCI device on bus 0.
    for (int dev = 0; dev < 32; dev++) {
        int bus = 0;
        int func = 0;
        int offset = 0;
        uint32 off = (bus << 16) | (dev << 11) | (func << 8) | (offset);
        volatile uint32 *base = ecam + off;
        uint32 id = base[0];
        base[1] = 7;

        // 100e:8086 is an e1000
        if (id == 0x100e8086) {
            // [4.1.3]
            // command and status register.
            // bit 0 : I/O access enable
            // bit 1 : memory access enable
            // bit 2 : enable mastering
            base[1] = 7;
            __sync_synchronize();

            for (int i = 0; i < 6; i++) {
                uint32 old = base[4 + i];

                // writing all 1's to the BAR causes it to be
                // replaced with its size.
                base[4 + i] = 0xffffffff;
                __sync_synchronize();

                base[4 + i] = old;
            }

            // tell the e1000 to reveal its registers at
            // physical address 0x40000000.
            base[4 + 0] = K2P(e1000_regs);

            e1000_init((uint32 *)e1000_regs);
        }
    }

    // not found.

    // set handler.
    set_interrupt_handler(PCIE_IRQ, e1000_intr);
}
