include config.mk

.PHONY: all bon disk_image stage0 clean always

all: bin disk_image

bin: $(BUILD_DIR)/BootLoader-MBR-i686.bin

$(BUILD_DIR)/BootLoader-MBR-i686.bin: deps stage0 stage1
	@dd if=/dev/zero of=$@ bs=1 count=16384 >/dev/null
	@dd if=$(BUILD_DIR)/stage0.bin of=$@ conv=notrunc >/dev/null
	@dd if=$(BUILD_DIR)/stage1.bin of=$@ bs=512 seek=1 conv=notrunc >/dev/null
	@echo "--> Done: $@ (16 KB)"

# Used for testing
disk_image: $(BUILD_DIR)/diskimage.dd

$(BUILD_DIR)/diskimage.dd: $(BUILD_DIR)/BootLoader-MBR-i686.bin test
	@dd if=/dev/zero of=$@ bs=512 count=8192 >/dev/null

	@dd if=$(BUILD_DIR)/BootLoader-MBR-i686.bin of=$@ conv=notrunc >/dev/null

	@dd if=$(BUILD_DIR)/test.bin of=$@ bs=512 seek=32 conv=notrunc >/dev/null
	@echo '80' | xxd -r -p | dd of=build/diskimage.dd bs=1 seek=446 conv=notrunc
	@echo '20' /| xxd -r -p | dd of=build/diskimage.dd bs=1 seek=454 conv=notrunc

	@echo "--> Created: " $@

deps:
	@$(MAKE) -C $(SOURCE_DIR)/arch BUILD_DIR=$(abspath $(BUILD_DIR))
	@$(MAKE) -C $(SOURCE_DIR)/util BUILD_DIR=$(abspath $(BUILD_DIR))
	@$(MAKE) -C $(SOURCE_DIR)/driver BUILD_DIR=$(abspath $(BUILD_DIR))

stage0: $(BUILD_DIR)/stage0.bin

$(BUILD_DIR)/stage0.bin: always
	@$(MAKE) -C $(SOURCE_DIR)/stage0 BUILD_DIR=$(abspath $(BUILD_DIR))


stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin: always
	@$(MAKE) -C $(SOURCE_DIR)/stage1 BUILD_DIR=$(abspath $(BUILD_DIR))

test: $(BUILD_DIR)/test.bin

$(BUILD_DIR)/test.bin: always
	@$(MAKE) -C $(SOURCE_DIR)/test BUILD_DIR=$(abspath $(BUILD_DIR))

always:
	@mkdir -p $(BUILD_DIR)

clean:
	@rm -rf $(BUILD_DIR)/*