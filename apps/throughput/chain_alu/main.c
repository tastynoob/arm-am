/*
 * Chained ALU throughput.
 *
 * Each chain is a dependent sequence of `add` instructions: the next add to a
 * register uses the result of the previous add to that register.  With
 * CHAINS=1 this measures the ALU latency directly (cyc/op); with several
 * independent chains it measures how well the scheduler can interleave the
 * chains (ops/cyc).
 *
 * There are 8 add slots per chain in each loop iteration, and CHAINS can be
 * 1..8.  Parameters (make): CHAINS=4 ITERS=100000
 */
#include <bench.h>

#ifndef CHAINS
#define CHAINS 4
#endif
#if CHAINS < 1 || CHAINS > 8
#error CHAINS must be between 1 and 8
#endif

#define ADD1 "add x1, x1, #1\n\t"
#define ADD2 "add x2, x2, #1\n\t"
#define ADD3 "add x3, x3, #1\n\t"
#define ADD4 "add x4, x4, #1\n\t"
#define ADD5 "add x5, x5, #1\n\t"
#define ADD6 "add x6, x6, #1\n\t"
#define ADD7 "add x7, x7, #1\n\t"
#define ADD8 "add x8, x8, #1\n\t"

#if CHAINS == 1
#define CHAIN_BLOCK ADD1
#elif CHAINS == 2
#define CHAIN_BLOCK ADD1 ADD2
#elif CHAINS == 3
#define CHAIN_BLOCK ADD1 ADD2 ADD3
#elif CHAINS == 4
#define CHAIN_BLOCK ADD1 ADD2 ADD3 ADD4
#elif CHAINS == 5
#define CHAIN_BLOCK ADD1 ADD2 ADD3 ADD4 ADD5
#elif CHAINS == 6
#define CHAIN_BLOCK ADD1 ADD2 ADD3 ADD4 ADD5 ADD6
#elif CHAINS == 7
#define CHAIN_BLOCK ADD1 ADD2 ADD3 ADD4 ADD5 ADD6 ADD7
#else
#define CHAIN_BLOCK ADD1 ADD2 ADD3 ADD4 ADD5 ADD6 ADD7 ADD8
#endif

#define CHAIN8 \
    CHAIN_BLOCK CHAIN_BLOCK CHAIN_BLOCK CHAIN_BLOCK \
    CHAIN_BLOCK CHAIN_BLOCK CHAIN_BLOCK CHAIN_BLOCK

static uint64_t __attribute__((noinline))
chain_alu_bench(uint64_t iters)
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
        CHAIN8
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
    bench_banner("chain_alu");
    printf("chains=%u adds/iteration=%u\n", (unsigned)CHAINS,
           (unsigned)(CHAINS * 8));
    bench_run("chain_alu", chain_alu_bench, (uint64_t)CHAINS * 8U, 0);
    return 0;
}
