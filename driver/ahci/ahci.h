#pragma once

#include <stdint.h>

#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4

extern uint8_t AHCI_drives[32];

int AHCI_probe_drives();