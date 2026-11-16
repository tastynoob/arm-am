#ifndef ARM_AM_H
#define ARM_AM_H

#include <stddef.h>
#include <stdint.h>

/*
 * A small subset of the Nexus AbstractMachine (AM) interface.
 *
 * The Turing Machine (TRM) part is compatible with nexus-am, so unmodified
 * nexus-am tests that only use _putc/_halt/_heap can be built here.  arm-am
 * specific helpers are kept at the bottom of this file.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Memory area for [start, end). */
typedef struct _Area {
    void *start, *end;
} _Area;

extern _Area _heap;

void _putc(char character);
__attribute__((noreturn)) void _halt(int status);

/* I/O extension.  Only timer and serial are implemented on mini-virt. */
int _ioe_init(void);
size_t _io_read(uint32_t dev, uintptr_t reg, void *buf, size_t size);
size_t _io_write(uint32_t dev, uintptr_t reg, const void *buf, size_t size);

/* ====================== arm-am specific helpers ====================== */

void am_putc(char character);
void am_puts(const char *string);

/*
 * Cycle counter used by the microbenchmarks.
 *
 * When the implementation exposes a programmable PMU, this reads
 * PMCCNTR_EL0 after enabling it in am_early_init().  Otherwise it falls back
 * to CNTVCT_EL0, which is a fixed-frequency timer rather than a cycle count.
 */
uint64_t am_read_cycles(void);

/* CNTVCT_EL0 based monotonic microsecond counter. */
uint64_t am_read_time_us(void);

__attribute__((noreturn)) void am_halt(int status);

/* Called by start.S before main(); not part of the AM API. */
void am_early_init(void);

#ifdef __cplusplus
}
#endif

#endif
