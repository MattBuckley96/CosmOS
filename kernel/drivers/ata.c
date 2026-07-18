#include "ata.h"
#include "cpu/io.h"
#include "util.h"
#include "console.h"

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

u32 lba_count;

static void delay(void) {
    for (u32 i = 0; i < 4; i++) {
        inb(REG_ALT_STATUS);
    }
}

static void wait_bsy_rdy(void) {
    delay();
    while (inb(REG_STATUS) & STATUS_BSY);
    while (!(inb(REG_STATUS) & STATUS_RDY));
}

static void wait_drq(void) {
    delay();
    while (!(inb(REG_STATUS) & STATUS_DRQ)); 
}

static void cache_flush(void) {
    wait_bsy_rdy();
    outb(REG_CMD, CMD_CACHE_FLUSH);
}

static void ata_error(const u8* msg) {
    kprintf("%F[%Fata%F] ", VGA_WHITE, VGA_LIGHT_BLUE, VGA_WHITE);
    kprintf("%F%s%F\n", VGA_LIGHT_RED, msg, VGA_GRAY);
}

void ata_init(void) {
    wait_bsy_rdy();
    outb(REG_DRIVE, 0xA0);

    wait_bsy_rdy();
    outb(REG_CMD, CMD_CACHE_FLUSH);

    wait_bsy_rdy();
    outb(REG_SECTOR_COUNT, 0);
    outb(REG_LBA_LO, 0);
    outb(REG_LBA_MID, 0);
    outb(REG_LBA_HI, 0);
    outb(REG_CMD, CMD_IDENTIFY);

    u16 buf[256];
    wait_drq();
    insw(REG_DATA, buf, 256);

    if (buf[83] & (1 << 10)) {
        lba_count = ((u32)buf[61] << 16) |
                        ((u32)buf[60]);
    }

    ata_read_sectors(0, &buf, 1);
    if (buf[255] != 0xAA55) {
        ata_error("drive not bootable!");
        return;
    }
}

void ata_read_sectors(u32 lba, void* out, u8 count) {
    wait_bsy_rdy();
    outb(REG_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));

    wait_bsy_rdy();
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

void ata_write_sectors(u32 lba, void* in, u8 count) {
    wait_bsy_rdy();
    outb(REG_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));

    wait_bsy_rdy();
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

u32 ata_get_lba_count(void) {
    return lba_count;
}
