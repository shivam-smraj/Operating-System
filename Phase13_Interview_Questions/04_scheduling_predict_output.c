/* Phase13/04_scheduling_predict_output.c
 * Tricky OS interview questions: predict CPU scheduling output
 * Work through each question BEFORE running!
 * Compile: gcc -Wall -o sched_tricky 04_scheduling_predict_output.c
 */
#include <stdio.h>
#include <string.h>

typedef struct { int pid, arr, burst, rem, done, fin, wt, tat; } P;

/* ─── QUESTION 1: FCFS — what is avg WT? ─── */
void q1_fcfs_avg_wt(void) {
    printf("\n=== Q1: FCFS — Calculate Average Waiting Time ===\n");
    printf("P1: arrival=0, burst=4\n");
    printf("P2: arrival=1, burst=3\n");
    printf("P3: arrival=2, burst=5\n");
    printf("P4: arrival=3, burst=2\n");
    printf("Think before reading below!\n...\n");

    P p[] = {{1,0,4},{2,1,3},{3,2,5},{4,3,2}};
    int n=4, time=0;
    for(int i=0;i<n;i++) {
        if(time<p[i].arr) time=p[i].arr;
        p[i].wt  = time - p[i].arr;
        time    += p[i].burst;
        p[i].fin = time;
        p[i].tat = p[i].fin - p[i].arr;
    }
    printf("Gantt: P1[0-4] P2[4-7] P3[7-12] P4[12-14]\n");
    float tw=0;
    for(int i=0;i<n;i++){printf("P%d: WT=%d\n",p[i].pid,p[i].wt);tw+=p[i].wt;}
    printf("ANSWER: Avg WT = %.2f\n",tw/n);
}

/* ─── QUESTION 2: SJF — is starvation possible? ─── */
void q2_sjf_starvation(void) {
    printf("\n=== Q2: SJF — Starvation Scenario ===\n");
    printf("New short jobs keep arriving. Can P_long starve forever?\n");
    printf("YES! In non-preemptive SJF: if a 10-unit job arrives at t=0,\n");
    printf("and new 1-unit jobs keep arriving every tick,\n");
    printf("the scheduler always picks the 1-unit jobs first.\n");
    printf("P_long NEVER gets the CPU → STARVATION!\n");
    printf("\nFix: AGING — priority increases with waiting time.\n");
    printf("Linux CFS: vruntime ensures no permanent starvation.\n");
}

/* ─── QUESTION 3: Round Robin — response time ─── */
void q3_rr_response(void) {
    printf("\n=== Q3: Round Robin — First Response Time ===\n");
    printf("3 CPU-bound processes: P1(burst=12), P2(burst=8), P3(burst=6)\n");
    printf("All arrive at t=0. Quantum q=3.\n");
    printf("When does P3 FIRST get the CPU?\n\n");
    printf("Gantt: P1[0-3] P2[3-6] P3[6-9] P1[9-12]...\n");
    printf("ANSWER: P3 first runs at t=6 (after P1 and P2 each get one quantum)\n");
    printf("Response time of P3 = 6 - 0 = 6\n");
    printf("\nGeneral: Pn's first response = (n-1) * q for n processes arriving at t=0\n");
}

/* ─── QUESTION 4: Priority scheduling — priority inversion ─── */
void q4_priority_inversion(void) {
    printf("\n=== Q4: Priority Inversion — The Mars Pathfinder Bug ===\n");
    printf("3 tasks: High(H), Medium(M), Low(L)\n");
    printf("L holds mutex. H arrives, needs mutex, BLOCKS.\n");
    printf("M arrives with no mutex needed → preempts L!\n");
    printf("Now: H waits for L, but L can't run because M preempts it.\n");
    printf("H is blocked waiting for L, M runs freely.\n");
    printf("Effective priority: H < M — INVERSION!\n\n");
    printf("Solution 1: Priority Inheritance\n");
    printf("  When L holds mutex needed by H: temporarily boost L's priority to H.\n");
    printf("  L runs at H's priority → finishes → releases mutex → H runs.\n\n");
    printf("Solution 2: Priority Ceiling\n");
    printf("  Mutex has a 'ceiling priority' = max priority of any thread that may lock it.\n");
    printf("  Thread acquires mutex only if its priority >= ceiling.\n\n");
    printf("Real world: Mars Pathfinder 1997 used priority inheritance fix\n");
    printf("  (uploaded remotely to the rover running VxWorks RTOS!)\n");
}

/* ─── QUESTION 5: Context switch cost ─── */
void q5_context_switch_math(void) {
    printf("\n=== Q5: Context Switch Overhead Calculation ===\n");
    printf("System: 1 GHz CPU, each context switch costs 1000 cycles.\n");
    printf("Round Robin with quantum q=10ms.\n");
    printf("What fraction of CPU time is spent on context switches?\n\n");
    printf("Context switch time = 1000 cycles / 1,000,000,000 cycles/s = 1 microsecond\n");
    printf("Quantum = 10ms = 10,000 microseconds\n");
    printf("Overhead fraction = 1 / (10000+1) ≈ 0.01%% — negligible!\n\n");
    printf("Now with q=1ms:\n");
    printf("Overhead = 1 / (1000+1) ≈ 0.1%% — still OK\n\n");
    printf("With q=10 microseconds:\n");
    printf("Overhead = 1 / (10+1) ≈ 9%% — significant!\n\n");
    printf("LESSON: Smaller quantum = better response time but more overhead.\n");
    printf("Linux default: ~4ms - 40ms adaptive quantum (CFS target latency)\n");
}

int main(void) {
    printf("╔═══════════════════════════════════════════════╗\n");
    printf("║   SCHEDULING INTERVIEW PREDICT-OUTPUT QUIZ     ║\n");
    printf("║   Work through each before reading the answer! ║\n");
    printf("╚═══════════════════════════════════════════════╝\n");

    q1_fcfs_avg_wt();
    q2_sjf_starvation();
    q3_rr_response();
    q4_priority_inversion();
    q5_context_switch_math();
    return 0;
}
