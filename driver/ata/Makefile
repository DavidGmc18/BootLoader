HEADERS_C = 	$(shell find . -type f -name "*.h")
SOURCES_C = 	$(shell find . -type f -name "*.c")

HEADERS_ASM = 	$(shell find . -type f -name "*.inc")
SOURCES_ASM = 	$(shell find . -type f -name "*.asm")

OBJECTS_C = 	$(patsubst %.c, $(BUILD_DIR)/driver/ata/c/%.o, $(SOURCES_C))
OBJECTS_ASM = 	$(patsubst %.asm, $(BUILD_DIR)/driver/ata/asm/%.o, $(SOURCES_ASM))

.PHONY: all ata clean always

all: ata

ata: $(ATA_DRIVER)

$(ATA_DRIVER): $(OBJECTS_ASM) $(OBJECTS_C)
	@mkdir -p $(@D)
	@$(TARGET_AR) rcs $@ $^
	@echo "--> Created:  ata.a"

$(BUILD_DIR)/driver/ata/c/%.o: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/driver/ata/asm/%.o: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

clean:
	rm -f $(ATA_DRIVER)