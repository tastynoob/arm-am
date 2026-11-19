/*
 * Shared harness for the arm-am microbenchmark apps.
 *
 * A benchmark is a standalone app under apps/throughput/<name>/ or apps/<name>/:
 * its main.c includes this header, implements one timed loop, and calls
 * bench_run() (or uses the pass helpers).  Each app builds one .bin, so
 * benchmarks never share a run, a warm-up, or a PMU counter window.
 */
#ifndef ARM_AM_BENCH_H
#define ARM_AM_BENCH_H

#include <am.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef BENCH_APP
#define BENCH_APP "unknown"
#endif
#ifndef BENCH_TEST
#define BENCH_TEST "unknown"
#endif

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

static inline void bench_pmu_banner(void)
{
    printf("PMU: cycle=%s instret=%s\n",
           am_pmu_available() ? "yes" : "no (CNTVCT fallback)",
           am_instret_available() ? "yes" : "no");
}

static inline void bench_banner(const char *name)
{
    printf("arm-am: bench %s (iters=%llu)\n", name,
           (unsigned long long)BENCH_ITERS);
    bench_pmu_banner();
}

/* Banner for tests whose trip count is not BENCH_ITERS (streaming passes). */
static inline void bench_banner_plain(const char *name)
{
    printf("arm-am: bench %s\n", name);
    bench_pmu_banner();
}

/*
 * Large-footprint tests allocate their working set from the AM heap instead
 * of .bss, so the boot-time BSS clear never touches a 64 MiB+ buffer.  _sbrk
 * advances the heap break; the returned pointer is aligned to 64 B.
 */
extern void *_sbrk(ptrdiff_t increment);

static inline void *bench_alloc(uint64_t bytes)
{
    void *raw;

    if (bytes == 0) {
        return NULL;
    }
    raw = _sbrk((ptrdiff_t)(bytes + 63U));
    if (raw == (void *)-1) {
        return NULL;
    }
    return (void *)(((uintptr_t)raw + 63U) & ~(uintptr_t)63U);
}

/* Parse "64M", "1G", "512K" or a plain byte count. */
static inline uint64_t bench_parse_size(const char *text)
{
    uint64_t value = 0;

    while (*text >= '0' && *text <= '9') {
        value = value * 10U + (uint64_t)(*text - '0');
        text++;
    }
    if (*text == 'K' || *text == 'k') {
        value <<= 10;
    } else if (*text == 'M' || *text == 'm') {
        value <<= 20;
    } else if (*text == 'G' || *text == 'g') {
        value <<= 30;
    }
    return value;
}

/* Untimed setup helpers: a fill pass and a separate-buffer cache flush. */
static inline void bench_fill(uint64_t base, uint64_t bytes, uint64_t seed)
{
    for (uint64_t offset = 0; offset < bytes; offset += 8) {
        *(volatile uint64_t *)(uintptr_t)(base + offset) = seed ^ offset;
    }
}

static inline void bench_flush_cache(uint64_t base, uint64_t bytes)
{
    volatile uint64_t checksum = 0;

    for (uint64_t offset = 0; offset < bytes; offset += 64) {
        checksum += *(const volatile uint64_t *)(uintptr_t)(base + offset);
    }
    bench_sink ^= checksum;
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

/*
 * Canonical machine-readable result.  `test` and `ipc` are mandatory; the
 * extra key=value fields are caller-defined.  This is the only line scripts
 * need to parse; human-friendly bandwidth/latency output may remain separate.
 */
static inline void bench_format(uint64_t cycles, uint64_t instret,
                                const char *fmt, ...)
{
    uint64_t ipc_x100 = cycles ? (instret * 100ULL) / cycles : 0;

    printf("BENCH_FORMAT test=%s/%s ipc=%llu.%02llu",
           BENCH_APP, BENCH_TEST,
           (unsigned long long)(ipc_x100 / 100),
           (unsigned long long)(ipc_x100 % 100));
    if (fmt && *fmt) {
        va_list ap;

        putchar(' ');
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
    }
    putchar('\n');
}

static inline int bench_run(bench_fn fn)
{
    uint64_t cycle_start = am_read_cycles();
    uint64_t instret_start = am_read_instret();
    uint64_t result = fn(BENCH_ITERS);
    uint64_t cycles = am_read_cycles() - cycle_start;
    uint64_t instret = am_read_instret() - instret_start;

    bench_sink ^= result;
    bench_format(cycles, instret,
                 "cycles=%llu instret=%llu status=PASS",
                 (unsigned long long)cycles, (unsigned long long)instret);
    return 0;
}

/*
 * Time an arbitrary test expression and emit the canonical line.  The `name`
 * is only used for the startup banner; the result line uses BENCH_APP/TEST.
 */
#define BENCH_RUN(name, expr) do {                                      \
    bench_banner_plain(name);                                           \
    uint64_t _bench_cyc0 = am_read_cycles();                            \
    uint64_t _bench_ins0 = am_read_instret();                           \
    expr;                                                               \
    uint64_t _bench_cyc = am_read_cycles() - _bench_cyc0;               \
    uint64_t _bench_ins = am_read_instret() - _bench_ins0;              \
    bench_format(_bench_cyc, _bench_ins,                                 \
                 "cycles=%llu instret=%llu status=PASS",                    \
                 (unsigned long long)_bench_cyc,                            \
                 (unsigned long long)_bench_ins);                           \
} while (0)

#endif /* ARM_AM_BENCH_H */
