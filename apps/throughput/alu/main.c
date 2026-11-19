/*
 * ALU throughput: 64 independent 64-bit adds per loop iteration.
 *
 * No memory is touched, so this measures rename/issue/ALU width and the
 * sustained integer ALU throughput of the core.
 */
#include <bench.h>

#define ALU8 \
    "add x1, x1, #1\n\t" \
    "add x2, x2, #1\n\t" \
    "add x3, x3, #1\n\t" \
    "add x4, x4, #1\n\t" \
    "add x5, x5, #1\n\t" \
    "add x6, x6, #1\n\t" \
    "add x7, x7, #1\n\t" \
    "add x8, x8, #1\n\t"

static uint64_t __attribute__((noinline))
alu_bench(uint64_t iters)
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
        "mov x9, %[iters]\n\t"
        "1:\n\t"
        ALU8 ALU8 ALU8 ALU8 ALU8 ALU8 ALU8 ALU8
        "subs x9, x9, #1\n\t"
        "b.ne 1b\n\t"
        "add %[out], x1, x2\n\t"
        : [out] "=r"(result)
        : [iters] "r"(iters)
        : "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
          "cc", "memory");

    return result;
}

int main(void)
{
    bench_banner("alu");
    bench_run(alu_bench);
    return 0;
}
