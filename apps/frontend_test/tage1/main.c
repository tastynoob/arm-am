#include <bench.h>
#include "common.h"

static void __attribute__((noinline)) branch_test(int cnt)
{
    int tmp = 0;

#define ONE \
    "and w12, w11, #1\n\t" \
    "cbz w12, 2f\n\t" \
    "nop\n\t" \
    "2:\n\t"

    __asm__ volatile(
        "mov w10, #0\n\t"
        "mov w11, #0\n\t"
        ".align 4\n\t"
        "1:\n\t"
        TEN
        "add w11, w11, #1\n\t"
        "add w10, w10, #1\n\t"
        "cmp w10, %w1\n\t"
        "b.lt 1b\n\t"
        : "+r"(tmp)
        : "r"(cnt)
        : "w10", "w11", "w12", "cc", "memory");

#undef ONE
}

int main(void)
{
    branch_test(1);
    BENCH_RUN("tage1", branch_test(1000));
    return 0;
}
