# DOC 2 — MEMORY MANAGEMENT & VIRTUAL MEMORY
## Complete OS Interview Preparation | SDE-2 / SDE-3 / FAANG Level
### Version 1.0 | June 2026

---

# ═══════════════════════════════════════════════════════
# CHAPTER 1: PHYSICAL MEMORY MANAGEMENT FUNDAMENTALS
# ═══════════════════════════════════════════════════════

---

## 1.1 Memory Hierarchy and the Memory Wall

### THEORY BLOCK

Modern computers have a **memory hierarchy** — multiple levels of storage, each with different capacity, speed, and cost. Understanding this is FUNDAMENTAL to OS performance.

```
MEMORY HIERARCHY (fast → slow, small → large, expensive → cheap):

Level       │ Technology  │ Size       │ Access Time    │ Managed by
────────────┼─────────────┼────────────┼────────────────┼───────────
Registers   │ Flip-flop   │ ~1KB       │ 0.3–0.5 ns     │ Compiler
L1 Cache    │ SRAM        │ 32–64KB    │ 0.5–1 ns       │ Hardware
L2 Cache    │ SRAM        │ 256–512KB  │ 3–10 ns        │ Hardware
L3 Cache    │ SRAM        │ 4–32MB     │ 10–40 ns       │ Hardware
RAM (DRAM)  │ DRAM        │ 4–512 GB   │ 50–100 ns      │ OS
NVMe SSD    │ Flash       │ 256GB–8TB  │ 70,000 ns      │ OS + Filesystem
SATA SSD    │ Flash       │ 256GB–8TB  │ 100,000 ns     │ OS + Filesystem
HDD         │ Magnetic    │ 1TB–20TB   │ 5,000,000 ns   │ OS + Filesystem
Network     │ Various     │ Unlimited  │ 1,000,000+ ns  │ Application/OS

Key: 1 ns = 1 nanosecond = 0.000000001 second
     HDD access is ~10,000,000× slower than L1 cache!
```

### Locality of Reference — Why Programs Are Fast

Programs don't access memory randomly. They exhibit two types of locality:

1. **Temporal Locality:** If you access memory address X now, you're likely to access X again soon (variables in loops, frequently called functions).

2. **Spatial Locality:** If you access memory address X, you're likely to access X+1, X+2, etc. soon (arrays, sequential code execution).

**Why this matters:** Caches exploit locality! A cache LINE is 64 bytes. When you access one byte, the CPU loads the entire 64-byte line. If your code has good spatial locality, you get many accesses for the price of one cache miss.

```c
/* BAD: column-major traversal of row-major array
 * Terrible spatial locality → many cache misses */
for (int j = 0; j < N; j++)
    for (int i = 0; i < N; i++)
        sum += matrix[i][j];   /* jumps N*4 bytes each iteration! */

/* GOOD: row-major traversal → sequential memory access
 * Excellent spatial locality → almost no cache misses */
for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
        sum += matrix[i][j];   /* sequential 4 bytes each iteration */
```

**Real performance impact:** For a 1000×1000 matrix, the bad version can be 10-100× slower due to cache misses!

---

## 1.2 Memory Address Spaces

### Physical vs Logical Address Space

```
LOGICAL (VIRTUAL) ADDRESS SPACE          PHYSICAL ADDRESS SPACE
(What the process sees)                  (What the hardware sees)

Process 1 sees:                          
  0x0000 → 0xFFFF...FFFF                
  │                                      
  │   MMU Translation                   
  ▼ (Memory Management Unit)            
Physical RAM:
  0x0000 → available_RAM                 
  [OS Kernel]                           
  [Process 1 pages] (scattered!)        
  [Process 2 pages] (scattered!)        
  [Free frames]                         
```

**The key insight:** Process 1's virtual address 0x400000 might map to physical address 0x1234000. Process 2's virtual address 0x400000 maps to physical address 0x5678000. Each process has its OWN virtual address space — complete isolation!

### Address Binding: When are Addresses Resolved?

```
SOURCE CODE (symbolic names: "count", "buffer")
    │
    ▼ COMPILE TIME: If we know where program will load
    │ Compiler generates absolute physical addresses
    │ (old approach: MS-DOS .COM files, embedded systems)
    │ Problem: Program must always load at same address!
    │
    ▼ LOAD TIME: At program load, linker/loader relocates
    │ Compiler generates relocatable code with offsets
    │ Linker adds base address to all references
    │ Problem: Once loaded, cannot move (still needs relocation table)
    │
    ▼ EXECUTION TIME: Modern approach (virtual memory)
      CPU generates virtual address
      MMU translates to physical on EVERY memory access
      Process can be moved in physical memory while running!
      This is what modern OS (Linux, Windows, macOS) do.
```

---

## 1.3 Contiguous Memory Allocation

Before paging was invented, memory was allocated in **contiguous chunks**. Understanding this is critical for OS interviews because it motivates WHY paging was invented.

### Fixed Partitioning

Memory is divided into **fixed-size partitions** at boot time. Each process gets one partition.

```
PHYSICAL MEMORY (256MB total):
┌────────────────────────────────────┐ 256MB
│         Partition 4 (64MB)         │ ← Process D (size=50MB uses 50MB, 14MB wasted!)
├────────────────────────────────────┤ 192MB
│         Partition 3 (64MB)         │ ← Empty (no process fits exactly 64MB)
├────────────────────────────────────┤ 128MB
│         Partition 2 (64MB)         │ ← Process B (size=60MB, 4MB wasted!)
├────────────────────────────────────┤ 64MB
│         Partition 1 (64MB)         │ ← Process A (size=30MB, 34MB wasted!)
├────────────────────────────────────┤ 0MB
│         OS Kernel (fixed)          │
└────────────────────────────────────┘

INTERNAL FRAGMENTATION: Wasted space INSIDE a partition
  Partition 1: 64 - 30 = 34MB wasted!
  Partition 2: 64 - 60 =  4MB wasted!
  Partition 4: 64 - 50 = 14MB wasted!
  Total waste: 52MB out of 192MB = 27% waste!
```

