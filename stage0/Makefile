.PHONY: all clean

all: stage0

stage0: $(BUILD_DIR)/stage0.bin

$(BUILD_DIR)/stage0.bin: boot.asm
	@mkdir -p $(@D)
	@nasm $< -f bin -o $@
	@echo "--> Created  stage0.bin"

clean:
	@rm -f $(BUILD_DIR)/stage0.bin