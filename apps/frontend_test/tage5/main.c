#include <bench.h>

#define FORCE_BRANCH(cond) \
    __asm__ volatile("cbnz %w0, 1f\n\t" \
                     "nop\n\t" \
                     "1:\n\t" \
                     "nop\n\t" \
                     : \
                     : "r"(cond) \
                     : "memory")

static void run_separator(void)
{
    for (int i = 0; i < 50; i++) {
        FORCE_BRANCH(1);
    }
}

static void test_bimodal_toggle(void)
{
    int iterations = 4000;
    for (int i = 0; i < iterations; i++) {
        int cond = (i % 2 != 0);
        FORCE_BRANCH(cond);
    }
}

static void test_long_history(void)
{
    int pattern[] = {1, 0, 0, 1, 1, 0, 1, 0, 0, 0,
                     1, 1, 1, 0, 1, 0, 1, 1, 0, 0};
    int len = 20;
    int iterations = 10000;

    for (int i = 0; i < iterations; i++) {
        int cond = pattern[i % len];
        FORCE_BRANCH(cond);
    }
}

static void recursive_brancher(int depth)
{
    if (depth == 0)
        return;
    int cond1 = (depth % 3 == 0);
    FORCE_BRANCH(cond1);
    recursive_brancher(depth - 1);
    int cond2 = (depth % 2 == 0);
    FORCE_BRANCH(cond2);
}

static void test_path_interference(void)
{
    int iterations = 500;
    for (int i = 0; i < iterations; i++) {
        recursive_brancher(10);
    }
}

static void test_ubit_set(void)
{
    int pattern[] = {1, 1, 0};
    int len = 3;
    int iterations = 3000;

    for (int i = 0; i < iterations; i++) {
        int cond = pattern[i % len];
        FORCE_BRANCH(cond);
    }
}

static void test_ubit_decay(void)
{
    int pattern[] = {1, 1, 1, 0, 0};
    int len = 5;

    for (int i = 0; i < 2000; i++) {
        int cond = pattern[i % len];
        FORCE_BRANCH(cond);
    }

    int age_cycles = 100000;
    for (int i = 0; i < age_cycles; i++) {
        FORCE_BRANCH(1);
    }

    for (int i = 0; i < 20; i++) {
        int cond = pattern[i % len];
        FORCE_BRANCH(cond);
    }
}

static void run_tage5(void)
{
    test_bimodal_toggle();
    run_separator();

    test_long_history();
    run_separator();

    test_path_interference();
    run_separator();

    test_ubit_set();
    run_separator();

    test_ubit_decay();
}

int main(void)
{
    BENCH_RUN("tage5", run_tage5());
    return 0;
}
