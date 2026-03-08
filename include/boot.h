#pragma once

#include "types.h"

typedef struct {
    Disk disk;
    MemoryInfo memory_info;
} BootInfo;

typedef struct {
    void (*printk)(const char* fmt, ...);
    int (*disk_read)(void* abar, uint8_t port, LBA48 lba, uint32_t count, uint16_t* buffer);
} BootServices;
