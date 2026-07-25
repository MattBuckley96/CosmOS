#include "fs.h"
#include "kernel.h"

#define MAX_INODES 2048
#define MAX_BLOCKS 4096
#define SUPERBLOCK_START 100

u8 fs_buf[512];
u8 inode_bitmap[256];
u8 block_bitmap[512];
superblock_t super;

static bool fs_exists(void) {
    s32 result = memcmp(super.magic, "CosmOSFS", 8);
    return (result == 0);
}

static void fs_error(const u8* msg) {
    kprintf("%F[%Ffs%F] ", VGA_WHITE, VGA_LIGHT_CYAN, VGA_WHITE);
    kprintf("%F%s%F\n", VGA_LIGHT_RED, msg, VGA_GRAY);
}

static u32 fs_create_root(void) {
    u32 id = fs_alloc_inode();
    if (id == 0) {
        return 0;
    }

    inode_t inode = {
        .type = FS_DIR,
    };
    fs_update_inode(id, &inode);

    dentry_t self = {
        .id = id,
        .type = inode.type,
        .name_len = 1
    };
    fs_add_dir_entry(id, &self, ".");

    dentry_t parent = {
        .id = id,
        .type = inode.type,
        .name_len = 2
    };
    fs_add_dir_entry(id, &parent, "..");

    return id;
}

void fs_create(void) {
    memcpy(super.magic, "CosmOSFS", 8);

    u32 start = SUPERBLOCK_START;

    super.inodes_count = MAX_INODES;
    super.free_inodes_count = MAX_INODES;
    super.blocks_count = ata_get_lba_count();
    super.free_blocks_count = MAX_BLOCKS;
    super.block_bitmap = start + 1;
    super.inode_bitmap = start + 2;
    super.inode_table = start + 3;
    super.first_data_block = (super.inode_table + 
            (MAX_INODES / 8));

    fs_update();

    fs_create_root();
}

void fs_print(void) {
    kprintf("%F[%Ffs%F]%F\n", VGA_WHITE, VGA_LIGHT_CYAN, VGA_WHITE,
        VGA_GRAY);

    kprintf("inodes_count: %u\n", super.inodes_count);
    kprintf("free_inodes_count: %u\n", super.free_inodes_count);
    kprintf("blocks_count: %u\n", super.blocks_count);
    kprintf("free_blocks_count: %u\n", super.free_blocks_count);
    kprintf("block_bitmap: %u\n", super.block_bitmap);
    kprintf("inode_bitmap: %u\n", super.inode_bitmap);
    kprintf("inode_table: %u\n", super.inode_table);
    kprintf("first_data_block: %u\n", super.first_data_block);
}

void fs_init(void) {
    mem_zero(fs_buf, sizeof(fs_buf));

    ata_read_sectors(SUPERBLOCK_START, fs_buf, 1);
    memcpy(&super, fs_buf, sizeof(super));

    if (!fs_exists()) {
        fs_error("no filesystem found!");
        return;
    }

    ata_read_sectors(super.block_bitmap, fs_buf, 1);
    memcpy(&block_bitmap, fs_buf, sizeof(block_bitmap));

    ata_read_sectors(super.inode_bitmap, fs_buf, 1);
    memcpy(&inode_bitmap, fs_buf, sizeof(inode_bitmap));
}

void fs_update(void) {
    mem_zero(fs_buf, sizeof(fs_buf));

    memcpy(fs_buf, &super, sizeof(super));
    ata_write_sectors(SUPERBLOCK_START, fs_buf, 1);

    memcpy(fs_buf, &block_bitmap, sizeof(block_bitmap));
    ata_write_sectors(super.block_bitmap, fs_buf, 1);

    memcpy(fs_buf, &inode_bitmap, sizeof(inode_bitmap));
    ata_write_sectors(super.inode_bitmap, fs_buf, 1);
}

void bitmap_set_bit(u8* bitmap, u32 bit) {
    u32 byte = bit / 8;
    u32 index = bit % 8;
    bitmap[byte] |= (1 << index);
}

void bitmap_clear_bit(u8* bitmap, u32 bit) {
    u32 byte = bit / 8;
    u32 index = bit % 8;
    bitmap[byte] &= ~(1 << index);
}

