#include <bench.h>

/* Fixed-seed irregular mix: seed 0x5eed0000, 64 ops. */
static uint64_t __attribute__((noinline)) alu_fp_bench(uint64_t iters)
{
    uint64_t result;

    __asm__ volatile(
        "mov x0, #1\n\t"
        "fmov d30, #1.0\n\t"
        "fmov d31, #2.0\n\t"
        "mov x20, %[iters]\n\t"
        "1:\n\t"
        "add x1, x0, x0\n\t"
        "add x2, x0, x0\n\t"
        "fadd d0, d30, d31\n\t"
        "fadd d1, d30, d31\n\t"
        "fadd d2, d30, d31\n\t"
        "add x3, x0, x0\n\t"
        "fadd d3, d30, d31\n\t"
        "add x4, x0, x0\n\t"
        "add x5, x0, x0\n\t"
        "add x6, x0, x0\n\t"
        "fadd d4, d30, d31\n\t"
        "fadd d5, d30, d31\n\t"
        "fadd d6, d30, d31\n\t"
        "fadd d7, d30, d31\n\t"
        "fadd d0, d30, d31\n\t"
        "fadd d1, d30, d31\n\t"
        "add x7, x0, x0\n\t"
        "add x8, x0, x0\n\t"
        "fadd d2, d30, d31\n\t"
        "fadd d3, d30, d31\n\t"
        "fadd d4, d30, d31\n\t"
        "add x1, x0, x0\n\t"
        "add x2, x0, x0\n\t"
        "fadd d5, d30, d31\n\t"
        "fadd d6, d30, d31\n\t"
        "fadd d7, d30, d31\n\t"
        "fadd d0, d30, d31\n\t"
        "fadd d1, d30, d31\n\t"
        "fadd d2, d30, d31\n\t"
        "add x3, x0, x0\n\t"
        "fadd d3, d30, d31\n\t"
        "add x4, x0, x0\n\t"
        "add x5, x0, x0\n\t"
        "add x6, x0, x0\n\t"
        "fadd d4, d30, d31\n\t"
        "fadd d5, d30, d31\n\t"
        "add x7, x0, x0\n\t"
        "add x8, x0, x0\n\t"
        "add x1, x0, x0\n\t"
        "add x2, x0, x0\n\t"
        "fadd d6, d30, d31\n\t"
        "add x3, x0, x0\n\t"
        "add x4, x0, x0\n\t"
        "fadd d7, d30, d31\n\t"
        "add x5, x0, x0\n\t"
        "add x6, x0, x0\n\t"
        "fadd d0, d30, d31\n\t"
        "fadd d1, d30, d31\n\t"
        "add x7, x0, x0\n\t"
        "add x8, x0, x0\n\t"
        "add x1, x0, x0\n\t"
        "fadd d2, d30, d31\n\t"
        "add x2, x0, x0\n\t"
        "fadd d3, d30, d31\n\t"
        "add x3, x0, x0\n\t"
        "add x4, x0, x0\n\t"
        "add x5, x0, x0\n\t"
        "fadd d4, d30, d31\n\t"
        "add x6, x0, x0\n\t"
        "add x7, x0, x0\n\t"
        "add x8, x0, x0\n\t"
        "fadd d5, d30, d31\n\t"
        "fadd d6, d30, d31\n\t"
        "fadd d7, d30, d31\n\t"
        "9:\n\t"
        "subs x20, x20, #1\n\t"
        "b.ne 1b\n\t"
        "mov %[out], x0\n\t"
        : [out] "=r"(result)
        : [iters] "r"(iters)
        : "x0",
          "x1",
          "x2",
          "x3",
          "x4",
          "x5",
          "x6",
          "x7",
          "x8",
          "x20",
          "v0",
          "v1",
          "v2",
          "v3",
          "v4",
          "v5",
          "v6",
          "v7",
          "v30",
          "v31",
          "cc",
          "memory");

    return result;
}

int main(void)
{
    bench_banner("mix_alu_fp");
    bench_run(alu_fp_bench);
    return 0;
}
