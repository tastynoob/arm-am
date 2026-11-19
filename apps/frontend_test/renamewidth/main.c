#include <bench.h>
#include "common.h"

static void __attribute__((noinline)) renamewidth(int cnt)
{
#define ONE \
    "fadd d2, d0, d1\n\t" \
    "add w9, w0, wzr\n\t" \
    "add w10, w0, wzr\n\t" \
    "fadd d3, d0, d1\n\t" \
    "add w11, w0, wzr\n\t" \
    "add w12, w0, wzr\n\t" \
    "fadd d4, d0, d1\n\t" \
    "add w13, w0, wzr\n\t" \
    "add w14, w0, wzr\n\t"

    __asm__ volatile(
        "mov w17, %w0\n\t"
        "fmov d0, xzr\n\t"
        "fmov d1, #1.0\n\t"
        "mov w0, #123\n\t"
        ".align 4\n\t"
        "1:\n\t"
        HUNDRED
        "subs w17, w17, #1\n\t"
        "b.ne 1b\n\t"
        :
        : "r"(cnt)
        : "w0", "w9", "w10", "w11", "w12", "w13", "w14", "w17",
          "v0", "v1", "v2", "v3", "v4", "cc", "memory");

#undef ONE
}

int main(void)
{
    BENCH_RUN("renamewidth", renamewidth(__LOOP_COUNT));
    return 0;
}
