#pragma once

#include <stdint.h>

typedef union {
    struct {
        uint16_t low;
        uint16_t mid;
        uint16_t high;
    }__attribute__((packed));
    uint8_t raw[6];
} __attribute__((packed)) BL_LBA48;

typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t ACPI;
} BL_MemoryBlock;

typedef struct {
    uint8_t block_count;
    BL_MemoryBlock blocks[256];
} BL_MemoryInfo;

typedef struct {
    uint8_t id;
    uint32_t lba;
    uint32_t sectors;
} BL_Parition;

typedef struct {
    void* abar;
    uint8_t port;
    BL_Parition partition;
    char drive_name[41];
} BL_Disk;

typedef void (*BL_PrintK)(const char* fmt, ...);
typedef int (*BL_DiskRead)(void* abar, uint8_t port, BL_LBA48 lba, uint32_t count, uint16_t* buffer);