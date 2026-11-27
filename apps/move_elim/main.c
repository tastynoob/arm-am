/*
 * Move-elimination test.
 *
 * A move whose destination can reuse the source physical register should not
 * consume a new physical register.  The measured body is one streaming load
 * plus MOVE_COPIES unrolled blocks of 24 register-to-register moves, repeated
 * by an assembly outer loop.  With move elimination there should be no IPC
 * knee as the number of move blocks grows.
 */
#include <bench.h>

#ifndef MOVE_ITERS
#define MOVE_ITERS 1000U
#endif
#ifndef MOVE_COPIES
#define MOVE_COPIES 16
#endif

/* load + pointer add + MOVE_COPIES*24 moves + subs + b.ne */
#define BODY_INSNS (MOVE_COPIES * 24 + 4)

#define MB (1024U * 1024U)
#define POOL_SIZE (64U * MB)
#define STREAM_STRIDE (64U)

_Static_assert((uint64_t)MOVE_ITERS * STREAM_STRIDE <= POOL_SIZE,
               "stream footprint exceeds pool");

static uint8_t stream_pool[POOL_SIZE]
    __attribute__((section(".test"), aligned(2 * 1024 * 1024)));

#define MOV24 \
    "mov w1,  w0\n\t" \
    "mov w2,  w0\n\t" \
    "mov w3,  w0\n\t" \
    "mov w4,  w0\n\t" \
    "mov w5,  w0\n\t" \
    "mov w6,  w0\n\t" \
    "mov w7,  w0\n\t" \
    "mov w8,  w0\n\t" \
    "mov w9,  w0\n\t" \
    "mov w10, w0\n\t" \
    "mov w11, w0\n\t" \
    "mov w12, w0\n\t" \
    "mov w13, w0\n\t" \
    "mov w14, w0\n\t" \
    "mov w15, w0\n\t" \
    "mov w16, w0\n\t" \
    "mov w17, w0\n\t" \
    "mov w18, w0\n\t" \
    "mov w19, w0\n\t" \
    "mov w20, w0\n\t" \
    "mov w21, w0\n\t" \
    "mov w22, w0\n\t" \
    "mov w23, w0\n\t" \
    "mov w24, w0\n\t"

#define MOVE6  MOV24 MOV24 MOV24 MOV24 MOV24 MOV24
#define MOVE7  MOVE6 MOV24
#define MOVE8  MOVE7 MOV24
#define MOVE9  MOVE8 MOV24
#define MOVE10 MOVE9 MOV24
#define MOVE11 MOVE10 MOV24
#define MOVE12 MOVE11 MOV24
#define MOVE14 MOVE12 MOV24 MOV24
#define MOVE16 MOVE14 MOV24 MOV24

#if MOVE_COPIES == 6
#define MOVE_BODY MOVE6
#elif MOVE_COPIES == 7
#define MOVE_BODY MOVE7
#elif MOVE_COPIES == 8
#define MOVE_BODY MOVE8
#elif MOVE_COPIES == 9
#define MOVE_BODY MOVE9
#elif MOVE_COPIES == 10
#define MOVE_BODY MOVE10
#elif MOVE_COPIES == 11
#define MOVE_BODY MOVE11
#elif MOVE_COPIES == 12
#define MOVE_BODY MOVE12
#elif MOVE_COPIES == 14
#define MOVE_BODY MOVE14
#elif MOVE_COPIES == 16
#define MOVE_BODY MOVE16
#else
#error "MOVE_COPIES must be 6..12, 14 or 16"
#endif

int main(void)
{
    uint64_t pool_base = (uint64_t)(uintptr_t)stream_pool;

    bench_banner_plain("move_elim");
    bench_fence();

    uint64_t cycle_start = am_read_cycles();
    uint64_t instret_start = am_read_instret();

    __asm__ volatile(
        "mov x25, %[base]\n\t"
        "mov x27, #0x40\n\t"  /* STREAM_STRIDE */
        "mov x28, %[iters]\n\t"
        "1:\n\t"
        "ldr x26, [x25, x27]\n\t"
        "add x25, x25, x27\n\t"
        MOVE_BODY
        "subs x28, x28, #1\n\t"
        "b.ne 1b\n\t"
        :
        : [base] "r"(pool_base), [iters] "r"((uint64_t)MOVE_ITERS)
        : "x25", "x26", "x27", "x28", "w1", "w2", "w3", "w4", "w5",
          "w6", "w7", "w8", "w9", "w10", "w11", "w12", "w13", "w14",
          "w15", "w16", "w17", "w18", "w19", "w20", "w21", "w22",
          "w23", "w24", "cc", "memory");

    uint64_t cycles = am_read_cycles() - cycle_start;
    uint64_t instret = am_read_instret() - instret_start;
    bench_format(cycles, instret,
                 "copies=%u insns=%u iters=%u cycles=%llu instret=%llu status=PASS",
                 MOVE_COPIES, BODY_INSNS, MOVE_ITERS,
                 (unsigned long long)cycles,
                 (unsigned long long)instret);
    return 0;
}
