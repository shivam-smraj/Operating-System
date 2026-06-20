/* Phase13/07_memory_interview_problems.c
 * Tricky memory management interview questions with worked solutions
 * Compile: gcc -Wall -o mem_interview 07_memory_interview_problems.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ─── Q1: Calculate page table size ─── */
void q1_page_table_size(void) {
    printf("=== Q1: Calculate Page Table Size ===\n");
    printf("System: 32-bit virtual address, 4KB pages, 4-byte PTE\n");
    printf("How many PTEs? How big is the page table?\n\n");

    long page_size = 4096;           /* 4KB */
    long addr_space = 1L << 32;     /* 4GB for 32-bit */
    long num_pages = addr_space / page_size;
    long pte_size  = 4;              /* bytes */
    long table_size = num_pages * pte_size;

    printf("Number of pages  = 2^32 / 4096 = %ld (= 2^20 = 1 million)\n", num_pages);
    printf("Page table size  = %ld pages × %ld bytes/PTE = %ld MB\n\n",
           num_pages, pte_size, table_size/(1024*1024));
    printf("PROBLEM: 4MB per process just for page table!\n");
    printf("With 100 processes: 400MB wasted on page tables!\n\n");
    printf("SOLUTIONS:\n");
    printf("  1. Multi-level page table (only allocate PTEs when needed)\n");
    printf("  2. Inverted page table (one entry per FRAME, not per page)\n");
    printf("  3. Hashed page table\n");
    printf("  4. 64-bit: Use 4-level paging (PML4) — 48-bit effective space\n\n");
}

/* ─── Q2: Virtual address breakdown ─── */
void q2_address_breakdown(void) {
    printf("=== Q2: Break Down Virtual Address ===\n");
    printf("System: 2-level paging, 32-bit VA, 4KB pages\n");
    printf("Virtual Address: 0x00403000\n\n");

    uint32_t va = 0x00403000;
    /* Layout: [31..22 = PDE(10)] [21..12 = PTE(10)] [11..0 = offset(12)] */
    uint32_t pde_idx = va >> 22;           /* Top 10 bits */
    uint32_t pte_idx = (va >> 12) & 0x3FF; /* Middle 10 bits */
    uint32_t offset  = va & 0xFFF;         /* Bottom 12 bits */

    printf("Binary: %08X = ", va);
    for(int b=31;b>=0;b--) {
        printf("%d",(va>>b)&1);
        if(b==22||b==12) printf("|");
    }
    printf("\n");
    printf("PDE index  (bits 31-22): %4u  (= 0x%03X)\n", pde_idx, pde_idx);
    printf("PTE index  (bits 21-12): %4u  (= 0x%03X)\n", pte_idx, pte_idx);
    printf("Offset     (bits 11- 0): 0x%03X (%u bytes into page)\n\n", offset, offset);

    printf("Translation steps:\n");
    printf("1. PDE[%u] → points to page table for this 4MB region\n", pde_idx);
    printf("2. PTE[%u] → gives physical frame number (say frame 512)\n", pte_idx);
    printf("3. PA = frame 512 × 4096 + 0x%03X = 0x%08X\n",
           offset, 512*4096 + offset);
}

/* ─── Q3: Segmentation vs Paging ─── */
void q3_seg_vs_paging(void) {
    printf("\n=== Q3: Segmentation vs Paging — Key Differences ===\n\n");
    printf("╔═══════════════════╦══════════════════════╦═══════════════════════╗\n");
    printf("║ Property          ║ Segmentation         ║ Paging                ║\n");
    printf("╠═══════════════════╬══════════════════════╬═══════════════════════╣\n");
    printf("║ Unit              ║ Variable-size segs   ║ Fixed-size pages      ║\n");
    printf("║ Fragmentation     ║ External only        ║ Internal only         ║\n");
    printf("║ Programmer visible║ YES (code/data/stack)║ NO (transparent)      ║\n");
    printf("║ Sharing           ║ Easy (share segment) ║ Harder (share page)   ║\n");
    printf("║ Protection        ║ Per-segment (r/w/x)  ║ Per-page (r/w/x)      ║\n");
    printf("║ Modern x86-64     ║ Mostly disabled      ║ Primary mechanism     ║\n");
    printf("╠═══════════════════╬══════════════════════╬═══════════════════════╣\n");
    printf("║ BEST APPROACH     ║                   Segmentation + Paging       ║\n");
    printf("║                   ║    Segments define logical regions,            ║\n");
    printf("║                   ║    pages handle physical allocation            ║\n");
    printf("╚═══════════════════╩══════════════════════╩═══════════════════════╝\n\n");
}

