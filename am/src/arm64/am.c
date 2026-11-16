#include <am.h>
#include <amdev.h>
#include <baremetal/platform.h>

extern char __heap_start[];
extern char __heap_end[];

extern __attribute__((noreturn)) void _exit(int status);

/* 1: PMCCNTR_EL0, 0: CNTVCT_EL0 fallback. */
static int cycle_counter_is_pmu;
/* 1: PMEVCNTR0_EL0 counts INST_RETIRED (event 0x08). */
static int instret_counter_is_pmu;

_Area _heap;

/*
 * Enable the PMU cycle counter when the implementation provides one.
 * QEMU, gem5 (with an ArmPMU) and RTL can all expose PMCCNTR_EL0; the
 * counter is the only portable *cycle* counter on AArch64.  When it is
 * missing we fall back to CNTVCT_EL0 so that tests still make progress.
 */
void am_early_init(void)
{
    uint64_t id_aa64dfr0;
    uint64_t pmceid0;
    uint64_t pmcr;

    _heap.start = __heap_start;
    _heap.end = __heap_end;

    __asm__ volatile("mrs %0, id_aa64dfr0_el1" : "=r"(id_aa64dfr0));
    if (((id_aa64dfr0 >> 8) & 0xfU) == 0U ||
        ((id_aa64dfr0 >> 8) & 0xfU) == 0xfU) {
        return;
    }

    /*
     * PMCR_EL0: E=1 (enable), P=1 (reset event counters),
     * C=1 (reset cycle counter), D=0 (do not divide by 64).
     */
    pmcr = (1U << 0) | (1U << 1) | (1U << 2);
    __asm__ volatile("msr pmcr_el0, %0" ::"r"(pmcr));

    /*
     * Event 0x08 is the architected INST_RETIRED event.  It is still worth
     * checking PMCEID0: PMUv3 is optional and not every implementation
     * offers every event.
     */
    __asm__ volatile("mrs %0, pmceid0_el0" : "=r"(pmceid0));
    if ((pmceid0 & (1ULL << 8)) != 0U) {
        uint64_t event = 0x08;

        __asm__ volatile("msr pmevtyper0_el0, %0" ::"r"(event));
        __asm__ volatile("msr pmevcntr0_el0, xzr");
        instret_counter_is_pmu = 1;
    }

    /* PMCNTENSET_EL0: bit 31 is PMCCNTR, bit 0 is PMEVCNTR0. */
    pmcr = (1ULL << 31) | (instret_counter_is_pmu ? 1ULL : 0ULL);
    __asm__ volatile("msr pmcntenset_el0, %0" ::"r"(pmcr));
    __asm__ volatile("isb");

    __asm__ volatile("mrs %0, pmcr_el0" : "=r"(pmcr));
    if ((pmcr & 1U) != 0U) {
        cycle_counter_is_pmu = 1;
    }
}


uint64_t am_read_cycles(void)
{
    uint64_t value;

    if (cycle_counter_is_pmu) {
        __asm__ volatile("isb\n\tmrs %0, pmccntr_el0" : "=r"(value));
    } else {
        __asm__ volatile("isb\n\tmrs %0, cntvct_el0" : "=r"(value));
    }
    return value;
}

uint64_t am_read_instret(void)
{
    uint64_t value = 0;

    if (instret_counter_is_pmu) {
        __asm__ volatile("isb\n\tmrs %0, pmevcntr0_el0" : "=r"(value));
        value &= 0xffffffffULL;
    }
    return value;
}

int am_pmu_available(void)
{
    return cycle_counter_is_pmu;
}

int am_instret_available(void)
{
    return instret_counter_is_pmu;
}

uint64_t am_read_time_us(void)
{
    uint64_t frequency;
    uint64_t count;

    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(frequency));
    __asm__ volatile("isb\n\tmrs %0, cntvct_el0" : "=r"(count));
    if (frequency == 0U) {
        return 0;
    }
    return (count / frequency) * 1000000ULL +
           ((count % frequency) * 1000000ULL) / frequency;
}

void am_putc(char character)
{
    baremetal_platform_putc(character);
}

void _putc(char character)
{
    am_putc(character);
}

void am_puts(const char *string)
{
    while (*string != '\0') {
        am_putc(*string++);
    }
    am_putc('\n');
}

__attribute__((noreturn)) void _halt(int status)
{
    _exit(status);
}

__attribute__((noreturn)) void am_halt(int status)
{
    _exit(status);
}
