/*
 * Port of nexus-am apps/mem_test/mem_test_latency (memory-test-v3,
 * nexus-am@812b18f) to AArch64 / mini-virt.
 *
 * The original uses a RISC-V pointer-chasing loop and reads mcycle/minstret.
 * Here the same loop is expressed in C over volatile loads and timed with
 * am_read_cycles(), which uses PMCCNTR_EL0 when the implementation exposes a
 * PMU.  Each loop iteration follows two links, so 500 iterations are 1000
 * dependent loads, matching the upstream test.
 */

#include <klib.h>
#include <klib-macros.h>
#include <bench.h>

#define BYTE (1)
#define KB (1024 * BYTE)
#define MB (1024 * KB)
#define GB (1024 * MB)

#ifndef INST
#define INST 16
#endif

#ifndef MEM_TEST_ADDR_BASE
#define MEM_TEST_ADDR_BASE 0x80010000ULL
#endif

#define _PERF_TEST_ADDR_BASE MEM_TEST_ADDR_BASE

static uint32_t lfsr_next(uint32_t last)
{
    uint32_t lfsr = last;

    lfsr ^= lfsr >> 7;
    lfsr ^= lfsr << 9;
    lfsr ^= lfsr >> 13;
    return lfsr;
}

static uint64_t random_num_to_addr(uint64_t base_addr, uint32_t num,
                                   uint32_t size_limit)
{
    uint32_t size_mask = size_limit - 1;
    uint32_t cache_line_mask = 0x7;
    uint32_t masked_num = num & (size_mask & (~cache_line_mask));

    return base_addr + masked_num;
}

static void pseudo_random_warmup(uint64_t base_addr, uint32_t seed,
                                 unsigned block_count, uint32_t size_limit)
{
    unsigned count = 0;
    uint64_t current_addr = base_addr;
    uint32_t initial_seed = seed;

    while (count < block_count) {
        uint32_t next_number = lfsr_next(seed);

        if (next_number == initial_seed) {
            printf("Loop detected, stop warmup\n");
            assert(0);
        }
        uint64_t next_addr =
            random_num_to_addr(base_addr, next_number, size_limit);
        *(volatile uint64_t *)(uintptr_t)current_addr = next_addr;

        count++;
        current_addr = next_addr;
        seed = next_number;
    }
    printf("Pseudo random warmup %u cache blocks\n", block_count);
}

static void flush_cache_efficient(unsigned size_in_kb, uint64_t start_addr)
{
    unsigned bytes = 1024 * size_in_kb;
    uint64_t blackhole = 0;

    for (unsigned index = 0; index < bytes; index += 64) {
        blackhole += *(const volatile uint64_t *)(uintptr_t)(start_addr + index);
    }
    printf("Flushed %u KB cache, blackhole = %llx\n", size_in_kb,
           (unsigned long long)blackhole);
}

static __attribute__((noinline, aligned(64)))
void cache_iloop(unsigned long long *load_count,
                 unsigned long long *cycle_count)
{
    uint64_t node = _PERF_TEST_ADDR_BASE;
    uint64_t start = am_read_cycles();

    for (unsigned index = 0; index < 500; index++) {
        node = *(const volatile uint64_t *)(uintptr_t)node;
        node = *(const volatile uint64_t *)(uintptr_t)node;
    }

    *cycle_count = am_read_cycles() - start;
    *load_count = 1000;
}

int main(void)
{
    unsigned long long loads;
    unsigned long long cycles;
    uint64_t start_addr = _PERF_TEST_ADDR_BASE;
    unsigned footprint_kb = 256 * INST;
    int block_count = 4000;
    uint64_t flush_start = start_addr + 0x40000000;

    printf("Intializing linked-list of %i nodes in range 0x%llx-0x%llx with "
           "seed %#x.\n"
           "Then flushing cache of %i KB starting from %#llx\n",
           block_count, (unsigned long long)start_addr,
           (unsigned long long)flush_start, 0xdeadbeaf, footprint_kb,
           (unsigned long long)flush_start);

    pseudo_random_warmup(start_addr, 0xdeadbeaf /* non-zero */, block_count,
                         0x40000000);
    flush_cache_efficient(footprint_kb, flush_start);

    uint64_t instret_1 = am_read_instret();
    cache_iloop(&loads, &cycles);
    uint64_t instret = am_read_instret() - instret_1;
    printf("load-to-use latency: %llu cycles/load\n", cycles / loads);
    bench_format(cycles, instret,
                 "cycles=%llu instret=%llu status=PASS",
                 (unsigned long long)cycles,
                 (unsigned long long)instret);
    return 0;
}
