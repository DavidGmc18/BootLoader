ENTRY(entry)
OUTPUT_FORMAT("binary")
phys = 0x0500;

MEMORY {
    RAM (rwx) :     ORIGIN = 0x0500, LENGTH = 0x7500
    STACK (rw) :    ORIGIN = 0x8000, LENGTH = 0x2000
    AHCI (rw) :     ORIGIN = 0x10000, LENGTH = 0x4800
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
    }  > RAM

    .ahci_mem (NOLOAD) : {
        __ahci_start = .;
        *(.ahci_mem)
        __ahci_end = .;
    } > AHCI
    ASSERT(__ahci_end <= ORIGIN(AHCI) + LENGTH(AHCI), "ERROR: .ahci_mem overflows AHCI memory region!")
    
    __end = .;

    __stack_top = ORIGIN(STACK) + LENGTH(STACK);
}