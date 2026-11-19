#include "common.h"

static volatile uint64_t train_x;
static volatile uint64_t train_y;

static void __attribute__((noinline)) store_s1(volatile uint64_t *p)
{
    *p = MEM_NEW;
}

static void __attribute__((noinline)) store_s2(volatile uint64_t *p)
{
    *p = MEM_NEW;
}

static uint64_t __attribute__((noinline)) load_x(void)
{
    return train_x;
}

static void __attribute__((noinline)) test_raw_train_flip(void)
{
    for (int i = 0; i < MEM_TRAIN_ITERS; i++) {
        train_x = MEM_OLD;
        store_s1(&train_x);
        (void)load_x();
    }

    train_x = MEM_OLD;
    for (int i = 0; i < MEM_TRAIN_ITERS; i++) {
        train_y = MEM_OLD;
        store_s2(&train_y);
        (void)load_x();
    }

    for (int i = 0; i < MEM_TRAIN_ITERS; i++) {
        train_x = MEM_OLD;
        store_s2(&train_x);
        (void)load_x();
    }
}

int main(void)
{
    mem_dep_run("raw_train_flip", test_raw_train_flip);
    return 0;
}
