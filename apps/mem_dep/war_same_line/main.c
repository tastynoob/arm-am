#include "common.h"

static void __attribute__((noinline)) test_war_same_line(void)
{
    for (int i = 0; i < MEM_ITERS; i++) {
        mem_line[0] = MEM_OLD;
        (void)mem_ld_st_line((uint64_t *)&mem_line[0],
                             (uint64_t *)&mem_line[1], MEM_NEW);
    }
}

int main(void)
{
    mem_dep_run(test_war_same_line);
    return 0;
}
