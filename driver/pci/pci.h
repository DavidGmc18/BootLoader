#pragma once

#include <stdint.h>

typedef union {
    struct {
        uint8_t offset      : 8;
        uint8_t function    : 3;
        uint8_t device      : 5;
        uint8_t bus         : 8;
        uint8_t reserved    : 7;
        uint8_t enabled     : 1;
    } __attribute__((packed));
    uint32_t raw;
} pci_address_t;
pci_address_t pci_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);

uint32_t PCI_read_config(pci_address_t address);

pci_address_t PCI_find_SATA();