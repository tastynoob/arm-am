#include <bench.h>

static void __attribute__((noinline)) branch_test(int cnt)
{
    int sum = 0;

    __asm__ volatile(
        "mov w9, #0\n\t"
        "1:\n\t"
        "add %w0, %w0, #1\n\t"
        "add w9, w9, #1\n\t"
        "cmp w9, %w1\n\t"
        "b.lt 1b\n\t"
        : "+r"(sum)
        : "r"(cnt)
        : "w9", "cc", "memory");
}

int main(void)
{
    BENCH_RUN("brsimple", branch_test(100));
    return 0;
}
