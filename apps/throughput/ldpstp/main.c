/*
 * Mixed pair load/store throughput: 8 LDP pair loads followed by 8 STP pair
 * stores per loop iteration.  The stores consume the loaded register pairs,
 * preserving the original microbench's pair load-then-store dependency.
 *
 * The 256 B working window (loads 0..127, stores 128..255) is inside the
 * 8 KiB L1-resident bench_buffer.  x9 = base, x17 = loop counter,
 * x0..x16 = pair values.
 */
#include <bench.h>

#define LDP8 \
    "ldp x1, x2, [x9, #0]\n\t" \
    "ldp x3, x4, [x9, #16]\n\t" \
    "ldp x5, x6, [x9, #32]\n\t" \
    "ldp x7, x8, [x9, #48]\n\t" \
    "ldp x0, x10, [x9, #64]\n\t" \
    "ldp x11, x12, [x9, #80]\n\t" \
    "ldp x13, x14, [x9, #96]\n\t" \
    "ldp x15, x16, [x9, #112]\n\t"

#define STP8 \
    "stp x1, x2, [x9, #128]\n\t" \
    "stp x3, x4, [x9, #144]\n\t" \
    "stp x5, x6, [x9, #160]\n\t" \
    "stp x7, x8, [x9, #176]\n\t" \
    "stp x0, x10, [x9, #192]\n\t" \
    "stp x11, x12, [x9, #208]\n\t" \
    "stp x13, x14, [x9, #224]\n\t" \
    "stp x15, x16, [x9, #240]\n\t"

static uint64_t __attribute__((noinline))
ldpstp_bench(uint64_t iters)
{
    uint64_t result;

    __asm__ volatile(
        "mov x9, %[base]\n\t"
        "mov x17, %[iters]\n\t"
        "1:\n\t"
        LDP8 STP8
        "subs x17, x17, #1\n\t"
        "b.ne 1b\n\t"
        "add %[out], x1, x2\n\t"
        : [out] "=r"(result)
        : [base] "r"(bench_buffer), [iters] "r"(iters)
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
          "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
          "x16", "x17", "cc", "memory");

    return result;
}

int main(void)
{
    bench_banner("ldpstp");
    bench_warm_buffer();
    bench_run(ldpstp_bench);
    return 0;
}
