# DOC 4 — FILE SYSTEMS, I/O, STORAGE & LINUX INTERNALS
## Complete OS Interview Preparation | SDE-2 / SDE-3 / FAANG Level
### Version 1.0 | June 2026

---

# ═══════════════════════════════════════════════════════
# CHAPTER 1: FILE SYSTEM CONCEPTS
# ═══════════════════════════════════════════════════════

## 1.1 What is a File?

### THEORY BLOCK

A **file** is a named collection of related data stored on persistent storage (disk). The OS provides a uniform abstraction: regardless of whether data is on HDD, SSD, NFS, RAM disk, or optical disc, your program just `open()`s, `read()`s, `write()`s, and `close()`s.

**File types in Unix/Linux:**
```
Regular file (-):    Most common. Text, binary, executables.
Directory (d):       Special file containing name→inode mappings.
Symbolic link (l):   Contains a path string pointing to another file.
Character device (c): Byte-stream device (keyboard, serial port, /dev/tty).
Block device (b):    Block-addressable device (disk drives, /dev/sda).
Socket (s):          Network endpoint file (/var/run/nginx.sock).
FIFO / Named pipe (p): Half-duplex channel via filesystem name.
```

**File metadata (stored in inode):**
```
Every file has metadata stored separately from its data:
  size:          File size in bytes
  permissions:   Owner/group/world read/write/execute bits
  owner uid:     User ID of file owner
  owner gid:     Group ID of file owner
  atime:         Last ACCESS time (when file was last read)
  mtime:         Last MODIFICATION time (when content last changed)
  ctime:         Last STATUS CHANGE time (when metadata last changed)
  link_count:    Number of hard links pointing to this inode
  block_count:   Number of 512-byte blocks used
  data_pointers: Pointers to data blocks (12 direct + indirect)
```

---

### File Descriptor and the Three-Level File System Structure

```
PROCESS A                    KERNEL SPACE
File Descriptor Table        Open File Table          Inode Table
(per process)                (system-wide)            (system-wide)

fd 0 (stdin)  ──────────▶  Entry: /dev/tty  ──────▶  Inode of /dev/tty
fd 1 (stdout) ──────────▶  Entry: /dev/tty           (shared! same inode)
fd 2 (stderr) ──────────▶  Entry: /dev/tty
fd 3 (file.txt) ────────▶  Entry: file.txt ──────▶  Inode of /home/u/file.txt
                             pos=1024                  size=8192
                             flags=O_RDWR              nlinks=2
                             ref_count=1               ...

PROCESS B
fd 0 ──────────────────────▶ Entry: /dev/tty (same entry as A's fd0!)
fd 1 ──────────────────────▶ Entry: file.txt ──────▶ Inode of /home/u/file.txt
                              pos=0 (B reads from pos 0, A reads from 1024!)
                              ref_count=1

Key insight:
- Two processes opening same file get SEPARATE open file table entries
  (separate file positions → read independently)
- dup() / fork() share open file table entries
  (shared position → read from same point)
- Hard links → multiple directory entries point to same INODE
  (same file, multiple names)
```

---

### File System Calls — Complete Reference

```c
/* Opening files */
int fd = open("file.txt", O_RDONLY);                   /* Read only */
int fd = open("file.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644); /* Write, create, truncate */
int fd = open("file.txt", O_RDWR | O_APPEND);          /* Read+write, always append */

/* O_flags:
 * O_RDONLY, O_WRONLY, O_RDWR: access mode
 * O_CREAT: create if doesn't exist (needs mode arg)
 * O_EXCL: fail if file already exists (with O_CREAT → atomic create)
 * O_TRUNC: truncate to zero length on open
 * O_APPEND: all writes go to end of file (atomic)
 * O_NONBLOCK: don't block on open or subsequent operations
 * O_SYNC: writes are synchronous (flushed to hardware)
 * O_CLOEXEC: close this fd when exec() is called
 */

/* Reading / Writing */
ssize_t n = read(fd, buffer, count);   /* n = bytes actually read (may be < count!) */
ssize_t n = write(fd, buffer, count);  /* n = bytes written */

/* File position */
off_t pos = lseek(fd, offset, SEEK_SET);  /* Move to absolute position */
lseek(fd, 0, SEEK_END);   /* Move to end of file */
lseek(fd, -100, SEEK_CUR); /* Move 100 bytes before current position */

/* File metadata */
struct stat st;
stat("file.txt", &st);     /* File's stat (follows symlinks) */
fstat(fd, &st);            /* Same but takes fd instead of path */
lstat("symlink", &st);     /* Stat of symlink itself, not target */

printf("Size: %lld\n", (long long)st.st_size);
printf("Permissions: %o\n", st.st_mode & 0777);
printf("Inode: %lu\n", st.st_ino);

/* Truncating */
truncate("file.txt", 0);   /* Zero-length file (content gone) */
ftruncate(fd, 100);        /* Truncate open file to 100 bytes */

/* Syncing */
sync();           /* Flush ALL dirty pages to disk (no wait) */
fsync(fd);        /* Flush this file's dirty data+metadata to disk (WAIT) */
fdatasync(fd);    /* Like fsync but skips metadata if not critical for recovery */

/* Permissions */
chmod("file.txt", 0755);  /* rwxr-xr-x */
chown("file.txt", uid, gid);
```

