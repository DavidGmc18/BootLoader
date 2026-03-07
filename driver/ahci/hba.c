#include "hba.h"
#include "fis.h"

void HBA_set_cmd_header(HBA_CMD_HEADER* cmdheader, uint8_t write, uint16_t prdtl) {
	cmdheader->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
    cmdheader->w = write; // Read = 0; Write = 1
    cmdheader->prdtl = prdtl;
}

void HBA_set_cmd_table(HBA_CMD_TBL* cmdtbl, uint16_t prdt_entry_no, uint16_t* buffer, uint32_t count, uint8_t interrupt_on_completion) {
	_Static_assert(sizeof(HBA_CMD_TBL) % sizeof(uint32_t) == 0);
	uint32_t* u32ptr = (uint32_t*)cmdtbl;
	for (int i = 0; i < sizeof(HBA_CMD_TBL) / sizeof(uint32_t); i++) {
		u32ptr[i] = 0;
	}

	// Physical Region Descriptor Table
	HBA_PRDT_ENTRY* prdt_entry = &cmdtbl->prdt_entry[prdt_entry_no];
    prdt_entry->dba = (uint32_t)(uintptr_t)buffer;
    prdt_entry->dbau = (uint64_t)(uintptr_t)buffer << 32;
    prdt_entry->dbc = (count * 512) - 1; // Byte count (0-indexed)
    prdt_entry->i = interrupt_on_completion;
}