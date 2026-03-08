#pragma once

#include <stdint.h>
#include <driver/pci/pci.h>
#include <include/types.h>

void* AHCI_get_abar(pci_address_t pci_address);

#define AHCI_DEV_NULL   0
#define AHCI_DEV_UNK    1
#define AHCI_DEV_SATA   2
#define AHCI_DEV_SATAPI 3
#define AHCI_DEV_PM     4
#define AHCI_DEV_SEMB   5

typedef struct {
    uint8_t dev[32];
} AHCI_dev_map;

void AHCI_init(void* abar, AHCI_dev_map* dev_map);

int AHCI_identify(void* abar, uint8_t port, uint16_t* buffer);

int AHCI_read(void* abar, uint8_t port, LBA48 lba, uint32_t count, uint16_t* buffer);