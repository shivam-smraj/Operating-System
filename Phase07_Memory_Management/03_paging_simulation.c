/* Phase07/03_paging_simulation.c
 * TOPIC: Paging — address translation, page tables, TLB
 * Compile: gcc -Wall -o paging 03_paging_simulation.c
 *
 * Simulates a two-level page table and TLB with EAT (Effective Access Time) calculation
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ─── System configuration ─── */
#define PAGE_SIZE       4096    /* 4KB pages (12-bit offset) */
#define PAGE_BITS       12
#define ADDR_BITS       32
#define PAGE_ENTRIES    1024    /* 10 bits per level */
#define LEVEL_BITS      10

/* Page Table Entry */
typedef struct {
    uint32_t frame_num : 20;  /* Physical frame number */
    uint32_t present   : 1;   /* Is page in memory? */
    uint32_t dirty     : 1;   /* Was page written? */
    uint32_t accessed  : 1;   /* Was page accessed? */
    uint32_t rw        : 1;   /* Read/Write permission */
    uint32_t user      : 1;   /* User/Kernel accessible */
    uint32_t reserved  : 7;
} PTE;

/* TLB Entry */
typedef struct {
    uint32_t vpn;        /* Virtual Page Number */
    uint32_t frame;      /* Physical frame */
    int      valid;
} TLBEntry;

/* ─── Our simulated address space ─── */
#define TLB_SIZE    8
#define MEM_FRAMES  16  /* Total physical frames */

TLBEntry tlb[TLB_SIZE];
int      tlb_next = 0;          /* Round-robin replacement */
PTE      page_table[PAGE_ENTRIES]; /* Single-level for simplicity */
uint8_t  physical_memory[MEM_FRAMES * PAGE_SIZE];
int      next_free_frame = 0;

/* Stats */
int tlb_hits = 0, tlb_misses = 0, page_faults = 0;
long total_accesses = 0;

/* ─── TLB lookup ─── */
int tlb_lookup(uint32_t vpn, uint32_t *frame) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].vpn == vpn) {
            *frame = tlb[i].frame;
            return 1;  /* HIT */
        }
    }
    return 0;  /* MISS */
}

void tlb_insert(uint32_t vpn, uint32_t frame) {
    tlb[tlb_next % TLB_SIZE] = (TLBEntry){vpn, frame, 1};
    tlb_next++;
}

/* ─── Page fault handler ─── */
void handle_page_fault(uint32_t vpn) {
    page_faults++;
    printf("  [PAGE FAULT] VPN=%u — loading from disk to frame %d\n",
           vpn, next_free_frame);
    /* Allocate a new frame */
    if (next_free_frame >= MEM_FRAMES) {
        printf("  [OOM] No free frames! Would need page replacement.\n");
        return;
    }
    page_table[vpn].frame_num = next_free_frame++;
    page_table[vpn].present   = 1;
    page_table[vpn].rw        = 1;
    page_table[vpn].accessed  = 0;
    page_table[vpn].dirty     = 0;
}

/* ─── Virtual → Physical address translation ─── */
uint32_t translate(uint32_t virtual_addr, int write) {
    total_accesses++;
    uint32_t vpn    = virtual_addr >> PAGE_BITS;         /* Top 20 bits */
    uint32_t offset = virtual_addr & (PAGE_SIZE - 1);   /* Bottom 12 bits */
    uint32_t frame;

    printf("VA=0x%08X  VPN=%-5u  offset=0x%03X  → ", virtual_addr, vpn, offset);

    /* Step 1: Check TLB */
    if (tlb_lookup(vpn, &frame)) {
        tlb_hits++;
        printf("TLB HIT  frame=%-3u  ", frame);
    } else {
        tlb_misses++;
        printf("TLB MISS → page table  ");

        /* Step 2: Walk page table */
        if (!page_table[vpn].present) {
            handle_page_fault(vpn);
        }
        page_table[vpn].accessed = 1;
        if (write) page_table[vpn].dirty = 1;
        frame = page_table[vpn].frame_num;
        tlb_insert(vpn, frame);   /* Update TLB */
    }

    uint32_t physical = (frame << PAGE_BITS) | offset;
    printf("PA=0x%08X\n", physical);
    return physical;
}

