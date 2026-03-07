#include <stdint.h>
#include <driver/vga/vga_text.h>
#include <driver/mbr/mbr.h>
#include <selector.h>
#include <util/printk.h>
#include <driver/ata/ata.h>
#include "E820.h"
#include <driver/pci/pci.h>
#include <driver/ahci/ahci.h>

typedef void (*Start)(ATA_disk_t drive, uint8_t partition, E820_MemoryInfo* mem_info);

void __attribute__((cdecl)) start() {
    VGA_Initialize(80, 25, (uint8_t*)0xB8000);
    VGA_clrscr();

    pci_address_t sata_address = PCI_find_SATA();
    void* ahci_abar = AHCI_get_abar(sata_address);
    AHCI_dev_map dev_map;
    AHCI_map_devs(&dev_map, ahci_abar);

    for (int i = 0; i < 32; i++) {
        printk("%d ", dev_map.dev[i]);
    }
    printk("\n");

    uint16_t buffer[256];
    int error = AHCI_read(ahci_abar, 0, (AHCI_LBA_48){0}, 1, buffer);
    printk("AHCI: read error=%d\n", error);
    printk("%x\n", buffer[255]);

    // MBR_Drive drives[4];
    // uint16_t count = MBR_discover(drives, 4);
    // if (count == 0) {
    //     printk("No present drives found!");
    //     goto end;
    // }

    // E820_MemoryInfo mem_info;
    // E820_detect(&mem_info);

    // VGA_clrscr();
    // VGA_set_color(0x0F);    

    // printk("Select boot partition:\n");
    // SELECTOR_Initialize(drives);

    // VGA_setcursor(0, 22);

    // VGA_set_color(0x07);
    // printk("ARROW UP/DOWN - go up/down\n");
    // printk("ENTER - select partiton and boot\n");
    // printk("Color: ");
    // VGA_set_color(0x0F);
    // printk("WHITE");
    // VGA_set_color(0x07);
    // printk(" - marked as bootable; GRAY - marked as not bootable;");

    // SELECTOR_selection selection;
    // selection = SELECTOR_loop();

    // VGA_clrscr();
    // VGA_set_color(0x07);

    // printk("Selected: Drive %d Partition %d\n", selection.drive, selection.partition);

    // MBR_Drive boot_drive = drives[selection.drive];
    // MBR_Entry boot_partition = boot_drive.partitions[selection.partition];

    // printk("LBA=%d Sectors=%d\n", boot_partition.lba, boot_partition.sectors);

    // void* location = (void*)0x100000;

    // int error = ATA_read28(boot_drive.drive, boot_partition.lba, 32, location);

    // if (error != ATA_ERRC_SUCCESS) {
    //     printk("BOOT failed!\n");
    //     goto end;
    // }

    // Start start = (Start)location;
    // start(boot_drive.drive, selection.partition, &mem_info);

end:
    for (;;);
}