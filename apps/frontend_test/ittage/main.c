#include <bench.h>

#define FORCE_BRANCH(cond) \
    __asm__ volatile("cbnz %w0, 1f\n\t" \
                     "nop\n\t" \
                     "1:\n\t" \
                     "nop\n\t" \
                     : \
                     : "r"(cond) \
                     : "memory")

typedef void (*func_ptr_t)(void);

static void __attribute__((noinline)) target_1(void) { __asm__ volatile("nop"); }
static void __attribute__((noinline)) target_2(void) { __asm__ volatile("nop"); }
static void __attribute__((noinline)) target_3(void) { __asm__ volatile("nop"); }
static void __attribute__((noinline)) target_4(void) { __asm__ volatile("nop"); }

#define FORCE_INDIRECT_CALL(target_addr) do { \
    volatile func_ptr_t _target = (target_addr); \
    _target(); \
} while (0)

static void run_separator(void)
{
    for (int i = 0; i < 50; i++) {
        FORCE_BRANCH(1);
    }
}

static void test_simple_correlation(void)
{
    int iterations = 2000;
    func_ptr_t t1 = target_1;
    func_ptr_t t2 = target_2;

    for (int i = 0; i < iterations; i++) {
        int cond = (i % 2 != 0);
        FORCE_BRANCH(cond);
        FORCE_INDIRECT_CALL(cond ? t1 : t2);
    }
}

static void test_complex_correlation(void)
{
    int iterations = 4000;
    func_ptr_t targets[4] = {target_1, target_2, target_3, target_4};

    for (int i = 0; i < iterations; i++) {
        int pattern = i % 4;
        int cond1 = (pattern >> 1) & 1;
        int cond2 = (pattern & 1);

        FORCE_BRANCH(cond1);
        FORCE_BRANCH(cond2);
        FORCE_INDIRECT_CALL(targets[pattern]);
    }
}

static void test_path_correlation(void)
{
    int iterations = 2000;
    func_ptr_t t1 = target_1;
    func_ptr_t t2 = target_2;

    for (int i = 0; i < iterations; i++) {
        int is_path_a = (i % 2 == 0);

        if (is_path_a) {
            FORCE_BRANCH(1);
            FORCE_BRANCH(1);
            FORCE_BRANCH(1);
            FORCE_BRANCH(0);
            FORCE_INDIRECT_CALL(t1);
        } else {
            FORCE_BRANCH(0);
            FORCE_BRANCH(0);
            FORCE_BRANCH(0);
            FORCE_BRANCH(1);
            FORCE_INDIRECT_CALL(t2);
        }
    }
}

static void test_stability(void)
{
    int iterations = 2000;
    func_ptr_t t1 = target_1;

    for (int i = 0; i < iterations; i++) {
        int cond = (i % 3 == 0);
        FORCE_BRANCH(cond);
        FORCE_INDIRECT_CALL(t1);
    }
}

static void run_ittage(void)
{
    test_simple_correlation();
    run_separator();
    test_complex_correlation();
    run_separator();
    test_path_correlation();
    run_separator();
    test_stability();
}

int main(void)
{
    BENCH_RUN("ittage", run_ittage());
    return 0;
}
