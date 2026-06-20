/* Phase02/04_pcb_and_context_switch.c
 * TOPIC: PCB (Process Control Block) structure + Context Switch simulation
 * Compile: gcc -Wall -o pcb_ctx 04_pcb_and_context_switch.c
 *
 * This shows what the kernel saves/restores during a context switch.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>

/* ─── Simulated PCB (mirrors Linux task_struct, simplified) ─── */
typedef struct {
    /* Identity */
    int   pid;
    int   ppid;
    char  name[32];

    /* State */
    int   state;        /* 0=RUNNING 1=READY 2=WAITING 3=ZOMBIE */
    int   priority;     /* 0-139 in Linux (0=RT, 100-139=normal) */
    int   nice;         /* -20 to +19 (user-visible priority adjustment) */

    /* CPU Context (saved during context switch) */
    struct {
        unsigned long rax, rbx, rcx, rdx; /* General registers */
        unsigned long rsi, rdi, rbp, rsp; /* Stack + index registers */
        unsigned long rip;                /* Instruction pointer */
        unsigned long rflags;             /* Status flags */
        unsigned long cr3;               /* Page table base (physical addr) */
    } cpu_ctx;

    /* Memory */
    unsigned long stack_ptr;      /* Kernel stack pointer */
    unsigned long pgdir;          /* Page directory (virtual memory) */
    long          vm_size_kb;     /* Virtual memory size */
    long          rss_kb;         /* Resident Set Size (RAM used) */

    /* Scheduling (CFS) */
    long long     vruntime;       /* Virtual runtime (ns) — for CFS */
    long long     sum_exec_ns;    /* Total CPU time used */
    int           time_slice_ms;  /* Remaining time slice */

    /* I/O */
    int           open_fds[8];    /* Open file descriptors (first 8) */
    long          io_read_bytes;
    long          io_write_bytes;

    /* Signal handling */
    unsigned long pending_signals; /* Bitmask of pending signals */
    unsigned long blocked_signals; /* Bitmask of blocked signals */

    /* Timing */
    long          created_sec;    /* Creation timestamp */
    long          utime_ms;       /* User-mode CPU time */
    long          stime_ms;       /* Kernel-mode CPU time */
} PCB;

const char *state_names[] = { "RUNNING", "READY", "WAITING", "ZOMBIE" };

void pcb_init(PCB *p, int pid, int ppid, const char *name) {
    memset(p, 0, sizeof(PCB));
    p->pid           = pid;
    p->ppid          = ppid;
    strncpy(p->name, name, 31);
    p->state         = 1;   /* READY */
    p->priority      = 120; /* Default nice=0 priority */
    p->nice          = 0;
    p->time_slice_ms = 4;   /* 4ms default */
    p->created_sec   = time(NULL);
    /* Simulate some register values */
    p->cpu_ctx.rip   = 0x400100 + (pid * 0x1000); /* Fake code address */
    p->cpu_ctx.rsp   = 0x7fff0000 - (pid * 0x10000); /* Fake stack addr */
    p->cpu_ctx.cr3   = 0x100000 + (pid * 0x4000); /* Fake page table */
    p->pgdir         = p->cpu_ctx.cr3;
    p->vm_size_kb    = 8192 + (pid * 512);
    p->rss_kb        = 2048 + (pid * 128);
    /* Simulate open fds: 0=stdin, 1=stdout, 2=stderr */
    p->open_fds[0] = 0; p->open_fds[1] = 1; p->open_fds[2] = 2;
    for (int i = 3; i < 8; i++) p->open_fds[i] = -1;
}

void pcb_print(const PCB *p) {
    printf("\n┌────────────────────────────────────────────────────┐\n");
    printf("│ PCB: PID=%-4d  %-20s  State=%-8s│\n",
           p->pid, p->name, state_names[p->state]);
    printf("├────────────────────────────────────────────────────┤\n");
    printf("│ PPID=%-5d  Priority=%-3d  Nice=%-3d  Slice=%dms    │\n",
           p->ppid, p->priority, p->nice, p->time_slice_ms);
    printf("│ VM=%-6ldKB  RSS=%-6ldKB  vruntime=%-10lldns │\n",
           p->vm_size_kb, p->rss_kb, p->vruntime);
    printf("├────────────────────────────────────────────────────┤\n");
    printf("│ CPU CONTEXT (saved during context switch):          │\n");
    printf("│   RIP=0x%08lx  RSP=0x%08lx  CR3=0x%08lx │\n",
           p->cpu_ctx.rip, p->cpu_ctx.rsp, p->cpu_ctx.cr3);
    printf("│   RFLAGS=0x%016lx                         │\n",
           p->cpu_ctx.rflags);
    printf("├────────────────────────────────────────────────────┤\n");
    printf("│ Open FDs: ");
    for (int i = 0; i < 8; i++)
        if (p->open_fds[i] >= 0) printf("%d ", p->open_fds[i]);
    printf("\n");
    printf("│ CPU time: user=%ldms  sys=%ldms\n",p->utime_ms,p->stime_ms);
    printf("└────────────────────────────────────────────────────┘\n");
}