---

## 1.2 File Permissions — Deep Dive

```
Permission bits (9 bits for user/group/other):

rwxrwxrwx
│││││││││
│││││││└└── Others: r=4, w=2, x=1
│││└└└──── Group:  r=4, w=2, x=1
└└└──────── User:   r=4, w=2, x=1

Plus 3 special bits:
  setuid  (4000): Execute with file owner's permissions
  setgid  (2000): Execute with file group's permissions
  sticky  (1000): On directory: only owner can delete their files

Example: chmod 755 script.sh
  7 = rwx (owner can read, write, execute)
  5 = r-x (group can read and execute, not write)
  5 = r-x (others can read and execute, not write)

chmod 4755: setuid bit set (rwsr-xr-x)
  When ANYONE runs this program, it runs with the FILE OWNER'S identity
  Classic example: /usr/bin/passwd (setuid root → can write /etc/shadow)

sticky bit on /tmp (chmod +t /tmp → drwxrwxrwt):
  Everyone can create files in /tmp
  But only the FILE'S OWNER can delete it!
  (Prevents Alice from deleting Bob's /tmp files)
```

---

## 1.3 Directory Structure

```
UNIX Directory Implementation:

Directory = special file containing: [(filename, inode_number), ...]

/home/alice/   directory inode = 1234
  entries:
    "."    → inode 1234   (current directory itself)
    ".."   → inode 1000   (parent directory)
    "code" → inode 5678   (subdirectory)
    "notes.txt" → inode 9012  (regular file)

Hard link vs Symbolic (Soft) link:
  
  Hard link: Two directory entries point to the SAME inode
    $ ln original.txt hardlink.txt
    Both "original.txt" and "hardlink.txt" → inode 9012
    inode 9012: link_count = 2
    Deleting "original.txt" → link_count = 1 (file still exists!)
    Deleting "hardlink.txt" → link_count = 0 (file data deleted!)
    
    Constraints: Cannot cross filesystem boundaries.
                 Cannot hard-link directories (prevents cycles).
  
  Symbolic link: File whose CONTENT is a path string
    $ ln -s /home/alice/original.txt symlink.txt
    symlink.txt → inode 2345 (new inode, content="/home/alice/original.txt")
    Following symlink: OS reads the path string, then looks up that path.
    
    Advantages: Can cross filesystem boundaries.
                Can link to directories.
                Can create dangling symlinks (point to nonexistent file).
    
    $ ls -la
    lrwxrwxrwx  1 alice alice  26 Jun 20 12:00 symlink.txt -> /home/alice/original.txt
    (l = symlink, permissions are always 777 but target's permissions apply)
```

---

# ═══════════════════════════════════════════════════════
# CHAPTER 2: FILE SYSTEM IMPLEMENTATION
# ═══════════════════════════════════════════════════════

## 2.1 Free Space Management

The file system must track which disk blocks are FREE and which are USED.

### Bit Map (Bit Vector)

Most common approach. One bit per disk block.

```
Block numbers: 0  1  2  3  4  5  6  7  8  9  10 11 ...
Bitmap:        1  1  0  0  1  1  1  0  0  1  0  0  ...
               (1=used, 0=free)

Free blocks:  2, 3, 7, 8, 10, 11, ...

Space: 1 bit per block
  For 1TB disk with 4KB blocks:
  Number of blocks = 1TB / 4KB = 256M blocks
  Bitmap size = 256M bits = 32MB
  (Small! Fits in kernel memory on most systems)

Advantages:
  Finding contiguous free blocks: bit operation scan (fast with bitmask)
  Space efficient: 1 bit per block

Linux ext4: uses "block bitmap" — one bitmap per "block group" (~128MB)
```

---

## 2.2 File Allocation Methods

### Contiguous Allocation

