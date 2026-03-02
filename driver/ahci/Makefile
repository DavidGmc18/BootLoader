HEADERS_C = 	$(shell find . -type f -name "*.h")
SOURCES_C = 	$(shell find . -type f -name "*.c")

HEADERS_ASM = 	$(shell find . -type f -name "*.inc")
SOURCES_ASM = 	$(shell find . -type f -name "*.asm")

OBJECTS_C = 	$(patsubst %.c, $(BUILD_DIR)/driver/ahci/c/%.o, $(SOURCES_C))
OBJECTS_ASM = 	$(patsubst %.asm, $(BUILD_DIR)/driver/ahci/asm/%.o, $(SOURCES_ASM))

.PHONY: all ahci clean always

all: ahci

ahci: $(AHCI_DRIVER)

$(AHCI_DRIVER): $(OBJECTS_ASM) $(OBJECTS_C)
	@mkdir -p $(@D)
	@i686-elf-ar rcs $@ $^
	@echo "--> Created:  ahci.a"

$(BUILD_DIR)/driver/ahci/c/%.o: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@i686-elf-gcc $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/driver/ahci/asm/%.o: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@nasm $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

clean:
	rm -f $(AHCI_DRIVER)