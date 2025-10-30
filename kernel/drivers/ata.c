#include "ata.h"
#include "cpu/io.h"

///////////////////////////////////////////////

#define PRIMARY_IO   0x1F0

#define REG_DATA     (PRIMARY_IO + 0)
#define REG_ERR      (PRIMARY_IO + 1)
#define REG_FEATURES (PRIMARY_IO + 1)
#define REG_SECTORS  (PRIMARY_IO + 2)
#define REG_LBA_LO   (PRIMARY_IO + 3)
#define REG_LBA_MID  (PRIMARY_IO + 4)
#define REG_LBA_HI   (PRIMARY_IO + 5)
#define REG_DRIVE    (PRIMARY_IO + 6)
#define REG_COMMAND  (PRIMARY_IO + 7)
#define REG_STATUS   (PRIMARY_IO + 7)

#define STATUS_ERR   0x01
#define STATUS_IDX   0x02
#define STATUS_CORR  0x04
#define STATUS_DRQ   0x08
#define STATUS_SRV   0x10
#define STATUS_DF    0x20
#define STATUS_RDY   0x40
#define STATUS_BSY   0x80

#define DRIVE_MASTER 0xE0
#define DRIVE_SLAVE  0xF0

#define COMMAND_READ 0x20

#define SECTOR_SIZE      512
#define WORDS_PER_SECTOR (SECTOR_SIZE / 2)

///////////////////////////////////////////////

void delay(void)
{
    for (int i = 0; i < 15; i++)
    {
        inb(REG_STATUS);
    }
}

///////////////////////////////////////////////

int ata_read(u32 lba, u8 count, void* out)
{
    int err;

    while (inb(REG_STATUS) & STATUS_BSY);

    outb(REG_DRIVE, DRIVE_MASTER | ((lba >> 24) & 0x0F));
    delay();
    while (!(inb(REG_STATUS) & STATUS_RDY));

    outb(REG_SECTORS, count);
    outb(REG_LBA_LO, (u8)(lba));
    outb(REG_LBA_MID, (u8)(lba >> 8));
    outb(REG_LBA_HI, (u8)(lba >> 16));
    outb(REG_COMMAND, COMMAND_READ);

    u16* buf = (u16*)out;

    for (int i = 0; i < count; i++)
    {
        while (!(inb(REG_STATUS) & STATUS_DRQ));

        for (int j = 0; j < WORDS_PER_SECTOR; j++)
        {
            buf[j] = inw(REG_DATA);
        }

        buf += WORDS_PER_SECTOR;
    }

    return err;
}