/* ─── Effective Access Time calculation ─── */
void calculate_eat(void) {
    printf("\n=== Effective Access Time (EAT) Calculation ===\n\n");

    /* Parameters */
    double tlb_time  = 0.5;   /* ns — TLB access time */
    double mem_time  = 100.0; /* ns — main memory access time */
    double disk_time = 10000000.0; /* ns — 10ms disk access (page fault) */

    double hit_rate     = 0.95;  /* 95% TLB hit rate */
    double fault_rate   = 0.001; /* 0.1% page fault rate */

    /* EAT with TLB (no page faults) */
    double eat_tlb = hit_rate * (tlb_time + mem_time)
                   + (1-hit_rate) * (tlb_time + 2*mem_time);
    printf("EAT (TLB, no page faults):\n");
    printf("  Hit  path: %.0f%% × (TLB + 1 mem) = %.0f%%× %.1f = %.1f ns\n",
           hit_rate*100, hit_rate*100, tlb_time+mem_time,
           hit_rate*(tlb_time+mem_time));
    printf("  Miss path: %.0f%% × (TLB + 2 mem) = %.0f%%× %.1f = %.1f ns\n",
           (1-hit_rate)*100, (1-hit_rate)*100, tlb_time+2*mem_time,
           (1-hit_rate)*(tlb_time+2*mem_time));
    printf("  EAT = %.2f ns  (vs pure memory: %.0f ns, speedup: %.1f%%)\n\n",
           eat_tlb, mem_time, 100*(mem_time-eat_tlb)/mem_time);

    /* EAT with page faults */
    double eat_pf = (1-fault_rate) * eat_tlb + fault_rate * disk_time;
    printf("EAT (with %.1f%% page fault rate):\n", fault_rate*100);
    printf("  No fault: %.1f%% × %.2f ns = %.2f ns\n",
           (1-fault_rate)*100, eat_tlb, (1-fault_rate)*eat_tlb);
    printf("  Fault:    %.1f%% × %.0f ns = %.2f ns\n",
           fault_rate*100, disk_time, fault_rate*disk_time);
    printf("  EAT = %.2f ns (page faults dominate!)\n", eat_pf);
    printf("\n  LESSON: Even 0.1%% page fault rate causes HUGE slowdown.\n");
    printf("  This is why THRASHING is so catastrophic.\n");
}

int main(void) {
    printf("╔═══════════════════════════════════════════════╗\n");
    printf("║   PAGING — Address Translation + TLB + EAT    ║\n");
    printf("╚═══════════════════════════════════════════════╝\n");
    printf("Page size: %d bytes (%d bit offset)  |  TLB: %d entries\n\n",
           PAGE_SIZE, PAGE_BITS, TLB_SIZE);

    memset(tlb, 0, sizeof(tlb));
    memset(page_table, 0, sizeof(page_table));

    printf("=== Address Translation Trace ===\n");
    printf("%-30s %-s\n", "Address breakdown", "Result");
    printf("%-30s %-s\n", "----", "------");

    /* Access various virtual addresses */
    translate(0x00001000, 0);  /* VPN=1 */
    translate(0x00002500, 0);  /* VPN=2 */
    translate(0x00001F00, 0);  /* VPN=1 again — TLB hit! */
    translate(0x00002800, 1);  /* VPN=2 again — TLB hit (write) */
    translate(0x00003000, 0);  /* VPN=3 — page fault */
    translate(0x00004000, 0);  /* VPN=4 — page fault */
    translate(0x00003100, 0);  /* VPN=3 again — TLB hit */
    translate(0x00001200, 0);  /* VPN=1 again */

    printf("\n=== Access Statistics ===\n");
    printf("Total accesses: %ld\n", total_accesses);
    printf("TLB Hits:       %d (%.1f%%)\n", tlb_hits, 100.0*tlb_hits/total_accesses);
    printf("TLB Misses:     %d (%.1f%%)\n", tlb_misses, 100.0*tlb_misses/total_accesses);
    printf("Page Faults:    %d\n", page_faults);

    calculate_eat();

    printf("\n[ADDRESS DECOMPOSITION — 32-bit, 4KB pages]\n");
    printf("Virtual Address: [31..12 = VPN (20 bits)] [11..0 = Offset (12 bits)]\n");
    printf("Two-level:       [31..22 = PDE] [21..12 = PTE] [11..0 = Offset]\n");
    printf("Physical Address = Frame_Number × PageSize + Offset\n");
    return 0;
}