**Problem:** What if process needs 70MB? No partition is big enough (even though 128MB is free total)! This is external fragmentation even in fixed partitioning!

---

### Dynamic Partitioning

Partitions created on-demand, sized exactly to the process.

```
Initial state:
┌────────────────────────────────────┐
│           FREE (240MB)             │
├────────────────────────────────────┤
│  OS Kernel (16MB)                  │
└────────────────────────────────────┘

After loading Process A (50MB), B (80MB), C (30MB):
┌────────────────────────────────────┐ 256MB
│          FREE (80MB)               │ ← "hole" — external fragment
├────────────────────────────────────┤ 176MB
│      Process C (30MB)              │
├────────────────────────────────────┤ 146MB
│      Process B (80MB)              │
├────────────────────────────────────┤ 66MB
│      Process A (50MB)              │
├────────────────────────────────────┤ 16MB
│      OS Kernel (16MB)              │
└────────────────────────────────────┘ 0MB

Process B exits:
┌────────────────────────────────────┐ 256MB
│          FREE (80MB)               │
├────────────────────────────────────┤ 176MB
│      Process C (30MB)              │
├────────────────────────────────────┤ 146MB
│          FREE (80MB)               │ ← Another hole!
├────────────────────────────────────┤ 66MB
│      Process A (50MB)              │
├────────────────────────────────────┤ 16MB
│      OS Kernel (16MB)              │
└────────────────────────────────────┘ 0MB

Now: 160MB free, but in TWO separate holes of 80MB each!
A process needing 100MB CANNOT be allocated, even though 160MB is free!
This is EXTERNAL FRAGMENTATION!
```

---

### Memory Allocation Algorithms

Given a set of "holes" (free memory regions) and a request for N bytes, how do we pick which hole to use?

#### First Fit

```
Scan holes from the BEGINNING, use the FIRST one that's big enough.

Example: holes at [100MB, 40MB, 200MB, 60MB]. Request: 50MB.
→ First hole with ≥50MB: 100MB hole. Allocate there. Done.

Result: [50MB_used, 50MB_free, 40MB_free, 200MB_free, 60MB_free]

Pros: Fast (stops at first fit), tends to keep large holes at end
Cons: Creates many small holes at the beginning (front becomes fragmented)
```

#### Best Fit

```
Scan ALL holes, use the SMALLEST one that's big enough.

Example: holes at [100MB, 40MB, 200MB, 60MB]. Request: 50MB.
→ All holes ≥50MB: 100MB, 200MB, 60MB
→ Smallest of these: 60MB hole. Allocate there.

Result: [100MB_free, 40MB_free, 200MB_free, 10MB_leftover]

Pros: Minimizes waste for this request (leaves smallest residual)
Cons: Creates LOTS of tiny unusable holes! (leftover is often too small)
     Slower (must scan ALL holes)
```

#### Worst Fit

```
Scan ALL holes, use the LARGEST one.

Example: holes at [100MB, 40MB, 200MB, 60MB]. Request: 50MB.
→ Largest hole: 200MB. Allocate there.

Result: [100MB_free, 40MB_free, 150MB_leftover, 60MB_free]

Pros: Leaves the LARGEST possible residual hole (more likely to be useful!)
Cons: Destroys the large hole that might have fit a big future request
     Slowest (must find maximum)

Counterintuitive! "Worst fit" tries to be clever but often performs worst.
```

#### Next Fit

```
Like First Fit but starts from where the LAST allocation was made.
A "roving pointer" that wraps around.

Pros: More uniform distribution of allocations across memory
Cons: Can break up the large hole at end that First Fit preserves
```

#### Comparison Summary

| Algorithm | Speed | External Fragmentation | Notes |
|-----------|-------|----------------------|-------|
| First Fit | O(holes) avg | Medium | Often best in practice |
| Best Fit | O(holes) | Worst (tiny holes) | Counterintuitively bad |
| Worst Fit | O(holes) | Very bad | Destroys large holes |
| Next Fit | O(holes) avg | Medium | Like First Fit, more uniform |

**Interview answer:** "In practice, First Fit is usually best — fast and leaves larger holes at the end of memory."

---

### Compaction: The Defragmentation Solution

**Problem:** After many allocations and frees, memory looks like Swiss cheese — many small holes scattered throughout.

**Solution:** **Compaction** — move all processes to one end, merge all holes into one big hole.

```
BEFORE compaction:
│OS│P1(50)│HOLE(30)│P2(40)│HOLE(20)│P3(60)│HOLE(60)│
0   16     66  96  136    156      216    276  300MB

AFTER compaction:
│OS│P1(50)│P2(40)│P3(60)│    HOLE(110)    │
0   16     66   106    166               276MB
```

