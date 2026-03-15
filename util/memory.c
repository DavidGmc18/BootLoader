#include "memory.h"
#include <stdint.h>

void* memset (void* ptr, int value, size_t num) {
    uintptr_t fill = (uint8_t)value * (uintptr_t)0x0101010101010101ULL;
    size_t words = num / sizeof(uintptr_t);

    uintptr_t* native_ptr = ptr;
    for (size_t i = 0; i < words; i++)
        native_ptr[i] = fill;

    uint8_t* byte_ptr = ptr;
    for (size_t i = words * sizeof(uintptr_t); i < num; i++)
        byte_ptr[i] = value;

    return ptr;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    uint8_t* u8s1 = (uint8_t*)s1;
    uint8_t* u8s2 = (uint8_t*)s2;

    for (size_t i = 0; i < n; i++) {
        int ret;
        if (ret = u8s1[i] - u8s2[i])
            return ret;
    }

    return 0;
}