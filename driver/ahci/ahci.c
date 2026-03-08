#include "ahci.h"
#include <util/printk.h>
#include <stddef.h>
#include "hba.h"
#include "fis.h"
#include <arch/i686/i686.h>
#include <util/memory.h>

#define MAX_PORTS 8
#define MAX_SLOTS 4

static uint8_t cmd_list_buf[MAX_PORTS][1024] __attribute__((aligned(1024), section(".ahci_mem")));
static uint8_t fis_buf[MAX_PORTS][256]       __attribute__((aligned(256), section(".ahci_mem")));
static uint8_t cmd_table_buf[MAX_PORTS][MAX_SLOTS][256] __attribute__((aligned(128), section(".ahci_mem")));

static HBA_CMD_HEADER* cmd_headers[MAX_PORTS];

void* AHCI_get_abar(pci_address_t pci_address) {
    if (pci_address.enabled == 0) {
        printk("ERROR: AHCI not found!\n");
        return NULL;
    }

    pci_address.offset = 0x24;
    uint32_t bar = PCI_read_config(pci_address);

    if (bar == 0 || bar == 0xFFFFFFFF) {
        printk("ERROR: AHCI BAR5 invalid!\n");
        return NULL;
    }

    bar &= ~0xF; // mask off lower 4 flag bits
    return (void*)bar;
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

#define HBA_PxCMD_ST    0x0001
#define HBA_PxCMD_FRE   0x0010
#define HBA_PxCMD_FR    0x4000
#define HBA_PxCMD_CR    0x8000

static void start_cmd(HBA_PORT* port) {
    uint32_t timeout = 500000; // ~ 0.2s
	while ((port->cmd & HBA_PxCMD_CR) && --timeout) // wait until CR (bit15) is cleared
        i686_iowait();

	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST; 
}

static void stop_cmd(HBA_PORT* port) {
    port->cmd &= ~HBA_PxCMD_ST;
    uint32_t timeout = 500000; // ~ 0.2s
    while ((port->cmd & HBA_PxCMD_CR) && --timeout)  // wait CR clear FIRST
        i686_iowait();

    port->cmd &= ~HBA_PxCMD_FRE;
    timeout = 500000;
    while ((port->cmd & HBA_PxCMD_FR) && --timeout)  // then wait FR clear
        i686_iowait();
}

static int wait_for_link(HBA_PORT* port) {
    uint32_t timeout = 1000000; // ~0.4s
    while (--timeout) {
        if ((port->ssts & 0xF) == 3)
            return 0;
        i686_iowait();
    }
    return -1; // no link
}

static int port_rebase(HBA_PORT* port, int port_no) {
    if (port_no >= MAX_PORTS)
        return -1;

    if ((port->ssts & 0xF) != 3) {
        // No device, just zero the slot in our map
        cmd_headers[port_no] = NULL;
        return -1;
    }

    stop_cmd(port);

    port->serr = 0xFFFFFFFF;
    port->is   = 0xFFFFFFFF;

    // Zero buffers
    memset(cmd_list_buf[port_no], 0, 1024);
    memset(fis_buf[port_no],      0, 256);

    // Program physical addresses into registers
    port->clb  = (uint32_t)(uintptr_t)cmd_list_buf[port_no];
    port->clbu = 0;
    port->fb   = (uint32_t)(uintptr_t)fis_buf[port_no];
    port->fbu  = 0;

    // Keep software pointers
    cmd_headers[port_no] = (HBA_CMD_HEADER*)cmd_list_buf[port_no];

    // Wire each command slot to its table
    for (int i = 0; i < MAX_SLOTS; i++) {
        memset(cmd_table_buf[port_no][i], 0, 256);
        cmd_headers[port_no][i].ctba  = (uint32_t)(uintptr_t)cmd_table_buf[port_no][i];
        cmd_headers[port_no][i].ctbau = 0;
        cmd_headers[port_no][i].prdtl = 1; // you only use 1 PRDT entry anyway
    }

    start_cmd(port);

    if (wait_for_link(port)) {
        stop_cmd(port);
        cmd_headers[port_no] = NULL;
        return -1;
    }

    return 0;
}

void AHCI_init(void* abar, AHCI_dev_map* dev_map) {
    HBA_MEM* hba = (HBA_MEM*)abar;

    // Enable AHCI mode
    hba->ghc |= (1 << 31);

    uint32_t pi = hba->pi;
    for (int i = 0; i < MAX_PORTS; i++) {
        if (pi & (1 << i)) {
            if (port_rebase(&hba->ports[i], i)) {
                dev_map->dev[i] = 0;
            } else {
                dev_map->dev[i] = get_dev_type(&hba->ports[i]);
            }
        } else {
            dev_map->dev[i] = 0;
        }
    }
}

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

static int wait_port_idle(HBA_PORT* port) {
    int spin = 0;
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) {
		i686_iowait();
        spin++;
    }
    if (spin == 1000000) return -1; // Port hung (timed out after ~ 0.4s)

    return 0;
}