**Problems with compaction:**
- Need to MOVE actual process pages in memory → requires updating all pointers!
- Can only do if relocation is **dynamic** (execution-time binding with MMU)
- Very expensive: copying gigabytes of data takes seconds
- Process must be stopped during compaction (can't run while being moved)

**Real-world use:** Linux/modern OS use PAGING instead, which eliminates external fragmentation entirely without needing compaction.

---

### The Buddy System — Linux's Physical Memory Allocator

Linux uses the **buddy system** for allocating physical pages:

```
Idea: Maintain free lists for blocks of size 2^0, 2^1, 2^2, ..., 2^MAX pages

When allocating 2^k pages:
1. If free list for 2^k has blocks → allocate one. Done.
2. Otherwise, split a block from 2^(k+1) list into TWO buddies of size 2^k
3. Put one buddy in the 2^k free list, return the other.
4. If 2^(k+1) is also empty, recurse upward.

When freeing a 2^k block:
1. Check if its BUDDY is also free (buddy = block XOR 2^k → simple calculation!)
2. If buddy is free → COALESCE (merge) into a 2^(k+1) block
3. Recurse: try to coalesce the merged block with ITS buddy

Example: 1MB RAM divided into 256KB blocks (512 4KB pages total)

Free list after booting:
  2^9 (2MB): [block at 0x000000]   ← entire RAM as one block
  
Allocate 4KB (2^0 = 1 page):
  Need 2^0 block. None. Split 2^9 → two 2^8. Split one → 2^7...
  Eventually: 4KB block allocated, free list has blocks at 4KB, 8KB, 16KB, ... 1MB
  
Buddy system:
  FAST: O(log n) allocation and deallocation
  COALESCING: automatic merging of adjacent free blocks
  FRAGMENTATION: only 2^k fragmentation (internal), no external
```

**In Linux:** `alloc_pages()` kernel function uses the buddy allocator. `kmalloc()` for small kernel allocations uses the **Slab allocator** on top of buddy.

---

# ═══════════════════════════════════════════════════════
# CHAPTER 2: PAGING — THE CORNERSTONE OF MODERN OS
# ═══════════════════════════════════════════════════════

---

## 2.1 Paging Concept

### The Problem Paging Solves

Dynamic partitioning had **external fragmentation** — scattered holes that can't be used.

**Paging's solution:** Break BOTH physical memory and logical memory into **fixed-size chunks**:
- Physical memory → **frames** (fixed size, typically 4KB)
- Logical memory → **pages** (same size as frames, 4KB)

**Key insight:** Pages don't need to be in CONTIGUOUS frames! Process pages can be scattered throughout physical memory.

```
LOGICAL ADDRESS SPACE of Process P:    PHYSICAL MEMORY FRAMES:
┌──────────────┐                       Frame 0: [OS data    ]
│   Page 0     │────────────────────▶  Frame 1: [OS code    ]
│   (code)     │                       Frame 2: [P's Page 3 ] ←─────────┐
├──────────────┤                       Frame 3: [Q's Page 0 ]           │
│   Page 1     │──────────────────────▶Frame 4: [P's Page 0 ] ←──┐     │
│   (data)     │                       Frame 5: [Q's Page 1 ]     │     │
├──────────────┤                       Frame 6: [P's Page 2 ] ←─┐│     │
│   Page 2     │─────────────────────▶ Frame 7: [P's Page 1 ] ←┐││     │
│   (heap)     │                       Frame 8: [Free        ]  ││││    │
├──────────────┤                       ...                       ││││    │
│   Page 3     │──────────────────────────────────────────────────┘│││    │
│   (stack)    │                                                    │││    │
└──────────────┘                                                    │││    │
                                                                     ││    │
P's Page Table:  [Page 0 → Frame 4] ─────────────────────────────┘│    │
                 [Page 1 → Frame 7] ──────────────────────────────┘    │
                 [Page 2 → Frame 6] ───────────────────────────────────┘
                 [Page 3 → Frame 2] 
```

**Result:** No external fragmentation! Frames are scattered but the page table maps them. The process doesn't know or care where its pages physically are.

**Internal fragmentation:** The LAST page of a process might not be fully used. A process needing 9KB gets 3 pages (12KB) → 3KB wasted (internal fragmentation). Average waste = 0.5 × page_size = 2KB per process.

---

## 2.2 Address Translation with Page Tables

### How Virtual Address → Physical Address

```
Virtual Address (32-bit system, 4KB pages = 12-bit offset):

  ┌─────────────────────────┬──────────────────────┐
  │      Page Number (20)   │   Offset (12 bits)   │
  └─────────────────────────┴──────────────────────┘
  bit 31..12                 bit 11..0

Page Number = virtual_address >> 12       (drop last 12 bits)
Offset      = virtual_address & 0xFFF     (last 12 bits)

Translation:
  1. page_number = virtual_addr >> 12
  2. page_table_entry = page_table[page_number]
  3. frame_number = page_table_entry.frame_number  (if valid bit = 1)
  4. physical_addr = (frame_number << 12) | offset

Example:
  Virtual address: 0x00003ABC
  Page number: 0x3 (first 20 bits = 3)
  Offset: 0xABC (last 12 bits = 2748)
  
  page_table[3].frame = 7 (mapped to physical frame 7)
  
  Physical address: (7 << 12) | 0xABC = 0x7000 + 0xABC = 0x7ABC
```

### Page Table Entry (PTE) Structure

Each entry in the page table contains:

```
x86-64 Page Table Entry (64 bits):

  Bit 63: No-Execute (NX) bit — page cannot contain executable code
  Bits 62-52: Available for OS use (swap slot number, etc.)
  Bits 51-12: Physical Frame Number (the actual physical page address)
  Bit 11: Ignored
  Bit 10: Ignored  
  Bit 9: Available for OS use
  Bit 8: Global — don't flush from TLB on context switch (for kernel pages)
  Bit 7: Page Size — 1 = 2MB huge page, 0 = 4KB normal page
  Bit 6: Dirty — CPU sets this when page is WRITTEN to
  Bit 5: Accessed — CPU sets this when page is READ or WRITTEN
  Bit 4: Page Cache Disable
  Bit 3: Page Write-Through
  Bit 2: User/Supervisor — 0 = kernel only, 1 = user can access
  Bit 1: Read/Write — 0 = read-only, 1 = read+write
  Bit 0: Present (Valid) — 1 = page is in RAM, 0 = page fault!
```

**Critical bits for OS interviews:**
- **Valid/Present:** Is this page in RAM? If 0 → page fault!
- **Dirty:** Has this page been modified? If evicted, must write to swap.
- **Accessed:** Was this page recently used? Used by LRU approximations.
- **NX:** Cannot execute code in this page → prevents buffer overflow attacks.

---

### Page Table Size Problem

```
32-bit address space, 4KB pages:
  Number of pages = 2^32 / 2^12 = 2^20 = 1,048,576 pages
  Page table entry size = 4 bytes
  Page table size = 4 bytes × 1,048,576 = 4MB PER PROCESS!

With 100 processes: 400MB just for page tables!
(This is unacceptable — page tables ARE stored in RAM)

64-bit address space (48-bit used), 4KB pages:
  Number of pages = 2^48 / 2^12 = 2^36 = 68 BILLION pages
  Page table size = 8 bytes × 68B = 512 GB PER PROCESS!!!
  (Obviously impossible — solution: hierarchical page tables)
```

---

## 2.3 TLB — Translation Lookaside Buffer

### The Performance Problem with Page Tables

Every memory access requires:
1. FIRST: Access page table (in RAM) to get physical address → 1 memory access
2. THEN: Access the actual data at physical address → 1 memory access

**Problem:** Every memory access is now DOUBLED! (Page table access + data access)

**Solution:** **TLB** — a hardware cache for recently used page table entries.

```
CPU generates virtual address
        │
        ▼
┌─────────────────────────────────────────────────────┐
│                    TLB                               │
│  (Translation Lookaside Buffer)                      │
│  Hardware cache: ~32-1024 entries                   │
│  ┌─────────────┬──────────────────────────────────┐ │
│  │ page_num=3  │ frame=7, dirty=0, accessed=1, ... │ │
│  │ page_num=5  │ frame=2, dirty=1, accessed=1, ... │ │
│  │ page_num=10 │ frame=15, ...                     │ │
│  │ ...         │                                   │ │
│  └─────────────┴──────────────────────────────────┘ │
└──────────┬──────────────────────┬────────────────────┘
           │ TLB HIT (found!)     │ TLB MISS (not found)
           │ ~1 CPU cycle          │ must walk page table
           ▼                       ▼
    Physical Address          RAM: page table lookup
    (fast path!)              (~100 cycles on modern CPU)
                              + Update TLB with new entry
                              (evict oldest if TLB full)
```

### Effective Access Time (EAT) Calculation

**Critical formula for interviews and GATE!**

```
Let:
  α = TLB hit ratio (e.g., 0.90 = 90% of accesses found in TLB)
  t_TLB = TLB access time (e.g., 10ns)
  t_mem = Memory access time (e.g., 100ns)

EAT = α × (t_TLB + t_mem)          ← TLB hit: TLB + 1 memory access
    + (1-α) × (t_TLB + 2×t_mem)    ← TLB miss: TLB + 2 memory accesses
                                        (page table + actual data)

Example: α=0.90, t_TLB=10ns, t_mem=100ns
  EAT = 0.90 × (10 + 100) + 0.10 × (10 + 200)
      = 0.90 × 110 + 0.10 × 210
      = 99 + 21
      = 120 ns

Compare: without TLB = 2 × 100 = 200ns
With 90% TLB hit: 120ns → 40% speedup!
With 99% TLB hit: EAT = 0.99×110 + 0.01×210 = 108.9 + 2.1 = 111ns → 55% speedup!
```

### TLB and Context Switching

**The TLB problem:** When a context switch occurs, the new process's virtual addresses map to DIFFERENT physical addresses. The old TLB entries are WRONG for the new process!

**Solution 1: Flush TLB on every context switch**
- Simple: on `CR3` reload (x86), TLB is automatically flushed
- Problem: Every context switch = cold TLB → many misses for the new process

**Solution 2: ASID (Address Space Identifiers) — ARM64**
- Each TLB entry tagged with ASID (process identifier)
- On context switch, just change ASID register
- TLB entries from old process are IGNORED (wrong ASID) but not deleted
- Multiple processes' TLB entries coexist! Less flushing needed

**Solution 3: PCID (Process Context IDentifiers) — x86-64**
- Similar to ASID, introduced in x86-64
- Linux uses PCID since kernel 4.14
- Avoids TLB flush for recent processes (cached entries survive context switch)

---

## 2.4 Multi-Level Page Tables

### Why Multi-Level?

Single-level page tables are too large. **Solution:** Make the page table itself a tree!

#### Two-Level Page Tables (32-bit)

```
32-bit virtual address:
  ┌──────────────┬──────────────┬──────────────────────┐
  │  p1 (10 bits)│  p2 (10 bits)│    offset (12 bits)  │
  └──────────────┴──────────────┴──────────────────────┘
  Outer PT index   Inner PT index   Byte offset in page

Outer Page Table: 2^10 = 1024 entries × 4 bytes = 4KB (ONE PAGE!)
Each entry points to an Inner Page Table (only if that region is used!)

If process only uses 4MB of its 4GB address space:
  → Only 1 outer PT entry is used (points to 1 inner PT)
  → Inner PT: 1024 entries × 4 bytes = 4KB
  → Total page table memory: 4KB (outer) + 4KB (inner) = 8KB!
  → vs. single-level: 4MB! → 500× savings!

The KEY insight: Most processes use only a tiny fraction of their
address space (code + stack + heap). With 2-level, you only create
inner page tables for the regions actually MAPPED.
```

#### Four-Level Page Tables (x86-64 — What Linux Uses!)

```
48-bit virtual address (x86-64 uses 48 of 64 bits for user space):

  ┌────────┬────────┬────────┬────────┬──────────────────┐
  │ PGD(9) │ PUD(9) │ PMD(9) │ PTE(9) │   offset (12)    │
  └────────┴────────┴────────┴────────┴──────────────────┘
  Page       Page     Page     Page      Byte
  Global     Upper    Middle   Table     offset
  Directory  Direc.   Direc.   Entry     in page

Each level has 2^9 = 512 entries × 8 bytes = 4KB (one page!)
CR3 register holds physical address of PGD.

Address Translation Walk:
  1. CR3 → physical address of PGD
  2. PGD[p1] → physical address of PUD
  3. PUD[p2] → physical address of PMD
  4. PMD[p3] → physical address of PTE table
  5. PTE[p4] → physical frame number + flags
  6. Physical address = frame_number << 12 | offset

Worst case: 4 memory accesses just to FIND the actual data!
(That's why TLB is so critical — avoids all 4 hops on TLB hit)
```

**Linux kernel page table macros:**
```c
pgd_t *pgd = pgd_offset(mm, vaddr);         /* PGD entry */
p4d_t *p4d = p4d_offset(pgd, vaddr);        /* P4D (5-level paging) */
pud_t *pud = pud_offset(p4d, vaddr);        /* PUD entry */
pmd_t *pmd = pmd_offset(pud, vaddr);        /* PMD entry */
pte_t *pte = pte_offset_kernel(pmd, vaddr); /* PTE entry */
```

---

# ═══════════════════════════════════════════════════════
# CHAPTER 3: VIRTUAL MEMORY — THE GREATEST OS ILLUSION
# ═══════════════════════════════════════════════════════

---

## 3.1 Virtual Memory Concept

**Virtual memory** is the technique that makes each process believe it has the ENTIRE address space (e.g., 0 to 256TB on x86-64) even if actual RAM is only 16GB.

**How?** The OS stores process pages either in RAM OR on disk (swap space). Pages not currently needed are on disk. When needed, they're loaded on demand.

```
Virtual Memory = RAM + Swap Space

Process sees:      16TB of address space (each process!)
Physical RAM:      16GB
Swap space:        100GB (on fast SSD)
Total "memory":    16GB RAM + 100GB swap = 116GB "effective"
Max processes:     Many more than would fit in RAM alone!

Cost: Accessing swap (disk) is ~10,000× slower than RAM.
       If a process's active pages don't fit in RAM → THRASHING!
```

---

## 3.2 Demand Paging

**Demand paging:** Don't load a process's pages into RAM when the process starts. Load each page ONLY when it's first accessed.

**Lazy swapper / pager:** The component that loads pages on demand.

### Page Fault Sequence — MUST KNOW FOR INTERVIEWS!

```
Process accesses virtual address 0x12345678:
        │
        ▼
CPU checks TLB → TLB MISS
        │
        ▼
CPU walks page table → Finds PTE for this page → VALID BIT = 0!
        │
        ▼
CPU triggers PAGE FAULT (trap/interrupt to OS)
        │
        ▼
OS Page Fault Handler:
  Step 1: Is this a VALID virtual address? (within process's mapped regions?)
           Check mm_struct → vm_area_structs
           If NOT valid → send SIGSEGV to process (segmentation fault!)
           If valid → continue
           
  Step 2: Find a FREE physical frame
           Check free frame list
           If no free frame → run page replacement algorithm to evict a page
           
  Step 3: If evicted page is DIRTY → write it to swap (I/O operation)
           Process blocks on I/O (goes to WAITING state)
           
  Step 4: Read the needed page from disk (swap/file) into the free frame
           (Another I/O operation — expensive! ~70,000ns for SSD)
           
  Step 5: Update page table: set frame number, valid bit = 1
           
  Step 6: Update TLB with new mapping
           
  Step 7: RESTART the faulting instruction
           (The instruction that caused the fault is re-executed from scratch)
           
  Step 8: Process resumes as if no fault occurred!
```

**Why restart the instruction?** The CPU doesn't know it was in the middle of a page fault. It just gets re-run with the page now in RAM.

### EAT with Page Faults

```
Let:
  p = page fault rate (probability any access causes a fault)
      Typical values: p = 1/1000 to 1/1,000,000
  t_mem  = memory access time = 100ns
  t_page = page fault service time = 8,000,000ns (8ms for HDD)
           or = 70,000ns (70μs for NVMe SSD)

EAT = (1-p) × t_mem + p × t_page

Example (HDD swap):
  p = 1/1000 = 0.001
  EAT = 0.999 × 100 + 0.001 × 8,000,000
      = 99.9 + 8000
      = 8100 ns

That's 81× slower than no page faults! (100ns vs 8100ns)

For performance within 10% of no-faults:
  EAT < 1.1 × t_mem = 110ns
  (1-p) × 100 + p × 8,000,000 < 110
  8,000,000p < 10
  p < 0.00000125  (less than 1 fault per 800,000 accesses!)
```

**Interview lesson:** This shows why keeping page fault rate LOW is critical. Minimizing page faults is the goal of page replacement algorithms.

---

## 3.3 Copy-on-Write (COW)

```c
/* In the parent process, variable x = 100 at address 0x7000 */
int x = 100;
pid_t pid = fork();
if (pid == 0) {
    /* Child: try to write to x */
    x = 200;  /* This triggers a page fault! */
    /* OS sees: page is COW → allocate new physical frame for child
     *          copy page contents to new frame
     *          update child's page table to point to NEW frame
     *          now child's x = 200, parent's x still = 100 */
}
```

```
BEFORE fork():
  Parent page table:    virtual 0x7000 → physical frame 5 [x=100]
  
AFTER fork() (before any writes):
  Parent page table:    virtual 0x7000 → frame 5 [x=100] (READ-ONLY)
  Child page table:     virtual 0x7000 → frame 5 [x=100] (READ-ONLY, SHARED!)
  
AFTER child writes x=200:
  Page fault! Frame 5 is COW-shared.
  OS allocates new frame 9. Copies frame 5 to frame 9.
  Child page table:     virtual 0x7000 → frame 9 [x=200] (READ-WRITE)
  Parent page table:    virtual 0x7000 → frame 5 [x=100] (READ-WRITE again)
  
Only the PAGE that was written gets copied. All other pages remain shared!
```

---

## 3.4 Page Replacement Algorithms

When RAM is full and a new page is needed, which existing page do we evict?

**Goal:** Minimize the total number of page faults.

### How to Trace Page Replacement — Reference String Method

```
Reference string: sequence of page numbers accessed
Example: 7 0 1 2 0 3 0 4 2 3 0 3 2

With 3 physical frames available.

Trace each algorithm by:
1. If page already in a frame: HIT (no fault)
2. If page NOT in a frame: FAULT (evict one, load new)
```

---

### Optimal (OPT) — Bélády's Algorithm

**Rule:** Evict the page that will NOT be used for the LONGEST TIME in the future.

This is **provably optimal** — no algorithm can have fewer page faults. But it requires knowing the future! Used as a benchmark.

```
Reference: 7 0 1 2 0 3 0 4 2 3 0 3 2
Frames: 3

        7   0   1   2   0   3   0   4   2   3   0   3   2
Frame1: 7   7   7   2   2   2   2   2   2   2   2   2   2
Frame2: -   0   0   0   0   0   0   4   4   4   0   0   0
Frame3: -   -   1   1   1   3   3   3   3   3   3   3   3
Fault:  F   F   F   F   -   F   -   F   -   -   F   -   -
                                                          
Total faults: 7

At each fault, OPT evicts the page used FURTHEST in future:
- At ref 2 (frame contains 7,0,1): next use of 7=never, 0=ref4, 1=never → evict 7 or 1
- At ref 3 (frames 2,0,1): next use of 2=ref9, 0=ref6, 1=never → evict 1
etc.
```

---

### FIFO (First-In First-Out)

**Rule:** Evict the page that has been in memory the LONGEST (oldest page).

**Implementation:** Queue of pages. Front = oldest. New pages added at back.

```
Reference: 7 0 1 2 0 3 0 4 2 3 0 3 2
Frames: 3

        7   0   1   2   0   3   0   4   2   3   0   3   2
Frame1: 7   7   7   2   2   2   2   4   4   4   0   0   0
Frame2: -   0   0   0   0   3   3   3   3   3   3   3   3
Frame3: -   -   1   1   1   1   0   0   2   2   2   2   2
        
Queue:  [7] [7,0][7,0,1] [0,1,2][0,1,2][1,2,3][2,3,0][3,0,4][0,4,2][4,2,3][2,3,0][...][...]
Fault:  F   F   F   F   -   F   F   F   F   F   F   -   -

Total faults: 9 (vs OPT's 7)

FIFO's problem: Might evict a FREQUENTLY USED page just because it arrived early!
```

### Bélády's Anomaly

**Surprising fact:** With FIFO, increasing the number of frames can INCREASE page faults!

```
Reference: 1 2 3 4 1 2 5 1 2 3 4 5

With 3 frames: 9 faults
With 4 frames: 10 faults  ← MORE FAULTS WITH MORE MEMORY!

This CANNOT happen with OPT or LRU (they are "stack algorithms").
FIFO is NOT a stack algorithm → Bélády's Anomaly possible.
```

---

### LRU (Least Recently Used)

**Rule:** Evict the page that hasn't been used for the LONGEST TIME in the PAST.

**Rationale:** Past behavior predicts future — if a page wasn't used recently, it probably won't be used soon (exploiting temporal locality).

**No Bélády's Anomaly!** LRU is a stack algorithm.

```
Reference: 7 0 1 2 0 3 0 4 2 3 0 3 2
Frames: 3

        7   0   1   2   0   3   0   4   2   3   0   3   2
Frame1: 7   7   7   2   2   2   0   0   0   0   0   0   0
Frame2: -   0   0   0   0   3   3   3   3   3   3   3   3
Frame3: -   -   1   1   0→- -   -   4   2   2   -   -   2
        (Evict at each fault: least recently used page)
Fault:  F   F   F   F   -   F   F   F   F   F   F   -   -

Total faults: 9 (same as FIFO here, but usually better)
```

**Implementation challenge:** True LRU requires knowing the EXACT time of last use for every page. Options:

1. **Counter-based:** Attach a clock counter to each PTE. On access, copy current clock value. Evict page with SMALLEST counter. O(n) to find minimum.

2. **Stack-based:** Maintain a stack of page numbers. On access, move page to TOP. Bottom of stack = LRU page. O(n) update on each access.

Both have HIGH overhead — accessing a page requires updating a data structure!

---

### LRU Approximation: Clock Algorithm (Second-Chance)

**Hardware-supported approximation** used in practice (Linux uses this!):

**Reference bit:** Hardware sets this bit to 1 whenever a page is accessed. OS can clear it.

```
Clock algorithm (circular list of frames, clock hand sweeps):

┌────────────────────────────────────────────────┐
│                                                │
│  Frame 0: [Page 7, ref=1]                     │
│  Frame 1: [Page 2, ref=0]  ← clock hand       │
│  Frame 2: [Page 5, ref=1]                     │
│  Frame 3: [Page 3, ref=1]                     │
│  Frame 4: [Page 9, ref=0]                     │
│                                                │
└────────────────────────────────────────────────┘

On page fault:
  At clock hand position (Frame 1, Page 2, ref=0):
    ref=0 → EVICT this page! Replace with new page. Advance clock.
  
  If ref=1:
    Clear ref bit (give "second chance"), advance clock, check next frame.
  
Analogy: Give each page a second chance before evicting.
         Pages with ref=1 are "reprieved" but their ref bit is cleared.
         Pages that accumulate ref=1 between clock sweeps are "popular".
```

**Enhanced Second-Chance (Linux's approach):** Uses BOTH reference bit (R) and dirty bit (D):

| (R, D) | Priority to evict | Meaning |
|--------|-------------------|---------|
| (0, 0) | Best victim (page 1)| Not used, not dirty → free for eviction |
| (0, 1) | Good victim (page 2)| Not used but dirty → must write to disk |
| (1, 0) | Poor victim (page 3)| Used recently but clean → give chance |
| (1, 1) | Last resort (page 4)| Used recently AND dirty → expensive |

---

## 3.5 Thrashing — The Deadliest Memory Problem

### What is Thrashing?

```
THRASHING: Processes spend more time paging than doing useful work.
CPU utilization plummets to near zero despite many processes.

Normal situation (low multiprogramming):
  Process A's working set = 8 pages, RAM = 20 pages
  Process B's working set = 5 pages
  Process C's working set = 6 pages
  Total needed: 19 pages < 20 frames → NO THRASHING, CPU busy

Thrashing situation (too many processes):
  Now add D (5 pages), E (4 pages), F (3 pages):
  Total needed: 25 pages > 20 frames!
  
  Every process's pages get evicted → every access = page fault!
  All processes are BLOCKED waiting for disk I/O
  CPU IDLE! (scheduler sees no runnable processes)
  But MORE processes are admitted → MORE thrashing!
```

### The Thrashing Curve

```
CPU Utilization
    │
100%│         ┌───────────────────────┐
   │        /                         \
80% │       /                           ↓ Thrashing begins!
   │      /                              \
60% │     /                               \
   │    /                                  \
40% │   /                                   \
   │  /                                     \
20% │ /                                       \
   │/                                          ↓ CPU idle (all waiting for disk!)
 0% └─────────────────────────────────────────────────────▶
    0    5    10   15   20   25   30   35   40   # processes
```

### How Linux Handles Thrashing: OOM Killer

When RAM AND swap are exhausted, Linux's **OOM (Out Of Memory) Killer** selects and kills a process to free memory.

**OOM score calculation:**
```
OOM score ∝ (process's memory usage) / (total memory)
           + adjustments for: nice value, root process, swappiness, etc.

You can see: cat /proc/<pid>/oom_score
Adjust:      echo -500 > /proc/<pid>/oom_score_adj  (protect from OOM killer)
             echo 1000 > /proc/<pid>/oom_score_adj  (mark as first to kill)
```

The process with the **highest OOM score** is killed first. The score tends to be highest for:
- Processes using the most memory
- Processes that are NOT root
- Processes without explicit protection

---

## 3.6 Page Fault Frequency (PFF) Algorithm

A self-tuning algorithm to prevent thrashing:

```
If process's page fault rate > upper threshold:
    Allocate MORE frames to this process

If process's page fault rate < lower threshold:
    Reclaim some frames from this process (give to others)

This dynamically adjusts frame allocation to match each process's
working set size, preventing thrashing while maximizing utilization.
```

---

## Numerical Problems — Memory Management

### Problem 1: Page Table Size

```
System: 32-bit logical addresses, 4KB page size, 4-byte PTE

Number of pages = 2^32 / 2^12 = 2^20 = 1,048,576
Page table size = 2^20 × 4 = 4,194,304 bytes = 4MB

Now: How large is each LEVEL in a 2-level page table?
32-bit address split as [10-bit p1 | 10-bit p2 | 12-bit offset]

Outer page table: 2^10 = 1024 entries × 4 bytes = 4KB
Each inner page table: 2^10 = 1024 entries × 4 bytes = 4KB
Number of inner page tables needed: 1 per used outer entry

For a process using only 1MB:
  1MB = 256 pages → 1 inner page table (256 entries used, rest empty)
  Outer PT: 4KB, One Inner PT: 4KB = 8KB total
  vs. single-level: 4MB → 512× savings!
```

### Problem 2: EAT with TLB

```
TLB access time = 20ns
Memory access time = 200ns
TLB hit ratio = 80% = 0.8

EAT = 0.8 × (20 + 200) + 0.2 × (20 + 200 + 200)
    = 0.8 × 220 + 0.2 × 420
    = 176 + 84
    = 260 ns

Without TLB: 2 × 200 = 400ns
With TLB: 260ns → 35% speedup
```

### Problem 3: Page Replacement (FIFO with 3 frames)

```
Reference string: 1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5

Step  Ref  Frames          Fault?
1     1    [1,-,-]         YES
2     2    [1,2,-]         YES
3     3    [1,2,3]         YES
4     4    [4,2,3]         YES (evict 1, oldest)
5     1    [4,1,3]         YES (evict 2, oldest)
6     2    [4,1,2]         YES (evict 3, oldest)
7     5    [5,1,2]         YES (evict 4, oldest)
8     1    [5,1,2]         NO  (1 is in frame)
9     2    [5,1,2]         NO  (2 is in frame)
10    3    [5,3,2]         YES (evict 1, oldest)
11    4    [5,3,4]         YES (evict 2, oldest)
12    5    [5,3,4]         NO  (5 is in frame)

Total page faults: 9
```

### Problem 4: LRU Page Replacement

```
Reference string: 1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5
Frames: 3

At each fault, evict the LEAST RECENTLY USED page.

Step  Ref  Frames (LRU→MRU)   Evict   Fault?
1     1    [1]                 -       YES
2     2    [1,2]               -       YES
3     3    [1,2,3]             -       YES
4     4    evict 1 [4,2,3]     1       YES (1 was LRU: used at step 1, not since)
5     1    evict 3 [4,2,1]     3       YES (3 was LRU: used at step 3)
           Wait — let me redo more carefully:
           
After step 3: frames={1,2,3}, last used: 1@step1, 2@step2, 3@step3
Step4: ref=4, fault: LRU=1(used at step1) → evict 1, frames={4,2,3}
After: last used: 4@step4, 2@step2, 3@step3  LRU=2
Step5: ref=1, fault: LRU=2(used at step2) → evict 2, frames={4,1,3}
After: last used: 4@step4, 1@step5, 3@step3. LRU=3
Step6: ref=2, fault: LRU=3(used at step3) → evict 3, frames={4,1,2}
After: last used: 4@step4, 1@step5, 2@step6. LRU=4
Step7: ref=5, fault: LRU=4(used at step4) → evict 4, frames={5,1,2}
After: last used: 5@step7, 1@step5, 2@step6. LRU=1
Step8: ref=1, HIT. frames={5,1,2}. Update 1's time to step8.
After: 5@step7, 1@step8, 2@step6. LRU=2
Step9: ref=2, HIT. frames={5,1,2}. Update 2's time.
After: 5@step7, 1@step8, 2@step9. LRU=5
Step10: ref=3, fault: LRU=5(used at step7) → evict 5, frames={3,1,2}
After: 3@step10, 1@step8, 2@step9. LRU=1
Step11: ref=4, fault: LRU=1(used at step8) → evict 1, frames={3,4,2}
Step12: ref=5, fault: LRU=2(used at step9) → evict 2, frames={3,4,5}

Total faults: 10 (vs FIFO's 9 here — sometimes LRU worse on specific strings!)
```

---

## MCQ Bank — Memory Management

**Q1.** Internal fragmentation occurs in: [EASY]
- A) Linked file allocation
- B) Dynamic partitioning
- C) Fixed partitioning ✓ (CORRECT — process is smaller than partition)
- D) Linked allocation

