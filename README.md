# BootLoader

A simple MBR bootloader living in the first 32 sectors of a hard drive.

### Features:
- Occupies 32 sectors
- Uses ATA to scan for available drives
- Reads MBR for valid drives
- Boot partition selector
- Loads the first 32 sectors of the selected partition (this is where OS or its bootloader should live)