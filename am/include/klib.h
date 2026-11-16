#ifndef ARM_KLIB_H
#define ARM_KLIB_H

/*
 * Compatibility header for nexus-am's klib.
 *
 * On arm-am the C library comes from newlib instead of klib, so this header
 * only maps the extra pieces (device helpers, assert/panic) that nexus-am
 * tests expect.  printf/string/stdlib declarations are the newlib ones.
 */

#include <am.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* AM device helpers (klib/src/io.c). */
uint32_t uptime(void);
void get_timeofday(void *rtc);
int read_key(void);
void draw_rect(uint32_t *pixels, int x, int y, int w, int h);
void draw_sync(void);
int screen_width(void);
int screen_height(void);

/* Prints to the AM console and halts with a non-zero status. */
void panic(const char *message) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

/* klib's assert: report the location and stop the machine. */
#undef assert
#ifdef NDEBUG
#define assert(ignore) ((void)0)
#else
#define assert(condition)                                                  \
    do {                                                                   \
        if (!(condition)) {                                                \
            printf("Assertion fail at %s:%d\n", __FILE__, __LINE__);       \
            _halt(1);                                                      \
        }                                                                  \
    } while (0)
#endif

#endif
