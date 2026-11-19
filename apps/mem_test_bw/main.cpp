/*
 * Port of nexus-am apps/mem_test/mem_test_bw (memory-test-v3,
 * nexus-am@812b18f) to AArch64 / mini-virt.
 *
 * The upstream probe walks a region with a compile-time stride and unroll
 * factor and reports bytes per cycle.  Only two things are target specific:
 * the load instruction (ld -> ldr) and the cycle source (CSR_MCYCLE ->
 * am_read_cycles(), i.e. PMCCNTR_EL0 when available).
 *
 * NOTE: on QEMU PMCCNTR_EL0 is a simulation-time proxy, not a faithful count
 * of microarchitectural cycles.  Meaningful bandwidth numbers require gem5
 * with an ArmPMU or RTL with a real PMU; QEMU only validates the flow.
 */

#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>
#include <bench.h>

#define BYTE (1)
#define KB (1024 * BYTE)
#define MB (1024 * KB)
#define GB (1024 * MB)

#ifndef _PERF_TEST_ADDR_BASE
#define _PERF_TEST_ADDR_BASE 0x80400000ULL
#define _PERF_TEST_ADDR_BASE1 0x80100000ULL
#endif

#define _PERF_CACHELINE_SIZE_BYTE (64 * BYTE)
#define _PERF_STRIDE_AVOID_BC (8 * BYTE)
#define _PERF_STRIDE_AVOID_BC_NEWLINE \
    (_PERF_CACHELINE_SIZE_BYTE + _PERF_STRIDE_AVOID_BC)
#define _PERF_PAGE_SIZE_BYTE (4 * KB)
#define _PERF_L1_NOALIAS_SIZE_BYTE (16 * KB)
#define _PERF_L1_SIZE_BYTE (64 * KB)
#define _PERF_L1_SIZE_AVOID_BC_BYTE (512 * _PERF_STRIDE_AVOID_BC_NEWLINE)
#define _PERF_L2_SIZE_BYTE (1 * MB)
#define _PERF_L3_SIZE_BYTE (6 * MB)
#define _PERF_MEM_SIZE_BYTE (1024 * MB)
#define _PERF_L1_NUM_WAYS 4
#define _PERF_L1_NUM_SETS 256
#define _PERF_L2_NUM_WAYS 8
#define _PERF_L2_NUM_SLICES 4
#define _PERF_L2_NUM_SETS 512

#define _PERF_ADDR_STRIDE_L1_SAME_BANK _PERF_CACHELINE_SIZE_BYTE
#define _PERF_ADDR_STRIDE_L1_SAME_SET \
    (_PERF_L1_NUM_SETS * _PERF_CACHELINE_SIZE_BYTE)
#define _PERF_ADDR_STRIDE_L2_SAME_SLICE \
    (_PERF_L2_NUM_SLICES * _PERF_CACHELINE_SIZE_BYTE)
#define _PERF_ADDR_STRIDE_L2_SAME_SET \
    (_PERF_L2_NUM_SLICES * _PERF_L2_NUM_SETS * _PERF_CACHELINE_SIZE_BYTE)
#define _PERF_ADDR_STRIDE_NEXT_PAGE (_PERF_PAGE_SIZE_BYTE)

#define _PERF_BLACKHOLE _PERF_TEST_ADDR_BASE

template <unsigned int Stride, unsigned int N>
static inline void accessNElements(uint64_t addr)
{
    if constexpr (N > 0) {
        accessNElements<Stride, N - 1>(addr);
        uint64_t __attribute__((unused)) useless;
        asm volatile("ldr %0, [%1, #%2]"
                     : "=r"(useless)
                     : "r"(addr), "i"(Stride * (N - 1))
                     : "memory");
    }
}

template <unsigned int Stride, unsigned int Unroll>
void access_region_with_step_by_xtimes(uint64_t times, uint64_t start_addr,
                                       uint64_t region_size)
{
    uint64_t remain = times;
    uint64_t access_addr = start_addr;

    assert(region_size % (Unroll * Stride) == 0);
    assert(remain % (region_size / (4 * Stride)) == 0);

    if (region_size / Stride >= times) {
        while (remain -= Unroll) {
            accessNElements<Stride, Unroll>(access_addr);
            access_addr += Stride * Unroll;
        }
    } else {
        while (remain > 0) {
            int64_t inner_remain = region_size / Stride;

            remain -= inner_remain;
            while (inner_remain -= Unroll) {
                accessNElements<Stride, Unroll>(access_addr);
                access_addr += Stride * Unroll;
            }
            inner_remain -= region_size / Stride;
        }
    }
}

