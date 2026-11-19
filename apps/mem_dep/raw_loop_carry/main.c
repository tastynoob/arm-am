#include "common.h"

static void __attribute__((noinline)) test_raw_loop_carry(void)
{
    uint64_t v0;
    uint64_t v1;

    for (int i = 0; i < MEM_ITERS; i++)
        mem_2ld_st((uint64_t *)&mem_x, MEM_NEW, &v0, &v1);
    (void)v0;
    (void)v1;
}

int main(void)
{
    mem_x = MEM_OLD;
    mem_dep_run("raw_loop_carry", test_raw_loop_carry);
    return 0;
}
