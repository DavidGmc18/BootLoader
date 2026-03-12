#include <stdint.h>
#include <driver/vga/vga_text.h>
#include <util/printk.h>
#include "E820.h"
#include <driver/pci/pci.h>
#include <driver/ahci/ahci.h>
#include "mbr.h"
#include <arch/i686/i686.h>
#include <bl/boot.h>

extern uint8_t __os_start;
extern uint8_t __os_size;

typedef void (*Start)(BL_BootInfo*, BL_BootServices*);

BL_BootInfo boot_info __attribute__((section(".boot_info")));
BL_BootServices boot_services __attribute__((section(".boot_info")));

#define MAX_BOOTABLE_DISKS 20
BL_Disk bootable_disk[MAX_BOOTABLE_DISKS];
uint8_t bootable_disk_count = 0;

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

        for (uint8_t partition_id = 0; partition_id < 4; partition_id++) {

            if (!MBR_is_bootable(mbr_table.entries[partition_id]))
                continue;

            bootable_disk[bootable_disk_count].abar = abar;
            bootable_disk[bootable_disk_count].port = port;

            bootable_disk[bootable_disk_count].partition.id = partition_id;
            bootable_disk[bootable_disk_count].partition.lba = mbr_table.entries[partition_id].lba;
            bootable_disk[bootable_disk_count].partition.sectors = mbr_table.entries[partition_id].sectors;

            bool flag = false;
            uint8_t i = 40;
            while (i-- > 0) {
                char ch = (i & 1) ? (buffer[i/2+27] & 0xFF) : ((buffer[i/2+27] >> 8) & 0xFF);
                if (ch != 0x20 && ch != 0x00)
                    flag = true;
                bootable_disk[bootable_disk_count].drive_name[i] = (flag) ? ch : 0x00;
            }
            bootable_disk[bootable_disk_count].drive_name[40] = '\0';

            bootable_disk_count++;
            if (bootable_disk_count >= MAX_BOOTABLE_DISKS)
                break;
        }

        if (bootable_disk_count >= MAX_BOOTABLE_DISKS)
            break;
    }

    VGA_clrscr();
    printk("Select partition:\n");

    for (uint8_t i = 0; i < bootable_disk_count; i++) {
        printk("  Drive-%d  %s  Partition-%d ", bootable_disk[i].port, bootable_disk[i].drive_name, bootable_disk[i].partition);

        if (bootable_disk[i].partition.sectors < 2048) {
            printk("(%d.%dKib)\n", bootable_disk[i].partition.sectors/2, (bootable_disk[i].partition.sectors%2)*5);
        } else if (bootable_disk[i].partition.sectors < 2097152) {
            printk("(%d.%dMib)\n", bootable_disk[i].partition.sectors/2048, ((bootable_disk[i].partition.sectors%2048)*10)/2048);
        } else {
            printk("(%d.%dGib)\n", bootable_disk[i].partition.sectors/2097152, (((uint64_t)bootable_disk[i].partition.sectors%2097152)*10)/2097152);
        }
    }

    if (bootable_disk_count == 0) {
        printk("  No bootable partitions!\n");
        goto end;
    }

    VGA_setcursor(0, 22);
    printk("ARROW UP/DOWN - go up/down\n");
    printk("ENTER - select partiton and boot\n");
    printk("HIGHLIGHTED - current selection");
    for (int x = 0; x < 11; x ++) {
        VGA_putcolor(x, 24, 0x70);
    }

    uint8_t CURSOR = 0;
    for (int x = 2; x < 78; x ++) {
        VGA_putcolor(x, 1, 0x70);
    }
    VGA_setcursor(25, 80);

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
                    if (CURSOR+1 >= bootable_disk_count)
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

    BL_BootSector boot_sector;
    if (AHCI_read(bootable_disk[CURSOR].abar, bootable_disk[CURSOR].port, (BL_LBA48){bootable_disk[CURSOR].partition.lba}, 1, (uint16_t*)&boot_sector)) {
        printk("BOOT failed!\n");
        goto end;
    }

    if (boot_sector.boot_signature != 0xAA55 || boot_sector.vbr.signature != BL_VBR_SIGNATURE) {
        printk("BOOT failed: invalid VBR!\n");
        goto end;
    }

    BL_LBA48 lba = {
        .low = bootable_disk[CURSOR].partition.lba + boot_sector.vbr.boot_lba
    };

    uint32_t sectors = boot_sector.vbr.boot_sectors;
    if (sectors * 512 > ((uint32_t)&__os_size)) {
        printk("BOOT failed: OS too large!\n");
        goto end;
    }

    void* location = &__os_start;

    if (AHCI_read(bootable_disk[CURSOR].abar, bootable_disk[CURSOR].port, lba, sectors, (uint16_t*)location)) {
        printk("BOOT failed!\n");
        goto end;
    }

    boot_info.disk = bootable_disk[CURSOR];
    E820_detect(&boot_info.memory_info);

    boot_services.printk = printk;
    boot_services.disk_read = AHCI_read;

    Start start = (Start)((uint8_t*)location + boot_sector.vbr.entry_offset);
    start(&boot_info, &boot_services);




    // void* location = &__os_start;

    // if (AHCI_read(bootable_disk[CURSOR].abar, bootable_disk[CURSOR].port, (BL_LBA48){0}, 64, (uint16_t*)location)) {
    //     printk("BOOT failed!\n");
    //     goto end;
    // }

    // boot_info.disk = bootable_disk[CURSOR];
    // E820_detect(&boot_info.memory_info);

    // boot_services.printk = printk;
    // boot_services.disk_read = AHCI_read;

    // Start start = (Start)((uint8_t*)location + 512*33);
    // start(&boot_info, &boot_services);

end:
    while (1) i686_hlt();
}