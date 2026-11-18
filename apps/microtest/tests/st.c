/*
 * Scalar store throughput: 64 independent 64-bit stores per loop iteration,
 * spread over eight consecutive cache lines (512 B).
 *
 * The loop contains no loads, so ops/cyc is bounded by the L1 store path
 * (store ports, store buffer / store queue drain rate) rather than by load
 * resources.  Source registers are constant, so there is no data dependency
 * between iterations.
 *
 * Registers: x9 = base, x10 = loop counter, x1..x8 = sources.
 */
#include "bench.h"

#define ST8(off) \
    "str x1, [x9, #" #off "]\n\t" \
    "str x2, [x9, #" #off "+8]\n\t" \
    "str x3, [x9, #" #off "+16]\n\t" \
    "str x4, [x9, #" #off "+24]\n\t" \
    "str x5, [x9, #" #off "+32]\n\t" \
    "str x6, [x9, #" #off "+40]\n\t" \
    "str x7, [x9, #" #off "+48]\n\t" \
    "str x8, [x9, #" #off "+56]\n\t"

#define ST64 \
    ST8(0) ST8(64) ST8(128) ST8(192) \
    ST8(256) ST8(320) ST8(384) ST8(448)

static uint64_t __attribute__((noinline))
st_bench(uint64_t iters)
{
    uint64_t result;

    __asm__ volatile(
        "mov x1, #1\n\t"
        "mov x2, #2\n\t"
        "mov x3, #3\n\t"
        "mov x4, #4\n\t"
        "mov x5, #5\n\t"
        "mov x6, #6\n\t"
        "mov x7, #7\n\t"
        "mov x8, #8\n\t"
        "mov x9, %[base]\n\t"
        "mov x10, %[iters]\n\t"
        "1:\n\t"
        ST64
        "subs x10, x10, #1\n\t"
        "b.ne 1b\n\t"
        "ldr %[out], [x9]\n\t"
        : [out] "=r"(result)
        : [base] "r"(bench_buffer), [iters] "r"(iters)
        : "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8",
          "x9", "x10", "cc", "memory");

    return result;
}

int main(void)
{
    bench_banner("st");
    bench_warm_buffer();
    bench_run("st", st_bench, 64, 512);
    return 0;
}
