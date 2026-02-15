ENTRY(entry)
OUTPUT_FORMAT("binary")
phys = 0x00000500;

MEMORY {
    RAM (rwx) : ORIGIN = 0x0500, LENGTH = 0x7700
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
    
    __end = .;
}