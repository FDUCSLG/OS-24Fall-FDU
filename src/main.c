#include <aarch64/intrinsic.h>
#include <driver/uart.h>
#include <kernel/printk.h>
#include <kernel/core.h>

static volatile bool boot_secondary_cpus = false;

void main()
{
    /**
     * LAB 1 TASK 1
     * 
     * Print "Hello, world!" on all four cores. The following questions may help:
     * (1) How to print strings.
     * (2) Where to place the print statements.
     */

    if (cpuid() == 0) {
        /**
         * Core 0 is responsible for initializing the kernel.
         * 
         * The `smp_init` function wakes up the other three cores, which will
         * subsequently enter the `main` function and take the `else` branch below.
         * 
         * For Lab 1, carefully consider the execution path for core 0 and the other
         * three cores.
         */
        smp_init();

        /**
         * Initialize the UART.
         * 
         * After initialization, the `uart_put_char` function can be used to print
         * characters to the screen.
         */
        uart_init();

        /**
         * Initialize printk, which is analogous to printf.
         */
        printk_init();

        // Set a flag indicating that the secondary CPUs can start executing.
        boot_secondary_cpus = true;
    } else {
        while (!boot_secondary_cpus)
            ;
    }

    set_return_addr(idle_entry);
}
