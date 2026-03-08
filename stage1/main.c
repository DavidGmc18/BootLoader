#include <stdint.h>
#include <driver/vga/vga_text.h>
#include <util/printk.h>
#include "E820.h"
#include <driver/pci/pci.h>
#include <driver/ahci/ahci.h>
#include "mbr.h"
#include <arch/i686/i686.h>

typedef struct {
    void* abar;
    uint8_t port;
    uint8_t partition_id;
} BootDisk;

typedef void (*Start)(BootDisk*, E820_MemoryInfo*);

BootDisk boot_disk __attribute__((section(".boot_info")));
E820_MemoryInfo mem_info __attribute__((section(".boot_info")));

#define MAX_BOOTABLE_PARTITIONS 20
MBR_Bootable_Partition bootable_partitions[MAX_BOOTABLE_PARTITIONS];
uint8_t bootable_partitions_count = 0;

void __attribute__((cdecl)) start() {
    VGA_Initialize(80, 25, (uint8_t*)0xB8000);
    VGA_clrscr();

    pci_address_t sata_address = PCI_find_SATA();
    void* abar = AHCI_get_abar(sata_address);

    AHCI_dev_map dev_map;
    AHCI_init(abar, &dev_map);

    uint16_t buffer[256];
    for (uint8_t port = 0; port < 32; port++) {
        if (dev_map.dev[port] != AHCI_DEV_SATA)
            continue;

        MBR_Table mbr_table;
        if (MBR_get_table(&mbr_table, abar, port))
            continue;

        if (AHCI_identify(abar, port, buffer))
            continue;

        for (uint8_t partition = 0; partition < 4; partition++) {

            if (!MBR_is_bootable(mbr_table.entries[partition]))
                continue;

            bootable_partitions[bootable_partitions_count].port = port;
            bootable_partitions[bootable_partitions_count].partition = partition;
            bootable_partitions[bootable_partitions_count].base = mbr_table.entries[partition].lba;
            bootable_partitions[bootable_partitions_count].sectors = mbr_table.entries[partition].sectors;

            bool flag = false;
            uint8_t i = 40;
            while (i-- > 0) {
                char ch = (i & 1) ? (buffer[i/2+27] & 0xFF) : ((buffer[i/2+27] >> 8) & 0xFF);
                if (ch != 0x20 && ch != 0x00)
                    flag = true;
                bootable_partitions[bootable_partitions_count].drive_name[i] = (flag) ? ch : 0x00;
            }
            bootable_partitions[bootable_partitions_count].drive_name[40] = '\0';

            bootable_partitions_count++;
            if (bootable_partitions_count >= MAX_BOOTABLE_PARTITIONS)
                break;
        }

        if (bootable_partitions_count >= MAX_BOOTABLE_PARTITIONS)
            break;
    }

    VGA_clrscr();
    printk("Select partition:\n");

    for (uint8_t i = 0; i < bootable_partitions_count; i++) {
        printk("  Drive-%d  %s  Partition-%d ", bootable_partitions[i].port, bootable_partitions[i].drive_name, bootable_partitions[i].partition);

        if (bootable_partitions[i].sectors < 2048) {
            printk("(%d.%dKib)\n", bootable_partitions[i].sectors/2, (bootable_partitions[i].sectors%2)*5);
        } else if (bootable_partitions[i].sectors < 2097152) {
            printk("(%d.%dMib)\n", bootable_partitions[i].sectors/2048, ((bootable_partitions[i].sectors%2048)*10)/2048);
        } else {
            printk("(%d.%dGib)\n", bootable_partitions[i].sectors/2097152, (((uint64_t)bootable_partitions[i].sectors%2097152)*10)/2097152);
        }
    }

    if (bootable_partitions_count == 0) {
        printk("  No bootable partitions!\n");
        goto end;
    }

    uint8_t CURSOR = 0;
    for (int x = 2; x < 78; x ++) {
        VGA_putcolor(x, 1, 0x70);
    }

    int running = 1;
    while (running) {
        uint8_t status = i686_inb(0x64);
        if (status & 0x01) {
            uint8_t key_code = i686_inb(0x60);
            switch (key_code) {
                // UP
                case 0x48:
                    if (CURSOR == 0)
                        break;
                    for (int x = 2; x < 78; x ++) {
                        VGA_putcolor(x, CURSOR+1, 0x07);
                    }
                    CURSOR--;
                    for (int x = 2; x < 78; x ++) {
                        VGA_putcolor(x, CURSOR+1, 0x70);
                    }
                    break;
                
                // DOWN
                case 0x50:
                    if (CURSOR+1 >= bootable_partitions_count)
                        break;
                    for (int x = 2; x < 78; x ++) {
                        VGA_putcolor(x, CURSOR+1, 0x07);
                    }
                    CURSOR++;
                    for (int x = 2; x < 78; x ++) {
                        VGA_putcolor(x, CURSOR+1, 0x70);
                    }
                    break;
                
                // ENTER
                case 0x1C:
                    running = 0;
                    break;
                
                default:
                    break;
            }
        }
        i686_iowait();
    }

    VGA_clrscr();

    void* location = (void*)0x100000;

    if (AHCI_read(abar, bootable_partitions[CURSOR].port, (AHCI_LBA_48){.low=bootable_partitions[CURSOR].base}, 32, location)) {
        printk("BOOT failed!\n");
        goto end;
    }

    boot_disk.abar = abar;
    boot_disk.port = bootable_partitions[CURSOR].port;
    boot_disk.partition_id = bootable_partitions[CURSOR].partition;

    E820_detect(&mem_info);

    sizeof(E820_MemoryBlock);
    sizeof(E820_MemoryInfo);
    sizeof(BootDisk);

    Start start = (Start)location;
    start(&boot_disk, &mem_info);

end:
    for (;;);
}