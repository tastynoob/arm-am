#ifndef MEM_DEP_COMMON_H
#define MEM_DEP_COMMON_H

#include <bench.h>

#define MEM_OLD 0x1111111111111111ULL
#define MEM_NEW 0x2222222222222222ULL
#define MEM_ITERS 100000
#define MEM_TRAIN_ITERS 2000

static volatile uint64_t mem_x;
static volatile uint64_t mem_y;
static volatile uint64_t mem_line[8] __attribute__((aligned(64)));

/* Older store, younger load (RAW). */
static inline uint64_t mem_st_ld(uint64_t *p, uint64_t new_value)
{
    uint64_t value;

    __asm__ volatile(
        "str %1, [%2]\n\t"
        "ldr %0, [%2]\n\t"
        : "=r"(value)
        : "r"(new_value), "r"(p)
        : "memory");
    return value;
}

/* Older load, younger store (WAR). */
static inline uint64_t mem_ld_st(uint64_t *p, uint64_t new_value)
{
    uint64_t value;

    __asm__ volatile(
        "ldr %0, [%1]\n\t"
        "str %2, [%1]\n\t"
        : "=r"(value)
        : "r"(p), "r"(new_value)
        : "memory");
    return value;
}

/* Two stores to the same address, then a load. */
static inline uint64_t mem_2st_ld(uint64_t *p, uint64_t v0, uint64_t v1)
{
    uint64_t value;

    __asm__ volatile(
        "str %1, [%3]\n\t"
        "str %2, [%3]\n\t"
        "ldr %0, [%3]\n\t"
        : "=r"(value)
        : "r"(v0), "r"(v1), "r"(p)
        : "memory");
    return value;
}

/* Two older loads, then a younger store. */
static inline void mem_2ld_st(uint64_t *p, uint64_t new_value,
                              uint64_t *v0, uint64_t *v1)
{
    __asm__ volatile(
        "ldr %0, [%2]\n\t"
        "ldr %1, [%2]\n\t"
        "str %3, [%2]\n\t"
        : "=&r"(*v0), "=&r"(*v1)
        : "r"(p), "r"(new_value)
        : "memory");
}

/* Same-line load then store to a different word. */
static inline uint64_t mem_ld_st_line(uint64_t *p0, uint64_t *p1,
                                      uint64_t new_value)
{
    uint64_t value;

    __asm__ volatile(
        "ldr %0, [%1]\n\t"
        "str %2, [%3]\n\t"
        : "=r"(value)
        : "r"(p0), "r"(new_value), "r"(p1)
        : "memory");
    return value;
}

static void mem_dep_run(const char *name, void (*workload)(void))
{
    uint64_t cycles0 = am_read_cycles();
    uint64_t instret0 = am_read_instret();
    workload();
    uint64_t cycles = am_read_cycles() - cycles0;
    uint64_t instret = am_read_instret() - instret0;
    uint64_t ipc = cycles ? (instret * 100ULL) / cycles : 0;

    printf("%s cycles=%llu instret=%llu ipc=%llu.%02llu\n",
           name, (unsigned long long)cycles,
           (unsigned long long)instret,
           (unsigned long long)(ipc / 100),
           (unsigned long long)(ipc % 100));
    printf("bench %s: done\n", name);
}

#endif
