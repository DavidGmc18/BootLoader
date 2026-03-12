ENTRY(entry)
OUTPUT_FORMAT("binary")
phys = 0x0500;

MEMORY {
    RAM (rwx) :     ORIGIN = 0x00500, LENGTH = 0x07B00
    STACK (rw) :    ORIGIN = 0x08000, LENGTH = 0x02000
    AHCI (rw) :     ORIGIN = 0x10000, LENGTH = 0x04800
    BOOT_INFO (rw): ORIGIN = 0x14800, LENGTH = 0x02800
    OS (rwx):       ORIGIN = 0x20000, LENGTH = 0x60000
}

SECTIONS {
    . = phys;

    .entry : {
        __entry_start = .;
        *(.entry)
    } > RAM

    .text : {
        __text_start = .;
        *(.text)
    } > RAM

    .data : {
        __data_start = .;
        *(.data)
    } > RAM

    .rodata : {
        __rodata_start = .;
        *(.rodata)
    } > RAM

    .bss : {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }  > RAM

    .ahci_mem (NOLOAD) : {
        __ahci_start = .;
        *(.ahci_mem)
        __ahci_end = .;
    } > AHCI
    ASSERT(__ahci_end <= ORIGIN(AHCI) + LENGTH(AHCI), "ERROR: .ahci_mem overflows AHCI memory region!")

    .boot_info (NOLOAD) : {
        __boot_info_start = .;
        *(.boot_info)
        __boot_info_end = .;
    } > BOOT_INFO

    .os (NOLOAD) : {
        __os_start = .;
        *(.os)
    } > OS

    __stack_top = ORIGIN(STACK) + LENGTH(STACK);
    __os_size = LENGTH(OS);
}