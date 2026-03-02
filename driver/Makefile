.PHONY: all drivers clean always

all: drivers

drivers:
	@$(MAKE) -C $(SOURCE_DIR)/driver/vga BUILD_DIR=$(abspath $(BUILD_DIR))
	@$(MAKE) -C $(SOURCE_DIR)/driver/ata BUILD_DIR=$(abspath $(BUILD_DIR))
	@$(MAKE) -C $(SOURCE_DIR)/driver/mbr BUILD_DIR=$(abspath $(BUILD_DIR))
	@$(MAKE) -C $(SOURCE_DIR)/driver/pci BUILD_DIR=$(abspath $(BUILD_DIR))