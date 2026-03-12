.PHONY: all

all: stage0

stage0: $(BUILD_DIR)/stage0.bin

$(STAGE0_BIN): boot.asm
	@mkdir -p $(@D)
	@nasm $< -f bin -o $@
	@echo "--> Created  stage0.bin"