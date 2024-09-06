#include <aarch64/intrinsic.h>
#include <driver/uart.h>
#include <kernel/printk.h>
#include <kernel/core.h>
#include <common/string.h>

static volatile bool boot_secondary_cpus = false;

// Refer to `linker.ld`, `edata` and `end` is the start and end address of .bss
extern char *edata, *end;

void main()
{
    if (cpuid() == 0) {
        /* @todo: Clear BSS section.*/
        memset(edata, 0, end - edata);

        smp_init();
        uart_init();
        printk_init();

        /* @todo: Print "Hello, world! (Core 0)" */
        printk("Hello, world! (Core 0)\n");

        arch_fence();

        // Set a flag indicating that the secondary CPUs can start executing.
        boot_secondary_cpus = true;
    } else {
        while (!boot_secondary_cpus)
            ;
        arch_fence();

        /* @todo: Print "Hello, world! (Core <core id>)" */
        printk("Hello, world! (Core %llu)\n", cpuid());
    }

    set_return_addr(idle_entry);
}
