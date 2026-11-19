/*
 * Streaming chained load (pointer chasing) over a configurable footprint.
 *
 * The footprint is organised as 64-byte nodes.  Node 0 is the head and each
 * node's first word points at the next node; the chain is a single cycle
 * built with a full-period LCG, so one pass follows `nodes` dependent loads
 * and returns to the head.  Unlike `stream_ld` these loads are serial: each
 * address comes from the previous load.
 *
 * PASSES=1 measures the cold chain (> LLC => DDR latency); PASSES=2+ starts
 * the next pass with the cache state left by the previous one.  A separate
 * FLUSH buffer evicts the footprint before pass 0 (FLUSH=0 disables it).
 *
 * Parameters (make): FOOTPRINT=1M PASSES=2 FLUSH=64M
 * (64M footprint = 1,048,576 dependent loads per pass; too slow for a
 * detailed model, but useful on RTL/real hardware.)
 */
#include <bench.h>

#ifndef FOOTPRINT
#define FOOTPRINT "1M"
#endif
#ifndef PASSES
#define PASSES 1
#endif
#ifndef FLUSH
#define FLUSH "64M"
#endif

#define NODE_BYTES 64U

#if PASSES < 1
#error PASSES must be >= 1
#endif

static uint32_t largest_power_of_two(uint64_t value)
{
    uint32_t result = 1;

    while (((uint64_t)result << 1) <= value && result < (1U << 30)) {
        result <<= 1;
    }
    return result;
}

/*
 * Build a single-cycle linked list in chain order.  Iterating in chain order
 * (rather than address order) leaves the tail of the chain, not the head, in
 * cache; the flush then evicts everything anyway.
 */
static void chain_build(uint64_t base, uint32_t nodes)
{
    uint32_t mask = nodes - 1U;
    uint32_t index = 0;

    for (uint32_t i = 0; i < nodes; i++) {
        uint32_t next = (index * 1664525U + 1013904223U) & mask;

        *(volatile uint64_t *)(uintptr_t)
            (base + (uint64_t)index * NODE_BYTES) =
                base + (uint64_t)next * NODE_BYTES;
        index = next;
    }
}

static uint64_t __attribute__((noinline))
chain_load_pass(uint64_t base, uint64_t nodes)
{
    uint64_t result;

    __asm__ volatile(
        "mov x0, %[base]\n\t"
        "mov x1, %[nodes]\n\t"
        "1:\n\t"
        "ldr x0, [x0]\n\t"
        "subs x1, x1, #1\n\t"
        "b.ne 1b\n\t"
        "mov %[out], x0\n\t"
        : [out] "=r"(result)
        : [base] "r"(base), [nodes] "r"(nodes)
        : "x0", "x1", "cc", "memory");

    return result;
}

int main(void)
{
    uint64_t requested = bench_parse_size(FOOTPRINT);
    uint64_t flush_bytes = bench_parse_size(FLUSH);
    uint64_t max_nodes = requested / NODE_BYTES;
    uint32_t nodes = max_nodes >= 2U ? largest_power_of_two(max_nodes) : 0;
    uint64_t bytes = (uint64_t)nodes * NODE_BYTES;
    uint64_t total_cycles = 0;
    uint64_t total_instret = 0;
    uint64_t base = 0;
    uint64_t flush = 0;

    bench_banner_plain("chain_ld");

    if (nodes < 2U) {
        printf("chain_ld: footprint %s is too small (need >= 128 B)\n",
               FOOTPRINT);
        return 1;
    }

    base = (uint64_t)bench_alloc(bytes);
    if (base == 0) {
        printf("chain_ld: cannot allocate %llu byte footprint\n",
               (unsigned long long)bytes);
        return 1;
    }
    if (flush_bytes != 0) {
        flush = (uint64_t)bench_alloc(flush_bytes);
        if (flush == 0) {
            printf("chain_ld: cannot allocate %llu byte flush buffer\n",
                   (unsigned long long)flush_bytes);
            return 1;
        }
    }

    printf("footprint=%llu bytes requested, using %llu bytes (%u nodes, "
           "%llu MiB) passes=%u flush=%llu bytes\n",
           (unsigned long long)requested, (unsigned long long)bytes,
           (unsigned)nodes, (unsigned long long)(bytes >> 20),
           (unsigned)PASSES, (unsigned long long)flush_bytes);

    chain_build(base, nodes);
    if (flush_bytes != 0) {
        bench_flush_cache(flush, flush_bytes);
    }

    for (uint32_t pass = 0; pass < (uint32_t)PASSES; pass++) {
        uint64_t cycle_start = am_read_cycles();
        uint64_t instret_start = am_read_instret();
        uint64_t result = chain_load_pass(base, nodes);
        uint64_t cycles = am_read_cycles() - cycle_start;
        uint64_t instret = am_read_instret() - instret_start;

        bench_sink ^= result;
        total_cycles += cycles;
        total_instret += instret;
        uint64_t b_x100 = cycles ? (bytes * 100ULL) / cycles : 0;

        printf("chain pass %u: bytes=%llu B/cyc=%llu.%02llu\n",
               (unsigned)pass, (unsigned long long)bytes,
               (unsigned long long)(b_x100 / 100),
               (unsigned long long)(b_x100 % 100));
        bench_format(cycles, instret,
                     "phase=pass%u cycles=%llu instret=%llu status=PASS",
                     (unsigned)pass, (unsigned long long)cycles,
                     (unsigned long long)instret);
    }

    if (PASSES > 1) {
        uint64_t total_bytes = bytes * (uint64_t)PASSES;
        uint64_t b_x100 = total_cycles
            ? (total_bytes * 100ULL) / total_cycles : 0;

        printf("chain total: bytes=%llu B/cyc=%llu.%02llu\n",
               (unsigned long long)total_bytes,
               (unsigned long long)(b_x100 / 100),
               (unsigned long long)(b_x100 % 100));
        bench_format(total_cycles, total_instret,
                     "phase=total cycles=%llu instret=%llu status=PASS",
                     (unsigned long long)total_cycles,
                     (unsigned long long)total_instret);
    }

    return 0;
}
