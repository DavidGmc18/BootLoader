#include "E820.h"

int __attribute__((cdecl)) i686_E820GetNextBlock(E820_MemoryBlock* block, uint32_t* continuationId);

#define MAX_BLOCKS 256

static E820_MemoryBlock mem_blocks[MAX_BLOCKS];
static uint32_t block_count;

void E820_detect(E820_MemoryInfo* mem_info) {
    E820_MemoryBlock block;
    uint32_t continuation = 0;

    block_count = 0;
    int ret = i686_E820GetNextBlock(&block, &continuation);

    while (ret > 0 && continuation != 0 && block_count < MAX_BLOCKS) {
        mem_blocks[block_count].base = block.base;
        mem_blocks[block_count].length = block.length;
        mem_blocks[block_count].type = block.type;
        mem_blocks[block_count].ACPI = block.ACPI;

        block_count++;

        ret = i686_E820GetNextBlock(&block, &continuation);
    }

    mem_info->block_count = block_count;
    mem_info->blocks = mem_blocks;
}