All blocks of a file stored sequentially on disk.

```
Directory entry: filename, start_block, length

File "A": start=0, length=4
File "B": start=5, length=3
File "C": start=11, length=5

Disk layout:
Block: 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
       [A][A][A][A][--][B][B][B][--][--][--][C][C][C][C][C]
                  ↑unused gap

Pros: Simple, fast sequential AND random access (start + offset = block number)
Cons: External fragmentation. Must know file size at creation.
      Cannot extend file if no contiguous space.

Used by: CD/DVD ISO 9660 filesystem, some embedded systems
```

### Linked Allocation (FAT)

Each block contains a pointer to the next block.

```
Directory entry: filename, start_block

"A": start=9
Block 9: [data] next=16
Block 16: [data] next=1
Block 1: [data] next=-1 (end of file)

File A's data is in blocks 9, 16, 1 (in that order, non-contiguous!)

PROBLEM: Random access requires scanning from start.
         To read block 500 of a large file: follow 500 pointers!
         
FAT SOLUTION: File Allocation Table
  Store ALL next-pointers in a TABLE in memory (not in data blocks).
  Then to find block 500: scan the FAT array 500 times (still O(n)!)
  But the FAT is in MEMORY → fast scanning even if disk is slow.

FAT (File Allocation Table):
  FAT12: 12-bit entries (max 4096 clusters) — old floppy disks
  FAT16: 16-bit entries (max 65536 clusters × 64KB = 4GB max volume)
  FAT32: 32-bit entries (max 4GB volume with 4KB clusters = 2TB)
  exFAT: 32-bit (up to 512 TB) — used for large USB drives today

FAT layout:
  [FAT1 copy][FAT2 copy][Root Dir][Data clusters]
  Cluster 0: reserved
  Cluster 1: reserved
  Cluster 2: first data cluster (maps to FAT[2] entry)
  ...
```

### Indexed Allocation (Inode-Based — Modern Standard)

An **index block** contains pointers to all data blocks. The inode IS the index.

```
UNIX/Linux inode structure:
  ┌────────────────────────────────────────────┐
  │                    INODE                   │
  │  size, permissions, timestamps, etc.       │
  │                                            │
  │  Direct pointers [0-11]: 12 × 4KB = 48KB │
  │  ┌──┐ ┌──┐ ... ┌──┐                       │
  │  │B5│ │B9│     │B3│ → Direct data blocks  │
  │  └──┘ └──┘     └──┘                       │
  │                                            │
  │  Single Indirect pointer [12]:             │
  │  ┌────────────────────┐                   │
  │  │ Indirect Block     │ → 1024 pointers   │
  │  │ [ptr ptr ptr ...]  │   each → 4KB data │
  │  └────────────────────┘   Total: 4MB      │
  │                                            │
  │  Double Indirect pointer [13]:             │
  │  ┌────────────────┐                        │
  │  │ Dbl Ind Block  │ → 1024 indirect blocks│
  │  └────────────────┘   each has 1024 ptrs  │
  │                        Total: 4GB          │
  │                                            │
  │  Triple Indirect pointer [14]:             │
  │  3-level tree → 4TB maximum file size      │
  └────────────────────────────────────────────┘

MAXIMUM FILE SIZE CALCULATION (Classic exam question!):
  Block size = 4KB = 4096 bytes
  Pointer size = 4 bytes
  Pointers per block = 4096 / 4 = 1024
  
  Direct:          12 × 4KB              =          48 KB
  Single indirect: 1024 × 4KB           =           4 MB
  Double indirect: 1024² × 4KB          =           4 GB
  Triple indirect: 1024³ × 4KB          =           4 TB
  
  TOTAL MAX: ≈ 4 TB (approximately, ext3 actually limited to 2TB by other factors)
  
  Note: ext4 uses 48-bit block numbers → max 1EB (1 Exabyte) file size!
```

**How many disk accesses to read byte B of a file?**

```
If file uses only direct blocks (file size ≤ 48KB):
  1 access: inode already in memory → 1 access for data block

If single indirect (48KB < B ≤ 4MB):
  1 access: read indirect block (pointer table)
  1 access: read actual data block
  Total: 2 accesses (+ inode if not cached)

If double indirect (4MB < B ≤ 4GB):
  1 access: double indirect block
  1 access: single indirect block
  1 access: data block
  Total: 3 accesses

If triple indirect:
  4 accesses
```

---

## 2.3 The inode — Linux ext4 Deep Dive

