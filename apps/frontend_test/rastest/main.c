#include <bench.h>

static void __attribute__((noinline)) ras_test(int cnt)
{
    __asm__ volatile(
        "sub sp, sp, #16\n\t"
        "str x30, [sp, #8]\n\t"
        "mov w20, #0\n\t"
        "mov w21, #0\n\t"
        "1:\n\t"
        "add w20, w20, #1\n\t"
        "add w21, w21, #1\n\t"
        "bl 2f\n\t"
        "cmp w21, %w0\n\t"
        "b.lt 1b\n\t"
        "b 3f\n\t"
        "2:\n\t"
        "add w20, w20, #2\n\t"
        "ret\n\t"
        "3:\n\t"
        "ldr x30, [sp, #8]\n\t"
        "add sp, sp, #16\n\t"
        :
        : "r"(cnt)
        : "w20", "w21", "x30", "cc", "memory");
}

int main(void)
{
    BENCH_RUN("rastest", ras_test(100));
    return 0;
}
