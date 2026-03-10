export SOURCE_DIR = $(abspath .)
export BUILD_DIR = $(abspath build)

export CFLAGS = -std=c99 -g
export ASMFLAGS =
export LINKFLAGS =
export LIBS =

export TARGET = $(SOURCE_DIR)/toolchain/i686-elf/bin/i686-elf
export TARGET_ASMFLAGS = -f elf
export TARGET_CFLAGS = -std=c99 -g -Os -DDEBUG -ffreestanding -nostdlib -I. -I$(SOURCE_DIR) -I$(SOURCE_DIR)/include
export TARGET_LINKFLAGS =
export TARGET_LIBS = -lgcc

export ARCH_i686_LIB = $(BUILD_DIR)/arch/i686.a
export UTIL_LIB = $(BUILD_DIR)/util.a

export VGA_DRIVER = $(BUILD_DIR)/driver/vga.a
export PCI_DRIVER = $(BUILD_DIR)/driver/pci.a
export AHCI_DRIVER = $(BUILD_DIR)/driver/ahci.a