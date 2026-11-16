#ifndef BAREMETAL_PLATFORM_H
#define BAREMETAL_PLATFORM_H

void baremetal_platform_putc(char character);
__attribute__((noreturn)) void baremetal_platform_halt(int status);

#endif
