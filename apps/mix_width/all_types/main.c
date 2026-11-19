#include <bench.h>

/* Fixed-seed irregular mix: seed 0x5eed0008, 48 ops. */
static uint64_t __attribute__((noinline)) all_types_bench(uint64_t iters)
{
    uint64_t result;

    __asm__ volatile(
        "mov x0, #1\n\t"
        "fmov d30, #1.0\n\t"
        "fmov d31, #2.0\n\t"
        "mov x9, %[buf]\n\t"
        "mov x20, %[iters]\n\t"
        "1:\n\t"
        "str w0, [x9, #4096]\n\t"
        "add x1, x0, x0\n\t"
        "add x2, x0, x0\n\t"
        "fadd d0, d30, d31\n\t"
        "fadd d1, d30, d31\n\t"
        "str w0, [x9, #4160]\n\t"
        "cbz x0, 9f\n\t"
        "str w0, [x9, #4224]\n\t"
        "add x3, x0, x0\n\t"
        "fadd d2, d30, d31\n\t"
        "str w0, [x9, #4288]\n\t"
        "cbz x0, 9f\n\t"
        "mul x10, x0, x0\n\t"
        "ldr w21, [x9, #0]\n\t"
        "fadd d3, d30, d31\n\t"
        "fadd d4, d30, d31\n\t"
        "add x4, x0, x0\n\t"
        "ldr w22, [x9, #64]\n\t"
        "mul x11, x0, x0\n\t"
        "add x5, x0, x0\n\t"
        "cbz x0, 9f\n\t"
        "mul x12, x0, x0\n\t"
        "mul x13, x0, x0\n\t"
        "fadd d5, d30, d31\n\t"
        "cbz x0, 9f\n\t"
        "str w0, [x9, #4352]\n\t"
        "mul x14, x0, x0\n\t"
        "ldr w23, [x9, #128]\n\t"
        "add x6, x0, x0\n\t"
        "ldr w24, [x9, #192]\n\t"
        "ldr w25, [x9, #256]\n\t"
        "str w0, [x9, #4416]\n\t"
        "cbz x0, 9f\n\t"
        "str w0, [x9, #4480]\n\t"
        "mul x15, x0, x0\n\t"
        "mul x16, x0, x0\n\t"
        "cbz x0, 9f\n\t"
        "str w0, [x9, #4544]\n\t"
        "ldr w26, [x9, #320]\n\t"
        "cbz x0, 9f\n\t"
        "mul x17, x0, x0\n\t"
        "add x7, x0, x0\n\t"
        "ldr w27, [x9, #384]\n\t"
        "ldr w28, [x9, #448]\n\t"
        "add x8, x0, x0\n\t"
        "fadd d6, d30, d31\n\t"
        "cbz x0, 9f\n\t"
        "fadd d7, d30, d31\n\t"
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
          "x10",
          "x11",
          "x12",
          "x13",
          "x14",
          "x15",
          "x16",
          "x17",
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
    bench_banner("mix_all_types");
    bench_run("mix_all_types", all_types_bench, 48, 0);
    return 0;
}