/* ─── Q4: Thrashing scenario ─── */
void q4_thrashing(void) {
    printf("=== Q4: Thrashing — What and Why ===\n\n");
    printf("Scenario:\n");
    printf("  System has 4 frames. Process needs 5 pages simultaneously.\n");
    printf("  No matter what page is evicted, the next access causes a fault!\n\n");

    /* Simulate thrashing reference string */
    int refs[] = {1,2,3,4,1,2,5,1,2,3,4,5};
    int n = sizeof(refs)/sizeof(refs[0]);
    int frames[4] = {-1,-1,-1,-1};
    int ptr=0, faults=0;

    printf("Reference string (4 frames, FIFO):\n");
    printf("Ref | Frames        | Fault?\n");
    printf("----+---------------+-------\n");
    for(int t=0;t<n;t++) {
        int hit=0;
        for(int i=0;i<4;i++) if(frames[i]==refs[t]){hit=1;break;}
        if(!hit){
            faults++;
            frames[ptr]=refs[t];
            ptr=(ptr+1)%4;
        }
        printf(" %2d | [%d %d %d %d] | %s\n",
               refs[t],frames[0],frames[1],frames[2],frames[3],
               hit?"hit":"FAULT");
    }
    printf("\nTotal faults: %d / %d accesses = %.0f%% fault rate\n\n",
           faults, n, 100.0*faults/n);
    printf("THIS IS THRASHING! Process spends more time swapping than executing.\n\n");
    printf("Solutions:\n");
    printf("  1. Working Set Model: track last W references, keep those pages resident\n");
    printf("  2. PFF (Page Fault Frequency): if fault rate > threshold, give more frames\n");
    printf("  3. Swap out processes (reduce degree of multiprogramming)\n");
    printf("  4. Buy more RAM :-)\n");
}

/* ─── Q5: Buddy system ─── */
void q5_buddy_system(void) {
    printf("\n=== Q5: Buddy System Memory Allocation ===\n");
    printf("Total memory: 1MB = 2^20 bytes\n\n");

    printf("Request sequence: alloc(70KB), alloc(35KB), alloc(80KB)\n\n");
    printf("Buddy system always rounds UP to power of 2:\n");
    printf("  70KB → 128KB block\n");
    printf("  35KB →  64KB block\n");
    printf("  80KB → 128KB block\n\n");
    printf("1MB split sequence:\n");
    printf("  1MB →  2×512KB\n");
    printf("  512KB →  2×256KB\n");
    printf("  256KB →  2×128KB\n");
    printf("  Allocate first 128KB to request A (70KB) ← 58KB wasted!\n");
    printf("  Remaining 128KB: split → 2×64KB\n");
    printf("  Allocate first 64KB to request B (35KB) ← 29KB wasted!\n");
    printf("  Remaining 64KB: split + combine with other 64KB → 128KB\n");
    printf("  Allocate 128KB to request C (80KB)\n\n");
    printf("Buddy Deallocation:\n");
    printf("  When block freed, check if its 'buddy' is also free\n");
    printf("  If yes: merge them into a larger block (coalescing)\n");
    printf("  Buddy of block at addr X with size S = X XOR S\n");
    printf("  Repeat merging until buddy is not free\n\n");
    printf("Used in: Linux kernel (page allocator uses buddy system!)\n");
    printf("  cat /proc/buddyinfo  — shows Linux buddy allocator state\n");
}

int main(void) {
    printf("╔══════════════════════════════════════════════════╗\n");
    printf("║   MEMORY MANAGEMENT INTERVIEW QUESTIONS           ║\n");
    printf("║   (Worked solutions — study these carefully!)     ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");

    q1_page_table_size();
    q2_address_breakdown();
    q3_seg_vs_paging();
    q4_thrashing();
    q5_buddy_system();
    return 0;
}
