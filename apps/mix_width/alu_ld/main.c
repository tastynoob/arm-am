#include <bench.h>

/* Fixed-seed irregular mix: seed 0x5eed0001, 64 ops. */
static uint64_t __attribute__((noinline)) alu_ld_bench(uint64_t iters)
{
    uint64_t result;

    __asm__ volatile(
        "mov x0, #1\n\t"
        "mov x9, %[buf]\n\t"
        "mov x20, %[iters]\n\t"
        "1:\n\t"
        "add x1, x0, x0\n\t"
        "ldr w21, [x9, #0]\n\t"
        "ldr w22, [x9, #64]\n\t"
        "ldr w23, [x9, #128]\n\t"
        "ldr w24, [x9, #192]\n\t"
        "add x2, x0, x0\n\t"
        "ldr w25, [x9, #256]\n\t"
        "ldr w26, [x9, #320]\n\t"
        "ldr w27, [x9, #384]\n\t"
        "add x3, x0, x0\n\t"
        "add x4, x0, x0\n\t"
        "add x5, x0, x0\n\t"
        "add x6, x0, x0\n\t"
        "ldr w28, [x9, #448]\n\t"
        "ldr w21, [x9, #0]\n\t"
        "add x7, x0, x0\n\t"
        "add x8, x0, x0\n\t"
        "ldr w22, [x9, #64]\n\t"
        "ldr w23, [x9, #128]\n\t"
        "add x1, x0, x0\n\t"
        "ldr w24, [x9, #192]\n\t"
        "add x2, x0, x0\n\t"
        "ldr w25, [x9, #256]\n\t"
        "ldr w26, [x9, #320]\n\t"
        "ldr w27, [x9, #384]\n\t"
        "ldr w28, [x9, #448]\n\t"
        "ldr w21, [x9, #0]\n\t"
        "add x3, x0, x0\n\t"
        "add x4, x0, x0\n\t"
        "ldr w22, [x9, #64]\n\t"
        "ldr w23, [x9, #128]\n\t"
        "add x5, x0, x0\n\t"
        "ldr w24, [x9, #192]\n\t"
        "ldr w25, [x9, #256]\n\t"
        "add x6, x0, x0\n\t"
        "ldr w26, [x9, #320]\n\t"
        "add x7, x0, x0\n\t"
        "add x8, x0, x0\n\t"
        "add x1, x0, x0\n\t"
        "add x2, x0, x0\n\t"
        "add x3, x0, x0\n\t"
        "add x4, x0, x0\n\t"
        "ldr w27, [x9, #384]\n\t"
        "ldr w28, [x9, #448]\n\t"
        "add x5, x0, x0\n\t"
        "add x6, x0, x0\n\t"
        "add x7, x0, x0\n\t"
        "add x8, x0, x0\n\t"
        "ldr w21, [x9, #0]\n\t"
        "ldr w22, [x9, #64]\n\t"
        "add x1, x0, x0\n\t"
        "add x2, x0, x0\n\t"
        "ldr w23, [x9, #128]\n\t"
        "add x3, x0, x0\n\t"
        "add x4, x0, x0\n\t"
        "ldr w24, [x9, #192]\n\t"
        "ldr w25, [x9, #256]\n\t"
        "ldr w26, [x9, #320]\n\t"
        "add x5, x0, x0\n\t"
        "ldr w27, [x9, #384]\n\t"
        "add x6, x0, x0\n\t"
        "ldr w28, [x9, #448]\n\t"
        "add x7, x0, x0\n\t"
        "add x8, x0, x0\n\t"
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
          "w21",
          "w22",
          "w23",
          "w24",
          "w25",
          "w26",
          "w27",
          "w28",
          "cc",
          "memory");

    return result;
}

int main(void)
{
    bench_warm_buffer();
    bench_banner("mix_alu_ld");
    bench_run("mix_alu_ld", alu_ld_bench, 64, 0);
    return 0;
}