```c
/* Simplified version of Linux ext4 inode structure */
struct ext4_inode {
    __le16 i_mode;        /* File mode (type + permissions) */
    __le16 i_uid;         /* Lower 16 bits of User ID */
    __le32 i_size_lo;     /* Size in bytes (lower 32 bits) */
    __le32 i_atime;       /* Access time (Unix timestamp) */
    __le32 i_ctime;       /* Inode change time */
    __le32 i_mtime;       /* Modification time */
    __le32 i_dtime;       /* Deletion time (when file was deleted) */
    __le16 i_gid;         /* Group ID */
    __le16 i_links_count; /* Hard link count */
    __le32 i_blocks_lo;   /* Block count (in 512-byte units) */
    __le32 i_flags;       /* File flags (EXT4_EXTENTS_FL, etc.) */
    __le32 i_block[15];   /* Block pointers:
                             [0-11]: direct
                             [12]:   single indirect
                             [13]:   double indirect
                             [14]:   triple indirect
                             (or: extents if EXT4_EXTENTS_FL set) */
    __le32 i_generation;  /* File version (for NFS) */
    __le32 i_file_acl_lo; /* Extended attributes block */
    __le32 i_size_high;   /* Size in bytes (upper 32 bits) → 64-bit size! */
    __le16 i_uid_high;    /* Upper 16 bits of User ID */
    __le16 i_gid_high;    /* Upper 16 bits of Group ID */
    /* ... more fields ... */
};
```

---

## 2.4 Common File Systems Compared

| Feature | FAT32 | NTFS | ext4 | XFS | ZFS |
|---------|-------|------|------|-----|-----|
| **Max file size** | 4GB | 16TB | 16TB | 8EB | 16EB |
| **Max volume** | 2TB | 256TB | 1EB | 8EB | 256ZB |
| **Journaling** | No | Yes | Yes | Yes | CoW |
| **Permissions** | No | ACL | POSIX | POSIX | POSIX |
| **Compression** | No | Yes | No | No | Yes |
| **Encryption** | No | BitLocker | ext4 enc | No | Yes |
| **Checksums** | No | Metadata | No | Metadata | All |
| **OS** | Windows/USB | Windows | Linux | Linux/RHEL | Solaris/BSD |
| **Use case** | USB drives | Windows boot | Linux boot | High-perf | Enterprise storage |

### Journaling — Preventing Filesystem Corruption

Without journaling, a crash during a multi-step write can leave the filesystem inconsistent:

```
Problem: Updating a file requires:
  1. Write data to data block
  2. Update inode (size, timestamps, block pointer)
  3. Update free block bitmap
  
  If crash between steps 2 and 3: 
    Block is used but bitmap says free → future allocation will CORRUPT the file!
  
  Recovery WITHOUT journal: Must scan ENTIRE filesystem to find inconsistencies.
  On large disks: fsck can take HOURS!

Solution: Journal (Write-Ahead Log)
  1. Write JOURNAL RECORD describing all planned changes
  2. Commit journal (flush to disk)
  3. Apply changes to actual filesystem
  4. Mark journal record as complete (checkpoint)
  
  If crash after step 2: On next mount, REPLAY journal → consistent state
  If crash before step 2: Journal never committed → no changes applied → consistent

ext4 journal modes:
  writeback: Only metadata journaled. Data might be out of order.
             Fastest. Use for scratch data.
  ordered (DEFAULT): Metadata journaled, data written to disk BEFORE metadata.
             Good balance of performance and safety.
  journal:   Both data AND metadata journaled.
             Safest but 2x write traffic.
```

---

# ═══════════════════════════════════════════════════════
# CHAPTER 3: I/O SYSTEMS AND DISK SCHEDULING
# ═══════════════════════════════════════════════════════

## 3.1 Disk Hardware Concepts

### HDD (Hard Disk Drive) Anatomy

```
Physical structure:
  ┌─────────────────────────────────────────────────────┐
  │  Read/Write Head                                    │
  │      ↓                                              │
  │  ┌───────────────────────────────────┐              │
  │  │           Platter (spinning)       │              │
  │  │   Track 0    ← Outermost track     │              │
  │  │   Track 1                          │              │
  │  │   Track 2                          │              │
  │  │   ...                              │              │
  │  │   Track N    ← Innermost track     │              │
  │  └───────────────────────────────────┘              │
  │                                                     │
  │  Each track = many sectors (512 bytes or 4096 bytes)│
  │  Cylinder = same track number across all platters   │
  │  The arm moves to position head over correct track  │
  └─────────────────────────────────────────────────────┘

Disk access time components:
  Seek time:         Moving arm to correct track (3-15ms average!)
  Rotational latency: Waiting for sector to rotate under head
                      Average = half rotation = 4ms at 7200 RPM
  Transfer time:     Actually reading/writing (usually < 1ms)

Total = Seek + Rotational + Transfer ≈ 10-20ms per random I/O
(Compare: NVMe SSD = 0.07ms = 70μs! = 100-300× faster)
```

