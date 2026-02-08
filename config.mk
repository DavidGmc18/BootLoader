export SOURCE_DIR = $(abspath .)
export BUILD_DIR = $(abspath build)

export CFLAGS = -std=c99 -g
export ASMFLAGS =
export CC = gcc
export CXX = g++
export LD = gcc
export ASM = nasm
export LINKFLAGS =
export LIBS =

export TARGET = $(SOURCE_DIR)/toolchain/i686-elf/bin/i686-elf
export TARGET_ASM = nasm
export TARGET_ASMFLAGS = -f elf
export TARGET_CFLAGS = -std=c99 -g -Os -DDEBUG -ffreestanding -nostdlib -I. -I$(SOURCE_DIR)
export TARGET_CC = $(TARGET)-gcc
export TARGET_CXX = $(TARGET)-g++
export TARGET_LD = $(TARGET)-gcc
export TARGET_AR = $(TARGET)-ar
export TARGET_LINKFLAGS =
export TARGET_LIBS = -lgcc

BINUTILS_VERSION = 2.45.1
BINUTILS_URL = https://ftp.gnu.org/gnu/binutils/binutils-$(BINUTILS_VERSION).tar.xz

GCC_VERSION = 15.2.0
GCC_URL = https://ftp.gnu.org/gnu/gcc/gcc-$(GCC_VERSION)/gcc-$(GCC_VERSION).tar.xz

export ARCH_i686_LIB = $(BUILD_DIR)/arch/i686.a
export UTIL_LIB = $(BUILD_DIR)/util.a

export VGA_DRIVER = $(BUILD_DIR)/driver/vga.a
export ATA_DRIVER = $(BUILD_DIR)/driver/ata.a
export MBR_DRIVER = $(BUILD_DIR)/driver/mbr.a