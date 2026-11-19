#include <bench.h>
#include "common.h"

static void __attribute__((noinline)) ifuwidth(int cnt)
{
#define ONE \
    "add w11, w9, w10\n\t" \
    "add w12, w9, w10\n\t" \
    "add w13, w9, w10\n\t" \
    "add w14, w9, w10\n\t" \
    "fadd d2, d0, d1\n\t" \
    "fadd d3, d0, d1\n\t" \
    "fadd d4, d0, d1\n\t" \
    "fadd d5, d0, d1\n\t"

    __asm__ volatile(
        "mov w17, %w0\n\t"
        "mov w9, #1\n\t"
        "mov w10, #2\n\t"
        "scvtf d0, w9\n\t"
        "scvtf d1, w10\n\t"
        ".align 4\n\t"
        "1:\n\t"
        HUNDRED
        "subs w17, w17, #1\n\t"
        "b.ne 1b\n\t"
        :
        : "r"(cnt)
        : "w9", "w10", "w11", "w12", "w13", "w14", "w17",
          "v0", "v1", "v2", "v3", "v4", "v5", "cc", "memory");

#undef ONE
}

int main(void)
{
    BENCH_RUN("ifuwidth", ifuwidth(100));
    return 0;
}
