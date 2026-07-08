#pragma once

#include "types.h"

void ata_init(void);
void ata_read_sectors(u32 lba, void* out, u8 count);
void ata_write_sectors(u32 lba, void* in, u8 count);
