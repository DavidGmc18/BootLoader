include config.mk

# TODO add size asserts

.PHONY: all bootloader_bin test stage0 stage1 deps build_dir clean

all: bootloader_bin test

bootloader_bin: $(BOOTLOADER_BIN)

$(BOOTLOADER_BIN): deps stage0 stage1
	@dd if=/dev/zero of=$@ bs=1 count=16384 >/dev/null
	@dd if=$(BUILD_DIR)/stage0.bin of=$@ conv=notrunc >/dev/null
	@dd if=$(BUILD_DIR)/stage1.bin of=$@ bs=512 seek=1 conv=notrunc >/dev/null
	@echo "--> Done: $@"

deps:
	@$(MAKE) -C $(SOURCE_DIR)/arch BUILD_DIR=$(BUILD_DIR)
	@$(MAKE) -C $(SOURCE_DIR)/util BUILD_DIR=$(BUILD_DIR)
	@$(MAKE) -C $(SOURCE_DIR)/driver BUILD_DIR=$(BUILD_DIR)

stage0: $(STAGE0_BIN)

$(STAGE0_BIN): build_dir
	@$(MAKE) -C $(SOURCE_DIR)/stage0 BUILD_DIR=$(BUILD_DIR)


stage1: $(STAGE1_BIN)

$(STAGE1_BIN): build_dir
	@$(MAKE) -C $(SOURCE_DIR)/stage1 BUILD_DIR=$(BUILD_DIR)

test: $(TEST_IMAGE)

$(TEST_IMAGE): bootloader_bin build_dir
	@$(MAKE) -C $(SOURCE_DIR)/test BUILD_DIR=$(BUILD_DIR)

	@dd if=/dev/zero of=$@ bs=$(DISK_IMAGE_BS) count=$(DISK_IMAGE_SECTORS) >/dev/null
	@dd if=$(BOOTLOADER_BIN) of=$@ conv=notrunc >/dev/null
	@echo "32,,0,*" | sfdisk $@ 2>/dev/null | grep "Created a new partition"
	@dd if=$(BUILD_DIR)/test.bin of=$@ bs=512 seek=32 conv=notrunc >/dev/null
	@echo "--> Created: " $@

#
# Run
#
run:
	@qemu-system-i386 \
	-debugcon stdio \
	-machine q35,smbus=off \
	-cpu pentium3 \
	-m 128M \
	-nodefaults \
	-device ich9-ahci,id=ahci \
	-drive file=$(TEST_IMAGE),id=disk0,format=raw,if=none \
	-device ide-hd,drive=disk0,bus=ahci.0 \
	-vga std

#
# Util
#
build_dir:
	@mkdir -p $(BUILD_DIR)
	@if [ "$(BUILD_ON_RAM)" = "1" ]; then \
		mountpoint -q $(BUILD_DIR) || sudo mount -t tmpfs -o size=$(BUILD_ON_RAM_SIZE) tmpfs $(BUILD_DIR); \
	else \
		mountpoint -q $(BUILD_DIR) && sudo umount $(BUILD_DIR) || true; \
	fi

clean:
	@test -n "$(BUILD_DIR)" && rm -rf $(BUILD_DIR)/*