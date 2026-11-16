#include <am.h>
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

static void report(const char *name, uint64_t (*bench)(uint64_t),
                   uint64_t iterations)
{
    uint64_t cycle_start = am_read_cycles();
    uint64_t instret_start = am_read_instret();
    uint64_t result = bench(iterations);
    uint64_t cycles = am_read_cycles() - cycle_start;
    uint64_t instret = am_read_instret() - instret_start;
    uint64_t ipc_x100 = cycles ? (instret * 100U) / cycles : 0U;

    printf("%-8s iterations=%llu cycles=%llu instret=%llu ipc=%llu.%02llu "
           "result=0x%016llx\n",
           name, (unsigned long long)iterations,
           (unsigned long long)cycles, (unsigned long long)instret,
           (unsigned long long)(ipc_x100 / 100U),
           (unsigned long long)(ipc_x100 % 100U),
           (unsigned long long)result);
}

int main(void)
{
    puts("arm-am: mini-virt minibench");
    printf("PMU: cycle=%s instret=%s\n",
           am_pmu_available() ? "yes" : "no (CNTVCT fallback)",
           am_instret_available() ? "yes" : "no");
    report("integer", integer_bench, 1000000);
    report("memory", memory_bench, 1000000);
    printf("minibench: PASS sink=0x%016llx\n", (unsigned long long)sink);
    return 0;
}
