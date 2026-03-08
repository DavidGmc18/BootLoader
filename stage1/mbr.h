#pragma once

#include <stdint.h>
#include <driver/ahci/ahci.h>
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

typedef struct {
    MBR_Entry entries[4];
} MBR_Table;

int MBR_get_table(MBR_Table* table, void* abar, uint8_t port);

bool MBR_is_bootable(MBR_Entry entry);