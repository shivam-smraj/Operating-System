/* Phase09/05_file_system_simulation.c
 * TOPIC: File system internals — inode-based FS simulation
 * Simulates: inodes, directory entries, file creation/read/write/delete
 * Compile: gcc -Wall -o fs_sim 05_file_system_simulation.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ─── Simulated disk parameters ─── */
#define BLOCK_SIZE     512    /* 512 bytes per block */
#define TOTAL_BLOCKS   1024   /* 1024 blocks = 512KB */
#define INODE_COUNT    64     /* Max files */
#define NAME_LEN       32
#define DIRECT_BLOCKS  8      /* Direct block pointers in inode */

/* ─── Data structures ─── */
typedef struct {
    int    valid;
    int    is_dir;
    int    uid;
    int    size;        /* File size in bytes */
    int    nlinks;      /* Hard link count */
    time_t atime, mtime, ctime;
    int    blocks[DIRECT_BLOCKS];  /* Direct block pointers */
    int    n_blocks;               /* Number of allocated blocks */
} Inode;

typedef struct {
    char name[NAME_LEN];
    int  inode_num;   /* -1 = empty slot */
} DirEntry;

typedef struct {
    DirEntry entries[32];  /* Up to 32 entries per directory */
    int      n_entries;
} Directory;

/* ─── Simulated "disk" ─── */
Inode     inode_table[INODE_COUNT];
uint8_t   data_blocks[TOTAL_BLOCKS][BLOCK_SIZE];
int       block_bitmap[TOTAL_BLOCKS];   /* 0=free, 1=used */
int       inode_bitmap[INODE_COUNT];    /* 0=free, 1=used */
Directory root_dir;

/* ─── Superblock info ─── */
typedef struct {
    int total_blocks, free_blocks;
    int total_inodes, free_inodes;
    int block_size;
} Superblock;

Superblock sb;

void fs_init(void) {
    memset(inode_table, 0, sizeof(inode_table));
    memset(data_blocks, 0, sizeof(data_blocks));
    memset(block_bitmap, 0, sizeof(block_bitmap));
    memset(inode_bitmap, 0, sizeof(inode_bitmap));
    memset(&root_dir, 0, sizeof(root_dir));
    for (int i=0;i<32;i++) root_dir.entries[i].inode_num = -1;

    sb.total_blocks = TOTAL_BLOCKS;
    sb.free_blocks  = TOTAL_BLOCKS;
    sb.total_inodes = INODE_COUNT;
    sb.free_inodes  = INODE_COUNT;
    sb.block_size   = BLOCK_SIZE;

    /* Create root inode (always inode 0) */
    inode_bitmap[0] = 1;
    inode_table[0].valid   = 1;
    inode_table[0].is_dir  = 1;
    inode_table[0].nlinks  = 2;  /* '.' and '..' */
    inode_table[0].ctime   = inode_table[0].mtime = inode_table[0].atime = time(NULL);
    sb.free_inodes--;

    printf("[FS] Initialized: %dKB disk, %d inodes, block_size=%d\n",
           TOTAL_BLOCKS*BLOCK_SIZE/1024, INODE_COUNT, BLOCK_SIZE);
}

/* ─── Allocate a free inode ─── */
int alloc_inode(void) {
    for (int i=1; i<INODE_COUNT; i++) {
        if (!inode_bitmap[i]) {
            inode_bitmap[i] = 1;
            sb.free_inodes--;
            inode_table[i].valid = 1;
            inode_table[i].ctime = inode_table[i].mtime = inode_table[i].atime = time(NULL);
            return i;
        }
    }
    return -1;  /* No free inodes */
}

/* ─── Allocate a free data block ─── */
int alloc_block(void) {
    for (int i=0; i<TOTAL_BLOCKS; i++) {
        if (!block_bitmap[i]) {
            block_bitmap[i] = 1;
            sb.free_blocks--;
            return i;
        }
    }
    return -1;  /* Disk full */
}

void free_block(int blk) {
    block_bitmap[blk] = 0;
    sb.free_blocks++;
}

/* ─── Add entry to root directory ─── */
int dir_add(const char *name, int ino) {
    for (int i=0; i<32; i++) {
        if (root_dir.entries[i].inode_num == -1) {
            strncpy(root_dir.entries[i].name, name, NAME_LEN-1);
            root_dir.entries[i].inode_num = ino;
            root_dir.n_entries++;
            return 0;
        }
    }
    return -1;  /* Directory full */
}

/* ─── Lookup file in root directory ─── */
int dir_lookup(const char *name) {
    for (int i=0; i<32; i++) {
        if (root_dir.entries[i].inode_num != -1 &&
            strcmp(root_dir.entries[i].name, name) == 0)
            return root_dir.entries[i].inode_num;
    }
    return -1;  /* Not found */
}

/* ─── Create a file ─── */
int fs_create(const char *name, int uid) {
    if (dir_lookup(name) != -1) {
        printf("[fs_create] ERROR: '%s' already exists\n", name);
        return -1;
    }
    int ino = alloc_inode();
    if (ino < 0) { printf("[fs_create] ERROR: No free inodes!\n"); return -1; }

    inode_table[ino].uid     = uid;
    inode_table[ino].nlinks  = 1;
    inode_table[ino].size    = 0;
    inode_table[ino].is_dir  = 0;

    dir_add(name, ino);
    printf("[fs_create] Created '%s' (inode=%d)\n", name, ino);
    return ino;
}

