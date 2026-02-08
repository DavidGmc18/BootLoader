TARGET_LINKFLAGS += -T linker.ld -nostdlib

HEADERS_C = 	$(shell find . -type f -name "*.h")
SOURCES_C = 	$(shell find . -type f -name "*.c")

HEADERS_ASM = 	$(shell find . -type f -name "*.inc")
SOURCES_ASM = 	$(shell find . -type f -name "*.asm")

OBJECTS_C = 	$(patsubst %.c, $(BUILD_DIR)/test/c/%.o, $(SOURCES_C))
OBJECTS_ASM = 	$(patsubst %.asm, $(BUILD_DIR)/test/asm/%.o, $(SOURCES_ASM))

.PHONY: all test clean always

all: test

test: $(BUILD_DIR)/test.bin

$(BUILD_DIR)/test.bin: $(OBJECTS_ASM) $(OBJECTS_C) $(VGA_DRIVER) $(ATA_DRIVER) $(ARCH_i686_LIB) $(UTIL_LIB)
	@$(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(BUILD_DIR)/test.map -o $@ $^ $(TARGET_LIBS) $(VGA_DRIVER) $(ATA_DRIVER) $(ARCH_i686_LIB) $(UTIL_LIB)
	@echo "--> Created:  test.bin"

$(BUILD_DIR)/test/c/%.o: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/test/asm/%.o: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

clean:
	@rm -f $(BUILD_DIR)/test.bin