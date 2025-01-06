#include <aarch64/intrinsic.h>
#include <driver/aux.h>
#include <driver/gpio.h>
#include <driver/uart.h>
#include <driver/interrupt.h>
#include <kernel/console.h>
#include <kernel/printk.h>

static void uartintr()
{
    /**
     * Invoke the console interrupt handler here. 
     * Without this, the shell may fail to properly handle user inputs.
     */
    char c = uart_get_char();
    if (c != 0xFF) {
        console_intr(c);
    }

    device_put_u32(UART_ICR, 1 << 4 | 1 << 5);
}

void uart_init()
{
    device_put_u32(UART_CR, 0);
    set_interrupt_handler(UART_IRQ, uartintr);
    device_put_u32(UART_LCRH, LCRH_FEN | LCRH_WLEN_8BIT);
    device_put_u32(UART_CR, 0x301);

    /**
     * Enabling Uart interrupt.
     */
    device_put_u32(UART_IMSC, 0);
    arch_fence();
    device_put_u32(UART_IMSC, 1 << 4 | 1 << 5);
}

char uart_get_char()
{
    if (device_get_u32(UART_FR) & FR_RXFE)
        return -1;
    return device_get_u32(UART_DR);
}

void uart_put_char(char c)
{
    while (device_get_u32(UART_FR) & FR_TXFF)
        ;
    device_put_u32(UART_DR, c);
}

__attribute__((weak, alias("uart_put_char"))) void putch(char);
