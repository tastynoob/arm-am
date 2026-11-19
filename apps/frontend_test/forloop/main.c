#include <bench.h>

static void three_level_loop_test(int outer_iterations, int middle_iterations,
                                  int inner_iterations)
{
    volatile int result = 0;

    for (int i = 0; i < outer_iterations; i++) {
        for (int j = 0; j < middle_iterations; j++) {
            for (int k = 0; k < inner_iterations; k++) {
                result += (i + j + k);
            }
        }
    }
}

static void run_forloop(void)
{
    for (int i = 0; i < 10; i++) {
        three_level_loop_test(10, 10, 10);
    }
}

int main(void)
{
    BENCH_RUN("forloop", run_forloop());
    return 0;
}
