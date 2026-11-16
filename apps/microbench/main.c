/*
 * Small AArch64 microbench suite, adapted from the NEMU/nexus-am
 * minibench style: ALU throughput, L1 load/store throughput, L1 pair
 * load/store throughput, and cmp+branch fusion.
 *
 * All loops are written in inline assembly so the compiler cannot remove
 * or vectorise the operations being measured.  Cycles and retired
 * instructions come from the architectural PMU.
 */
#include <am.h>
#include <stdint.h>
#include <stdio.h>

#define ITERS 100000ULL

static volatile uint64_t sink;
static uint64_t buffer[1024] __attribute__((aligned(64)));

#define ALU8 \
    "add x1, x1, #1\n\t" \
    "add x2, x2, #1\n\t" \
    "add x3, x3, #1\n\t" \
    "add x4, x4, #1\n\t" \
    "add x5, x5, #1\n\t" \
    "add x6, x6, #1\n\t" \
    "add x7, x7, #1\n\t" \
    "add x8, x8, #1\n\t"

#define LOAD8 \
    "ldr x1, [x9, #0]\n\t" \
    "ldr x2, [x9, #8]\n\t" \
    "ldr x3, [x9, #16]\n\t" \
    "ldr x4, [x9, #24]\n\t" \
    "ldr x5, [x9, #32]\n\t" \
    "ldr x6, [x9, #40]\n\t" \
    "ldr x7, [x9, #48]\n\t" \
    "ldr x8, [x9, #56]\n\t"

#define STORE8 \
    "str x1, [x9, #64]\n\t" \
    "str x2, [x9, #72]\n\t" \
    "str x3, [x9, #80]\n\t" \
    "str x4, [x9, #88]\n\t" \
    "str x5, [x9, #96]\n\t" \
    "str x6, [x9, #104]\n\t" \
    "str x7, [x9, #112]\n\t" \
    "str x8, [x9, #120]\n\t"

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

#define CMP8 \
    "cmp x10, x11\n\t" \
    "b.ne 2f\n\t" \
    "2:\n\t" \
    "cmp x10, x11\n\t" \
    "b.ne 2f\n\t" \
    "2:\n\t" \
    "cmp x10, x11\n\t" \
    "b.ne 2f\n\t" \
    "2:\n\t" \
    "cmp x10, x11\n\t" \
    "b.ne 2f\n\t" \
    "2:\n\t" \
    "cmp x10, x11\n\t" \
    "b.ne 2f\n\t" \
    "2:\n\t" \
    "cmp x10, x11\n\t" \
    "b.ne 2f\n\t" \
    "2:\n\t" \
    "cmp x10, x11\n\t" \
    "b.ne 2f\n\t" \
    "2:\n\t" \
    "cmp x10, x11\n\t" \
    "b.ne 2f\n\t" \
    "2:\n\t"

typedef uint64_t (*bench_fn)(uint64_t);

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

static uint64_t __attribute__((noinline))
ldst_l1_bench(uint64_t iters)
{
    uint64_t result;

    __asm__ volatile(
        "mov x9, %[base]\n\t"
        "mov x10, %[iters]\n\t"
        "1:\n\t"
        LOAD8 STORE8
        "subs x10, x10, #1\n\t"
        "b.ne 1b\n\t"
        "add %[out], x1, x2\n\t"
        : [out] "=r"(result)
        : [base] "r"(buffer), [iters] "r"(iters)
        : "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8",
          "x9", "x10", "cc", "memory");

    return result;
}

static uint64_t __attribute__((noinline))
pair_l1_bench(uint64_t iters)
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
        : [base] "r"(buffer), [iters] "r"(iters)
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8",
          "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16",
          "x17", "cc", "memory");

    return result;
}

static uint64_t __attribute__((noinline))
cmp_branch_bench(uint64_t iters)
{
    uint64_t result;

    __asm__ volatile(
        "mov x10, #1\n\t"
        "mov x11, #1\n\t"
        "mov x12, %[iters]\n\t"
        "1:\n\t"
        CMP8 CMP8 CMP8 CMP8
        "subs x12, x12, #1\n\t"
        "b.ne 1b\n\t"
        "mov %[out], x12\n\t"
        : [out] "=r"(result)
        : [iters] "r"(iters)
        : "x10", "x11", "x12", "cc", "memory");

    return result;
}

static void report(const char *name, uint64_t cycles, uint64_t instret,
                   uint64_t ops, uint64_t bytes)
{
    uint64_t ops_x100 = cycles ? (ops * 100ULL) / cycles : 0;
    uint64_t cyc_x100 = ops ? (cycles * 100ULL) / ops : 0;
    uint64_t ipc_x100 = cycles ? (instret * 100ULL) / cycles : 0;

    printf("%-7s cycles=%llu instret=%llu ops/cyc=%llu.%02llu "
           "cyc/op=%llu.%02llu ipc=%llu.%02llu",
           name, (unsigned long long)cycles, (unsigned long long)instret,
           (unsigned long long)(ops_x100 / 100),
           (unsigned long long)(ops_x100 % 100),
           (unsigned long long)(cyc_x100 / 100),
           (unsigned long long)(cyc_x100 % 100),
           (unsigned long long)(ipc_x100 / 100),
           (unsigned long long)(ipc_x100 % 100));
    if (bytes) {
        uint64_t b_x100 = cycles ? (bytes * 100ULL) / cycles : 0;
        printf(" B/cyc=%llu.%02llu",
               (unsigned long long)(b_x100 / 100),
               (unsigned long long)(b_x100 % 100));
    }
    putchar('\n');
}

static void run(const char *name, bench_fn fn, uint64_t ops_per_iter,
                uint64_t bytes_per_iter)
{
    uint64_t cycle_start = am_read_cycles();
    uint64_t instret_start = am_read_instret();
    uint64_t result = fn(ITERS);
    uint64_t cycles = am_read_cycles() - cycle_start;
    uint64_t instret = am_read_instret() - instret_start;

    sink ^= result;
    report(name, cycles, instret, ops_per_iter * ITERS,
           bytes_per_iter * ITERS);
}

static void warm_buffer(void)
{
    volatile uint64_t checksum = 0;

    for (uint64_t index = 0; index < 1024; index++) {
        buffer[index] = index ^ 0xa5a5a5a5a5a5a5a5ULL;
        checksum += buffer[index];
    }
    sink = checksum;
}

int main(void)
{
    puts("arm-am: microbench");
    printf("PMU: cycle=%s instret=%s\n",
           am_pmu_available() ? "yes" : "no (CNTVCT fallback)",
           am_instret_available() ? "yes" : "no");

    warm_buffer();
    run("alu", alu_bench, 64, 0);
    run("ldst", ldst_l1_bench, 16, 128);
    run("pair", pair_l1_bench, 16, 256);
    run("cmpbr", cmp_branch_bench, 32, 0);

    printf("microbench: PASS sink=0x%016llx\n", (unsigned long long)sink);
    return 0;
}
