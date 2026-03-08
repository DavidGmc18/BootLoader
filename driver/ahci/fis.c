#include "fis.h"

#define ATA_CMD_READ_DMA_EX 0x25

void FIS_set_reg_h2d_read(FIS_REG_H2D* cmdfis, BL_LBA48 lba, uint32_t count) {
	cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;	// Is a command
    cmdfis->command = ATA_CMD_READ_DMA_EX;
	
	// Map your AHCI_LBA_48 into the FIS
    cmdfis->lba0 = lba.raw[0];
    cmdfis->lba1 = lba.raw[1];
    cmdfis->lba2 = lba.raw[2];
    cmdfis->lba3 = lba.raw[3];
    cmdfis->lba4 = lba.raw[4];
    cmdfis->lba5 = lba.raw[5];
    cmdfis->device = 1 << 6; // LBA mode

    cmdfis->countl = (uint8_t)count;
    cmdfis->counth = (uint8_t)(count >> 8);
}

#define ATA_CMD_IDENTIFY 0xEC

void FIS_set_reg_h2d_identify(FIS_REG_H2D* cmdfis) {
    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;	// Is a command
    cmdfis->command = ATA_CMD_IDENTIFY;
	
	// Map your AHCI_LBA_48 into the FIS
    cmdfis->lba0 = 0;
    cmdfis->lba1 = 0;
    cmdfis->lba2 = 0;
    cmdfis->lba3 = 0;
    cmdfis->lba4 = 0;
    cmdfis->lba5 = 0;
    cmdfis->device = 0xA0;

    cmdfis->countl = 0;
    cmdfis->counth = 0;
}