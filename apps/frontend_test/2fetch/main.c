#include <bench.h>

static void __attribute__((noinline)) test_2fetch(int cnt)
{
    __asm__ volatile(
        "mov w9, #1\n\t"
        "mov w10, #2\n\t"
        "mov w11, #3\n\t"
        ".align 6\n\t"
        "block_a:\n\t"
        "add w12, w9, w10\n\t"
        "add w13, w10, w11\n\t"
        "b block_b\n\t"
        "nop\n\t"
        "nop\n\t"
        "block_b:\n\t"
        "add w14, w12, w13\n\t"
        "subs %w0, %w0, #1\n\t"
        "b.ne block_a\n\t"
        : "+r"(cnt)
        :
        : "w9", "w10", "w11", "w12", "w13", "w14", "cc", "memory");
}

int main(void)
{
    test_2fetch(1);
    BENCH_RUN("2fetch", test_2fetch(10000));
    return 0;
}
