ENTRY(entry)
OUTPUT_FORMAT("binary")
phys = 0x00020000;

SECTIONS {
    . = 0;
    .vbr : {
        KEEP(*(.vbr))
    }

    . = phys;
    .entry phys : AT(SIZEOF(.vbr)) {
        __entry_start = .;
        *(.entry)
    }

    .text : AT(SIZEOF(.vbr) + SIZEOF(.entry)) {
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