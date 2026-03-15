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
    BL_BootHeader boot_header;
    uint16_t boot_signature;
} __attribute__((packed)) BL_VBR;

_Static_assert(sizeof(BL_VBR) == 512, "BL_VBR must be 512 bytes");