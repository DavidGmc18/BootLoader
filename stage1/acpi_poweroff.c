#include "acpi_poweroff.h"
#include <arch/i686/i686.h>
#include <stddef.h>
#include <util/memory.h>
#include <util/printk.h>
#include <driver/vga/vga_text.h>

typedef struct __attribute__((packed)) {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision; // < 2 = ACPI 1.0 (RSDT only), >= 2 = ACPI 2.0+ (XSDT)
    uint32_t rsdt_address;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t ext_checksum;
    uint8_t reserved[3];
} rsdp_t;

typedef struct __attribute__((packed)) {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} acpi_header_t;

typedef struct __attribute__((packed)) {
    acpi_header_t header;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t reserved0;
    uint8_t preferred_pm_profile;
    uint16_t sci_interrupt;
    uint32_t smi_cmd;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_cnt;
    uint32_t pm1a_evt_blk;
    uint32_t pm1b_evt_blk;
    uint32_t pm1a_cnt_blk;
    uint32_t pm1b_cnt_blk;
} fadt_t;

static rsdp_t *find_rsdp(void) {
    // (a) first 1KB of EBDA
    uint8_t *ebda = (uint8_t *)(uintptr_t)((*(uint16_t *)0x40E) << 4);
    for (int i = 0; i < 1024; i += 16)
        if (!memcmp(ebda + i, "RSD PTR ", 8)) return (rsdp_t *)(ebda + i);

    // (b) BIOS region 0xE0000–0xFFFFF
    for (uintptr_t a = 0xE0000; a < 0x100000; a += 16)
        if (!memcmp((char *)a, "RSD PTR ", 8)) return (rsdp_t *)a;

    return NULL;
}

static fadt_t *find_fadt(rsdp_t *rsdp) {
    if (rsdp->revision >= 2 && rsdp->xsdt_address) {
        acpi_header_t *h = (acpi_header_t *)(uintptr_t)rsdp->xsdt_address;
        uint64_t *e = (uint64_t *)(h + 1);
        uint32_t n = (h->length - sizeof(acpi_header_t)) / 8;
        for (uint32_t i = 0; i < n; i++)
            if (!memcmp(((acpi_header_t *)(uintptr_t)e[i])->signature, "FACP", 4))
                return (fadt_t *)(uintptr_t)e[i];
    } else {
        acpi_header_t *h = (acpi_header_t *)(uintptr_t)rsdp->rsdt_address;
        uint32_t *e = (uint32_t *)(h + 1);
        uint32_t n = (h->length - sizeof(acpi_header_t)) / 4;
        for (uint32_t i = 0; i < n; i++)
            if (!memcmp(((acpi_header_t *)(uintptr_t)e[i])->signature, "FACP", 4))
                return (fadt_t *)(uintptr_t)e[i];
    }
    return NULL;
}

static int find_s5(fadt_t *fadt, uint8_t *typa, uint8_t *typb) {
    uint8_t *d = (uint8_t *)(uintptr_t)fadt->dsdt;
    uint32_t len = ((acpi_header_t *)d)->length;

    for (uint32_t i = 0; i < len - 8; i++) {
        if (memcmp(d + i, "_S5_", 4)) continue;

        uint8_t *p = d + i + 4;
        if (*p != 0x12) continue;                   // must be Package opcode
        p++;
        p += 1 + ((*p >> 6) & 3) + 1;              // skip pkg length + element count

        if (*p == 0x0A) p++;
        *typa = *p++ & 7;     // SLP_TYP_A
        if (*p == 0x0A) p++;
        *typb = *p   & 7;     // SLP_TYP_B
        
        return 1;
    }
    return 0;
}

void __attribute__((noreturn))  acpi_power_off(void) {
    rsdp_t *rsdp = find_rsdp();
    if (!rsdp) goto fail;

    fadt_t *fadt = find_fadt(rsdp);
    if (!fadt) goto fail;

    uint8_t typa = 0, typb = 0;
    if (!find_s5(fadt, &typa, &typb)) goto fail;

    uint16_t pm1a = (uint16_t)fadt->pm1a_cnt_blk;

    // Enable ACPI mode if not already active (SCI_EN = bit 0)
    if (!(i686_inw(pm1a) & 1) && fadt->smi_cmd && fadt->acpi_enable) {
        i686_outb((uint16_t)fadt->smi_cmd, fadt->acpi_enable);
        for (int t = 3000; t-- && !(i686_inw(pm1a) & 1);) i686_iowait();
    }

    i686_outw(pm1a, (uint16_t)((typa << 10) | (1 << 13)));
    i686_iowait();

    if (fadt->pm1b_cnt_blk) {
        i686_outw((uint16_t)fadt->pm1b_cnt_blk, (uint16_t)((typb << 10) | (1 << 13)));
        i686_iowait();
    }

fail:
    VGA_clrscr();
    printk("Power-off your computer");
    while (1) i686_hlt();
    __builtin_unreachable();
}