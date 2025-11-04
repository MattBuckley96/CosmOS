#include "ata.h"
#include "cpu/io.h"
#include "printf.h"

///////////////////////////////////////////////

#define PRIMARY_IO    0x1F0
#define CONTROL_BASE  0x3F6

#define REG_DATA      (PRIMARY_IO + 0)
#define REG_ERR       (PRIMARY_IO + 1)
#define REG_FEATURES  (PRIMARY_IO + 1)
#define REG_SECTORS   (PRIMARY_IO + 2)
#define REG_LBA_LO    (PRIMARY_IO + 3)
#define REG_LBA_MID   (PRIMARY_IO + 4)
#define REG_LBA_HI    (PRIMARY_IO + 5)
#define REG_DRIVE     (PRIMARY_IO + 6)
#define REG_COMMAND   (PRIMARY_IO + 7)
#define REG_STATUS    (PRIMARY_IO + 7)

#define REG_CONTROL   (CONTROL_BASE + 0)

#define STATUS_ERR    0x01
#define STATUS_IDX    0x02
#define STATUS_CORR   0x04
#define STATUS_DRQ    0x08
#define STATUS_SRV    0x10
#define STATUS_DF     0x20
#define STATUS_RDY    0x40
#define STATUS_BSY    0x80

#define COMMAND_READ     0x20
#define COMMAND_FLUSH    0xE7
#define COMMAND_IDENTIFY 0xEC
#define COMMAND_WRITE    0x30

#define DRIVE_MASTER  0xE0
#define DRIVE_SLAVE   0xF0

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

void wait(void)
{
    delay();
    while (inb(REG_STATUS) & STATUS_BSY);
}

///////////////////////////////////////////////

int ata_init(void)
{
    outb(REG_DRIVE, 0xA0);
    delay();
    outb(REG_CONTROL, 0);

    outb(REG_COMMAND, COMMAND_IDENTIFY);

    if (!(inb(REG_STATUS)))
        return ATA_ERR_NO_DRIVES; 

    while (inb(REG_STATUS) & STATUS_BSY);
    while (!(inb(REG_STATUS) & STATUS_DRQ));

    // HACK: reading first allows writes????
    // what bullshit is that
    ata_read(0, NULL, 1);

    return ATA_ERR_NONE;
}

int ata_read(u32 lba, void* out, u8 count)
{
    outb(REG_DRIVE, DRIVE_MASTER | ((lba >> 24) & 0x0F));
    wait();
    while (!(inb(REG_STATUS) & STATUS_RDY));

    outb(REG_SECTORS, count);
    outb(REG_LBA_LO, (u8)(lba));
    outb(REG_LBA_MID, (u8)(lba >> 8));
    outb(REG_LBA_HI, (u8)(lba >> 16));
    outb(REG_COMMAND, COMMAND_READ);

    wait();

    u16* buf = (u16*)out;

    for (int i = 0; i < count; i++)
    {
        while (!(inb(REG_STATUS) & STATUS_DRQ));

        for (int j = 0; j < WORDS_PER_SECTOR; j++)
        {
            buf[j] = inw(REG_DATA);
        }

        buf += WORDS_PER_SECTOR;

        if (!buf)
            // return no error, just trunc the read
            return ATA_ERR_NONE;
    }

    return ATA_ERR_NONE;
}

int ata_write(u32 lba, void* in, u8 count)
{
    u16* buf = (u16*)in;

    outb(REG_DRIVE, DRIVE_MASTER | ((lba >> 24) & 0x0F));
    wait();
    while (!(inb(REG_STATUS) & STATUS_RDY));

    outb(REG_SECTORS, count);
    outb(REG_LBA_LO, (u8)lba);
    outb(REG_LBA_MID, (u8)(lba >> 8));
    outb(REG_LBA_HI, (u8)(lba >> 16));
    outb(REG_COMMAND, COMMAND_WRITE);

    while (inb(REG_STATUS) & STATUS_BSY);

    for (int i = 0; i < count; i++)
    {
        while (!(inb(REG_STATUS) & STATUS_DRQ));

        for (int j = 0; j < WORDS_PER_SECTOR; j++)
        {
            outw(REG_DATA, buf[j]);
        }
        buf += WORDS_PER_SECTOR;

        if (inb(REG_STATUS) & STATUS_ERR)
            return ATA_ERR_WRITE;
    }

    outb(REG_COMMAND, COMMAND_FLUSH);
    while (inb(REG_STATUS) & STATUS_BSY);

    if (inb(REG_STATUS) & STATUS_ERR)
        return ATA_ERR_WRITE;
    
    return ATA_ERR_NONE;
}
