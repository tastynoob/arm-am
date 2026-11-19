#include "common.h"

static inline void mem_2ld_st_late(uint64_t *p, uint64_t new_value,
                                   uint64_t *v0, uint64_t *v1)
{
    __asm__ volatile(
        "ldr %0, [%2]\n\t"
        "ldr %1, [%2]\n\t"
        "add x3, %2, #1\n\t"
        "sub x3, x3, #1\n\t"
        "add x3, x3, #1\n\t"
        "sub x3, x3, #1\n\t"
        "add x3, x3, #1\n\t"
        "sub x3, x3, #1\n\t"
        "add x3, x3, #1\n\t"
        "sub x3, x3, #1\n\t"
        "add x3, x3, #1\n\t"
        "sub x3, x3, #1\n\t"
        "add x3, x3, #1\n\t"
        "sub x3, x3, #1\n\t"
        "add x3, x3, #1\n\t"
        "sub x3, x3, #1\n\t"
        "add x3, x3, #1\n\t"
        "sub x3, x3, #1\n\t"
        "str %3, [x3]\n\t"
        : "=&r"(*v0), "=&r"(*v1)
        : "r"(p), "r"(new_value)
        : "x3", "memory");
}

static void __attribute__((noinline)) test_raw_loop_late(void)
{
    uint64_t v0;
    uint64_t v1;

    for (int i = 0; i < MEM_ITERS; i++)
        mem_2ld_st_late((uint64_t *)&mem_x, MEM_NEW, &v0, &v1);
    (void)v0;
    (void)v1;
}

int main(void)
{
    mem_x = MEM_OLD;
    mem_dep_run("raw_loop_late", test_raw_loop_late);
    return 0;
}
