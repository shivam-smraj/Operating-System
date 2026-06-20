/* Phase04/05_all_schedulers_comparison.c
 * Implements ALL classic CPU scheduling algorithms and compares them
 * FCFS, SJF (non-preemptive), SRTF (preemptive SJF), Round Robin, Priority
 * Compile: gcc -Wall -o schedulers 05_all_schedulers_comparison.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_P 20

typedef struct {
    int pid, arrival, burst, priority;
    int remaining;
    int start, finish, first_run;
    int waiting, turnaround, response;
} Process;

void reset(Process *p, int n) {
    for (int i=0;i<n;i++) {
        p[i].remaining = p[i].burst;
        p[i].start = p[i].finish = p[i].first_run = -1;
        p[i].waiting = p[i].turnaround = p[i].response = 0;
    }
}

int all_done(Process *p, int n) {
    for(int i=0;i<n;i++) if(p[i].remaining>0) return 0;
    return 1;
}

void print_header(void) {
    printf("%-5s %-8s %-6s %-6s %-9s %-11s %-9s\n",
           "PID","Arrival","Burst","Finish","Waiting","Turnaround","Response");
    printf("%-5s %-8s %-6s %-6s %-9s %-11s %-9s\n",
           "---","-------","-----","------","-------","----------","--------");
}

void print_results(Process *p, int n, const char *algo) {
    printf("\n=== %s ===\n", algo);
    print_header();
    double tw=0,tt=0,tr=0;
    for(int i=0;i<n;i++) {
        printf("P%-4d %-8d %-6d %-6d %-9d %-11d %-9d\n",
               p[i].pid,p[i].arrival,p[i].burst,p[i].finish,
               p[i].waiting,p[i].turnaround,p[i].response);
        tw+=p[i].waiting; tt+=p[i].turnaround; tr+=p[i].response;
    }
    printf("Avg: Waiting=%.2f  Turnaround=%.2f  Response=%.2f\n",tw/n,tt/n,tr/n);
}

/* ─── FCFS ─── */
void fcfs(Process *p, int n) {
    reset(p, n);
    int time=0;
    /* Sort by arrival (already sorted in our test) */
    for(int i=0;i<n;i++) {
        if(time < p[i].arrival) time = p[i].arrival;
        p[i].start = p[i].first_run = time;
        p[i].finish     = time + p[i].burst;
        p[i].turnaround = p[i].finish - p[i].arrival;
        p[i].waiting    = p[i].turnaround - p[i].burst;
        p[i].response   = p[i].waiting;  /* Same as waiting for non-preemptive */
        time            = p[i].finish;
        p[i].remaining  = 0;
    }
}

/* ─── SJF Non-Preemptive ─── */
void sjf_np(Process *p, int n) {
    reset(p, n);
    int done[MAX_P]={0}, time=0, completed=0;
    while(completed < n) {
        int best=-1, min_burst=INT_MAX;
        for(int i=0;i<n;i++) {
            if(!done[i] && p[i].arrival<=time && p[i].remaining<min_burst) {
                min_burst=p[i].remaining; best=i;
            }
        }
        if(best==-1) { time++; continue; }
        p[best].first_run = time;
        time += p[best].burst;
        p[best].finish     = time;
        p[best].turnaround = time - p[best].arrival;
        p[best].waiting    = p[best].turnaround - p[best].burst;
        p[best].response   = p[best].waiting;
        p[best].remaining  = 0;
        done[best]=1; completed++;
    }
}

/* ─── SRTF (Preemptive SJF) ─── */
void srtf(Process *p, int n) {
    reset(p, n);
    int time=0, completed=0;
    while(completed < n) {
        int cur=-1, min_rem=INT_MAX;
        for(int i=0;i<n;i++) {
            if(p[i].arrival<=time && p[i].remaining>0 && p[i].remaining<min_rem) {
                min_rem=p[i].remaining; cur=i;
            }
        }
        if(cur==-1) { time++; continue; }
        if(p[cur].first_run==-1) p[cur].first_run=time;
        p[cur].remaining--;
        time++;
        if(p[cur].remaining==0) {
            p[cur].finish     = time;
            p[cur].turnaround = time - p[cur].arrival;
            p[cur].waiting    = p[cur].turnaround - p[cur].burst;
            p[cur].response   = p[cur].first_run - p[cur].arrival;
            completed++;
        }
    }
}

