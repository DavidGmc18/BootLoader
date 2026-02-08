#pragma once

#include <stdint.h>

#define ATA_ERRC_SUCCESS        0
#define ATA_ERRC_INVALID_BASE   1 // invalid bus
#define ATA_ERRC_SR_ERR         2
#define ATA_ERRC_TIMED_OUT      3
#define ATA_ERRC_FLOATING_BUS   4 // no disk present
#define ATA_ERRC_NOT_ATA_DISK   5

typedef uint16_t ATA_bus_t;
typedef uint16_t ATA_disk_t;

int ATA_io_delay(ATA_bus_t bus);

int ATA_soft_reset(ATA_bus_t bus);

int ATA_identify(ATA_disk_t disk, void* buffer);

int ATA_read28(ATA_disk_t disk, uint32_t LBA, uint8_t sectors, void* buffer);