**Q2.** Bélády's Anomaly can occur in which replacement algorithm? [MEDIUM]
- A) OPT
- B) LRU
- C) FIFO ✓ (CORRECT)
- D) Clock

*Explanation:* OPT and LRU are "stack algorithms" — pages in memory for n frames are always a subset of pages for n+1 frames. Stack algorithms CANNOT exhibit Bélády's Anomaly. FIFO is NOT a stack algorithm.

**Q3.** The TLB (Translation Lookaside Buffer) is a: [MEDIUM]
- A) Part of the disk controller
- B) Hardware cache for page table entries ✓ (CORRECT)
- C) Software data structure in the OS
- D) Register in the CPU holding the page table base

**Q4.** Which page replacement algorithm is theoretically optimal? [EASY]
- A) LRU
- B) FIFO
- C) Clock
- D) OPT (Bélády's) ✓ (CORRECT)

**Q5.** A process that has its pages constantly being swapped in and out is experiencing: [MEDIUM]
- A) Segmentation fault
- B) Priority inversion
- C) Thrashing ✓ (CORRECT)
- D) Deadlock

**Q6.** In a system with 32-bit virtual addresses and 4KB page size, the size of a single-level page table per process is: [HARD]
- A) 1MB
- B) 2MB
- C) 4MB ✓ (CORRECT: 2^20 entries × 4 bytes = 4MB)
- D) 8MB

