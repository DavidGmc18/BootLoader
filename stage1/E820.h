#pragma once

#include <stdint.h>

typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t ACPI;
} E820_MemoryBlock;

typedef struct {
    uint32_t block_count;
    E820_MemoryBlock* blocks;
} E820_MemoryInfo;

// enum E820_MemoryType {
//     E820_USABLE = 1,
//     E820_RESERVED = 2,
//     E820_ACPI_RECLAIMABLE = 3,
//     E820_ACPI_NVS = 4,
//     E820_BAD_MEMORY = 5,
// };

void E820_detect(E820_MemoryInfo* mem_info);