**SSD doesn't have seek time** — any cell can be read equally fast. Disk scheduling algorithms are primarily for HDDs. For SSDs, focus on request merging and I/O scheduling to minimize write amplification.

---

## 3.2 Disk Scheduling Algorithms

The OS maintains a queue of pending disk I/O requests. The **disk scheduler** decides which request to service next.

**Goal:** Minimize total disk arm movement (seek distance).

### FCFS (First-Come First-Served)

```
Requests: 98, 183, 37, 122, 14, 124, 65, 67
Current head position: 53

Serve in order: 53→98→183→37→122→14→124→65→67

Head movement:
  53→98:   |98-53|  = 45
  98→183:  |183-98| = 85
  183→37:  |183-37| = 146
  37→122:  |122-37| = 85
  122→14:  |122-14| = 108
  14→124:  |124-14| = 110
  124→65:  |124-65| = 59
  65→67:   |67-65|  = 2

Total head movement = 45+85+146+85+108+110+59+2 = 640 cylinders
```

---

### SSTF (Shortest Seek Time First)

Always service the request CLOSEST to current head position.

```
Requests: 98, 183, 37, 122, 14, 124, 65, 67
Current head: 53

At 53: closest is 65. Move to 65.
At 65: closest is 67. Move to 67.
At 67: closest is 37. Move to 37.
At 37: closest is 14. Move to 14.
At 14: closest is 98. Move to 98.
At 98: closest is 122. Move to 122.
At 122: closest is 124. Move to 124.
At 124: closest is 183. Move to 183.

Order: 53→65→67→37→14→98→122→124→183

Head movement:
  53→65:  12
  65→67:   2
  67→37:  30
  37→14:  23
  14→98:  84
  98→122: 24
  122→124: 2
  124→183: 59

Total = 12+2+30+23+84+24+2+59 = 236 cylinders ← MUCH BETTER than FCFS!

PROBLEM: Requests at extreme cylinders can STARVE if new requests
         keep arriving near current head position!
```

---

### SCAN (Elevator Algorithm)

Head moves in one direction, services requests, reaches end, reverses.

```
Requests: 98, 183, 37, 122, 14, 124, 65, 67
Current head: 53, moving in INCREASING direction (toward 200)

Going up from 53: service 65, 67, 98, 122, 124, 183
  Reach end (cylinder 199 or last cylinder)
Going down from 183: service 37, 14
  (14 is the last in our list going down)

Order: 53→65→67→98→122→124→183→37→14
Wait — for SCAN the head reverses after reaching the LAST REQUEST
  (if LOOK variant) or the disk end (if SCAN variant).

SCAN (goes all the way to edge):
  53→65→67→98→122→124→183→199→37→14
  (or reverses at 199 and goes down through requests not yet served)

LOOK (goes only to last request):
  53→65→67→98→122→124→183 ← then reverse
  183→37→14

Total (LOOK): |65-53|+|67-65|+|98-67|+|122-98|+|124-122|+|183-124|+|183-37|+|37-14|
             = 12+2+31+24+2+59+146+23 = 299 cylinders

Advantage over SSTF: No starvation! The head always makes progress.
```

---

### C-SCAN (Circular SCAN)

Only services requests going in ONE direction. When end is reached, jumps back to beginning WITHOUT servicing.

```
Requests: 98, 183, 37, 122, 14, 124, 65, 67
Current head: 53, moving INCREASING

Service going up: 65, 67, 98, 122, 124, 183
Reach end (199), jump to start (0), continue going up:
Service going up from 0: 14, 37

Order: 53→65→67→98→122→124→183→199→0→14→37

Total: 12+2+31+24+2+59+16(199-183)+199(199-0)+14+23 = 382

Advantage: More UNIFORM waiting time!
  With SCAN: requests just before reversal wait almost one full sweep.
  With C-SCAN: all requests wait approximately one full sweep in same direction.
```

---

### C-LOOK (Circular LOOK — Most Practical!)

Like C-SCAN but doesn't go all the way to disk ends.

