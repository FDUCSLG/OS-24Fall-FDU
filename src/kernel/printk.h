#pragma once

// Should be provided by drivers. By default, putch = uart_put_char.
extern void putch(char);

void printk_init();
__attribute__((format(printf, 1, 2))) void printk(const char *, ...);