bool bitmap_get_bit(u8* bitmap, u32 bit) {
    u32 byte = bit / 8;
    u32 index = bit % 8;
    return (bitmap[byte] >> (index)) & 1;
}

u32 fs_alloc_inode(void) {
    if (super.free_inodes_count == 0) {
        return 0;
    }

    u32 bits = sizeof(inode_bitmap) * 8;

    for (u32 i = 0; i < bits; i++) {
        if (bitmap_get_bit(inode_bitmap, i)) {
            continue;
        }

        // found free
        bitmap_set_bit(inode_bitmap, i);
        super.free_inodes_count--;
        fs_update();
        return i + 1;
    }

    return 0;
}

void fs_free_inode(u32 id) {
    u32 bit = id - 1;

    inode_t inode;
    fs_get_inode(id, &inode);
    if (inode.type == 0) {
        return;
    }

    for (u32 i = 0; i < inode_get_block_count(id); i++) {
        fs_free_block(inode.direct[i]);
    }

    if (!bitmap_get_bit(inode_bitmap, bit)) {
        return;
    }

    bitmap_clear_bit(inode_bitmap, bit);

    super.free_inodes_count++;
    fs_update();
}

void fs_get_inode(u32 id, inode_t* out) {
    u32 bit = id - 1;

    if (!bitmap_get_bit(inode_bitmap, bit)) {
        mem_zero(out, sizeof(inode_t));
        return;
    }

    u32 inodes_per_block = 512 / sizeof(inode_t);
    u32 offset = bit / inodes_per_block;
    u32 block = super.inode_table + offset;

    ata_read_sectors(block, fs_buf, 1);

    inode_t* table = (inode_t*)fs_buf;

    u32 index = bit % inodes_per_block;
    *out = table[index];
}

void fs_update_inode(u32 id, inode_t* in) {
    u32 bit = id - 1;

    if (!bitmap_get_bit(inode_bitmap, bit)) {
        return;
    }

    u32 inodes_per_block = 512 / sizeof(inode_t);
    u32 offset = bit / inodes_per_block;
    u32 block = super.inode_table + offset;

    ata_read_sectors(block, fs_buf, 1);

    inode_t* table = (inode_t*)fs_buf;

    u32 index = bit % inodes_per_block;
    table[index] = *in;

    ata_write_sectors(block, fs_buf, 1);
}

u32 fs_alloc_block(void) {
    if (super.free_blocks_count == 0) {
        return 0;
    }

    u32 bits = sizeof(block_bitmap) * 8;

    for (u32 i = 0; i < bits; i++) {
        if (bitmap_get_bit(block_bitmap, i)) {
            continue;
        }

        // found free
        bitmap_set_bit(block_bitmap, i);
        super.free_blocks_count--;
        fs_update();
        return super.first_data_block + i;
    }

    return 0;
}

void fs_free_block(u32 block) {
    if (block < SUPERBLOCK_START) {
        return;
    }

    block -= super.first_data_block;

    if (!bitmap_get_bit(block_bitmap, block)) {
        return;
    }

    bitmap_clear_bit(block_bitmap, block);

    super.free_blocks_count++;
    fs_update();
}

// TODO: implement indirect
u32 inode_get_block_count(u32 id) {
    inode_t inode;
    fs_get_inode(id, &inode);
    if (inode.type == 0) {
        return 0;
    }

    u32 count = 0;

    for (u32 i = 0; i < 12; i++) {
        if (inode.direct[i] > 0) {
            count++;
        }
    }

    return count;
}

// TODO: implement indirect
void inode_alloc_blocks(u32 id, u32 count) {
    if (id == 0) {
        return;
    }

    inode_t inode;
    fs_get_inode(id, &inode);
    if (inode.type == 0) {
        return;
    }

    u32 blocks = inode_get_block_count(id);

    // u32 inode_max_blocks = 12 + (512 / 12);
    u32 inode_max_blocks = 12;
    if ((blocks + count) > inode_max_blocks) {
        return;
    }

    for (u32 i = 0; i < 12; i++) {
        if (count == 0) {
            break;
        }

        if (inode.direct[i]) {
            continue;
        }

        u32 block = fs_alloc_block();
        if (block == 0) {
            return;
        }

        inode.direct[i] = block;

        count--;
    }

    fs_update_inode(id, &inode);
}

