#include "ata.h"
#include "io.h"
#include "util.h"

#define IO_BASE 0x1F0
#define CTRL_BASE 0x3F6

#define REG_DATA (IO_BASE + 0)
#define REG_ERR (IO_BASE + 1)
#define REG_FEATURES (IO_BASE + 1)
#define REG_SECTOR_COUNT (IO_BASE + 2)
#define REG_LBA_LO (IO_BASE + 3)
#define REG_LBA_MID (IO_BASE + 4)
#define REG_LBA_HI (IO_BASE + 5)
#define REG_DRIVE (IO_BASE + 6)
#define REG_STATUS (IO_BASE + 7)
#define REG_CMD (IO_BASE + 7)

#define REG_ALT_STATUS (CTRL_BASE + 0)
#define REG_DEV_CTRL (CTRL_BASE + 0)
#define REG_DRIVE_ADDR (CTRL_BASE + 1)

#define ERR_AMNF 0x01
#define ERR_TKZNF 0x02
#define ERR_ABRT 0x04
#define ERR_MCR 0x08
#define ERR_IDNF 0x10
#define ERR_MC 0x20
#define ERR_UNC 0x40
#define ERR_BBK 0x80

#define STATUS_ERR 0x01
#define STATUS_IDX 0x02
#define STATUS_CORR 0x04
#define STATUS_DRQ 0x08
#define STATUS_SRV 0x10
#define STATUS_DF 0x20
#define STATUS_RDY 0x40
#define STATUS_BSY 0x80

#define CMD_IDENTIFY 0xEC
#define CMD_CACHE_FLUSH 0xE7
#define CMD_READ_SECTORS 0x20
#define CMD_WRITE_SECTORS 0x30

typedef struct partition {
    u8 status;
    u8 head_start;
    u8 sect_start;
    u8 cyl_start;
    u8 sys_id;
    u8 head_end;
    u8 sect_end;
    u8 cyl_end;
    u32 lba;
    u32 sect_count;
} PACKED partition_t;

typedef struct mbr {
    u8 code[446];
    partition_t partitions[4];
    u16 boot_sig;
} PACKED mbr_t;

void delay(void)
{
    for (int i = 0; i < 4; i++) {
        inb(REG_ALT_STATUS);
    }
}

void wait_bsy_rdy(void)
{
    delay();
    while (inb(REG_STATUS) & STATUS_BSY);
    while (!(inb(REG_STATUS) & STATUS_RDY));
}

void wait_drq(void)
{
    delay();
    while (!(inb(REG_STATUS) & STATUS_DRQ));
}

void cache_flush(void)
{
    outb(REG_CMD, CMD_CACHE_FLUSH);
    wait_bsy_rdy();
}

void ata_init(void)
{
    outb(REG_DRIVE, 0xA0);
    delay();

    outb(REG_SECTOR_COUNT, 0);
    outb(REG_LBA_LO, 0);
    outb(REG_LBA_MID, 0);
    outb(REG_LBA_HI, 0);
    outb(REG_CMD, CMD_IDENTIFY);

    u16 indentify[256];
    insw(REG_DATA, indentify, 256);

    if (inb(REG_STATUS) == 0) {
        kprintf("[WOMP WOMP] ATA: no drive found!\n");
        return;
    }

    if (inb(REG_LBA_MID) || inb(REG_LBA_HI)) {
        kprintf("[WOMP WOMP] ATA: drive not ATA!\n");
        return;
    }

    wait_bsy_rdy();

    mbr_t mbr;
    ata_read_sectors(0, &mbr, 1);

    if (mbr.boot_sig != 0xAA55) {
        kprintf("ATA: boot signiature doesnt match 0xAA55!\n");
        return;
    }

    for (int i = 0; i < 4; i++) {
        partition_t* part = &mbr.partitions[i];

        kprintf("partition %i: ", i);
        kprintf("lba: %i, ", part->lba);
        kprintf("sect_count: %i\n", part->sect_count);
    }
}

void ata_read_sectors(u32 lba, void* out, u8 count)
{
    wait_bsy_rdy();

    outb(REG_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(REG_SECTOR_COUNT, count);
    outb(REG_LBA_LO, (u8)lba);
    outb(REG_LBA_MID, (u8)(lba >> 8));
    outb(REG_LBA_HI, (u8)(lba >> 16));
    outb(REG_CMD, CMD_READ_SECTORS);

    u16* buf = (u16*)out;

    for (u8 i = 0; i < count; i++) {
        wait_drq();

        insw(REG_DATA, buf, 256);
        buf += 256;
    }
}

void ata_write_sectors(u32 lba, void* in, u8 count)
{
    wait_bsy_rdy();

    outb(REG_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(REG_SECTOR_COUNT, count);
    outb(REG_LBA_LO, (u8)lba);
    outb(REG_LBA_MID, (u8)(lba >> 8));
    outb(REG_LBA_HI, (u8)(lba >> 16));
    outb(REG_CMD, CMD_WRITE_SECTORS);

    u16* buf = (u16*)in;

    for (u8 i = 0; i < count; i++) {
        wait_drq();

        for (u16 j = 0; j < 256; j++) {
            outw(REG_DATA, buf[j]);
        }
        buf += 256;
    }

    cache_flush();
}