/*
 * Pair-load throughput: 64 independent LDP (64-bit pair) loads per loop
 * iteration, covering 1 KiB across sixteen consecutive cache lines.
 *
 * LDP is the AArch64 two-register load that most cores crack into a single
 * load-port operation; measuring it separately from STP shows whether the
 * pair form has a different sustained load rate than scalar LDR.
 *
 * Registers: x9 = base (center of the 1 KiB window), x8 = loop counter,
 * x0..x7/x10..x17 = destinations.  The base is centered because the LDP
 * immediate range is only -512..504.
 */
#include <bench.h>

#define LDP8(off) \
    "ldp x0, x1, [x9, #" #off "]\n\t" \
    "ldp x2, x3, [x9, #" #off "+16]\n\t" \
    "ldp x4, x5, [x9, #" #off "+32]\n\t" \
    "ldp x6, x7, [x9, #" #off "+48]\n\t" \
    "ldp x10, x11, [x9, #" #off "+64]\n\t" \
    "ldp x12, x13, [x9, #" #off "+80]\n\t" \
    "ldp x14, x15, [x9, #" #off "+96]\n\t" \
    "ldp x16, x17, [x9, #" #off "+112]\n\t"

#define LDP64 \
    LDP8(-512) LDP8(-384) \
    LDP8(-256) LDP8(-128) \
    LDP8(0) LDP8(128) \
    LDP8(256) LDP8(384)

static uint64_t __attribute__((noinline))
ldp_bench(uint64_t iters)
{
    uint64_t result;

    /* bench_buffer + 64 elements = byte 512, the centre of the window. */
    __asm__ volatile(
        "mov x9, %[base]\n\t"
        "mov x8, %[iters]\n\t"
        "1:\n\t"
        LDP64
        "subs x8, x8, #1\n\t"
        "b.ne 1b\n\t"
        "add %[out], x0, x1\n\t"
        : [out] "=r"(result)
        : [base] "r"(bench_buffer + 64), [iters] "r"(iters)
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
          "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
          "x16", "x17", "cc", "memory");

    return result;
}

int main(void)
{
    bench_banner("ldp");
    bench_warm_buffer();
    bench_run("ldp", ldp_bench, 64, 1024);
    return 0;
}
