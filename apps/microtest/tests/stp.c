/*
 * Pair-store throughput: 64 independent STP (64-bit pair) stores per loop
 * iteration, covering 1 KiB across sixteen consecutive cache lines.
 *
 * The loop contains no loads, so ops/cyc isolates the store path for the
 * pair form.  Source registers are constant and never reloaded.
 *
 * Registers: x9 = base (center of the 1 KiB window), x8 = loop counter,
 * x0..x7/x10..x17 = sources.  The base is centered because the STP
 * immediate range is only -512..504.
 */
#include "bench.h"

#define STP8(off) \
    "stp x0, x1, [x9, #" #off "]\n\t" \
    "stp x2, x3, [x9, #" #off "+16]\n\t" \
    "stp x4, x5, [x9, #" #off "+32]\n\t" \
    "stp x6, x7, [x9, #" #off "+48]\n\t" \
    "stp x10, x11, [x9, #" #off "+64]\n\t" \
    "stp x12, x13, [x9, #" #off "+80]\n\t" \
    "stp x14, x15, [x9, #" #off "+96]\n\t" \
    "stp x16, x17, [x9, #" #off "+112]\n\t"

#define STP64 \
    STP8(-512) STP8(-384) \
    STP8(-256) STP8(-128) \
    STP8(0) STP8(128) \
    STP8(256) STP8(384)

static uint64_t __attribute__((noinline))
stp_bench(uint64_t iters)
{
    uint64_t result;

    /* bench_buffer + 64 elements = byte 512, the centre of the window. */
    __asm__ volatile(
        "mov x0, #0\n\t"
        "mov x1, #1\n\t"
        "mov x2, #2\n\t"
        "mov x3, #3\n\t"
        "mov x4, #4\n\t"
        "mov x5, #5\n\t"
        "mov x6, #6\n\t"
        "mov x7, #7\n\t"
        "mov x10, #10\n\t"
        "mov x11, #11\n\t"
        "mov x12, #12\n\t"
        "mov x13, #13\n\t"
        "mov x14, #14\n\t"
        "mov x15, #15\n\t"
        "mov x16, #16\n\t"
        "mov x17, #17\n\t"
        "mov x9, %[base]\n\t"
        "mov x8, %[iters]\n\t"
        "1:\n\t"
        STP64
        "subs x8, x8, #1\n\t"
        "b.ne 1b\n\t"
        "ldr %[out], [x9]\n\t"
        : [out] "=r"(result)
        : [base] "r"(bench_buffer + 64), [iters] "r"(iters)
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
          "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
          "x16", "x17", "cc", "memory");

    return result;
}

int main(void)
{
    bench_banner("stp");
    bench_warm_buffer();
    bench_run("stp", stp_bench, 64, 1024);
    return 0;
}
