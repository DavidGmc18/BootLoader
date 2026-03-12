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
