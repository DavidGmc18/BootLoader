#include <stdint.h>
#include <driver/vga/vga_text.h>
#include <util/printk.h>
#include <driver/ata/ata.h>

extern uint8_t __bss_start;
extern uint8_t __end;

void* memset(void* ptr, int value, uint16_t num) {
    uint8_t* u8Ptr = (uint8_t*)ptr;

    for (uint16_t i = 0; i < num; i++)
        u8Ptr[i] = (uint8_t)value;

    return ptr;
}

void __attribute__((section(".entry"))) start(uint16_t drive, uint8_t partition) {
    memset(&__bss_start, 0, (&__end) - (&__bss_start));

    VGA_Initialize(80, 25, (uint8_t*)0xB8000);
    VGA_clrscr();

    VGA_set_color(0xD0);
    printk("TEST OS!!!\n");

    VGA_set_color(0x07);
    printk("Boot params -> drive=%d partition=%d\n", drive, partition);

    ATA_read28(0, 0, 1, (void*)0x1000000);

    for (;;);
}