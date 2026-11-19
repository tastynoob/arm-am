/*
 * Scalar load throughput: 64 independent 64-bit loads per loop iteration,
 * spread over eight consecutive cache lines (512 B).
 *
 * The loop contains no stores, so ops/cyc is bounded by the L1 load path
 * (load ports, AGUs, tag/data bandwidth) and not by any store resource.
 * bench_warm_buffer() has already made the lines resident in L1D.
 *
 * Registers: x9 = base, x10 = loop counter, x1..x8 = destinations.
 */
#include <bench.h>

#define LD8(off) \
    "ldr x1, [x9, #" #off "]\n\t" \
    "ldr x2, [x9, #" #off "+8]\n\t" \
    "ldr x3, [x9, #" #off "+16]\n\t" \
    "ldr x4, [x9, #" #off "+24]\n\t" \
    "ldr x5, [x9, #" #off "+32]\n\t" \
    "ldr x6, [x9, #" #off "+40]\n\t" \
    "ldr x7, [x9, #" #off "+48]\n\t" \
    "ldr x8, [x9, #" #off "+56]\n\t"

#define LD64 \
    LD8(0) LD8(64) LD8(128) LD8(192) \
    LD8(256) LD8(320) LD8(384) LD8(448)

static uint64_t __attribute__((noinline))
ld_bench(uint64_t iters)
{
    uint64_t result;

    __asm__ volatile(
        "mov x9, %[base]\n\t"
        "mov x10, %[iters]\n\t"
        "1:\n\t"
        LD64
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
    bench_banner("ld");
    bench_warm_buffer();
    bench_run("ld", ld_bench, 64, 512);
    return 0;
}