```
Service going up: 65, 67, 98, 122, 124, 183
Jump to 14 (lowest pending), continue up:
Service: 14, 37

Order: 53→65→67→98→122→124→183→14→37

Total: 12+2+31+24+2+59+|183-14|+|37-14|
     = 12+2+31+24+2+59+169+23 = 322

BEST PRACTICE: C-LOOK is the most commonly used algorithm in modern OS.
Linux's elevator scheduler uses a variant of LOOK.
```

### Complete Comparison Table

| Algorithm | Total Movement (our example) | Starvation | Notes |
|-----------|------------------------------|------------|-------|
| FCFS | 640 | No | Simple, poor performance |
| SSTF | 236 | YES | Good avg, starvation possible |
| SCAN | ~299 | No | Good, directional |
| C-SCAN | ~382 | No | Uniform wait |
| LOOK | ~299 | No | Better than SCAN |
| C-LOOK | ~322 | No | Best overall for HDDs |

---

## 3.3 RAID Systems

RAID (Redundant Array of Independent Disks) combines multiple physical disks for performance, capacity, or redundancy.

### RAID 0 — Striping

```
Data striped across N disks. No redundancy.

   Disk 0    Disk 1    Disk 2    Disk 3
  ┌───────┐ ┌───────┐ ┌───────┐ ┌───────┐
  │Stripe0│ │Stripe1│ │Stripe2│ │Stripe3│
  │Stripe4│ │Stripe5│ │Stripe6│ │Stripe7│
  └───────┘ └───────┘ └───────┘ └───────┘

Capacity: N × disk_size
Performance: N× read AND write (parallel I/O!)
Reliability: 0! If ANY disk fails, ALL DATA LOST!

Use case: Scratch space, temp storage, gaming (speed critical, data expendable)
```

### RAID 1 — Mirroring

```
Data written to TWO disks simultaneously.

   Disk 0    Disk 1
  ┌───────┐ ┌───────┐
  │Stripe0│ │Stripe0│ ← exact copy
  │Stripe1│ │Stripe1│ ← exact copy
  └───────┘ └───────┘

Capacity: N/2 × disk_size (50% efficiency)
Read: 2× (can read from either disk)
Write: 1× (must write to BOTH)
Reliability: Survives ONE disk failure (any disk)

Use case: OS boot drive, critical databases, small high-availability deployments
```

### RAID 5 — Striping with Distributed Parity

```
N disks. Data striped, parity distributed across all disks.

   Disk 0    Disk 1    Disk 2    Disk 3
  ┌───────┐ ┌───────┐ ┌───────┐ ┌───────┐
  │  A0   │ │  A1   │ │  A2   │ │  Ap   │ ← Ap = parity of A0,A1,A2
  │  B0   │ │  B1   │ │  Bp   │ │  B2   │ ← Bp = parity of B0,B1,B2
  │  C0   │ │  Cp   │ │  C1   │ │  C2   │ ← Cp rotates position
  │  Dp   │ │  D0   │ │  D1   │ │  D2   │
  └───────┘ └───────┘ └───────┘ └───────┘

Parity: XOR of data stripes (A0 XOR A1 XOR A2 = Ap)
Recovery: If Disk 0 fails: A0 = A1 XOR A2 XOR Ap (reconstruct from remaining)

Capacity: (N-1)/N × total capacity
Performance: Good reads (N disks). Writes: read-modify-write for parity (overhead)
Reliability: Survives ONE disk failure

Write penalty: To update A0:
  1. Read old A0, read old Ap
  2. Compute new Ap = old_Ap XOR old_A0 XOR new_A0
  3. Write new A0 and new Ap  
  → 4 I/O operations per write (2 reads + 2 writes)!

Use case: Most common enterprise storage. NAS devices. Database servers.
```

### RAID 6 — Double Parity

Like RAID 5 but two parity blocks per stripe. Survives TWO simultaneous disk failures. Uses Reed-Solomon error correction (not just XOR).

### RAID 10 — Mirroring + Striping

RAID 1+0: Mirror first, then stripe across mirror pairs. Best of both worlds.

```
   Disk 0    Disk 1    Disk 2    Disk 3
  ┌───────┐ ┌───────┐ ┌───────┐ ┌───────┐
  │  A0   │ │  A0   │ │  A1   │ │  A1   │
  │  B0   │ │  B0   │ │  B1   │ │  B1   │
  └───────┘ └───────┘ └───────┘ └───────┘
   Mirror pair 1       Mirror pair 2
   
   Striped across mirror pairs

Capacity: 50% (same as RAID 1)
Performance: Excellent (reads from any drive in mirror, writes parallel)
Reliability: Can survive multiple failures if not in same mirror pair

Use case: High-performance databases (OLTP), financial systems, performance-critical with redundancy
```

