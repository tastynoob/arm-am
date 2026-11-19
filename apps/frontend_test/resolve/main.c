#include <bench.h>

__attribute__((optimize("O0")))
static void resolve_test(int cnt)
{
    volatile int A = 0;
    volatile int B = 1;
    volatile int C = 0;
    volatile int tmp = 0;

    while (cnt--) {
        if (A == B) {
            if (C > 0) {
                A = 0;
            }
            tmp = C;
            C = 0;
        } else {
            C = tmp + 1;
            A = 1;
        }
    }
}

int main(void)
{
    BENCH_RUN("resolve", resolve_test(100));
    return 0;
}
