ENTRY(entry)
OUTPUT_FORMAT("binary")
phys = 0x00020000;

SECTIONS {
    . = 0;
    .boot_sector : {
        KEEP(*(.boot_sector))
    }

    . = phys;
    .entry phys : AT(SIZEOF(.boot_sector)) {
        __entry_start = .;
        *(.entry)
    }

    .text : AT(SIZEOF(.boot_sector) + SIZEOF(.entry)) {
        __text_start = .;
        *(.text*)
    }

    .data : {
        *(.data)
    }

    .rodata : {
        *(.rodata)
    }

    .bss : {
        __bss_start = .;
        *(.bss)
    }
    
    __end = .;
}