**Q7.** Copy-on-Write (COW) in fork() means: [HARD]
- A) Parent's pages are physically copied at fork() time
- B) Child's pages are shared with parent, physically copied only on write ✓ (CORRECT)
- C) Both parent and child share a write buffer
- D) Writes are disabled after fork()

**Q8.** The "dirty bit" in a page table entry: [MEDIUM]
- A) Indicates if the page is corrupted
- B) Indicates if the page has been written to since being loaded ✓ (CORRECT)
- C) Indicates if the page is the oldest
- D) Indicates if the page is shared

**Q9.** Effective Access Time (EAT) formula with TLB: [HARD]
- A) EAT = hit_rate × mem_time + (1-hit_rate) × 2×mem_time
- B) EAT = hit_rate × (TLB_time + mem_time) + (1-hit_rate) × (TLB_time + 2×mem_time) ✓
- C) EAT = TLB_time + mem_time
- D) EAT = (TLB_time + mem_time) / hit_rate

**Q10.** A process with working set larger than available frames will experience: [MEDIUM]
- A) Normal execution
- B) Thrashing ✓ (CORRECT)
- C) Deadlock
- D) Priority inversion

---

## Interview Questions — Memory Management

### Q1: "Explain what happens when a process accesses an invalid memory address."

**Answer:**
> "When a process accesses an invalid virtual address, the MMU checks the page table and finds either: (1) the valid bit is 0 (page not in RAM), or (2) the page is mapped but the access violates permissions (e.g., writing to read-only page, or executing from non-executable page).
>
> The MMU raises a **page fault exception** — a trap to the OS. The OS page fault handler runs:
>
> For a genuinely INVALID address (e.g., NULL dereference, accessing freed memory): The OS checks its vm_area_struct (VMA) list for the process's mapped regions. The address isn't in any VMA → the OS sends **SIGSEGV** to the process → "Segmentation fault (core dumped)".
>
> For a valid address with a permission violation (e.g., writing to read-only code): SIGSEGV again.
>
> For a valid address that's just not in RAM (demand paging): The OS loads the page and restarts the instruction.
>
> For a COW page that's written: OS creates a private copy of the page, updates page table, restarts.
>
> The key: **page fault handler decides what kind of fault it is**, then responds appropriately."

