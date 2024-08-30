#include "driver/uart.h"
#include "kernel/init.h"
#include "kernel/printk.h"
#include <aarch64/intrinsic.h>
#include <common/string.h>
#include <kernel/init.h>
#include <driver/memlayout.h>
#include <aarch64/mmu.h>

__attribute__((aligned(16))) char kstack[4096 * 4];

static bool boot_secondary_cpus = false;

NO_RETURN void idle_entry();

void kernel_init()
{
	do_early_init();
	do_init();
	boot_secondary_cpus = true;
}

void main()
{
	for (int i = 1; i < 4; i++)
		psci_cpu_on(i, 0x40000000);

	if (cpuid() == 0) {
		kernel_init();
	} else {
		while (!boot_secondary_cpus)
			;
		arch_dsb_sy();
	}

	set_return_addr(idle_entry);
}
