#include <bench.h>

/* Fixed-seed irregular mix: seed 0x5eed0002, 64 ops. */
static uint64_t __attribute__((noinline)) alu_st_bench(uint64_t iters)
{
    uint64_t result;

    __asm__ volatile(
        "mov x0, #1\n\t"
        "mov x9, %[buf]\n\t"
        "mov x20, %[iters]\n\t"
        "1:\n\t"
        "str w0, [x9, #4096]\n\t"
        "str w0, [x9, #4160]\n\t"
        "add x1, x0, x0\n\t"
        "add x2, x0, x0\n\t"
        "str w0, [x9, #4224]\n\t"
        "add x3, x0, x0\n\t"
        "str w0, [x9, #4288]\n\t"
        "add x4, x0, x0\n\t"
        "str w0, [x9, #4352]\n\t"
        "str w0, [x9, #4416]\n\t"
        "add x5, x0, x0\n\t"
        "str w0, [x9, #4480]\n\t"
        "str w0, [x9, #4544]\n\t"
        "str w0, [x9, #4096]\n\t"
        "add x6, x0, x0\n\t"
        "str w0, [x9, #4160]\n\t"
        "add x7, x0, x0\n\t"
        "str w0, [x9, #4224]\n\t"
        "add x8, x0, x0\n\t"
        "str w0, [x9, #4288]\n\t"
        "str w0, [x9, #4352]\n\t"
        "str w0, [x9, #4416]\n\t"
        "str w0, [x9, #4480]\n\t"
        "add x1, x0, x0\n\t"
        "add x2, x0, x0\n\t"
        "add x3, x0, x0\n\t"
        "add x4, x0, x0\n\t"
        "str w0, [x9, #4544]\n\t"
        "add x5, x0, x0\n\t"
        "add x6, x0, x0\n\t"
        "str w0, [x9, #4096]\n\t"
        "str w0, [x9, #4160]\n\t"
        "add x7, x0, x0\n\t"
        "add x8, x0, x0\n\t"
        "add x1, x0, x0\n\t"
        "add x2, x0, x0\n\t"
        "str w0, [x9, #4224]\n\t"
        "add x3, x0, x0\n\t"
        "str w0, [x9, #4288]\n\t"
        "add x4, x0, x0\n\t"
        "str w0, [x9, #4352]\n\t"
        "add x5, x0, x0\n\t"
        "str w0, [x9, #4416]\n\t"
        "add x6, x0, x0\n\t"
        "add x7, x0, x0\n\t"
        "str w0, [x9, #4480]\n\t"
        "add x8, x0, x0\n\t"
        "str w0, [x9, #4544]\n\t"
        "add x1, x0, x0\n\t"
        "str w0, [x9, #4096]\n\t"
        "add x2, x0, x0\n\t"
        "str w0, [x9, #4160]\n\t"
        "add x3, x0, x0\n\t"
        "add x4, x0, x0\n\t"
        "str w0, [x9, #4224]\n\t"
        "add x5, x0, x0\n\t"
        "str w0, [x9, #4288]\n\t"
        "add x6, x0, x0\n\t"
        "str w0, [x9, #4352]\n\t"
        "add x7, x0, x0\n\t"
        "add x8, x0, x0\n\t"
        "str w0, [x9, #4416]\n\t"
        "str w0, [x9, #4480]\n\t"
        "str w0, [x9, #4544]\n\t"
        "9:\n\t"
        "subs x20, x20, #1\n\t"
        "b.ne 1b\n\t"
        "mov %[out], x0\n\t"
        : [out] "=r"(result)
        : [iters] "r"(iters), [buf] "r"(bench_buffer)
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
          "x9",
          "cc",
          "memory");

    return result;
}

int main(void)
{
    bench_warm_buffer();
    bench_banner("mix_alu_st");
    bench_run(alu_st_bench);
    return 0;
}
