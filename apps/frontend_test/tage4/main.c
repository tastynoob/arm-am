#include <bench.h>

static void __attribute__((noinline)) tage4(unsigned long n)
{
    register unsigned long i;
    for (i = 0; i < n; i++) {
        if (i % 2 == 0) {
            __asm__ volatile("nop");
        }
    }
}

int main(void)
{
    BENCH_RUN("tage4", tage4(1000));
    return 0;
}
