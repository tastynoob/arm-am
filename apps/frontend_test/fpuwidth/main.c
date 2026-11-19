#include <bench.h>
#include "common.h"

static void __attribute__((noinline)) fpuwidth(int cnt)
{
#define ONE \
    "fadd d2, d0, d1\n\t" \
    "fadd d3, d0, d1\n\t" \
    "fadd d4, d0, d1\n\t" \
    "fadd d5, d0, d1\n\t" \
    "fadd d6, d0, d1\n\t" \
    "fadd d7, d0, d1\n\t" \
    "fadd d8, d0, d1\n\t" \
    "fadd d9, d0, d1\n\t"

    __asm__ volatile(
        "mov w17, %w0\n\t"
        "fmov d0, xzr\n\t"
        "fmov d1, #1.0\n\t"
        ".align 4\n\t"
        "1:\n\t"
        HUNDRED
        "subs w17, w17, #1\n\t"
        "b.ne 1b\n\t"
        :
        : "r"(cnt)
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8",
          "v9", "w17", "cc", "memory");

#undef ONE
}

int main(void)
{
    BENCH_RUN("fpuwidth", fpuwidth(100));
    return 0;
}
