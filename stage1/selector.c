#include "selector.h"

#include <util/printk.h>
#include <driver/vga/vga_text.h>
#include <arch/i686/io.h>

// TODO disable/remove scroll in VGA

static int CURSOR;

static uint8_t drive_count = 0;
static ATA_disk_t drives_map[4];
static uint8_t partitions_y[16];
static bool partitions_map[16];

void SELECTOR_Initialize(MBR_Drive* drives) {
    // Render options
    for (int d = 0; d < 4; d++) {
        if (!drives[d].present) {
            continue;
        }

        drives_map[drive_count] = d;

        VGA_set_color(0x0F);
        printk("  Drive %d: %s\n", drives[d].drive, drives[d].name);

        for (int p = 0; p < 4; p++) {
            if (drives[d].partitions[p].attributes & 0x80) {
                partitions_map[drive_count*4+p] = true;
                VGA_set_color(0x0F);
            } else {
                partitions_map[drive_count*4+p] = false;
                VGA_set_color(0x07);
            }

            uint8_t x, y;
            VGA_get_cursor(&x, &y);
            partitions_y[drive_count*4+p] = y;

            printk("    Partition %d\n", p);
        }

        drive_count++;
    }

    // Set cursor to drive 0
    CURSOR = 0;

    uint8_t color;

    if (partitions_map[CURSOR]) {
        color= 0xF0;
    } else {
        color = 0x70;
    }

    for (int x = 2; x < 17; x ++) {
        VGA_putcolor(x, partitions_y[CURSOR], color);
    }
}

static void move_cursor(uint8_t new_cursor) { 
    if (new_cursor < 0 || new_cursor >= drive_count*4) {
        return;
    }

    uint8_t color;
    if (partitions_map[CURSOR]) {
        color= 0x0F;
    } else {
        color = 0x07;
    }

    for (int x = 2; x < 17; x ++) {
        VGA_putcolor(x, partitions_y[CURSOR], color);
    }

    if (partitions_map[new_cursor]) {
        color= 0xF0;
    } else {
        color = 0x70;
    }

    for (int x = 2; x < 17; x ++) {
        VGA_putcolor(x, partitions_y[new_cursor], color);
    }

    CURSOR = new_cursor;
} 

SELECTOR_selection SELECTOR_loop() {
    int running = 1;
    while (running) {
        uint8_t status = i686_inb(0x64);
        if (status & 0x01) {
            uint8_t key_code = i686_inb(0x60);
            switch (key_code) {
                case 0x48:
                    move_cursor(CURSOR - 1);
                    break;

                case 0x50:
                    move_cursor(CURSOR + 1);
                    break;

                case 0x1C:
                    running = 0;
                    break;
                
                default:
                    break;
            }
        }
    }

    ATA_disk_t drive = drives_map[CURSOR>>2];
    uint8_t partition = CURSOR % 4;

    return (SELECTOR_selection){drive, partition};
}