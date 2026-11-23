/*
 * Mixed scalar load/store throughput: 8 scalar loads followed by 8 scalar
 * stores per loop iteration, all within the first 128 B of the 8 KiB
 * L1-resident bench_buffer.  The stores consume the loaded values, so this
 * keeps the original microbench's load-then-store dependency rather than
 * measuring loads and stores in isolation.
 *
 * Registers: x9 = base, x10 = loop counter, x1..x8 = loaded values.
 */
#include <bench.h>

#define LOAD8 \
    "ldr x1, [x9, #0]\n\t" \
    "ldr x2, [x9, #8]\n\t" \
    "ldr x3, [x9, #16]\n\t" \
    "ldr x4, [x9, #24]\n\t" \
    "ldr x5, [x9, #32]\n\t" \
    "ldr x6, [x9, #40]\n\t" \
    "ldr x7, [x9, #48]\n\t" \
    "ldr x8, [x9, #56]\n\t"

#define STORE8 \
    "str x1, [x9, #64]\n\t" \
    "str x2, [x9, #72]\n\t" \
    "str x3, [x9, #80]\n\t" \
    "str x4, [x9, #88]\n\t" \
    "str x5, [x9, #96]\n\t" \
    "str x6, [x9, #104]\n\t" \
    "str x7, [x9, #112]\n\t" \
    "str x8, [x9, #120]\n\t"

static uint64_t __attribute__((noinline))
ldst_bench(uint64_t iters)
{
    uint64_t result;

    __asm__ volatile(
        "mov x9, %[base]\n\t"
        "mov x10, %[iters]\n\t"
        "1:\n\t"
        LOAD8 STORE8
        "subs x10, x10, #1\n\t"
        "b.ne 1b\n\t"
        "add %[out], x1, x2\n\t"
        : [out] "=r"(result)
        : [base] "r"(bench_buffer), [iters] "r"(iters)
        : "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8",
          "x9", "x10", "cc", "memory");

    return result;
}

int main(void)
{
    bench_banner("ldst");
    bench_warm_buffer();
    bench_run(ldst_bench);
    return 0;
}
