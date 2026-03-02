#include "pci.h"
#include <arch/i686/io.h>
#include <util/printk.h>
#include <stdbool.h>

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA 0xCFC

pci_address_t pci_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    return (pci_address_t){
        .enabled = 1,
        .reserved = 0,
        .bus = bus,
        .device = device,
        .function = function,
        .offset = offset
    };
}

uint32_t PCI_read_config(pci_address_t address) {
    uint8_t offset = address.offset & 0x3;
    address.offset &= 0xFC;
    i686_outl(CONFIG_ADDRESS, address.raw);
    return i686_inl(CONFIG_DATA) >> (offset * 8);
}

static pci_address_t scan_bus(uint8_t bus) {
    for (uint8_t device = 0; device < 32; device++) {
        // Check if device is present
        uint16_t vendor_id = PCI_read_config(pci_address(bus, device, 0, 0x0));
        if (vendor_id == 0xFFFF)
            continue;

        bool multi_function = PCI_read_config(pci_address(bus, device, 0, 0xE)) & 0x80;

        uint8_t function = 0;
        do {
            // Check if function is present
            uint16_t vendor_id = PCI_read_config(pci_address(bus, device, function, 0x0));
            if (vendor_id == 0xFFFF) continue;

            uint8_t header_type = PCI_read_config(pci_address(bus, device, function, 0xE)) & (~0x80);
            uint16_t class_reg = PCI_read_config(pci_address(bus, device, function, 0xA));

            // Mass Storage Controller - Serial ATA Controller
            if (class_reg == 0x0106 && header_type == 0x0)
                return pci_address(bus, device, function, 0);

            // Bridge - PCI-to-PCI Bridge
            if (class_reg == 0x0604  && header_type == 0x1) {
                uint8_t secondary_bus = PCI_read_config(pci_address(bus, device, function, 0x19));
                if (secondary_bus == bus)
                    continue;
                
                pci_address_t returned_address = scan_bus(secondary_bus);
                if (returned_address.enabled)
                    return returned_address;

                continue;
            }
        } while((++function < 8) && multi_function);
    }

    return (pci_address_t){.enabled = 0};
} 

pci_address_t PCI_find_SATA() {
    return scan_bus(0);
}