u32 inode_write(u32 id, void* in, u32 size) {
    inode_t inode;
    fs_get_inode(id, &inode);
    if (inode.type == 0) {
        return 0;
    }

    // grow inode if needed
    u32 blocks_needed = ALIGN_UP(inode.size + size, 512) / 512;
    u32 blocks = inode_get_block_count(id);
    if (blocks_needed > blocks) {
        inode_alloc_blocks(id, blocks_needed - blocks);
        fs_get_inode(id, &inode);
    }

    u8* buf = (u8*)in;
    u8 block_buf[512];

    u32 block_idx = inode.size / 512;
    u32 block = inode.direct[block_idx];
    u32 block_pos = inode.size % 512;
    u32 remaining = size;

    if (block_pos > 0) {
        mem_zero(block_buf, sizeof(block_buf));
        ata_read_sectors(block, block_buf, 1);

        u32 bytes_to_write;
        if (remaining > 512) {
            bytes_to_write = 512 - remaining;
        } else {
            bytes_to_write = remaining;
        }

        for (u32 i = 0; i < bytes_to_write; i++) {
            block_buf[block_pos + i] = buf[i];
        }

        ata_write_sectors(block, block_buf, 1);
        buf += 512;

        remaining -= bytes_to_write;
        block_idx++;
    }

    while (remaining > 0) {
        block = inode.direct[block_idx];
        block_idx++;

        mem_zero(block_buf, sizeof(block_buf));

        if (remaining < 512) {
            memcpy(block_buf, buf, remaining);

            ata_write_sectors(block, block_buf, 1);

            remaining = 0;
            continue;
        }

        memcpy(block_buf, buf, 512);
        ata_write_sectors(block, block_buf, 1);
        buf += 512;
        remaining -= 512;
    }

    inode.size += size;
    fs_update_inode(id, &inode);

    // TODO: return the bytes written
    return size;
}

u32 inode_read(u32 id, void* out, u32 size) {
    inode_t inode;
    fs_get_inode(id, &inode);
    if (inode.type == 0) {
        return 0;
    }

    u8* out_buf = (u8*)out;

    u32 remaining = size;
    u32 bytes_read = 0;

    u32 block_idx = 0;
    while (remaining > 0) {
        u32 block = inode.direct[block_idx];
        block_idx++;

        if (block == 0) {
            return bytes_read;
        }

        if (remaining < 512) {
            u8 block_buf[512];
            ata_read_sectors(block, block_buf, 1);
            memcpy(out_buf, block_buf, remaining);
            bytes_read += remaining;
            remaining = 0;
            continue;
        }

        ata_read_sectors(block, out_buf, 1);
        bytes_read += 512;
        out_buf += 512;
        remaining -= 512;
    }

    return bytes_read;
}

void fs_add_dir_entry(u32 id, dentry_t* dentry, const u8* name) {
    u8 buf[512];
    mem_zero(buf, sizeof(buf));

    dentry->entry_len = sizeof(dentry_t) + dentry->name_len;
    memcpy(buf, dentry, sizeof(dentry_t));
    memcpy(buf + sizeof(dentry_t), name, dentry->name_len);

    inode_write(id, buf, dentry->entry_len);
}

void fs_list_dir(u32 id) {
    inode_t inode;
    fs_get_inode(id, &inode);
    if (inode.type != FS_DIR) {
        return;
    }

    // 1 extra dentry to be able to read the next null length
    u8 buf[inode.size + sizeof(dentry_t)];
    mem_zero(buf, sizeof(buf));
    u8* ptr = (u8*)buf;

    inode_read(id, buf, inode.size);

    for (;;) {
        dentry_t* dentry = (dentry_t*)ptr;
        
        if (dentry->entry_len == 0) {
            break;
        }

        if (dentry->type == FS_DIR) {
            console_set_fg(VGA_LIGHT_CYAN);
        }

        u32 name_pos = sizeof(dentry_t);

        u8 name[dentry->name_len + 1];
        memcpy(&name, (ptr + name_pos), dentry->name_len);
        name[dentry->name_len] = '\0';
        kprintf("%s\n", name);

        console_set_fg(VGA_GRAY);

        ptr += dentry->entry_len;
    }

    console_set_fg(VGA_GRAY);
}

