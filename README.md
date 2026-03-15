# BootLoader

A minimal x86 bootloader for bare-metal OS development. Handles drive detection,
partition selection, and hands off execution to your boot code in 32-bit protected mode.

## Features

- Occupies the first 32 sectors (16KiB) of the drive
- AHCI disk driver with MBR partition tables
- Partition selector — shows bootable partitions, navigate with arrow keys, confirm with Enter
- BIOS E820 memory map detection
- Handles invalid BIOS drive numbers less than `0x80` by defaulting to `0x80`
- Tested on real hardware

## Memory Layout
```
0x00500 - 0x20000  Bootloader
0x08000 - 0x0A000  Stack (inside bootloader memory, grows downward from 0xA000)
0x20000 - 0x80000  Boot code load area (384KiB hard limit)
```

Bootloader memory can be freely reused once you no longer need any variables or services it provides. Note that the stack lives in this region — if you haven't set up your own stack. The bootloader stack is 8KiB. By the time your boot code is called, a small amount has already been consumed by the bootloader itself.

## Boot Code

Your boot code is loaded at `0x20000` according to the VBR parameters and handed off in 32-bit protected mode. The 384KiB range (`0x20000`–`0x80000`) is the hard limit enforced by the bootloader, but your program can freely use any available memory beyond that — keep the [x86 memory map](https://wiki.osdev.org/Memory_Map_(x86)) in mind.

When your entry point returns, the bootloader will attempt an ACPI power-off. If ACPI power-off fails the screen will clear and display a manual power-off prompt.

## Volume Boot Record (VBR)

The first sector of your partition must contain a valid VBR. The full sector struct is defined in `include/bl/boot.h` as `BL_BootSector`, which already places the VBR at the correct offset. The VBR struct and signature constant are defined in `include/bl/types.h`.

| Field          | Type       | Description                                  |
|----------------|------------|----------------------------------------------|
| `signature`    | `uint32_t` | Must match `BL_VBR_SIGNATURE` from `types.h` |
| `boot_lba`     | `uint32_t` | Sector offset relative to partition start    |
| `boot_sectors` | `uint16_t` | Number of sectors to load                    |
| `entry_offset` | `uint32_t` | Byte offset from load address to entry point |
| `name`         | `char[50]` | Name (currently unused)                      |

## Boot Services

Your entry point is called with the following signature:
```c
void __attribute__((cdecl)) entry(BL_BootInfo* boot_info, BL_BootServices* boot_services);
```

**`BL_BootInfo`** — information about the boot environment:
- Selected disk and partition (AHCI base address, port, LBA, sector count)
- Drive name string
- E820 memory map

**`BL_BootServices`** — services provided by the bootloader:
- `printk` — formatted text output to VGA
- `disk_read` — AHCI sector read

## Usage

See `include/bl/` for all type definitions and `test/` for a minimal working example
demonstrating a valid VBR, entry point, and boot services usage.