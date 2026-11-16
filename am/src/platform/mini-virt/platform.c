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
    (void)status;

    /* HLT #0 is the QEMU-compatible good-trap; MMIO stores are ordered. */
    __asm__ volatile("hlt #0" ::: "memory");
    for (;;) {
        __asm__ volatile("wfe");
    }
}