---

# ═══════════════════════════════════════════════════════
# CHAPTER 4: LINUX INTERNALS
# ═══════════════════════════════════════════════════════

## 4.1 The Linux Kernel Architecture

```
┌────────────────────────────────────────────────────────────────────┐
│                     USER SPACE APPLICATIONS                         │
│    shell, daemons, system utilities, user programs                  │
└────────────────────────────────────┬───────────────────────────────┘
                                     │ System Call Interface
┌────────────────────────────────────▼───────────────────────────────┐
│                       LINUX KERNEL                                  │
│                                                                     │
│  ┌─────────────┐  ┌─────────────┐  ┌───────────┐  ┌────────────┐ │
│  │   Process   │  │   Memory    │  │    VFS    │  │  Network   │ │
│  │  Scheduler  │  │  Manager    │  │  (Virtual │  │   Stack    │ │
│  │  (CFS, RT)  │  │  (paging,   │  │   FS     │  │  (TCP/IP)  │ │
│  │             │  │   buddy,    │  │  Layer)  │  │            │ │
│  │  task_struct│  │   slab)     │  │          │  │            │ │
│  └─────────────┘  └─────────────┘  └──────────┘  └────────────┘ │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐  │
│  │                 Device Drivers                               │  │
│  │   disk, NIC, GPU, USB, input, sound, ...                   │  │
│  └─────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐  │
│  │           Hardware Abstraction / Architecture Code           │  │
│  │           (x86-64, ARM64, RISC-V, MIPS, ...)               │  │
│  └─────────────────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────────────────┘
                                     │
                         Physical Hardware
```

---

## 4.2 /proc Filesystem — A Window into the Kernel

`/proc` is a **virtual filesystem** — no files exist on disk. The kernel generates content on-demand when you read.

```bash
# Process information
/proc/<pid>/status       # Process name, state, memory usage, signal masks
/proc/<pid>/maps         # Virtual memory areas (stack, heap, libraries)
/proc/<pid>/fd/          # Directory of open file descriptors (symlinks)
/proc/<pid>/cmdline      # Command line that started the process
/proc/<pid>/io           # I/O statistics (read/write bytes)
/proc/<pid>/net/tcp      # TCP sockets of this process

# System-wide information
/proc/cpuinfo            # CPU model, cores, features, cache sizes
/proc/meminfo            # RAM total, free, buffers, cached, swap
/proc/uptime             # System uptime in seconds
/proc/loadavg            # Load average (1min, 5min, 15min + running/total procs)
/proc/interrupts         # Interrupt counts per CPU per IRQ
/proc/net/dev            # Network interface statistics
/proc/sys/               # Kernel tunable parameters (sysctl interface)

# Kernel tuning via /proc/sys:
echo 1 > /proc/sys/net/ipv4/ip_forward         # Enable IP forwarding
echo 60 > /proc/sys/vm/swappiness              # How aggressively to swap
echo 3 > /proc/sys/vm/drop_caches              # Drop page cache (free memory)
```

---

## 4.3 cgroups — Resource Control Groups

**cgroups** let you limit, prioritize, and account for resources used by groups of processes.

```
cgroup hierarchy:
  /sys/fs/cgroup/
  ├── cpu/
  │   ├── docker/
  │   │   ├── container1/  (CPU-limited container)
  │   │   │   ├── tasks    (list of PIDs in this group)
  │   │   │   ├── cpu.shares  (relative CPU weight)
  │   │   │   └── cpu.cfs_quota_us  (max CPU time per period)
  │   │   └── container2/
  │   └── system/
  ├── memory/
  │   └── docker/
  │       └── container1/
  │           ├── memory.limit_in_bytes  (max RAM)
  │           └── memory.stat
  └── io/

Docker uses cgroups to:
  - Limit container CPU: --cpus=0.5 → cpu.cfs_quota_us = 50000 (50% of 100ms)
  - Limit container RAM: --memory=512m → memory.limit_in_bytes = 536870912
  - Limit container I/O: --device-read-bps → blkio.throttle.*
```

---

## 4.4 Linux Namespaces — Container Isolation

