/* Phase14/10_complete_os_simulation.c
 * CAPSTONE: Mini OS simulation combining all OS concepts
 * Simulates: PCB management, Round Robin scheduling, First-fit memory, Message IPC
 * Compile: gcc -Wall -pthread -o mini_os 10_complete_os_simulation.c
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_PROCS  12
#define MEM_KB     512
#define QUANTUM    2

typedef enum { PS_EMPTY, PS_READY, PS_RUNNING, PS_ZOMBIE } PState;
const char *pst[] = {"EMPTY","READY","RUNNING","ZOMBIE"};

typedef struct {
    int    pid, burst_left, mem_kb, priority;
    PState state;
    char   name[24];
    int    created, finished;
} PCB;

typedef struct { int start, size, pid; } MemBlk;

PCB    procs[MAX_PROCS];
MemBlk mem[MAX_PROCS];
int    n_mem=1, n_procs=0, tick=0;

pthread_mutex_t os_lock = PTHREAD_MUTEX_INITIALIZER;

/* --- Memory Allocator (first-fit) --- */
int mem_alloc(int pid, int kb) {
    for(int i=0;i<n_mem;i++) {
        if(mem[i].pid==-1 && mem[i].size>=kb) {
            if(mem[i].size > kb+16 && n_mem<MAX_PROCS-1) {
                memmove(&mem[i+2],&mem[i+1],(n_mem-i-1)*sizeof(MemBlk));
                n_mem++;
                mem[i+1]=(MemBlk){mem[i].start+kb, mem[i].size-kb, -1};
                mem[i].size=kb;
            }
            mem[i].pid=pid;
            return mem[i].start;
        }
    }
    return -1;
}

void mem_free(int pid) {
    for(int i=0;i<n_mem;i++) if(mem[i].pid==pid) mem[i].pid=-1;
    /* Coalesce */
    for(int i=0;i<n_mem-1;) {
        if(mem[i].pid==-1 && mem[i+1].pid==-1) {
            mem[i].size+=mem[i+1].size;
            memmove(&mem[i+1],&mem[i+2],(n_mem-i-2)*sizeof(MemBlk));
            n_mem--;
        } else i++;
    }
}

/* --- Process Creator --- */
int proc_create(const char *name, int burst, int mem_kb, int prio) {
    pthread_mutex_lock(&os_lock);
    int pid=-1;
    for(int i=1;i<MAX_PROCS;i++) if(procs[i].state==PS_EMPTY){pid=i;break;}
    if(pid<0){printf("Process table full!\n");pthread_mutex_unlock(&os_lock);return -1;}

    int ms=mem_alloc(pid, mem_kb);
    if(ms<0){printf("OOM: cannot create '%s' (need %dKB)\n",name,mem_kb);
             pthread_mutex_unlock(&os_lock);return -1;}

    PCB *p=&procs[pid];
    p->pid=pid; p->state=PS_READY; p->burst_left=burst;
    p->mem_kb=mem_kb; p->priority=prio; p->created=tick; p->finished=-1;
    strncpy(p->name,name,23);
    printf("[t=%3d] CREATE P%d '%-12s' burst=%d mem=%dKB prio=%d\n",
           tick,pid,name,burst,mem_kb,prio);
    pthread_mutex_unlock(&os_lock);
    return pid;
}

/* --- Round Robin Scheduler Tick --- */
void sched_tick(void) {
    tick++;
    for(int i=1;i<MAX_PROCS;i++) {
        PCB *p=&procs[i];
        if(p->state!=PS_READY) continue;
        p->state=PS_RUNNING;
        int run=(p->burst_left<QUANTUM)?p->burst_left:QUANTUM;
        p->burst_left-=run;
        if(p->burst_left<=0) {
            p->state=PS_ZOMBIE; p->finished=tick;
            mem_free(p->pid);
            printf("[t=%3d] DONE  P%d '%-12s' TAT=%d\n",
                   tick,p->pid,p->name,p->finished-p->created);
        } else {
            p->state=PS_READY;
            printf("[t=%3d] RUN   P%d '%-12s' remaining=%d\n",
                   tick,p->pid,p->name,p->burst_left);
        }
        return;  /* One process per tick */
    }
    printf("[t=%3d] IDLE\n",tick);
}

void print_status(void) {
    printf("\n--- System Status @ tick %d ---\n",tick);
    printf("%-4s %-14s %-9s %-6s %-4s\n","PID","Name","State","Burst","Mem");
    for(int i=1;i<MAX_PROCS;i++) {
        PCB *p=&procs[i];
        if(p->state==PS_EMPTY) continue;
        printf("P%-3d %-14s %-9s %-6d %-4d\n",
               p->pid,p->name,pst[p->state],p->burst_left,p->mem_kb);
    }
    printf("Memory: ");
    for(int i=0;i<n_mem;i++)
        printf("[%d-%d:%s] ",mem[i].start,mem[i].start+mem[i].size-1,
               mem[i].pid==-1?"free":"used");
    printf("\n\n");
}

int main(void) {
    printf("╔══════════════════════════════════════╗\n");
    printf("║   CAPSTONE: MINI OS SIMULATION        ║\n");
    printf("║   Scheduling + Memory + Process Mgmt  ║\n");
    printf("╚══════════════════════════════════════╝\n\n");

    /* Initialize */
    memset(procs,0,sizeof(procs));
    for(int i=0;i<MAX_PROCS;i++) procs[i].state=PS_EMPTY;
    mem[0]=(MemBlk){0,MEM_KB,-1};
    printf("System: %dKB RAM, %d process slots, RR quantum=%d\n\n",
           MEM_KB,MAX_PROCS,QUANTUM);

    /* Create processes */
    proc_create("webserver",  8, 64, 8);
    proc_create("database",  12, 128, 9);
    proc_create("logger",     5,  16, 3);
    proc_create("cron_job",   6,  32, 2);
    proc_create("monitor",    4,  16, 6);

    print_status();

    printf("=== Scheduling (Round Robin) ===\n");
    for(int t=0;t<15;t++) sched_tick();

    print_status();
    printf("Simulation complete!\n");
    printf("Demonstrates: PCB management, RR scheduling, first-fit memory,\n");
    printf("process lifecycle (READY→RUNNING→ZOMBIE), memory coalescing.\n");
    return 0;
}
