#ifndef ATA_H
#define ATA_H

#include "types.h"

///////////////////////////////////////////////

enum
{
    ATA_ERR_NONE      =  0,
    ATA_ERR_NO_DRIVES = -1,
    ATA_ERR_WRITE     = -2,
    ATA_ERR_READ      = -3,
};

///////////////////////////////////////////////

int ata_init(void);
int ata_read(u32 lba, void* out, u8 count);
int ata_write(u32 lba, void* in, u8 count);

#endif
