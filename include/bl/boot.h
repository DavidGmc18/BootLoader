#pragma once

#include "types.h"

typedef struct {
    BL_Disk disk;
    BL_MemoryInfo memory_info;
} BL_BootInfo;

typedef struct {
    void (*printk)(const char* fmt, ...);
    int (*disk_read)(void* abar, uint8_t port, BL_LBA48 lba, uint32_t count, uint16_t* buffer);
} BL_BootServices;
