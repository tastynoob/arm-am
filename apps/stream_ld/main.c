/*
 * Streaming load bandwidth over a configurable footprint.
 *
 * Every cache line is touched exactly once: one 8-byte load at a 64 B line
 * boundary.  Sixteen independent lines (1 KiB) are issued per loop iteration,
 * which exposes memory-level parallelism without turning half of the loads
 * into L1 hits on a line that was already fetched.
 *
 *   PASSES=1  -> cold footprint; its size versus the LLC determines DDR stream
 *   PASSES=2+ -> later passes start with the cache state left by the previous
 *
 * The footprint is never written, so pass 0 is cold without any fill.  FLUSH
 * remains available for special cases but defaults to 0.
 *
 * Operations are reported as cache lines: `ops/cyc` is lines/cycle and
 * `B/cyc` uses the full 64 B line footprint, not the 8 B payload.
 *
 * Parameters (make): FOOTPRINT=64M PASSES=2
 */
#include <bench.h>

#ifndef FOOTPRINT
#define FOOTPRINT "64M"
#endif
#ifndef PASSES
#define PASSES 1
#endif
#ifndef FLUSH
#define FLUSH "0"
#endif

#if PASSES < 1
#error PASSES must be >= 1
#endif

#define TEST_AREA_SIZE (1024ULL * 1024ULL * 1024ULL)
#define FLUSH_AREA_SIZE (64ULL * 1024ULL * 1024ULL)

static uint8_t test_area[TEST_AREA_SIZE]
    __attribute__((section(".test"), aligned(64)));
static uint8_t flush_area[FLUSH_AREA_SIZE]
    __attribute__((section(".test"), aligned(64)));

#define LINE_BYTES 64U
#define LINES_PER_ITER 16U
#define ITER_BYTES (LINE_BYTES * LINES_PER_ITER)

#define LD16 \
    "ldr x1,  [x9, #0]\n\t" \
    "ldr x2,  [x9, #64]\n\t" \
    "ldr x3,  [x9, #128]\n\t" \
    "ldr x4,  [x9, #192]\n\t" \
    "ldr x5,  [x9, #256]\n\t" \
    "ldr x6,  [x9, #320]\n\t" \
    "ldr x7,  [x9, #384]\n\t" \
    "ldr x8,  [x9, #448]\n\t" \
    "ldr x10, [x9, #512]\n\t" \
    "ldr x11, [x9, #576]\n\t" \
    "ldr x12, [x9, #640]\n\t" \
    "ldr x13, [x9, #704]\n\t" \
    "ldr x14, [x9, #768]\n\t" \
    "ldr x15, [x9, #832]\n\t" \
    "ldr x16, [x9, #896]\n\t" \
    "ldr x17, [x9, #960]\n\t"

static uint64_t __attribute__((noinline))
stream_load_pass(uint64_t base, uint64_t chunks)
{
    uint64_t result;

    __asm__ volatile(
        "mov x9, %[base]\n\t"
        "mov x0, %[chunks]\n\t"
        "1:\n\t"
        LD16
        "add x9, x9, #1024\n\t"
        "subs x0, x0, #1\n\t"
        "b.ne 1b\n\t"
        "add %[out], x1, x2\n\t"
        : [out] "=r"(result)
        : [base] "r"(base), [chunks] "r"(chunks)
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8",
          "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16",
          "x17", "cc", "memory");

    return result;
}

int main(void)
{
    uint64_t requested = bench_parse_size(FOOTPRINT);
    uint64_t flush_bytes = bench_parse_size(FLUSH);
    uint64_t bytes = (requested / ITER_BYTES) * ITER_BYTES;
    uint64_t chunks = bytes / ITER_BYTES;
    uint64_t lines = bytes / LINE_BYTES;
    uint64_t total_cycles = 0;
    uint64_t total_instret = 0;
    uint64_t base = 0;
    uint64_t flush = 0;

    bench_banner_plain("stream_ld");

    if (bytes < ITER_BYTES) {
        printf("stream_ld: footprint %s is too small (need >= %u B)\n",
               FOOTPRINT, (unsigned)ITER_BYTES);
        return 1;
    }

    if (bytes > sizeof(test_area)) {
        printf("stream_ld: footprint %llu exceeds test area %llu\n",
               (unsigned long long)bytes,
               (unsigned long long)sizeof(test_area));
        return 1;
    }
    if (flush_bytes > sizeof(flush_area)) {
        printf("stream_ld: flush %llu exceeds test area %llu\n",
               (unsigned long long)flush_bytes,
               (unsigned long long)sizeof(flush_area));
        return 1;
    }
    base = (uint64_t)(uintptr_t)test_area;
    if (flush_bytes != 0) {
        flush = (uint64_t)(uintptr_t)flush_area;
        bench_flush_cache(flush, flush_bytes);
    }

    printf("footprint=%llu bytes (%llu MiB) lines=%llu passes=%u "
           "flush=%llu bytes\n",
           (unsigned long long)bytes,
           (unsigned long long)(bytes >> 20),
           (unsigned long long)lines, (unsigned)PASSES,
           (unsigned long long)flush_bytes);

    for (uint32_t pass = 0; pass < (uint32_t)PASSES; pass++) {
        uint64_t cycle_start = am_read_cycles();
        uint64_t instret_start = am_read_instret();
        uint64_t result = stream_load_pass(base, chunks);
        uint64_t cycles = am_read_cycles() - cycle_start;
        uint64_t instret = am_read_instret() - instret_start;

        bench_sink ^= result;
        total_cycles += cycles;
        total_instret += instret;
        uint64_t b_x100 = cycles ? (bytes * 100ULL) / cycles : 0;

        printf("stream pass %u: bytes=%llu B/cyc=%llu.%02llu\n",
               (unsigned)pass, (unsigned long long)bytes,
               (unsigned long long)(b_x100 / 100),
               (unsigned long long)(b_x100 % 100));
        bench_format(cycles, instret,
                     "phase=pass%u cycles=%llu instret=%llu status=PASS",
                     (unsigned)pass, (unsigned long long)cycles,
                     (unsigned long long)instret);
    }

    if (PASSES > 1) {
        uint64_t total_bytes = bytes * (uint64_t)PASSES;
        uint64_t b_x100 = total_cycles
            ? (total_bytes * 100ULL) / total_cycles : 0;

        printf("stream total: bytes=%llu B/cyc=%llu.%02llu\n",
               (unsigned long long)total_bytes,
               (unsigned long long)(b_x100 / 100),
               (unsigned long long)(b_x100 % 100));
        bench_format(total_cycles, total_instret,
                     "phase=total cycles=%llu instret=%llu status=PASS",
                     (unsigned long long)total_cycles,
                     (unsigned long long)total_instret);
    }

    return 0;
}
