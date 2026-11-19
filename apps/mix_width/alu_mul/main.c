#include <bench.h>

/* Fixed-seed irregular mix: seed 0x5eed0003, 64 ops. */
static uint64_t __attribute__((noinline)) alu_mul_bench(uint64_t iters)
{
    uint64_t result;

    __asm__ volatile(
        "mov x0, #1\n\t"
        "mov x20, %[iters]\n\t"
        "1:\n\t"
        "add x1, x0, x0\n\t"
        "add x2, x0, x0\n\t"
        "add x3, x0, x0\n\t"
        "mul x10, x0, x0\n\t"
        "mul x11, x0, x0\n\t"
        "add x4, x0, x0\n\t"
        "mul x12, x0, x0\n\t"
        "add x5, x0, x0\n\t"
        "mul x13, x0, x0\n\t"
        "add x6, x0, x0\n\t"
        "mul x14, x0, x0\n\t"
        "mul x15, x0, x0\n\t"
        "mul x16, x0, x0\n\t"
        "mul x17, x0, x0\n\t"
        "mul x10, x0, x0\n\t"
        "add x7, x0, x0\n\t"
        "add x8, x0, x0\n\t"
        "add x1, x0, x0\n\t"
        "add x2, x0, x0\n\t"
        "add x3, x0, x0\n\t"
        "mul x11, x0, x0\n\t"
        "mul x12, x0, x0\n\t"
        "mul x13, x0, x0\n\t"
        "mul x14, x0, x0\n\t"
        "add x4, x0, x0\n\t"
        "mul x15, x0, x0\n\t"
        "mul x16, x0, x0\n\t"
        "add x5, x0, x0\n\t"
        "mul x17, x0, x0\n\t"
        "mul x10, x0, x0\n\t"
        "add x6, x0, x0\n\t"
        "add x7, x0, x0\n\t"
        "add x8, x0, x0\n\t"
        "mul x11, x0, x0\n\t"
        "mul x12, x0, x0\n\t"
        "mul x13, x0, x0\n\t"
        "add x1, x0, x0\n\t"
        "mul x14, x0, x0\n\t"
        "mul x15, x0, x0\n\t"
        "mul x16, x0, x0\n\t"
        "mul x17, x0, x0\n\t"
        "add x2, x0, x0\n\t"
        "add x3, x0, x0\n\t"
        "add x4, x0, x0\n\t"
        "add x5, x0, x0\n\t"
        "add x6, x0, x0\n\t"
        "mul x10, x0, x0\n\t"
        "mul x11, x0, x0\n\t"
        "mul x12, x0, x0\n\t"
        "add x7, x0, x0\n\t"
        "add x8, x0, x0\n\t"
        "mul x13, x0, x0\n\t"
        "add x1, x0, x0\n\t"
        "add x2, x0, x0\n\t"
        "mul x14, x0, x0\n\t"
        "add x3, x0, x0\n\t"
        "add x4, x0, x0\n\t"
        "add x5, x0, x0\n\t"
        "mul x15, x0, x0\n\t"
        "mul x16, x0, x0\n\t"
        "add x6, x0, x0\n\t"
        "add x7, x0, x0\n\t"
        "add x8, x0, x0\n\t"
        "mul x17, x0, x0\n\t"
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
          "x10",
          "x11",
          "x12",
          "x13",
          "x14",
          "x15",
          "x16",
          "x17",
          "cc",
          "memory");

    return result;
}

int main(void)
{
    bench_banner("mix_alu_mul");
    bench_run("mix_alu_mul", alu_mul_bench, 64, 0);
    return 0;
}
