#include <bl/boot.h>

BL_VBR vbr __attribute__((section(".vbr"))) = {
    .undefined = {0},
    .boot_header = {
        .signature = BL_BOOT_HEADER_SIGNATURE,
        .boot_lba = 1,
        .boot_sectors = 10,
        .entry_offset = 0,
        .name = "TEST"
    },
    .boot_signature = 0xAA55
};