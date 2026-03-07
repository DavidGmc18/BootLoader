#include "ahci.h"
#include <util/printk.h>
#include <stddef.h>
#include "hba.h"
#include "fis.h"
#include <arch/i686/io.h>

void* AHCI_get_abar(pci_address_t pci_address) {
    if (pci_address.enabled == 0) {
        printk("ERROR: AHCI not found!\n");
        return NULL;
    }

    pci_address.offset = 0x24;
    return (void*)PCI_read_config(pci_address);
}

#define	SATA_SIG_ATA	0x00000101
#define	SATA_SIG_ATAPI	0xEB140101
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM	    0x96690101	// Port multiplier

static int get_dev_type(HBA_PORT *port) {
	uint32_t ssts = port->ssts;

	uint8_t ipm = (ssts >> 8) & 0x0F;
	uint8_t det = ssts & 0x0F;

	if (det != HBA_PORT_DET_PRESENT)
		return 0;
	if (ipm != HBA_PORT_IPM_ACTIVE)
		return 0;

	switch (port->sig) {
        case SATA_SIG_ATA:
            return AHCI_DEV_SATA;
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEV_PM;
        default:
            return AHCI_DEV_UNK;
	}
}

void AHCI_map_devs(AHCI_dev_map* dev_map, void* abar) {
	HBA_MEM* hba = (HBA_MEM*)abar;

    uint32_t pi = hba->pi;
    int i = 0;
    while (i < 32) {
        if (pi & 1) {
            dev_map->dev[i] = get_dev_type(&hba->ports[i]);
        } else {
            dev_map->dev[i] = 0;
        }

        pi >>= 1;
        i++;
    }
}

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

// Only supports up to 2^13 sectors
// port->ci can change and became taken mid-function, this should not happen for single-threaded system
int AHCI_read(void* abar, uint8_t port_no, AHCI_LBA_48 lba, uint32_t count, uint16_t* buffer) {
	HBA_MEM* hba_mem = (HBA_MEM*)abar;
	HBA_PORT* port = &hba_mem->ports[port_no];

	// Find a free command slot (most controllers have 32)
	uint32_t used_slots = (port->ci | port->sact);
	uint8_t slot = 0;
	while (slot < 32) {
		if (!(used_slots & (1 << slot)))
			break;
		slot++;
	}
	if (slot >= 32)
		return -1;
    
    HBA_CMD_HEADER* cmdheader = (HBA_CMD_HEADER*)port->clb;
    cmdheader += slot;
	HBA_set_cmd_header(cmdheader, READ, 1);

    HBA_CMD_TBL* cmdtbl = (HBA_CMD_TBL*)(uintptr_t)cmdheader->ctba;
	HBA_set_cmd_table(cmdtbl, 0, buffer, count, NO_INTERRUPT_ON_COMPLETION);

    FIS_REG_H2D* cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);
	FIS_set_reg_h2d(cmdfis, lba, count);

	// Wait for port to be idle
    int spin = 0;
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) {
		i686_iowait();
        spin++;
    }
    if (spin == 1000000) return -1; // Port hung (timed out after ~ 0.4s)

	// Issue the command
    port->ci |= (1 << slot);

	// Wait for read to finish
    while (1) {
        if ((port->ci & (1 << slot)) == 0) break;

		if (port->is & (1 << 27)) return -1;
		if (port->is & (1 << 28)) return -1;
		if (port->is & (1 << 29)) return -1;
        if (port->is & (1 << 30)) return -1;
        if (port->tfd & 0x01) return -1;
		i686_iowait();
    }

	return 0;
}