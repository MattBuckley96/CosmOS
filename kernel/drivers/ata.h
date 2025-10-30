#ifndef ATA_H
#define ATA_H

#include "types.h"

///////////////////////////////////////////////

enum
{
    ATA_ERR_NONE = 0,
    ATA_ERR_NO_DRIVES = -1,
};

///////////////////////////////////////////////

int ata_read(u32 lba, u8 count, void* out);
int ata_write(u32 lba, u8 count, void* in);

#endif