/* ─── Round Robin ─── */
void round_robin(Process *p, int n, int q) {
    reset(p, n);
    int queue[MAX_P*100], head=0, tail=0;
    int in_q[MAX_P]={0}, time=0, completed=0;

    /* Enqueue processes that arrive at time 0 */
    for(int i=0;i<n;i++) if(p[i].arrival==0){queue[tail++]=i;in_q[i]=1;}

    while(completed < n) {
        if(head==tail) { time++; /* Idle */
            for(int i=0;i<n;i++)
                if(!in_q[i] && p[i].remaining>0 && p[i].arrival<=time)
                    {queue[tail++]=i;in_q[i]=1;}
            continue;
        }
        int cur=queue[head++]; in_q[cur]=0;
        if(p[cur].first_run==-1) p[cur].first_run=time;

        int run=(p[cur].remaining<q)?p[cur].remaining:q;
        p[cur].remaining-=run; time+=run;

        /* Enqueue newly arrived processes */
        for(int i=0;i<n;i++)
            if(!in_q[i] && p[i].remaining>0 && p[i].arrival<=time && i!=cur)
                {queue[tail++]=i;in_q[i]=1;}

        if(p[cur].remaining==0) {
            p[cur].finish     = time;
            p[cur].turnaround = time - p[cur].arrival;
            p[cur].waiting    = p[cur].turnaround - p[cur].burst;
            p[cur].response   = p[cur].first_run - p[cur].arrival;
            completed++;
        } else {
            queue[tail++]=cur; in_q[cur]=1;  /* Re-queue */
        }
    }
}

/* ─── Priority (Non-preemptive, lower number = higher priority) ─── */
void priority_np(Process *p, int n) {
    reset(p, n);
    int done[MAX_P]={0}, time=0, completed=0;
    while(completed < n) {
        int best=-1, min_prio=INT_MAX;
        for(int i=0;i<n;i++) {
            if(!done[i] && p[i].arrival<=time && p[i].priority<min_prio) {
                min_prio=p[i].priority; best=i;
            }
        }
        if(best==-1) { time++; continue; }
        p[best].first_run = time;
        time += p[best].burst;
        p[best].finish     = time;
        p[best].turnaround = time - p[best].arrival;
        p[best].waiting    = p[best].turnaround - p[best].burst;
        p[best].response   = p[best].waiting;
        p[best].remaining  = 0;
        done[best]=1; completed++;
    }
}

int main(void) {
    printf("╔═══════════════════════════════════════════════╗\n");
    printf("║   CPU SCHEDULING — ALL ALGORITHMS COMPARISON   ║\n");
    printf("╚═══════════════════════════════════════════════╝\n\n");

    /* Test dataset — matches classic textbook examples */
    Process orig[] = {
        /*pid arr burst prio*/
        {1,  0,  7,  3},
        {2,  2,  4,  1},
        {3,  4,  1,  4},
        {4,  5,  4,  2},
    };
    int n = sizeof(orig)/sizeof(orig[0]);

    printf("Input:\n");
    printf("%-5s %-8s %-6s %-9s\n","PID","Arrival","Burst","Priority");
    for(int i=0;i<n;i++)
        printf("P%-4d %-8d %-6d %-9d\n",
               orig[i].pid,orig[i].arrival,orig[i].burst,orig[i].priority);

    Process p[MAX_P];

    /* Run all algorithms */
    memcpy(p,orig,n*sizeof(Process)); fcfs(p,n);       print_results(p,n,"FCFS");
    memcpy(p,orig,n*sizeof(Process)); sjf_np(p,n);     print_results(p,n,"SJF (Non-Preemptive)");
    memcpy(p,orig,n*sizeof(Process)); srtf(p,n);       print_results(p,n,"SRTF (Preemptive SJF)");
    memcpy(p,orig,n*sizeof(Process)); round_robin(p,n,2); print_results(p,n,"Round Robin (q=2)");
    memcpy(p,orig,n*sizeof(Process)); round_robin(p,n,4); print_results(p,n,"Round Robin (q=4)");
    memcpy(p,orig,n*sizeof(Process)); priority_np(p,n);print_results(p,n,"Priority (Non-Preemptive)");

    printf("\n[COMPARISON SUMMARY]\n");
    printf("FCFS:         Simple. Convoy effect. Good for batch.\n");
    printf("SJF (NP):     Best avg WT for given set. Starvation possible.\n");
    printf("SRTF:         Best avg WT theoretically. High overhead.\n");
    printf("Round Robin:  Fair. Best response time. Good for timesharing.\n");
    printf("Priority:     Flexible. Starvation without aging.\n");
    printf("\nFormulas:\n");
    printf("  TAT = Finish - Arrival\n");
    printf("  WT  = TAT - Burst\n");
    printf("  Response = First_Run - Arrival\n");
    return 0;
}
