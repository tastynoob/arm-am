#include <bench.h>

volatile int global_sum = 0;

__attribute__((noinline))
static int recursive_test(int depth)
{
    global_sum += depth;

    if (depth <= 0) {
        return global_sum;
    }

    int result = recursive_test(depth - 1) + 1;
    global_sum += result;
    return result;
}

int main(void)
{
    BENCH_RUN("ras_recursive", recursive_test(12));
    return 0;
}