/* ─── Write to file ─── */
int fs_write(int ino, const char *data, int len) {
    Inode *inode = &inode_table[ino];
    int blocks_needed = (len + BLOCK_SIZE - 1) / BLOCK_SIZE;

    if (blocks_needed > DIRECT_BLOCKS) {
        printf("[fs_write] File too large for direct blocks only!\n");
        blocks_needed = DIRECT_BLOCKS;
        len = DIRECT_BLOCKS * BLOCK_SIZE;
    }

    int written = 0;
    for (int b=0; b < blocks_needed; b++) {
        int blk = (inode->n_blocks > b) ? inode->blocks[b] : alloc_block();
        if (blk < 0) { printf("[fs_write] DISK FULL!\n"); break; }
        if (inode->n_blocks <= b) {
            inode->blocks[b] = blk;
            inode->n_blocks++;
        }
        int chunk = (len - written < BLOCK_SIZE) ? len - written : BLOCK_SIZE;
        memcpy(data_blocks[blk], data + written, chunk);
        written += chunk;
    }
    inode->size  = written;
    inode->mtime = time(NULL);
    printf("[fs_write] Wrote %d bytes to inode %d (%d blocks used)\n",
           written, ino, inode->n_blocks);
    return written;
}

/* ─── Read from file ─── */
int fs_read(int ino, char *buf, int max_len) {
    Inode *inode = &inode_table[ino];
    int read_bytes = 0;
    for (int b=0; b < inode->n_blocks && read_bytes < max_len; b++) {
        int chunk = inode->size - read_bytes;
        if (chunk > BLOCK_SIZE) chunk = BLOCK_SIZE;
        if (chunk > max_len - read_bytes) chunk = max_len - read_bytes;
        memcpy(buf + read_bytes, data_blocks[inode->blocks[b]], chunk);
        read_bytes += chunk;
    }
    inode->atime = time(NULL);
    return read_bytes;
}

/* ─── Delete file ─── */
int fs_delete(const char *name) {
    int ino = dir_lookup(name);
    if (ino < 0) { printf("[fs_delete] '%s' not found\n", name); return -1; }

    Inode *inode = &inode_table[ino];
    inode->nlinks--;
    if (inode->nlinks == 0) {
        /* Free all data blocks */
        for (int b=0; b < inode->n_blocks; b++)
            free_block(inode->blocks[b]);
        /* Free inode */
        inode->valid = 0;
        inode_bitmap[ino] = 0;
        sb.free_inodes++;
    }

    /* Remove directory entry */
    for (int i=0; i<32; i++) {
        if (root_dir.entries[i].inode_num == ino) {
            root_dir.entries[i].inode_num = -1;
            root_dir.entries[i].name[0]   = 0;
            root_dir.n_entries--;
            break;
        }
    }
    printf("[fs_delete] Deleted '%s' (inode %d freed)\n", name, ino);
    return 0;
}

/* ─── List directory ─── */
void fs_ls(void) {
    printf("\n[ls /] (%d entries)\n", root_dir.n_entries);
    printf("%-5s %-16s %-6s %-4s\n","Inode","Name","Size","Links");
    printf("%-5s %-16s %-6s %-4s\n","-----","----","----","-----");
    for (int i=0; i<32; i++) {
        if (root_dir.entries[i].inode_num < 0) continue;
        int ino = root_dir.entries[i].inode_num;
        printf("%-5d %-16s %-6d %-4d\n",
               ino, root_dir.entries[i].name,
               inode_table[ino].size, inode_table[ino].nlinks);
    }
    printf("Free inodes: %d/%d  Free blocks: %d/%d\n",
           sb.free_inodes, sb.total_inodes, sb.free_blocks, sb.total_blocks);
}

int main(void) {
    printf("╔═══════════════════════════════════════════════╗\n");
    printf("║   FILE SYSTEM SIMULATION (inode-based)         ║\n");
    printf("╚═══════════════════════════════════════════════╝\n\n");

    fs_init();

    /* Create files */
    int f1 = fs_create("hello.txt", 1000);
    int f2 = fs_create("data.csv",  1000);
    int f3 = fs_create("image.png", 1001);

    /* Write data */
    char *msg1 = "Hello, File System World! This is a test file.\n";
    char  csv[] = "id,name,score\n1,Alice,95\n2,Bob,87\n3,Carol,92\n";
    fs_write(f1, msg1, strlen(msg1));
    fs_write(f2, csv,  strlen(csv));
    fs_write(f3, "PNG_BINARY_DATA_PLACEHOLDER", 26);

    /* List */
    fs_ls();

    /* Read back */
    char buf[1024] = {0};
    int n = fs_read(f1, buf, sizeof(buf)-1);
    printf("\n[fs_read] hello.txt (%d bytes): '%s'\n", n, buf);

    /* Delete */
    fs_delete("image.png");
    fs_ls();

    printf("\n[KEY CONCEPTS]\n");
    printf("• Directory maps filename → inode number\n");
    printf("• Inode holds: size, permissions, timestamps, block pointers\n");
    printf("• Data in blocks; metadata in inode (separate)\n");
    printf("• Hard link: add directory entry pointing to same inode (nlinks++)\n");
    printf("• rm: removes directory entry + decrements nlinks; frees inode when nlinks=0\n");
    printf("• Bitmap tracks free inodes and free blocks\n");
    return 0;
}
