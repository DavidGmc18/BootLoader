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