void simulate_context_switch(PCB *old_p, PCB *new_p) {
    printf("\n══════════════════════════════════════════\n");
    printf("        CONTEXT SWITCH\n");
    printf("══════════════════════════════════════════\n");
    printf("  Preempting: P%d (%s)\n", old_p->pid, old_p->name);
    printf("  Scheduling: P%d (%s)\n", new_p->pid, new_p->name);
    printf("──────────────────────────────────────────\n");
    printf("Step 1: Timer interrupt fires (or process yields)\n");
    printf("Step 2: CPU switches to kernel mode (Ring 0)\n");
    printf("Step 3: Save P%d's CPU state:\n", old_p->pid);
    printf("        RIP=%lx RSP=%lx RFLAGS=%lx\n",
           old_p->cpu_ctx.rip, old_p->cpu_ctx.rsp, old_p->cpu_ctx.rflags);
    printf("        (Saved to P%d's kernel stack)\n", old_p->pid);
    printf("Step 4: Switch page table: CR3 %lx → %lx\n",
           old_p->cpu_ctx.cr3, new_p->cpu_ctx.cr3);
    printf("        (This flushes TLB — expensive!)\n");
    printf("Step 5: Load P%d's CPU state:\n", new_p->pid);
    printf("        RIP=%lx RSP=%lx RFLAGS=%lx\n",
           new_p->cpu_ctx.rip, new_p->cpu_ctx.rsp, new_p->cpu_ctx.rflags);
    printf("Step 6: Update PCB states: P%d RUNNING→READY, P%d READY→RUNNING\n",
           old_p->pid, new_p->pid);
    printf("Step 7: CPU returns to user mode (Ring 3) executing P%d\n", new_p->pid);
    printf("══════════════════════════════════════════\n");
    printf("Total cost: ~1-10 microseconds (TLB flush + cache effects)\n");
    printf("For threads in same process: no CR3 change → faster!\n");

    /* Update states */
    old_p->state = 1;  /* READY */
    new_p->state = 0;  /* RUNNING */
}

void read_real_proc_info(void) {
    printf("\n=== Real PCB info from /proc/self/status ===\n");
    FILE *f = fopen("/proc/self/status", "r");
    if (!f) { printf("(Cannot open /proc/self/status)\n"); return; }
    char line[256];
    int printed = 0;
    while (fgets(line, sizeof(line), f) && printed < 12) {
        /* Print key fields */
        if (strncmp(line, "Name:", 5) == 0 ||
            strncmp(line, "Pid:", 4) == 0 ||
            strncmp(line, "PPid:", 5) == 0 ||
            strncmp(line, "State:", 6) == 0 ||
            strncmp(line, "VmRSS:", 6) == 0 ||
            strncmp(line, "VmSize:", 7) == 0 ||
            strncmp(line, "Threads:", 8) == 0 ||
            strncmp(line, "voluntary_ctxt_switches:", 24) == 0) {
            printf("  %s", line);
            printed++;
        }
    }
    fclose(f);
}

int main(void) {
    printf("╔═══════════════════════════════════════════════╗\n");
    printf("║   PCB Structure + Context Switch Deep Dive    ║\n");
    printf("╚═══════════════════════════════════════════════╝\n");

    /* Create two simulated PCBs */
    PCB p1, p2;
    pcb_init(&p1, 100, 1, "web_server");
    pcb_init(&p2, 101, 1, "database");

    p1.state       = 0;   /* P1 currently RUNNING */
    p1.vruntime    = 5000000;
    p1.utime_ms    = 12;
    p1.open_fds[3] = 5;   /* Has fd 5 open (e.g., socket) */

    p2.vruntime    = 4800000;  /* P2 has lower vruntime → should run next */
    p2.utime_ms    = 8;

    printf("\n--- Current Process Table ---");
    pcb_print(&p1);
    pcb_print(&p2);

    /* Simulate CFS picking next process */
    printf("\n--- CFS Decision ---\n");
    printf("P1 vruntime=%lld, P2 vruntime=%lld\n",
           p1.vruntime, p2.vruntime);
    printf("CFS picks process with MINIMUM vruntime → P2\n");

    /* Simulate context switch */
    simulate_context_switch(&p1, &p2);

    printf("\n--- After Context Switch ---");
    pcb_print(&p1);
    pcb_print(&p2);

    read_real_proc_info();

    printf("\n[KEY TAKEAWAYS]\n");
    printf("1. PCB holds ALL process state — OS rebuilds process from it\n");
    printf("2. Context switch saves/restores ~200+ bytes of CPU state\n");
    printf("3. CR3 switch flushes TLB → next memory access is slow\n");
    printf("4. Thread switch (same process): no CR3 change → faster\n");
    printf("5. CFS uses vruntime to decide who runs next (min-vruntime wins)\n");
    return 0;
}
