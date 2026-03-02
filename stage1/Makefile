TARGET_LINKFLAGS += -T linker.ld -nostdlib

HEADERS_C = 	$(shell find . -type f -name "*.h")
SOURCES_C = 	$(shell find . -type f -name "*.c")

HEADERS_ASM = 	$(shell find . -type f -name "*.inc")
SOURCES_ASM = 	$(shell find . -type f -name "*.asm")

OBJECTS_C = 	$(patsubst %.c, $(BUILD_DIR)/stage1/c/%.o, $(SOURCES_C))
OBJECTS_ASM = 	$(patsubst %.asm, $(BUILD_DIR)/stage1/asm/%.o, $(SOURCES_ASM))

.PHONY: all stage1 clean always

all: stage1

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin: $(OBJECTS_ASM) $(OBJECTS_C) $(MBR_DRIVER) $(ATA_DRIVER) $(VGA_DRIVER) $(ARCH_i686_LIB) $(UTIL_LIB) $(PCI_DRIVER)
	@mkdir -p $(@D)
	@i686-elf-gcc $(TARGET_LINKFLAGS) -Wl,-Map=$(BUILD_DIR)/stage1.map -o $@ $^ $(TARGET_LIBS)
	@echo "--> Created  stage1.bin"

$(BUILD_DIR)/stage1/c/%.o: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@i686-elf-gcc $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/stage1/asm/%.o: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@nasm $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

clean:
	@rm -f $(BUILD_DIR)/stage1.bin