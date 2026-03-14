#include "mbr.h"
#include <bl/boot.h>

#define MBR_START_WORD 223
#define MBR_ENTRY_WORDS 8

int MBR_get_table(MBR_Table* table, void* abar, uint8_t port) {
    uint16_t buffer[256];

    int error = AHCI_read(abar, port, (BL_LBA48){0}, 1, buffer);
    if (error)
        return error;

    for (int i = 0; i < 4; i++) {
        uint16_t* entry_words = buffer + MBR_START_WORD + i * MBR_ENTRY_WORDS;

        table->entries[i].raw[0] = entry_words[0] | (entry_words[1] << 16);
        table->entries[i].raw[1] = entry_words[2] | (entry_words[3] << 16);
        table->entries[i].raw[2] = entry_words[4] | (entry_words[5] << 16);
        table->entries[i].raw[3] = entry_words[6] | (entry_words[7] << 16);
    }

    return 0;
}

bool MBR_is_bootable(MBR_Entry entry, void* abar, uint8_t port) {
    if (!(entry.attributes & 0x80))
        return false;

    if (entry.attributes == 0xFF)
        return false;

    uint16_t buffer[256];
    if (AHCI_read(abar, port, (BL_LBA48){entry.lba}, 1, buffer))
        return false;

    BL_BootSector* boot_sector = (BL_BootSector*)buffer;
    if (boot_sector->boot_signature != BL_BOOT_SIGNATURE || boot_sector->vbr.signature != BL_VBR_SIGNATURE)
        return false;

    return true;
}