#include "common.h"

static void __attribute__((noinline)) test_war_same_addr(void)
{
    for (int i = 0; i < MEM_ITERS; i++) {
        mem_x = MEM_OLD;
        (void)mem_ld_st((uint64_t *)&mem_x, MEM_NEW);
    }
}

int main(void)
{
    mem_dep_run("war_same_addr", test_war_same_addr);
    return 0;
}
