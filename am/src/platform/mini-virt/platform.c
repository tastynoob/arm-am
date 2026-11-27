#include <stdint.h>

#include <baremetal/platform.h>

#ifndef UART_BASE
#define UART_BASE 0x13000000UL
#endif

void baremetal_platform_putc(char c)
{
    volatile uint8_t *const uart_data = (volatile uint8_t *)UART_BASE;

    if (c == '\n') {
        *uart_data = '\r';
    }
    *uart_data = (uint8_t)c;
}

__attribute__((noreturn)) void baremetal_platform_halt(int status)
{
    volatile uint8_t *const uart_data = (volatile uint8_t *)UART_BASE;

    (void)status;

    /*
     * SimpleUart with end_on_eot exits gem5 on EOT (0x04).  Keep HLT #0 as
     * the QEMU/physical fallback, after the EOT store is ordered.
     */
    *uart_data = 0x04;
    __asm__ volatile("dsb sy" ::: "memory");
    __asm__ volatile("hlt #0" ::: "memory");
    for (;;) {
        __asm__ volatile("wfe");
    }
}
