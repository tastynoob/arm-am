#include <bench.h>
#include "common.h"

static void __attribute__((noinline)) aluwidth(int cnt)
{
#define ONE \
    "add w9, w9, w0\n\t" \
    "add w10, w10, w0\n\t" \
    "add w11, w11, w0\n\t" \
    "add w12, w12, w0\n\t" \
    "add w13, w13, w0\n\t" \
    "add w14, w14, w0\n\t" \
    "add w15, w15, w0\n\t" \
    "add w16, w16, w0\n\t"

    __asm__ volatile(
        "mov w17, %w0\n\t"
        "mov w0, #1\n\t"
        "mov w9, #1\n\t"
        "mov w10, #1\n\t"
        "mov w11, #1\n\t"
        "mov w12, #1\n\t"
        "mov w13, #1\n\t"
        "mov w14, #1\n\t"
        "mov w15, #1\n\t"
        "mov w16, #1\n\t"
        ".align 4\n\t"
        "1:\n\t"
        HUNDRED
        "subs w17, w17, #1\n\t"
        "b.ne 1b\n\t"
        :
        : "r"(cnt)
        : "w0", "w9", "w10", "w11", "w12", "w13", "w14", "w15",
          "w16", "w17", "cc", "memory");

#undef ONE
}

int main(void)
{
    BENCH_RUN("aluwidth", aluwidth(200));
    return 0;
}