template <unsigned int Stride>
void legacy_test_mem_bandwidth(const char *cache_name, uint64_t iter,
                               uint64_t footprint_cachelines,
                               uint64_t footprint_per_sample)
{
    uint64_t cycle_1;
    uint64_t cycle_2;
    uint64_t cycle;
    uint64_t instret_1;
    uint64_t instret_2;

    if (strcmp(cache_name, "MEM") != 0) {
        access_region_with_step_by_xtimes<Stride, 8>(
            iter, _PERF_TEST_ADDR_BASE1, footprint_cachelines * Stride);
    } else {
        printf("MEM bandwidth test, skip warmup\n");
    }

    instret_1 = am_read_instret();
    cycle_1 = am_read_cycles();
    access_region_with_step_by_xtimes<Stride, 8>(
        iter, _PERF_TEST_ADDR_BASE, footprint_cachelines * Stride);
    cycle_2 = am_read_cycles();
    instret_2 = am_read_instret();
    cycle = cycle_2 - cycle_1;

    if (cycle == 0) {
        printf("%s: cycle counter did not advance\n", cache_name);
        return;
    }

    printf("%s bandwidth=%.2f Bytes/cycle (%llu samples, footprint of %llu "
           "cachelins) with Stride = %u\n",
           cache_name, (float)iter * footprint_per_sample / (float)cycle,
           (unsigned long long)iter,
           (unsigned long long)footprint_cachelines, Stride);
    bench_format(cycle, instret_2 - instret_1,
                "cache=%s stride=%u footprint=%llu cycles=%llu instret=%llu",
                cache_name, Stride,
                (unsigned long long)footprint_cachelines,
                (unsigned long long)cycle,
                (unsigned long long)(instret_2 - instret_1));
}

int main(void)
{
#ifdef TEST_TYPE
    if (strcmp(TOSTRING(TEST_TYPE), "mem") == 0) {
        printf("start DDR bandwidth test\n");
        legacy_test_mem_bandwidth<_PERF_CACHELINE_SIZE_BYTE>(
            "MEM", 32 * 1024, 64 * 1024, 64);
    } else if (strcmp(TOSTRING(TEST_TYPE), "cache") == 0) {
        printf("start cache bandwidth test\n");

        legacy_test_mem_bandwidth<_PERF_STRIDE_AVOID_BC_NEWLINE>(
            "L1", 2048, 128, 64);
        legacy_test_mem_bandwidth<_PERF_STRIDE_AVOID_BC>("L1", 2048, 128, 64);
        legacy_test_mem_bandwidth<_PERF_CACHELINE_SIZE_BYTE>(
            "L1", 2048, 128, 64);

        legacy_test_mem_bandwidth<_PERF_STRIDE_AVOID_BC_NEWLINE>(
            "L1", 2048, 256, 64);
        legacy_test_mem_bandwidth<_PERF_STRIDE_AVOID_BC>("L1", 2048, 256, 64);
        legacy_test_mem_bandwidth<_PERF_CACHELINE_SIZE_BYTE>(
            "L1", 2048, 256, 64);

        legacy_test_mem_bandwidth<_PERF_STRIDE_AVOID_BC_NEWLINE>(
            "L2", 2048, 1024, 64);
        legacy_test_mem_bandwidth<_PERF_CACHELINE_SIZE_BYTE>(
            "L2", 2048, 1024, 64);

        legacy_test_mem_bandwidth<_PERF_STRIDE_AVOID_BC_NEWLINE>(
            "L2", 2048, 2048, 64);
        legacy_test_mem_bandwidth<_PERF_CACHELINE_SIZE_BYTE>(
            "L2", 2048, 2048, 64);

        legacy_test_mem_bandwidth<_PERF_STRIDE_AVOID_BC_NEWLINE>(
            "L2", 4096, 4096, 64);
        legacy_test_mem_bandwidth<_PERF_CACHELINE_SIZE_BYTE>(
            "L2", 4096, 4096, 64);

        legacy_test_mem_bandwidth<_PERF_STRIDE_AVOID_BC_NEWLINE>(
            "L2", 8 * 1024, 8 * 1024, 64);
        legacy_test_mem_bandwidth<_PERF_CACHELINE_SIZE_BYTE>(
            "L2", 8 * 1024, 8 * 1024, 64);
    } else {
        printf("unknown test type\n");
    }
#else
    printf("TEST_TYPE not specified, start DDR bandwidth test\n");
    legacy_test_mem_bandwidth<_PERF_CACHELINE_SIZE_BYTE>(
        "MEM", 32 * 1024, 64 * 1024, 64);
#endif
    return 0;
}
