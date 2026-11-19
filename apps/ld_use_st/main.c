/*
 * Load-use-store chain (read-modify-writeback).
 *
 * One cache line is repeatedly loaded, incremented and stored back.  The
 * store and the next iteration's load are to the same address, so the chain
 * is
 *
 *     load -> add -> store -> load -> add -> store -> ...
 *
 * and cycles/op is the sustained load-use-store latency (store-to-load
 * forwarding included).  The loop counter and branch are independent, so the
 * measurement is not limited by loop control.
 *
 * Parameter (make): ITERS=100000
 */
#include <bench.h>

static uint64_t rmw_value __attribute__((aligned(64))) = 1;

static uint64_t __attribute__((noinline))
rmw_bench(uint64_t iters)
{
    uint64_t result;

    __asm__ volatile(
        "mov x0, %[addr]\n\t"
        "mov x2, %[iters]\n\t"
        "ldr x1, [x0]\n\t"
        "1:\n\t"
        "add x1, x1, #1\n\t"
        "str x1, [x0]\n\t"
        "ldr x1, [x0]\n\t"
        "subs x2, x2, #1\n\t"
        "b.ne 1b\n\t"
        "mov %[out], x1\n\t"
        : [out] "=r"(result)
        : [addr] "r"(&rmw_value), [iters] "r"(iters)
        : "x0", "x1", "x2", "cc", "memory");

    return result;
}

int main(void)
{
    bench_banner("ld_use_st");
    printf("same-line chain: load -> add -> store -> load\n");
    bench_run("ld_use_st", rmw_bench, 1, 0);
    return 0;
}
