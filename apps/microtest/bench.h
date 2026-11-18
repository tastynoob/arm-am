/*
 * Shared harness for the arm-am microtest benchmarks.
 *
 * Every benchmark is a standalone program under tests/: it includes this
 * header, implements one timed loop, and calls bench_run() from main().
 * suite.mk builds one .bin per test, so benchmarks never share a run, a
 * warm-up, or a PMU counter window with each other.
 */
#ifndef ARM_AM_MICROTEST_BENCH_H
#define ARM_AM_MICROTEST_BENCH_H

#include <am.h>
#include <stdint.h>
#include <stdio.h>

/*
 * Loop trip count.  Override from the command line with ITERS=<n>; suite.mk
 * forwards it as -DBENCH_ITERS=<n> (handy on slower models).
 */
#ifndef BENCH_ITERS
#define BENCH_ITERS 100000ULL
#endif

/* 8 KiB, 64-byte aligned: large enough for the L1-resident buffers used by
 * the memory tests and small enough to stay in L1D on every supported core. */
#define BENCH_BUFFER_U64 1024U

typedef uint64_t (*bench_fn)(uint64_t iterations);

static uint64_t bench_buffer[BENCH_BUFFER_U64]
    __attribute__((aligned(64), unused));
static volatile uint64_t bench_sink __attribute__((unused));

static inline void bench_banner(const char *name)
{
    printf("arm-am: microtest %s (iters=%llu)\n", name,
           (unsigned long long)BENCH_ITERS);
    printf("PMU: cycle=%s instret=%s\n",
           am_pmu_available() ? "yes" : "no (CNTVCT fallback)",
           am_instret_available() ? "yes" : "no");
}

/*
 * Fill the whole buffer and read it back through a volatile checksum.  The
 * timed loops only touch the first 512 B (scalar) or 1 KiB (pair), so this
 * also makes sure their lines are resident before the measurement starts.
 */
static inline void bench_warm_buffer(void)
{
    volatile uint64_t checksum = 0;

    for (uint64_t index = 0; index < BENCH_BUFFER_U64; index++) {
        bench_buffer[index] = index ^ 0xa5a5a5a5a5a5a5a5ULL;
        checksum += bench_buffer[index];
    }
    bench_sink ^= checksum;
}

static inline void bench_report(const char *name, uint64_t cycles,
                                uint64_t instret, uint64_t ops,
                                uint64_t bytes)
{
    uint64_t ops_x100 = cycles ? (ops * 100ULL) / cycles : 0;
    uint64_t cyc_x100 = ops ? (cycles * 100ULL) / ops : 0;
    uint64_t ipc_x100 = cycles ? (instret * 100ULL) / cycles : 0;

    printf("%-6s cycles=%llu instret=%llu ops/cyc=%llu.%02llu "
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

/*
 * Time one benchmark call and print the result line.  "ops" and "bytes" are
 * per loop iteration and describe what the loop body actually executes, so
 * the reported IPC includes the loop-control overhead while ops/cyc does not.
 */
static inline int bench_run(const char *name, bench_fn fn,
                            uint64_t ops_per_iter, uint64_t bytes_per_iter)
{
    uint64_t cycle_start = am_read_cycles();
    uint64_t instret_start = am_read_instret();
    uint64_t result = fn(BENCH_ITERS);
    uint64_t cycles = am_read_cycles() - cycle_start;
    uint64_t instret = am_read_instret() - instret_start;

    bench_sink ^= result;
    bench_report(name, cycles, instret, ops_per_iter * BENCH_ITERS,
                 bytes_per_iter * BENCH_ITERS);
    printf("microtest %s: PASS sink=0x%016llx\n", name,
           (unsigned long long)bench_sink);
    return 0;
}

#endif /* ARM_AM_MICROTEST_BENCH_H */
