#include <stdint.h>
#include <driver/vga/vga_text.h>
#include <util/printk.h>

extern uint8_t __bss_start;
extern uint8_t __end;

void* memset(void* ptr, int value, uint16_t num) {
    uint8_t* u8Ptr = (uint8_t*)ptr;

    for (uint16_t i = 0; i < num; i++)
        u8Ptr[i] = (uint8_t)value;

    return ptr;
}

typedef struct {
    void* abar;
    uint8_t port;
    uint8_t partition_id;
} BootDisk;

typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t ACPI;
} E820_MemoryBlock;

typedef struct {
    uint32_t block_count;
    E820_MemoryBlock* blocks;
} E820_MemoryInfo;

void __attribute__((section(".entry"))) start(BootDisk* boot_disk, E820_MemoryInfo* mem_info) {
    memset(&__bss_start, 0, (&__end) - (&__bss_start));

    VGA_Initialize(80, 25, (uint8_t*)0xB8000);
    VGA_clrscr();

    VGA_set_color(0xD0);
    printk("TEST OS!!!\n");

    VGA_set_color(0x07);
    printk("Boot params -> port=%d partition=%d\n", boot_disk->port, boot_disk->partition_id);

    printk("MEM: %d\n", mem_info->block_count);

    for (;;);
}