#pragma once

#include <driver/mbr/mbr.h>

// drives - len is 4
void SELECTOR_Initialize(MBR_Drive* drives);

typedef struct {
    ATA_disk_t drive;
    uint8_t partition;
} SELECTOR_selection;

SELECTOR_selection SELECTOR_loop();