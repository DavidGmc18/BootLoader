ENTRY(entry)
OUTPUT_FORMAT("binary")

MEMORY {
    BOOTLOADER (!rwx) : ORIGIN = 0x00000500, LENGTH = 0x00020000
    RAM (rwx) :         ORIGIN = 0x00020000, LENGTH = 0x00080000
}

SECTIONS {
    .vbr : AT(0x0000) {
        KEEP(*(.vbr))
    }
    ASSERT(SIZEOF(.vbr) <= 512, "ERROR: .vbr exceeds 512 bytes!")

    .text ORIGIN(RAM) : AT(0x0200) {
        *(.entry)
        *(.text)
    } > RAM

    .data : {
        *(.data)
    } > RAM

    .rodata : {
        *(.rodata)
    } > RAM

    .bss : {
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    } > RAM
}