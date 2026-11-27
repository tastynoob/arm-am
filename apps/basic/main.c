#include <am.h>
#include <bench.h>
#include <stdint.h>
#include <stdio.h>

static volatile uint64_t sink;

static uint64_t integer_bench(uint64_t iterations)
{
    uint64_t value = 1;

    for (uint64_t index = 0; index < iterations; ++index) {
        value = value * 1664525U + 1013904223U;
        value ^= value >> 13;
    }
    sink = value;
    return value;
}

static uint64_t memory_bench(uint64_t iterations)
{
    static uint64_t buffer[1024];
    uint64_t checksum = 0;

    for (uint64_t index = 0; index < iterations; ++index) {
        uint64_t slot = index & 1023U;
        buffer[slot] = index ^ 0xa5a5a5a5a5a5a5a5ULL;
        checksum += buffer[slot];
    }
    sink = checksum;
    return checksum;
}

static void report(uint64_t (*bench)(uint64_t), uint64_t iterations,
                   const char *loop)
{
    uint64_t cycle_start = am_read_cycles();
    uint64_t instret_start = am_read_instret();
    uint64_t result = bench(iterations);
    uint64_t cycles = am_read_cycles() - cycle_start;
    uint64_t instret = am_read_instret() - instret_start;
    (void)result;
    bench_format(cycles, instret,
                 "loop=%s cycles=%llu instret=%llu status=PASS",
                 loop, (unsigned long long)cycles,
                 (unsigned long long)instret);
}

int main(void)
{
    puts("arm-am: basic");
    printf("PMU: cycle=%s instret=%s\n",
           am_pmu_available() ? "yes" : "no (CNTVCT fallback)",
           am_instret_available() ? "yes" : "no");
    bench_fence();
    report(integer_bench, 1000000, "integer");
    report(memory_bench, 1000000, "memory");
    return 0;
}
