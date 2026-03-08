#pragma once

#include <stdint.h>

typedef union {
    struct {
        uint16_t low;
        uint16_t mid;
        uint16_t high;
    }__attribute__((packed));
    uint8_t raw[6];
} __attribute__((packed)) LBA48;

typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t ACPI;
} MemoryBlock;

typedef struct {
    uint8_t block_count;
    MemoryBlock blocks[256];
} MemoryInfo;

typedef struct {
    uint8_t id;
    uint32_t lba;
    uint32_t sectors;
} Parition;

typedef struct {
    void* abar;
    uint8_t port;
    Parition partition;
    char drive_name[41];
} Disk;
