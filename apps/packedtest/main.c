/*
 * 32-bit register-file pressure test.
 *
 * The measured body is one streaming load followed by PACKED_COPIES
 * unrolled copies of a 24-write 32-bit ALU block (144..384 ALU instructions
 * for the supported build variants).  The body is repeated by an outer
 * assembly loop; there is no inner ALU loop.  The 64 MiB pool is read as a
 * forward stream with a 64 KiB stride so each body load touches a fresh line.
 */
#include <bench.h>

#ifndef PACKED_ITERS
#define PACKED_ITERS 1000U
#endif
#ifndef PACKED_COPIES
#define PACKED_COPIES 12
#endif

/* load + pointer add + PACKED_COPIES*24 ALU + subs + b.ne */
#define BODY_INSNS (PACKED_COPIES * 24 + 4)

#define MB (1024U * 1024U)
#define POOL_SIZE (64U * MB)
#define STREAM_STRIDE (64U * 1024U)

_Static_assert((uint64_t)PACKED_ITERS * STREAM_STRIDE <= POOL_SIZE,
               "stream footprint exceeds pool");

static uint8_t stream_pool[POOL_SIZE]
    __attribute__((section(".test"), aligned(2 * 1024 * 1024)));

#define ADD24 \
    "add w1,  w1,  #1\n\t" \
    "add w2,  w2,  #1\n\t" \
    "add w3,  w3,  #1\n\t" \
    "add w4,  w4,  #1\n\t" \
    "add w5,  w5,  #1\n\t" \
    "add w6,  w6,  #1\n\t" \
    "add w7,  w7,  #1\n\t" \
    "add w8,  w8,  #1\n\t" \
    "add w9,  w9,  #1\n\t" \
    "add w10, w10, #1\n\t" \
    "add w11, w11, #1\n\t" \
    "add w12, w12, #1\n\t" \
    "add w13, w13, #1\n\t" \
    "add w14, w14, #1\n\t" \
    "add w15, w15, #1\n\t" \
    "add w16, w16, #1\n\t" \
    "add w17, w17, #1\n\t" \
    "add w18, w18, #1\n\t" \
    "add w19, w19, #1\n\t" \
    "add w20, w20, #1\n\t" \
    "add w21, w21, #1\n\t" \
    "add w22, w22, #1\n\t" \
    "add w23, w23, #1\n\t" \
    "add w24, w24, #1\n\t"

#define ALU6  ADD24 ADD24 ADD24 ADD24 ADD24 ADD24
#define ALU7  ALU6 ADD24
#define ALU8  ALU7 ADD24
#define ALU9  ALU8 ADD24
#define ALU10 ALU9 ADD24
#define ALU11 ALU10 ADD24
#define ALU12 ALU11 ADD24
#define ALU14 ALU12 ADD24 ADD24
#define ALU16 ALU14 ADD24 ADD24

#if PACKED_COPIES == 6
#define ALU_BODY ALU6
#elif PACKED_COPIES == 7
#define ALU_BODY ALU7
#elif PACKED_COPIES == 8
#define ALU_BODY ALU8
#elif PACKED_COPIES == 9
#define ALU_BODY ALU9
#elif PACKED_COPIES == 10
#define ALU_BODY ALU10
#elif PACKED_COPIES == 11
#define ALU_BODY ALU11
#elif PACKED_COPIES == 12
#define ALU_BODY ALU12
#elif PACKED_COPIES == 14
#define ALU_BODY ALU14
#elif PACKED_COPIES == 16
#define ALU_BODY ALU16
#else
#error "PACKED_COPIES must be 6..12, 14 or 16"
#endif

int main(void)
{
    uint64_t pool_base = (uint64_t)(uintptr_t)stream_pool;

    bench_banner_plain("packedtest");
    bench_fence();

    uint64_t cycle_start = am_read_cycles();
    uint64_t instret_start = am_read_instret();

    __asm__ volatile(
        "mov x25, %[base]\n\t"
        "mov x26, #0x10000\n\t"  /* STREAM_STRIDE */
        "mov x28, %[iters]\n\t"
        "1:\n\t"
        "ldr x0, [x25, x26]\n\t"
        "add x25, x25, x26\n\t"
        ALU_BODY
        "subs x28, x28, #1\n\t"
        "b.ne 1b\n\t"
        :
        : [base] "r"(pool_base), [iters] "r"((uint64_t)PACKED_ITERS)
        : "x0", "x25", "x26", "x28", "w1", "w2", "w3", "w4", "w5", "w6",
          "w7", "w8", "w9", "w10", "w11", "w12", "w13", "w14", "w15",
          "w16", "w17", "w18", "w19", "w20", "w21", "w22", "w23",
          "w24", "cc", "memory");

    uint64_t cycles = am_read_cycles() - cycle_start;
    uint64_t instret = am_read_instret() - instret_start;
    bench_format(cycles, instret,
                 "copies=%u insns=%u iters=%u cycles=%llu instret=%llu status=PASS",
                 PACKED_COPIES, BODY_INSNS, PACKED_ITERS,
                 (unsigned long long)cycles,
                 (unsigned long long)instret);
    return 0;
}
