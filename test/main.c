#include <bl/boot.h>
#include <stdint.h>

extern uint8_t __bss_start;
extern uint8_t __end;

void* memset(void* ptr, int value, uint16_t num) {
    uint8_t* u8Ptr = (uint8_t*)ptr;

    for (uint16_t i = 0; i < num; i++)
        u8Ptr[i] = (uint8_t)value;

    return ptr;
}

void __attribute__((section(".entry"))) start(BL_BootInfo* boot_info, BL_BootServices* boot_services) {
    memset(&__bss_start, 0, (&__end) - (&__bss_start));

    boot_services->printk("TEST!!!\n");

    boot_services->printk("boot_info = {\n");
    boot_services->printk("  disk = {\n");
    boot_services->printk("    abar = 0x%x\n", boot_info->disk.abar);
    boot_services->printk("    port = %d\n", boot_info->disk.port);
    boot_services->printk("    partition = {\n");
    boot_services->printk("      id = %d\n", boot_info->disk.partition.id);
    boot_services->printk("      lba = %d\n", boot_info->disk.partition.lba);
    boot_services->printk("      sectors = %d\n", boot_info->disk.partition.sectors);
    boot_services->printk("    }\n");
    boot_services->printk("  drive_name = '%s'\n", boot_info->disk.drive_name);
    boot_services->printk("  memory_info = {\n");
    boot_services->printk("    block_count = %d\n", boot_info->memory_info.block_count);
    boot_services->printk("    blocks[256]\n");
    boot_services->printk("  }\n");
    boot_services->printk("}\n");

    uint16_t buffer[256];
    boot_services->disk_read(boot_info->disk.abar, boot_info->disk.port, (BL_LBA48){0}, 1, buffer);
    boot_services->printk("Test disk read => 0x%x\n", buffer[255]);
    
    while (1)
        __asm__ volatile ("hlt" ::: "memory");
}