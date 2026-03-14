#pragma once

#include "types.h"

typedef struct {
    BL_Disk disk;
    BL_MemoryInfo memory_info;
} BL_BootInfo;

typedef struct {
    BL_PrintK printk;
    BL_DiskRead disk_read;
} BL_BootServices;

#define BL_BOOT_SIGNATURE 0xAA55

typedef struct {
    uint8_t undefined[446];
    BL_VBR vbr;
    uint16_t boot_signature;
} __attribute__((packed)) BL_BootSector;

_Static_assert(sizeof(BL_BootSector) == 512, "BL_BootSector must be 512 bytes");