#include "mbr.h"

#define MBR_START_WORD 223
#define MBR_ENTRY_WORDS 8

#define MBR_ERRC_SUCCESS 0

int MBR_identify(ATA_disk_t disk, MBR_Entry* entries) {
    uint16_t mbr_buffer[256];

    int error = ATA_read28(disk, 0, 1, mbr_buffer);
    if (error != ATA_ERRC_SUCCESS) {
        return error;
    }

    for (int i = 0; i < 4; i++) {
        uint16_t* entry_words = mbr_buffer + MBR_START_WORD + i * MBR_ENTRY_WORDS;

        entries[i].raw[0] = entry_words[0] | (entry_words[1] << 16);
        entries[i].raw[1] = entry_words[2] | (entry_words[3] << 16);
        entries[i].raw[2] = entry_words[4] | (entry_words[5] << 16);
        entries[i].raw[3] = entry_words[6] | (entry_words[7] << 16);
    }

    return MBR_ERRC_SUCCESS;
}

uint16_t MBR_discover(MBR_Drive* drives, int n) {
    uint16_t count = 0;

    for (ATA_disk_t d = 0; d < n; d++) {
        uint16_t buffer[256];
        int error = ATA_identify(d, buffer);

        if (error != ATA_ERRC_SUCCESS) {
            drives[d].present = false;
            continue;
        }

        drives[d].present = true;
        drives[d].drive = d;
        count++;
        
        for (int i = 0; i < 20; i++) {
            drives[d].name[i*2+0] = ((buffer[i+27] >> 8) & 0xFF);
            drives[d].name[i*2+1] = (buffer[i+27] & 0xFF);
        }
        drives[d].name[40] = '\0';

        error = MBR_identify(d, drives[d].partitions);
        if (error != MBR_ERRC_SUCCESS) {
            drives[d].present = false;
            continue;
        }
    }

    return count;
}