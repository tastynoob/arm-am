#ifndef ARM_KLIB_MACROS_H
#define ARM_KLIB_MACROS_H

#include <am.h>

#ifndef __cplusplus
#ifndef __bool_true_false_are_defined
typedef uint8_t bool;
#define false 0
#define true 1
#endif
#endif

#define ROUNDUP(a, sz)   ((((uintptr_t)a) + (sz) - 1) & ~((sz) - 1))
#define ROUNDDOWN(a, sz) ((((uintptr_t)a)) & ~((sz) - 1))

#define LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

#define RANGE(st, ed) (_Area) { .start = (void *)(st), .end = (void *)(ed) }
#define IN_RANGE(ptr, area) ((area).start <= (void *)(ptr) && (void *)(ptr) < (area).end)

#define STRINGIFY(s) #s
#define TOSTRING(s) STRINGIFY(s)

#define _CONCAT(x, y) x ## y
#define CONCAT(x, y) _CONCAT(x, y)

#ifndef __cplusplus
#define static_assert(const_cond) \
  static char CONCAT(_static_assert_, __LINE__)[(const_cond) ? 1 : -1] \
      __attribute__((unused))
#endif

static inline void *upcast(uint32_t ptr)
{
    return (void *)(uintptr_t)ptr;
}

static inline void putstr(const char *s)
{
    while (*s) {
        _putc(*s++);
    }
}

#define panic_on(cond, s)                                                  \
    do {                                                                   \
        if (cond) {                                                        \
            putstr("AM Panic: ");                                          \
            putstr(s);                                                     \
            putstr(" @ " __FILE__ ":" TOSTRING(__LINE__) "  \n");          \
            _halt(1);                                                      \
        }                                                                  \
    } while (0)

#define panic(s) panic_on(1, s)

#endif
