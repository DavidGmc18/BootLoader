#include <bl/boot.h>

BL_BootSector boot_sector __attribute__((section(".boot_sector"))) = {
    .undefined = {0},
    .vbr = {
        .signature = BL_VBR_SIGNATURE,
        .boot_lba = 1,
        .boot_sectors = 10,
        .entry_offset = 0,
        .name = "TEST"
    },
    .boot_signature = 0xAA55
};