export SOURCE_DIR = $(abspath .)
export BUILD_DIR = $(abspath build)

BUILD_ON_RAM = 1
BUILD_ON_RAM_SIZE = 32M

DISK_IMAGE_BS = 512
DISK_IMAGE_SECTORS = 256 # 128K

BOOTLOADER_BIN = $(BUILD_DIR)/BootLoader-MBR-i686.bin
TEST_IMAGE = $(BUILD_DIR)/test.dd
export STAGE0_BIN = $(BUILD_DIR)/stage0.bin
export STAGE1_BIN = $(BUILD_DIR)/stage1.bin

export TARGET_ASMFLAGS = -f elf
export TARGET_CFLAGS = -std=c99 -g -Os -DDEBUG -ffreestanding -nostdlib -I. -I$(SOURCE_DIR) -I$(SOURCE_DIR)/include

export ARCH_i686_LIB = $(BUILD_DIR)/arch/i686.a
export UTIL_LIB = $(BUILD_DIR)/util.a

export VGA_DRIVER = $(BUILD_DIR)/driver/vga.a
export PCI_DRIVER = $(BUILD_DIR)/driver/pci.a
export AHCI_DRIVER = $(BUILD_DIR)/driver/ahci.a