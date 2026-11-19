#include "common.h"

static void __attribute__((noinline)) test_raw_store_pair(void)
{
    for (int i = 0; i < MEM_ITERS; i++) {
        mem_x = MEM_OLD;
        (void)mem_2st_ld((uint64_t *)&mem_x, MEM_NEW, MEM_OLD ^ MEM_NEW);
    }
}

int main(void)
{
    mem_dep_run(test_raw_store_pair);
    return 0;
}
