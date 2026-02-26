#pragma once

#include <stdint.h>
#include <driver/ata/ata.h>
#include <stdbool.h>

typedef union {
    uint32_t raw[4];
    struct {
        uint8_t attributes;
        uint8_t chs_start[3];
        uint8_t type;
        uint8_t chs_last[3];
        uint32_t lba;
        uint32_t sectors;
    };
} MBR_Entry;

// entries - buffer of 4 (or more)
int MBR_identify(ATA_disk_t disk, MBR_Entry* entries);

typedef struct {
    bool present;
    ATA_disk_t drive;
    char name[41];
    MBR_Entry partitions[4];
} MBR_Drive;

// drives - buffer of size n
// return number of present drives
uint16_t MBR_discover(MBR_Drive* drives, int n);