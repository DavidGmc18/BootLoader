HEADERS_C = 	$(shell find . -type f -name "*.h")
SOURCES_C = 	$(shell find . -type f -name "*.c")

HEADERS_ASM = 	$(shell find . -type f -name "*.inc")
SOURCES_ASM = 	$(shell find . -type f -name "*.asm")

OBJECTS_C = 	$(patsubst %.c, $(BUILD_DIR)/driver/mbr/c/%.o, $(SOURCES_C))
OBJECTS_ASM = 	$(patsubst %.asm, $(BUILD_DIR)/driver/mbr/asm/%.o, $(SOURCES_ASM))

.PHONY: all mbr clean always

all: mbr

mbr: $(MBR_DRIVER)

$(MBR_DRIVER): $(OBJECTS_ASM) $(OBJECTS_C)
	@mkdir -p $(@D)
	@i686-elf-ar rcs $@ $^
	@echo "--> Created:  mbr.a"

$(BUILD_DIR)/driver/mbr/c/%.o: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@i686-elf-gcc $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/driver/mbr/asm/%.o: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@nasm $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

clean:
	rm -f $(MBR_DRIVER)