static int wait_command_finish(HBA_PORT* port, uint8_t slot) {
    if (slot >= MAX_SLOTS)
        return -1;

    uint32_t timeout = 5000000; // ~ 2s timeout
    while (--timeout) {
        if ((port->ci & (1 << slot)) == 0)
            return 0;

		if (port->is & ((1<<27)|(1<<28)|(1<<29)|(1<<30))) {
            port->serr = 0xFFFFFFFF; // must clear before next command
            port->is   = 0xFFFFFFFF;
            return -1;
        }

        if (port->tfd & 0x01) {
            port->serr = 0xFFFFFFFF;
            port->is   = 0xFFFFFFFF;
            return -1;
        }

		i686_iowait();
    }

	return -2;
}

static void issue_command(HBA_PORT* port, uint8_t slot) {
    port->serr = 0xFFFFFFFF;
    port->is   = 0xFFFFFFFF;
    port->ci |= (1 << slot);
}

int AHCI_identify(void* abar, uint8_t port_no, uint16_t* buffer) {
    if (port_no >= MAX_PORTS || cmd_headers[port_no] == NULL)
        return -1;

    if ((uintptr_t)buffer & 0x1)   // misaligned
    return -1;
    if ((uintptr_t)buffer > 0xFFFFFFFF)  // above 4GB (can't happen on i386 but good practice)
        return -1;

    HBA_MEM* hba_mem = (HBA_MEM*)abar;
	HBA_PORT* port = &hba_mem->ports[port_no];

    uint32_t used_slots = (port->ci | port->sact);
	uint8_t slot = 0;
	while (slot < MAX_SLOTS) {
		if (!(used_slots & (1 << slot)))
			break;
		slot++;
	}
	if (slot >= MAX_SLOTS)
		return -1;

    HBA_CMD_HEADER* cmdheader = &cmd_headers[port_no][slot];
	HBA_set_cmd_header(cmdheader, READ, 1);

    HBA_CMD_TBL* cmdtbl = (HBA_CMD_TBL*)cmd_table_buf[port_no][slot];
	HBA_set_cmd_table(cmdtbl, 0, buffer, 1, NO_INTERRUPT_ON_COMPLETION);

    FIS_REG_H2D* cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);
	FIS_set_reg_h2d_identify(cmdfis);

    if (wait_port_idle(port))
        return -1;

	issue_command(port, slot);

    return wait_command_finish(port, slot);
}

// Only supports up to 2^13 sectors
// port->ci can change and became taken mid-function, this should not happen for single-threaded system
int AHCI_read(void* abar, uint8_t port_no, AHCI_LBA_48 lba, uint32_t count, uint16_t* buffer) {
    if (port_no >= MAX_PORTS || cmd_headers[port_no] == NULL)
        return -1;

    if ((uintptr_t)buffer & 0x1)   // misaligned
    return -1;
    if ((uintptr_t)buffer > 0xFFFFFFFF)  // above 4GB (can't happen on i386 but good practice)
        return -1;

	HBA_MEM* hba_mem = (HBA_MEM*)abar;
	HBA_PORT* port = &hba_mem->ports[port_no];

	// Find a free command slot (most controllers have 32)
	uint32_t used_slots = (port->ci | port->sact);
	uint8_t slot = 0;
	while (slot < MAX_SLOTS) {
		if (!(used_slots & (1 << slot)))
			break;
		slot++;
	}
	if (slot >= MAX_SLOTS)
		return -1;
    
    HBA_CMD_HEADER* cmdheader = &cmd_headers[port_no][slot];
	HBA_set_cmd_header(cmdheader, READ, 1);

    HBA_CMD_TBL* cmdtbl = (HBA_CMD_TBL*)cmd_table_buf[port_no][slot];
	HBA_set_cmd_table(cmdtbl, 0, buffer, count, NO_INTERRUPT_ON_COMPLETION);

    FIS_REG_H2D* cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);
	FIS_set_reg_h2d_read(cmdfis, lba, count);

    if (wait_port_idle(port))
        return -1;

	issue_command(port, slot);

    return wait_command_finish(port, slot);
}