// TODO: check for existing directory
u32 fs_create_dir(u32 parent_id, const u8* name) {
    u32 id = fs_alloc_inode();
    if (id == 0) {
        return 0;
    }

    inode_t inode = {
        .type = FS_DIR,
    };
    fs_update_inode(id, &inode);

    dentry_t self = {
        .id = id,
        .type = inode.type,
        .name_len = 1
    };
    fs_add_dir_entry(id, &self, ".");

    self.name_len = strlen(name);
    fs_add_dir_entry(parent_id, &self, name);

    dentry_t parent = {
        .id = parent_id,
        .type = inode.type,
        .name_len = 2
    };
    fs_add_dir_entry(id, &parent, "..");

    return id;
}

u32 fs_dir_find(u32 id, const u8* name) {
    inode_t inode;
    fs_get_inode(id, &inode);
    if (inode.type != FS_DIR) {
        return 0;
    }

    u8 buf[inode.size + sizeof(dentry_t)];
    mem_zero(buf, sizeof(buf));
    u8* ptr = (u8*)buf;

    inode_read(id, buf, inode.size);

    for (;;) {
        dentry_t* dentry = (dentry_t*)ptr;
        
        if (dentry->entry_len == 0) {
            break;
        }

        u32 name_pos = sizeof(dentry_t);

        u8 entry_name[dentry->name_len + 1];
        memcpy(&entry_name, (ptr + name_pos), dentry->name_len);
        entry_name[dentry->name_len] = '\0';

        if (memcmp(entry_name, name, dentry->name_len) == 0) {
            return dentry->id;
        }

        ptr += dentry->entry_len;
    }

    return 0;
}

u32 fs_create_file(u32 parent_id, const u8* name) {
    u32 id = fs_alloc_inode();
    if (id == 0) {
        return 0;
    }

    inode_t inode = {
        .type = FS_FILE,
    };
    fs_update_inode(id, &inode);

    dentry_t self = {
        .id = id,
        .type = inode.type,
        .name_len = strlen(name),
    };
    fs_add_dir_entry(parent_id, &self, name);

    return id;
}

void fs_delete_file(u32 parent_id, const u8* name) {
    inode_t parent_inode;
    fs_get_inode(parent_id, &parent_inode);
    if (parent_inode.type == 0) {
        return;
    }

    u32 file_pos = 0;
    u8 buf[parent_inode.size + sizeof(dentry_t)];
    u8* ptr = (u8*)buf;
    dentry_t* prev = 0;
    dentry_t* dentry = 0;

    mem_zero(buf, sizeof(buf));

    inode_read(parent_id, buf, parent_inode.size);

    for (;;) {
        dentry = (dentry_t*)ptr;
        
        if (dentry->entry_len == 0) {
            break;
        }

        u32 name_pos = sizeof(dentry_t);

        u8 entry_name[dentry->name_len + 1];
        memcpy(&entry_name, (ptr + name_pos), dentry->name_len);
        entry_name[dentry->name_len] = '\0';

        if (memcmp(entry_name, name, dentry->name_len) == 0) {
            break;
        }

        prev = dentry;
        ptr += dentry->entry_len;
        file_pos += dentry->entry_len;
    }

    u32 block_idx = file_pos / 512;
    u32 block = parent_inode.direct[block_idx];

    prev->entry_len += dentry->entry_len;
    fs_free_inode(dentry->id);

    mem_zero(dentry, dentry->entry_len);

    // NOTE: keeping zeros in tact is slow but nice for debugging
    u8 block_buf[512];
    mem_zero(block_buf, sizeof(block_buf));
    memcpy(block_buf, buf, parent_inode.size);

    ata_write_sectors(block, (block_buf + (block_idx * 512)), 1);
}
