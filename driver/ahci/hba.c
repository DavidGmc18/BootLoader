#include "hba.h"
#include "fis.h"
#include <util/memory.h>

void HBA_set_cmd_header(HBA_CMD_HEADER* cmdheader, uint8_t write, uint16_t prdtl) {
	cmdheader->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
    cmdheader->w = write; // Read = 0; Write = 1
    cmdheader->prdtl = prdtl;
}

void HBA_set_cmd_table(HBA_CMD_TBL* cmdtbl, uint16_t prdt_entry_no, uint16_t* buffer, uint32_t count, uint8_t interrupt_on_completion) {
	memset(cmdtbl, 0, sizeof(HBA_CMD_TBL));

	// Physical Region Descriptor Table
	HBA_PRDT_ENTRY* prdt_entry = &cmdtbl->prdt_entry[prdt_entry_no];
    prdt_entry->dba = (uint32_t)(uintptr_t)buffer;
    prdt_entry->dbau = (uint32_t)((uint64_t)(uintptr_t)buffer >> 32);
    prdt_entry->dbc = ((count * 512) - 1) | 1; // bit 0 always set per spec; byte count (0-indexed)
    prdt_entry->i = interrupt_on_completion;
}