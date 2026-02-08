#include "ata.h"
#include <arch/i686/io.h>
#include <util/printk.h>
#include <stddef.h>

// ATA I/O port offsets
#define ATA_REG_DATA        0x0   // Data Register (16-bit)
#define ATA_REG_ERROR       0x1   // Error Register (read)
// #define ATA_REG_FEATURES    0x1   // Features Register (write)
#define ATA_REG_SECCOUNT    0x2   // Sector count
#define ATA_REG_LBA_LOW     0x3   // LBA low byte
#define ATA_REG_LBA_MID     0x4   // LBA mid byte
#define ATA_REG_LBA_HIGH    0x5   // LBA high byte
#define ATA_REG_DRIVE       0x6   // Drive/Head register
#define ATA_REG_STATUS      0x7   // Status (read)
#define ATA_REG_COMMAND     0x7   // Command (write)

// Drive selection Primary/Secondary
#define ATA_BUSES_LEN 4
uint16_t ATA_BUSES[ATA_BUSES_LEN] = {
    0x1F0,
    0x170,
    0x1E8,
    0x168
};

// Control port
#define ATA_CTRL_OFFSET     0x206

// Status bits
#define ATA_SR_ERR  0x01
#define ATA_SR_BSY  0x80
#define ATA_SR_DRQ  0x08

// Control
#define ATA_CMD_CLEAR 0x00
#define ATA_CMD_SRST 0x04 // Software Reset
#define ATA_CMD_SELECT 0xA0
#define ATA_CMD_LBA 0xE0
#define ATA_CMD_READ_SECTORS 0x20
#define ATA_CMD_IDENTIFY 0xEC

#define ATA_SECTOR_WORDS 256

#define ATA_TIMEOUT 1000000

port_t get_base(ATA_bus_t bus) {
    if (bus >= ATA_BUSES_LEN) {
        printk("ATA: Invalid bus 0x%x\n", bus);
        return NULL_PORT;
    }

    return ATA_BUSES[bus];
}

int ATA_io_delay(ATA_bus_t bus) {
    port_t base = get_base(bus);
    if (base == NULL_PORT) return ATA_ERRC_INVALID_BASE;

    i686_inb(base + ATA_CTRL_OFFSET);
    i686_inb(base + ATA_CTRL_OFFSET);
    i686_inb(base + ATA_CTRL_OFFSET);
    i686_inb(base + ATA_CTRL_OFFSET);

    return ATA_ERRC_SUCCESS;
}

int ATA_soft_reset(ATA_bus_t bus) {
    port_t base = get_base(bus);
    if (base == NULL_PORT) return ATA_ERRC_INVALID_BASE;

    i686_outb(base + ATA_CTRL_OFFSET, ATA_CMD_SRST);
    ATA_io_delay(bus);
    i686_outb(base + ATA_CTRL_OFFSET, ATA_CMD_CLEAR);

    uint32_t timeout = ATA_TIMEOUT;
    uint8_t status;
    while (timeout--) {
        status = i686_inb(base + ATA_REG_STATUS);
        if (!(status & ATA_SR_BSY)) break;
    }

    if (status & ATA_SR_ERR) {
        uint8_t error = i686_inb(base + ATA_REG_ERROR);
        printk("ATA: Error 0x%x\n", error);
        return ATA_ERRC_SR_ERR;
    }

    if (timeout <= 0) {
        printk("ATA: Timedout!\n");
        return ATA_ERRC_TIMED_OUT; 
    }

    return ATA_ERRC_SUCCESS;
}

