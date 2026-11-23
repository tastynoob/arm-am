#include <bench.h>
#include "common.h"

static void __attribute__((noinline)) brwidth(int cnt)
{
#define ONE "cbz w9, 1b\n\t"

    __asm__ volatile(
        "mov w9, #1\n\t"
        ".align 4\n\t"
        "1:\n\t"
        "add w9, w9, #1\n\t"
        HUNDRED
        "subs %w0, %w0, #1\n\t"
        "b.ne 1b\n\t"
        : "+r"(cnt)
        :
        : "w9", "cc", "memory");

#undef ONE
}

int main(void)
{
    brwidth(1);
    BENCH_RUN("brwidth", brwidth(100));
    return 0;
}
