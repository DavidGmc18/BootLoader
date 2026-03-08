#include "E820.h"

int __attribute__((cdecl)) i686_E820GetNextBlock(BL_MemoryBlock* block, uint32_t* continuationId);

int E820_detect(BL_MemoryInfo* mem_info) {
    BL_MemoryBlock block;
    uint32_t continuation = 0;

    mem_info->block_count = 0;

    do {
        if (i686_E820GetNextBlock(&block, &continuation) <= 0)
            break;

        if (mem_info->block_count >= 256)
            return -1;

        mem_info->blocks[mem_info->block_count] = block;
        mem_info->block_count++;
    } while (continuation != 0);

    return 0;
}