// TODO this behaves very weirdly with drive 2 (aka. hdc)
int ATA_identify(ATA_disk_t disk, void* buffer) {
    ATA_bus_t bus = disk >> 1;
    uint8_t slave_bit = (disk & 1) << 4;
    port_t base = get_base(bus);
    if (base == NULL_PORT) return ATA_ERRC_INVALID_BASE;

    int error = ATA_soft_reset(bus);
    if (error) return error;

    i686_outb(base + ATA_REG_DRIVE, ATA_CMD_SELECT | slave_bit);
    error = ATA_io_delay(bus);
    if (error) return error;

    i686_outb(base + ATA_REG_SECCOUNT, 0);
    i686_outb(base + ATA_REG_LBA_LOW, 0);
    i686_outb(base + ATA_REG_LBA_MID, 0);
    i686_outb(base + ATA_REG_LBA_HIGH, 0);

    i686_outb(base + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    uint8_t status = i686_inb(base + ATA_REG_STATUS);

    // TODO removed "status == 0" case, so it works with drive 2 (hdc), but its slower
    if (status == 0xFF) {
        printk("ATA: Floating bus 0x%x, disk=0x%x\n", bus, disk);
        return ATA_ERRC_FLOATING_BUS;
    }

    uint32_t timeout = ATA_TIMEOUT;
    while (timeout--) {
        status = i686_inb(base + ATA_REG_STATUS);
        if (!(status & ATA_SR_BSY)) break;
        if (status & ATA_SR_ERR) break;
    }

    if (status & ATA_SR_ERR) {
        uint8_t error = i686_inb(base + ATA_REG_ERROR);
        printk("ATA: Error 0x%x\n", error);
        return ATA_ERRC_SR_ERR;
    }

    if (timeout <= 0) {
        printk("ATA: Timedout!\n");
        return ATA_ERRC_TIMED_OUT; 
    }

    uint8_t mid = i686_inb(base + ATA_REG_LBA_MID);
    uint8_t high = i686_inb(base + ATA_REG_LBA_HIGH);

    timeout = ATA_TIMEOUT;
    while (timeout--) {
        status = i686_inb(base + ATA_REG_STATUS);
        if (status & ATA_SR_DRQ) break;
        if (status & ATA_SR_ERR) break;
    }

    if (status & ATA_SR_ERR) {
        uint8_t error = i686_inb(base + ATA_REG_ERROR);
        printk("ATA: Error 0x%x\n", error);
        return ATA_ERRC_SR_ERR;
    }

    if (timeout <= 0) {
        printk("ATA: Timedout!\n");
        return ATA_ERRC_TIMED_OUT; 
    }

    for (int i = 0; i < 256; i++) {
        ((uint16_t*)buffer)[i] = i686_inw(base + ATA_REG_DATA);
    }

    if (mid != 0 || high != 0)
        return ATA_ERRC_NOT_ATA_DISK;

    return ATA_ERRC_SUCCESS;
}

int ATA_read28(ATA_disk_t disk, uint32_t LBA, uint8_t sectors, void* buffer) {
    ATA_bus_t bus = disk >> 1;
    uint8_t slave_bit = (disk & 1) << 4;
    port_t base = get_base(bus);
    if (base == NULL_PORT) return ATA_ERRC_INVALID_BASE;

    i686_outb(base + ATA_REG_DRIVE, ATA_CMD_LBA | slave_bit | ((LBA >> 24) & 0x0F));
    i686_outb(base + ATA_REG_SECCOUNT, sectors);
    i686_outb(base + ATA_REG_LBA_LOW, LBA);
    i686_outb(base + ATA_REG_LBA_MID, LBA >> 8);
    i686_outb(base + ATA_REG_LBA_HIGH, LBA >> 16);

    i686_outb(base + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);

    uint8_t status = i686_inb(base + ATA_REG_STATUS);
    if (status == 0xFF || status == 0) {
        printk("ATA: Floating bus 0x%x, disk=0x%x\n", bus, disk);
        return ATA_ERRC_FLOATING_BUS;
    }

    if (status & ATA_SR_ERR) {
        uint8_t error = i686_inb(base + ATA_REG_ERROR);
        printk("ATA: Error 0x%x\n", error);
        return ATA_ERRC_SR_ERR;
    }

    for (int s = 0; s < sectors; s++) {
        uint32_t timeout = ATA_TIMEOUT;
        while (--timeout) {
            status = i686_inb(base + ATA_REG_STATUS);
            if (!(status & ATA_SR_BSY)) break;
            if (status & ATA_SR_DRQ) break;
            if (status & ATA_SR_ERR) break;
        }

        if (status & ATA_SR_ERR) {
            uint8_t error = i686_inb(base + ATA_REG_ERROR);
            printk("ATA: Error 0x%x\n", error);
            return ATA_ERRC_SR_ERR;
        }

        if (timeout <= 0) {
            printk("ATA: Timedout!\n");
            ATA_soft_reset(bus);
            return ATA_ERRC_TIMED_OUT; 
        }

        for (int w = 0; w < ATA_SECTOR_WORDS; w++) {
            ((uint16_t*)buffer)[s * ATA_SECTOR_WORDS + w] = i686_inw(base + ATA_REG_DATA);
        }
    }

    return ATA_ERRC_SUCCESS;
};