### Q2: "What is the difference between internal and external fragmentation?"

**Answer:**
> "**Internal fragmentation:** Wasted space INSIDE an allocated block. Occurs when the OS allocates a fixed-size chunk (partition or page) but the process only needs part of it. Example: Process needs 3KB but OS allocates a 4KB page → 1KB wasted internally. In paging, the last page of every process has internal fragmentation (average: half page size).
>
> **External fragmentation:** Free space is available in total, but not in contiguous chunks large enough for a request. Example: 40MB free, but in 10 holes of 4MB each. A process needing 10MB cannot be allocated, even though 40MB is free! Occurs in dynamic contiguous memory allocation.
>
> **Paging eliminates external fragmentation** (pages are scattered, OS doesn't need contiguous frames) **but not internal fragmentation** (last page of every process is partially used).
>
> **Segmentation eliminates internal fragmentation** (each segment is exactly the right size) **but not external fragmentation** (segments need contiguous memory)."

---

## Segmentation

Segmentation divides logical address space into **segments** — logical units like code, data, stack, heap. Each segment has a name and a length.

```
Segment Table (per process):
  Segment  Base       Limit
  CODE     0x00400000  0x1000  (code is 4096 bytes)
  DATA     0x10000000  0x2000  (data is 8192 bytes)
  STACK    0x7FFFF000  0x8000  (stack is 32768 bytes)
  HEAP     0x20000000  0x4000  (heap is 16384 bytes)

Address format: [segment_number | offset]

Example: address [DATA | 0x100] 
→ physical = DATA.base + 0x100 = 0x10000000 + 0x100 = 0x10000100
→ Check: 0x100 < DATA.limit (0x2000)? YES → valid
→ If offset ≥ limit → SEGMENTATION FAULT!
```

**Segmentation fault = offset exceeds segment limit** (not just "bad pointer" as commonly oversimplified!)

**Linux uses paging, not segmentation:** x86-64 Linux uses a "flat" memory model — all segment bases are 0, limit is maximum. So segmentation is effectively disabled. Page protection handles the same job.

---

*End of DOC_2 — Memory Management and Virtual Memory*
*Coverage: Memory hierarchy, fragmentation, paging, TLB, multi-level page tables, virtual memory, demand paging, page replacement (OPT/FIFO/LRU/Clock), thrashing, OOM killer, segmentation*
