#include <am.h>
#include <stdint.h>
#include <stdio.h>

/*
 * Keep the two 32-bit definitions adjacent.  The gem5 packed allocator should
 * map w9 to the low half and w10 to the high half of one physical register.
 * The final 64-bit add consumes both values and checks sign extension of w10.
 */
static uint64_t
packed_pair(void)
{
    uint64_t result;
    asm volatile(
        "mov w9, #1\n"
        "mov w10, #0x7ff\n"
        "add w9, w9, #1\n"
        "add w10, w10, #1\n"
        "add %0, x9, x10\n"
        : "=r"(result)
        :
        : "x9", "x10");
    return result;
}

int
main(void)
{
    const uint64_t value = packed_pair();
    printf("packedtest: value=0x%016llx\n", (unsigned long long)value);
    if (value != 0x0000000000000802ULL) {
        printf("packedtest: FAIL\n");
        return 1;
    }
    puts("packedtest: PASS");
    return 0;
}
