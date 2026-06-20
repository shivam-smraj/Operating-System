/* Phase04/02_fcfs_scheduler.c
 * Complete FCFS Scheduler with Gantt chart and metrics
 * Compile: gcc -Wall -o fcfs 02_fcfs_scheduler.c
 */
#include <stdio.h>
#include <string.h>

typedef struct {
    int pid, arrival, burst, start, finish, waiting, turnaround;
} Process;

void fcfs(Process *p, int n) {
    int time = 0;
    for (int i = 0; i < n; i++) {
        if (time < p[i].arrival) time = p[i].arrival;
        p[i].start      = time;
        p[i].finish     = time + p[i].burst;
        p[i].turnaround = p[i].finish - p[i].arrival;
        p[i].waiting    = p[i].turnaround - p[i].burst;
        time            = p[i].finish;
    }
}

void print_results(Process *p, int n) {
    printf("\nGantt Chart: |");
    for (int i = 0; i < n; i++) printf(" P%d |", p[i].pid);
    printf("\n             0");
    for (int i = 0; i < n; i++) printf("    %d", p[i].finish);
    printf("\n\n");

    printf("%-5s %-8s %-6s %-7s %-7s %-9s %-11s\n",
           "PID","Arrival","Burst","Start","Finish","Waiting","Turnaround");
    double tw=0,tt=0;
    for (int i=0;i<n;i++) {
        printf("P%-4d %-8d %-6d %-7d %-7d %-9d %-11d\n",
               p[i].pid,p[i].arrival,p[i].burst,
               p[i].start,p[i].finish,p[i].waiting,p[i].turnaround);
        tw+=p[i].waiting; tt+=p[i].turnaround;
    }
    printf("\nAverage Waiting Time:    %.2f\n", tw/n);
    printf("Average Turnaround Time: %.2f\n", tt/n);
}

int main(void) {
    printf("=== FCFS (First-Come First-Served) Scheduler ===\n");
    Process p[] = {
        {1, 0, 7, 0, 0, 0, 0},
        {2, 2, 4, 0, 0, 0, 0},
        {3, 4, 1, 0, 0, 0, 0},
        {4, 5, 4, 0, 0, 0, 0}
    };
    int n = sizeof(p)/sizeof(p[0]);
    fcfs(p, n);
    print_results(p, n);

    printf("\n[FCFS Analysis]\n");
    printf("+ Simple FIFO queue\n");
    printf("+ No starvation\n");
    printf("- Convoy effect: short tasks wait behind long ones\n");
    printf("- Poor for interactive systems\n");
    return 0;
}
