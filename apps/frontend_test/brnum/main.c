#include <bench.h>
#include "common.h"

static void __attribute__((noinline)) branch_test(int cnt)
{
    int tmp = 0;

#define ONE \
    "cbz w9, 2f\n\t" \
    "add %w0, %w0, #1\n\t" \
    "2:\n\t"

    __asm__ volatile(
        "mov w9, #0\n\t"
        "mov w10, #0\n\t"
        ".align 4\n\t"
        "1:\n\t"
        HUNDRED
        "add w10, w10, #1\n\t"
        "cmp w10, %w1\n\t"
        "b.lt 1b\n\t"
        : "+r"(tmp)
        : "r"(cnt)
        : "w9", "w10", "cc", "memory");

#undef ONE
}

static void __attribute__((noinline)) branch_test1(int cnt)
{
    int tmp = 0;

#define ONE \
    "add w11, w11, #1\n\t" \
    "and w9, w11, #1\n\t" \
    "cbz w9, 2f\n\t" \
    "2:\n\t" \
    "cbz w9, 3f\n\t" \
    "nop\n\t" \
    "3:\n\t" \
    "b 4f\n\t" \
    "nop\n\t" \
    "4:\n\t"

    __asm__ volatile(
        "mov w10, #0\n\t"
        "mov w11, #0\n\t"
        ".align 4\n\t"
        "1:\n\t"
        HUNDRED
        "add w10, w10, #1\n\t"
        "cmp w10, %w1\n\t"
        "b.lt 1b\n\t"
        : "+r"(tmp)
        : "r"(cnt)
        : "w9", "w10", "w11", "cc", "memory");

#undef ONE
}

static void __attribute__((noinline)) branch_test2(int cnt)
{
    int tmp = 0;

#define ONE \
    "add w11, w11, #1\n\t" \
    "cmp w11, w13\n\t" \
    "b.lt 2f\n\t" \
    "nop\n\t" \
    "2:\n\t" \
    "cmp w11, w13\n\t" \
    "b.lt 3f\n\t" \
    "nop\n\t" \
    "3:\n\t" \
    "b 4f\n\t" \
    "nop\n\t" \
    "4:\n\t"

    __asm__ volatile(
        "mov w10, #0\n\t"
        "mov w11, #0\n\t"
        "mov w13, #30\n\t"
        ".align 4\n\t"
        "1:\n\t"
        TEN
        "add w10, w10, #1\n\t"
        "cmp w10, %w1\n\t"
        "b.lt 1b\n\t"
        : "+r"(tmp)
        : "r"(cnt)
        : "w10", "w11", "w13", "cc", "memory");

#undef ONE
}

static void run_brnum(void)
{
    branch_test(100);
    branch_test1(100);
    branch_test2(100);
}

int main(void)
{
    BENCH_RUN("brnum", run_brnum());
    return 0;
}
