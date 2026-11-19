#include "common.h"

static inline uint64_t xorshift64(uint64_t *state)
{
    uint64_t x = *state;

    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

static void __attribute__((noinline)) test_mixed(void)
{
    uint64_t state = 0x9e3779b97f4a7c15ULL;

    for (int i = 0; i < MEM_ITERS; i++) {
        uint64_t pattern = xorshift64(&state) & 3;
        mem_x = MEM_OLD;
        mem_line[0] = MEM_OLD;

        if (pattern == 0) {
            (void)mem_st_ld((uint64_t *)&mem_x, MEM_NEW);
        } else if (pattern == 1) {
            (void)mem_ld_st((uint64_t *)&mem_x, MEM_NEW);
        } else if (pattern == 2) {
            uint64_t v0;
            uint64_t v1;
            mem_2ld_st((uint64_t *)&mem_x, MEM_NEW, &v0, &v1);
        } else {
            (void)mem_ld_st_line((uint64_t *)&mem_line[0],
                                 (uint64_t *)&mem_line[1], MEM_NEW);
        }
    }
}

int main(void)
{
    mem_dep_run(test_mixed);
    return 0;
}
