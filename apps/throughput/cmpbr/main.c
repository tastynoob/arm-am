/*
 * Compare + branch throughput: 32 not-taken "cmp; b.ne" pairs per loop
 * iteration.
 *
 * This is the one benchmark that is intentionally mixed: the interesting
 * number is the retired-instruction count, which reveals whether the core
 * fuses each compare/branch pair into one macro-op (32 pairs would then
 * retire as ~32 instructions instead of 64).  ops/cyc counts pairs.
 */
#include <bench.h>

#define CMP8 \
    "cmp x10, x11\n\t" \
    "b.ne 2f\n\t" \
    "2:\n\t" \
    "cmp x10, x11\n\t" \
    "b.ne 2f\n\t" \
    "2:\n\t" \
    "cmp x10, x11\n\t" \
    "b.ne 2f\n\t" \
    "2:\n\t" \
    "cmp x10, x11\n\t" \
    "b.ne 2f\n\t" \
    "2:\n\t" \
    "cmp x10, x11\n\t" \
    "b.ne 2f\n\t" \
    "2:\n\t" \
    "cmp x10, x11\n\t" \
    "b.ne 2f\n\t" \
    "2:\n\t" \
    "cmp x10, x11\n\t" \
    "b.ne 2f\n\t" \
    "2:\n\t" \
    "cmp x10, x11\n\t" \
    "b.ne 2f\n\t" \
    "2:\n\t"

static uint64_t __attribute__((noinline))
cmp_branch_bench(uint64_t iters)
{
    uint64_t result;

    __asm__ volatile(
        "mov x10, #1\n\t"
        "mov x11, #1\n\t"
        "mov x12, %[iters]\n\t"
        "1:\n\t"
        CMP8 CMP8 CMP8 CMP8
        "subs x12, x12, #1\n\t"
        "b.ne 1b\n\t"
        "mov %[out], x12\n\t"
        : [out] "=r"(result)
        : [iters] "r"(iters)
        : "x10", "x11", "x12", "cc", "memory");

    return result;
}

int main(void)
{
    bench_banner("cmpbr");
    bench_run("cmpbr", cmp_branch_bench, 32, 0);
    return 0;
}
