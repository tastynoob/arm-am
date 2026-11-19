#include <bench.h>
#include "common.h"

static void __attribute__((noinline)) branch_test2(int cnt)
{
    int tmp = 0;

#define ONE \
    "cmp w11, w13\n\t" \
    "b.lt 2f\n\t" \
    "nop\n\t" \
    "2:\n\t" \
    "cmp w12, w14\n\t" \
    "b.lt 3f\n\t" \
    "nop\n\t" \
    "3:\n\t" \
    "b 4f\n\t" \
    "nop\n\t" \
    "4:\n\t"

    __asm__ volatile(
        "mov w10, #0\n\t"
        "mov w11, #0\n\t"
        "mov w12, #0\n\t"
        "mov w13, #3\n\t"
        "mov w14, #5\n\t"
        ".align 4\n\t"
        "1:\n\t"
        TEN
        "add w11, w11, #1\n\t"
        "add w12, w12, #1\n\t"
        "add w10, w10, #1\n\t"
        "cmp w10, %w1\n\t"
        "b.lt 1b\n\t"
        : "+r"(tmp)
        : "r"(cnt)
        : "w10", "w11", "w12", "w13", "w14", "cc", "memory");

#undef ONE
}

int main(void)
{
    BENCH_RUN("brnum2_uftb", branch_test2(1000));
    return 0;
}