```
Namespaces isolate resources so containers see a private view:

PID namespace:      Container's processes start at PID 1 (own tree!)
Network namespace:  Container has its own network interfaces, routing table
Mount namespace:    Container has its own filesystem view (chroot on steroids)
UTS namespace:      Container has its own hostname and domain name
IPC namespace:      Container has isolated System V IPC, POSIX MQs
User namespace:     Container maps container root → host non-root user
Cgroup namespace:   Container sees only its own cgroup hierarchy
Time namespace:     Container can have different time (rarely used)

Checking namespaces:
  ls -la /proc/<pid>/ns/    # Shows namespace symlinks for a process
  lsns                      # List all active namespaces

Creating namespaces:
  unshare --pid --mount --net -- bash   # Start bash in new namespaces
  nsenter --pid=/proc/<pid>/ns/pid -- bash  # Enter existing namespace
```

---

## MCQ Bank — File Systems, I/O, Storage

**Q1.** The maximum file size with an inode having 12 direct, 1 single indirect pointer, 4KB blocks, 4-byte pointers: [HARD]

Direct:   12 × 4KB = 48KB
Single:   (4096/4) × 4KB = 1024 × 4KB = 4MB
Total ≈ 4MB + 48KB ≈ 4144KB

- A) 48KB
- B) 4MB ✓ (approximately correct — ignoring triple and double indirect)
- C) 4GB
- D) 4TB

**Q2.** Which disk scheduling algorithm can cause starvation? [MEDIUM]
- A) FCFS
- B) SCAN
- C) SSTF ✓ (CORRECT — requests far from head can starve)
- D) C-LOOK

**Q3.** In RAID 5 with N disks, the effective storage capacity is: [MEDIUM]
- A) N × disk_size
- B) (N-1) × disk_size ✓ (CORRECT — one disk equivalent used for parity)
- C) N/2 × disk_size
- D) 2N × disk_size

**Q4.** A symbolic link (symlink) differs from a hard link in that: [MEDIUM]
- A) Symlinks can only exist on the same filesystem
- B) Symlinks can cross filesystem boundaries ✓ (CORRECT)
- C) Symlinks increase the inode link count
- D) Symlinks cannot point to directories

**Q5.** The /proc filesystem is: [MEDIUM]
- A) A regular filesystem stored on disk
- B) A virtual filesystem generated by the kernel ✓ (CORRECT)
- C) A RAM-based filesystem for temporary files
- D) A network filesystem

**Q6.** Journaling in filesystems protects against: [MEDIUM]
- A) Data corruption from hardware errors
- B) Filesystem inconsistency after a crash ✓ (CORRECT)
- C) Running out of disk space
- D) Fragmentation

**Q7.** The setuid bit on an executable means: [HARD]
- A) Only the file owner can execute it
- B) It runs with the file owner's privileges ✓ (CORRECT)
- C) It is executable by the set of users in the group
- D) The file cannot be read by others

**Q8.** The sticky bit on a directory (/tmp) means: [HARD]
- A) The directory cannot be deleted
- B) Files in the directory can only be deleted by their owners ✓ (CORRECT)
- C) Only root can access the directory
- D) Files in the directory are permanent

---

## Interview Questions — File Systems

### Q1: "How does a Unix file system find the data of a file given its path?"

**Answer:**
> "The process is called **pathname resolution** and works as follows:
>
> Say we want to read `/home/alice/notes.txt`:
>
> 1. **Start at root:** The kernel has the root inode (typically inode 2) in memory. Look up the root directory's data block.
>
> 2. **Traverse 'home':** The root directory is a file containing name→inode pairs. Scan for 'home' → find inode number, say 1234. Load inode 1234.
>
> 3. **Traverse 'alice':** Inode 1234 points to the '/home' directory's data blocks. Scan for 'alice' → find inode 5678. Load inode 5678.
>
> 4. **Find 'notes.txt':** Inode 5678 points to '/home/alice' directory data. Scan for 'notes.txt' → find inode 9012. Load inode 9012.
>
> 5. **Read file data:** Inode 9012 contains direct/indirect block pointers to the actual file data. Load the requested data blocks.
>
> **Optimizations:** The kernel caches frequently accessed inodes and directory entries in the **dentry cache** and **inode cache**. Popular paths like `/home` are almost always in cache, making lookups fast.
>
> **Permission check:** At each step, the kernel verifies the current process has execute (x) permission on the directory — this is what 'x' on a directory means: permission to traverse it."

---

*End of DOC_4 — File Systems, I/O, Storage, and Linux Internals*
*Coverage: File concepts, inodes, file system calls, permissions, allocation methods (contiguous/linked/indexed), inode structure, FAT/ext4/ZFS, journaling, disk scheduling (FCFS/SSTF/SCAN/C-SCAN/LOOK/C-LOOK), RAID levels, /proc filesystem, cgroups, Linux namespaces*
