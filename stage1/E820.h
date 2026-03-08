#pragma once

#include <stdint.h>

typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t ACPI;
} E820_MemoryBlock;

#define E820_MemoryInfo_MAX_BLOCKS 256
typedef struct {
    uint8_t block_count;
    E820_MemoryBlock blocks[E820_MemoryInfo_MAX_BLOCKS];
} E820_MemoryInfo;

int E820_detect(E820_MemoryInfo* mem_info);